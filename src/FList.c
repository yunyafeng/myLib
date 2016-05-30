#include "FList.h"

struct f_list_private;

typedef struct f_list_node 
{
	struct f_list_private*	owner;
	struct f_list_node*		next;
	struct f_list_node*		prev;
	void*					data;
} FListNode;

typedef struct f_list_private 
{
	U32 			count;
	U32 			limit;
		
	FListNode*		head;
	FListNode*		tail;
	
	FListCompare	compare;
} FListPrivate;

/*
 * private:
 */
static void FListPrivate_ctor(FListPrivate* me)
{
	me->count = 0;
	me->limit = 0;
	me->head = NULL;
	me->tail = NULL;
	me->compare = NULL;
}

static void FListPrivate_dtor(FListPrivate * me)
{

}

static FListNode* FList_nodeOfIndex(FList* me, U32 index)
{
	//F_ASSERT(me && me->d);
	FListNode *node;
	U32 posi;
	
	if (index >= me->d->count)
		return NULL;

	//��ǰ������
	if ((me->d->count - index) > index) {
		for (
			posi = 0, node = me->d->head; 
			posi != index; 
			++posi, node = node->next
		); 
	}
	//�ɺ���ǰ����
	else {
		for (
			posi = me->d->count - 1, node = me->d->tail; 
			posi != index; 
			--posi, node = node->prev
		); 
	}
	return node;
}

static BOOL FList_removeNode(FList* me, FListNode* node)
{
	//F_ASSERT(me && me->d);
	
	if ((0 == me->d->count) || (node->owner != me->d))
		return FALSE;

	if (1 == me->d->count) {
		node = me->d->head;
		me->d->head = NULL;
		me->d->tail = NULL;
	}
	else {
		node->next->prev = node->prev;
		node->prev->next = node->next;
		if (node == me->d->head)
		{
			me->d->head = node->next;
		}
		else if (node == me->d->tail)
		{
			me->d->tail = node->prev;
		}
	}
	node->owner = NULL;
	node->next = NULL;
	node->prev = NULL;
	me->d->count--;
	
	return TRUE;
}

static BOOL FList_insertNode(FList* me, FListNode* beforeNode, FListNode* newNode)
{
	//F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	if (!beforeNode || !newNode)
		return FALSE;
	
	newNode->prev = beforeNode->prev;
	beforeNode->prev->next = newNode;
	beforeNode->prev = newNode;
	newNode->next = beforeNode;
	if (beforeNode == me->d->head) {
		me->d->head = newNode;
	}
	newNode->owner = me->d;
	me->d->count++;

	return TRUE;
}


static int FList_nodeCompare(const void* node1, const void*node2)
{
	FListNode *n1 = *((FListNode**)node1);
	FListNode *n2 = *((FListNode**)node2);
	FListCompare compare = n1->owner->compare;

	return !compare(n1->data, n2->data);
}


/*
 * public:
 */
void FList_ctor(FList* me)
{
	me->d = F_NEW(FListPrivate);
	FListPrivate_ctor(me->d);
}

void FList_dtor(FList* me)
{
	while (!FList_isEmpty(me)) {
		FList_takeAtIndex(me, 0);
	}
	FListPrivate_dtor(me->d);
	F_DELETE(me->d);
	me->d = NULL;
}

BOOL FList_isEmpty(FList* me)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	return (me->d->count == 0) ? TRUE : FALSE;
}

U32 FList_count(FList* me)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	return me->d->count;
}


BOOL FList_insertAtIndex(FList* me, U32 index, const void* data)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	if (index > me->d->count) {
		return FALSE;
	}

	FListNode *node = F_NEW(FListNode);
	if (!node) {
		return FALSE;
	}
	node->data = (void*)data;
	
	if (0 == me->d->count) {
		//��β���
		node->prev = node;
		node->next = node;
		me->d->head = node;
		me->d->tail = node;
	}
	else if(me->d->count == index) {
		node->prev = me->d->tail;
		node->next = me->d->head;
		me->d->tail->next = node;
		me->d->head->prev = node;
		me->d->tail = node;
	}
	else {
		FListNode* before = FList_nodeOfIndex(me, index);
		if (FList_insertNode(me, before, node)) {
			return TRUE;
		}
		else {
			F_DELETE(node);
			return FALSE;
		}
	}
	
	node->owner = me->d;
	me->d->count++;
	
	return TRUE;
}

