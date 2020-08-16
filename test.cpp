#include "markdownParser.h"

void test(){
	markdownParser mp("test.md");
	mp.transform();
	mp.generateHtml();
}
int main(){

	test();
	return 0;
}