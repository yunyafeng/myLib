#include "FList.h"

int compare(const void* a, const void *b)
{
	return ((int)a) - ((int)b);
}

int main()
{
	int a = 1, b = 3, c = 100, d = 1000, e = 99;
	FList list;
	FList_ctor(&list);

	FList_pushBack(&list, (void*)a);
	FList_pushBack(&list, (void*)b);
	FList_pushBack(&list, (void*)c);
	FList_pushBack(&list, (void*)d);
	FList_pushBack(&list, (void*)e);

	FList_output(&list, stdout);
	
	FList_quickSort(&list, compare);
	
	FList_output(&list, stdout);

	FListIterator begin = FList_begin(&list);
	FListIterator end = FList_end(&list);
	for(; !FListIterator_equal(&begin, &end); FListIterator_inc(&begin))
		printf("item %d:%d\n", begin.i, (int)FListIterator_dataOf(&begin));
	
	FList_dtor(&list);
	

	return 0;
}
