#include "StringOperations.h"
#include "cute.h"

void thisIsAStringOperationsTest() {
	ASSERTM("start writing tests", false);	
}

cute::suite make_suite_StringOperations() {
	cute::suite s { };
	s.push_back(CUTE(thisIsAStringOperationsTest));
	return s;
}