void* FList_takeAtIndex(FList* me, U32 index)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");

	FListNode* node = FList_nodeOfIndex(me, index);
	if (node) {
		void* result = node->data;
		if (FList_removeNode(me, node)) {
			F_DELETE(node);
			return result;
		}
	}
	else {
		F_LOG(F_LOG_ERROR, "FList(%p) take out of range\n", me);
	}

	return NULL;
}

void* FList_atIndex(FList* me, U32 index)
{
	FListNode *node = FList_nodeOfIndex(me, index);
	
	return node ? node->data : NULL;
}

BOOL FList_insertBeforeIter(FList* me, FListIterator* iter, const void* data)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	if (!FListIterator_isVaild(iter)) { 
		return FALSE;
	}
	
	FListNode *node = F_NEW(FListNode);
	if (!node) {
		return FALSE;
	}
	if (FList_insertNode(me, (FListNode*)(iter->p), node)) {
		iter->p = node;
		return TRUE;
	}
	else {
		F_DELETE(node);
		return FALSE;
	}
}

void* FList_takeAtIter(FList* me, FListIterator* iter)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	if (FListIterator_isVaild(iter))
	{
		FListNode* node = (FListNode*)(iter->p);
		void* result = node->data;
		FListIterator_inc(iter);
		if (FList_removeNode(me, node)) {
			F_DELETE(node);
			return result;
		}
	}
	return NULL;
}

void* FList_atIter(FList* me, FListIterator* iter)
{
	if (FListIterator_isVaild(iter))
	{
		FListNode *node = (FListNode*)(iter->p);
		if (node->owner != me->d) {
			return NULL;
		}
		else {
			return node->data;
		}
	}
	return NULL;
}

BOOL FList_pushBack(FList* me, const void* data)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	return FList_insertAtIndex(me, me->d->count, data);
}

void* FList_popBack(FList* me)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");

	if (0 == me->d->count) {
		F_LOG(F_LOG_ERROR, "FList(%p) is empty\n", me);
		return NULL;
	}

	FListNode *node = me->d->tail;
	void* result = node->data;
	
	if (FList_removeNode(me, node)) {
		F_DELETE(node);
		return result;
	}
	
	return NULL;
}

void FList_clear(FList* me) 
{
	while (!FList_isEmpty(me)) {
		FList_takeAtIndex(me, 0);
	}
}

FListIterator FList_begin(FList* me)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	FListIterator iter;
	iter.i = 0;
	iter.p = me->d->head;

	return iter;
}

FListIterator FList_end(FList* me)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	FListIterator iter;
	iter.i = (int)me->d->count;
	iter.p = NULL;
	
	return iter;
}


void FList_output(FList* me, FILE* stream)
{
	if (me && me->d) {
		FListNode *node;
		U32 i;
		
		fprintf(stream, "FList(%p):\n[\n", me);
		for (
			node = me->d->head, i = 0; 
			i < me->d->count; 
			++i, node = node->next
		) {
			fprintf(stream, "  (%d):\t%p\n", i, node->data);
		}
		fprintf(stream, "]\n");
	}
	else {
		F_LOG(F_LOG_ERROR, "FList(%p) is invalid\n", me);
	}
}

void FList_debug(FList* me)
{
	if (me && me->d) {
		fprintf(stdout, "FList(%p):\n[\n", me);
		fprintf(stdout, "  FList.count = %d\n", me->d->count);
		fprintf(stdout, "  FList.head = %p\n", me->d->head);
		fprintf(stdout, "  FList.tail = %p\n", me->d->tail);
		fprintf(stdout, "]\n");
	}
	else {
		F_LOG(F_LOG_ERROR, "FList(%p) is invalid\n", me);
	}
}

