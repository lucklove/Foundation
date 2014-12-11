#include "stringtokenizer.hh"
#include <iostream>
#include <string.h>

using namespace zbase;

int
main(int argc, char *argv[])
{
	StringTokenizer st{"the_quick_brown_fox_over_the_lazy_dog", '_'};
	while(st.hasMoreTokens())
		std::cout << st.nextToken() << std::endl;
	return 0;
}