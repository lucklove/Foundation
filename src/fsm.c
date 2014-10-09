#include "fsm.h"
#include "mem.h"
#include "rb_tree.h"
#include "container.h"

#include <stdio.h>

struct EventLink {
	int event;
	int dst_state;
};

typedef void (*Hander)(int, int, int);

//need by makeRBTree()
static void
toSwapNode(struct RBNode *src, struct RBNode *dst)
{
	int tmp_state = container_of(dst, struct FSMNode, rb_node)->state;	
	mem_t tmp_mem = container_of(dst, struct FSMNode, rb_node)->events;	
	Hander tmp_hander  = container_of(dst, struct FSMNode, rb_node)->trans_hander;
	container_of(dst, struct FSMNode, rb_node)->state =
		container_of(src, struct FSMNode, rb_node)->state;	
	container_of(dst, struct FSMNode, rb_node)->events =
		container_of(src, struct FSMNode, rb_node)->events;	
	container_of(dst, struct FSMNode, rb_node)->trans_hander =
		container_of(src, struct FSMNode, rb_node)->trans_hander;	
	container_of(src, struct FSMNode, rb_node)->state = tmp_state;	
	container_of(src, struct FSMNode, rb_node)->events = tmp_mem;	
	container_of(src, struct FSMNode, rb_node)->trans_hander = tmp_hander;
}

//need by makeRBTree()
static void *
toGetKey(struct RBNode *ptr)
{
	return &container_of(ptr, struct FSMNode, rb_node)->state;
}

//need by makeRBTree()
static int
toCmpKey(void *p1, void *p2)
{
	return *(int *)p1 - *(int *)p2;
}

//need by makeRBTree()
static struct RBNode *
toMakeNode(void *key)
{
	struct FSMNode *node = malloc(sizeof(struct FSMNode));
	node->state = *(int *)key;
	node->events = makeMem(sizeof(struct EventLink));
	node->trans_hander = NULL;
	return &node->rb_node;
}
	
//need by makeRBTree()
static void
toFreeNode(struct RBNode *node)
{
	destroyMem(container_of(node, struct FSMNode, rb_node)->events);
	printf("free %d\n", container_of(node, struct FSMNode, rb_node)->state);
	free(container_of(node, struct FSMNode, rb_node));
}

fsm_t
makeFSMType(void (*err_hander)(int, int))
{
	fsm_t fsm;
	fsm.err_hander = err_hander;
	fsm.rb_tree = makeRBTree(toGetKey, toCmpKey, toMakeNode, toSwapNode, toFreeNode);
	fsm.cur_state = NULL;
	return fsm;
}

void 
fsmLink(fsm_t_ptr fsm_p, int from_state, int event, int to_state)
{
	//确保from_state和to_state在红黑树中
	rb_insert(&fsm_p->rb_tree, &from_state);
	rb_insert(&fsm_p->rb_tree, &to_state);
	struct FSMNode *node = container_of(rb_search(fsm_p->rb_tree, &from_state), struct FSMNode, rb_node);
	struct EventLink link = { event, to_state };
	SET_TYPE_MEM(&node->events, struct EventLink, getMemIndex(node->events) / sizeof(struct EventLink), link);
}

void
fsmSetCurState(fsm_t_ptr fsm_p, int state)
{
	//确保state和在红黑树中
	rb_insert(&fsm_p->rb_tree, &state);
	fsm_p->cur_state = container_of(rb_search(fsm_p->rb_tree, &state), struct FSMNode, rb_node);
}

void
fsmSetHander(fsm_t_ptr fsm_p, int state, void (*hander)(int, int, int))
{
	//确保state和在红黑树中
	rb_insert(&fsm_p->rb_tree, &state);
	struct FSMNode *node = container_of(rb_search(fsm_p->rb_tree, &state), struct FSMNode, rb_node);
	node->trans_hander = hander;
}

void 
fsmPushEvent(fsm_t_ptr fsm_p, int event)
{
	int pre_state = fsm_p->cur_state->state;
	for(int i = 0; i < getMemIndex(fsm_p->cur_state->events) / sizeof(struct EventLink); i++) {
		if(GET_TYPE_MEM(&fsm_p->cur_state->events, struct EventLink, i)->event == event) {
			fsmSetCurState(fsm_p, GET_TYPE_MEM(&fsm_p->cur_state->events, struct EventLink, i)->dst_state);
			if(fsm_p->cur_state->trans_hander != NULL)
				fsm_p->cur_state->trans_hander(pre_state, event, fsm_p->cur_state->state);
			return;
		}
	}
	fsm_p->err_hander(fsm_p->cur_state->state, event);
}

void
destroyFSMType(fsm_t fsm)
{
	destroyRBTree(fsm.rb_tree);
}