BOOL FList_quickSort(FList* me, FListCompare compare)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");

	if (0 == me->d->count) { 
		F_LOG(F_LOG_ERROR, "FList(%p) is empty\n", me);
		return FALSE;
	}

	if (1 == me->d->count)
		return TRUE;

	me->d->compare = compare;
	U32 count = me->d->count;

	//�������Ի�����
	FListNode **sorted = F_NEWARR(FListNode*, count);
	if (!sorted) {
		F_LOG(F_LOG_ERROR, "FList(%p) sort need more memory\n", me);
		return FALSE;
	}

	U32 i;
	FListNode *node;
	
	//���б�����Ԫ��չ�������Ի��������Է�������
	for (i = 0, node = me->d->head; i < count; ++i, node = node->next) {
		sorted[i] = node;
	}

	//quick sort
	qsort((void*)sorted, count, sizeof(FListNode*), FList_nodeCompare);

	//����������˳����������
	for (i = 0; i < count - 1; ++i) {
		sorted[i]->next = sorted[i+1];
		sorted[i+1]->prev = sorted[i];
	}

	//����ͷ��β
	me->d->head = sorted[0];
	me->d->tail = sorted[count-1];
	me->d->head->prev = me->d->tail;
	me->d->tail->next = me->d->head;

	F_DELETE(sorted);
	return TRUE;
}

int FList_findForward(FList* me, const void* data, FListCompare compare)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	int find = -1;
	int posi;
	FListNode *node;
	
	//��ǰ������
	for (
		posi = 0, node = me->d->head; 
		posi < (int)me->d->count; 
		++posi, node = node->next
	) {
		if (compare(node->data, data)) {
			find = (I32)posi;
			break;
		}
	}
	
	return find;
}

int FList_findBackward(FList* me, const void* data, FListCompare compare)
{
	F_ASSERT(me && me->d, "You must call the [FList_ctor] before use it");
	
	int find = -1;
	int posi;
	FListNode *node;
	
	//��ǰ������
	for (
		posi = (int)me->d->count - 1, node = me->d->head; 
		posi >= 0; 
		++posi, node = node->next
	) {
		if (compare(node->data, data)) {
			find = (I32)posi;
			break;
		}
	}
	return find;
}


BOOL FListIterator_isVaild(FListIterator* me)
{
	return me->p != NULL;
}

void* FListIterator_dataOf(FListIterator* me)
{
	if (me->p) {
		return ((FListNode*)(me->p))->data;
	}
	return NULL;
}

void FListIterator_inc(FListIterator* me)
{
	FListNode *node = (FListNode*)(me->p);
	
	me->i++;
	if (node) {
		if (node != node->owner->tail) {
			me->p = node->next;
		}
		else {
			me->p = NULL;
		}
	}
}

void FListIterator_dec(FListIterator* me)
{
	FListNode *node = (FListNode*)(me->p);
	
	me->i--;
	if (node) {
		if (node->owner->head != node) {
			me->p = node->prev;
		}
		else {
			me->p = NULL;
		}
	}
}

void FListIterator_addSelf(FListIterator* me, U32 addend)
{
	U32 i;
	for (i = 0; i < addend; ++i) {
		FListIterator_inc(me); 
	}
}

void FListIterator_subSelf(FListIterator* me, U32 subtractor)
{
	U32 i;
	for (i = 0; i < subtractor; ++i) {
		FListIterator_dec(me); 
	}
}

BOOL FListIterator_lessThan(FListIterator* me, FListIterator* other)
{
	return (me->i - other->i) < 0 ? TRUE : FALSE;
}

BOOL FListIterator_largeThan(FListIterator* me, FListIterator* other)
{
	return (me->i - other->i) > 0 ? TRUE : FALSE;
}

BOOL FListIterator_equal(FListIterator* me, FListIterator* other)
{
	return (me->i == other->i) && (me->p == other->p);
}

BOOL FListIterator_notEqual(FListIterator* me, FListIterator* other)
{
	return (me->i != other->i) || (me->p != other->p);
}

