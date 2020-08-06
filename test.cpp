#include "markdownParser.h"
void test(){
	markdownParser mp("test.md");
	mp.transform();
}
int main(){

	test();
	return 0;
}