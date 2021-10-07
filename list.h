// List function declaration
/* Adapted from: Yu Zhang (yuzhang@ustc.edu.cn) */
#ifndef _UTIL_H_
#define _UTIL_H_
#define NEW(p) ((p) = malloc(sizeof *(p)))
#define NEW0(p) memset(NEW(p), 0, sizeof *(p))
#define bool	char
#define TRUE	1
#define FALSE	0

typedef struct listnode{
	void	*item;	// pointer to the item.	
	void 	*next;	// pointer to the next node
	void 	*prev;	// pointer to the previous node
	void	*list;
} *listNode;

typedef struct {
	int 	size;	// number of nodes
	listNode 	first;	// pointer to the first node
	listNode 	last;	// pointer to the last node
} *List;

listNode 	newlistNode	(void *item);   // 新建一个链表节点
List 	newList		(); 				// 新建一个链表
void 	*getFirst	(List list);		// 获取链表第一个元素
void 	*getLast	(List list);		// 返回链表中最后一个元素
void 	*removeFirst	(List list);	// 删除并返回链表中第一个元素
void 	*removeLast	(List list);		// 删除并返回链表中最后一个元素
void 	addFirst	(List list, void *item);	 // 添加元素到链表开始
void 	addLast		(List list, void *item);	 // 添加元素到链表结尾
int  	indexof		(List list, void *item);	 // 返回元素第一次出现的下标 
bool	listcontains	(List list, void *item); // 判断链表是否包含对应元素
int	    listsize	(List list);					 // 返回链表中元素个数
bool	listaddItem	(List list, void *item);		 // 相应元素添加到链表
bool	listremoveItem	(List list, void *item);	 // 链表中删除第一次出现的元素
void 	listclear	(List list, void (*destroyItem)()); // 删除链表中的所有元素
listNode	listnode	(List list, int index);		 // 返回对应的下标中的节点
void	*listget	(List list, int index);			 // 返回对应下标中的元素
void	*listset	(List list, int index, void *item); // 替换对应下标中的元素
void	listadd	(List list, int index, void *item);	 // 向链表中特定位置插入元素
void	*listremove	(List list, int index);			 // 删除链表中的对应下标的元素
void 	destroyList	(List *list, void (*destroyItem)()); // 销毁链表

void destroyItem(void* item);  // destroyItem函数例子-实际上要看item结构

// List Iterator
typedef struct {
	List	list;
	int	nextIndex;
	listNode	lastRet;
	listNode	next;
} *ListItr;

ListItr newListItr	(List list, int index);
ListItr getGListItr	(List list, int index);
void 	resetListItr	(ListItr itr, List list, int index);
bool	hasNext		(ListItr itr);
void	*nextItem	(ListItr itr);
bool	hasPrevious	(ListItr itr);
void	*prevItem	(ListItr itr);
void	destroyListItr	(ListItr *itr);


ListItr nextItr (ListItr itr);
ListItr prevItr (ListItr itr);
void iterList (ListItr itr, void hook());

void removeNode(listNode node);
listNode getFirstNode(List list);


#endif //!def(_UTIL_H_)
