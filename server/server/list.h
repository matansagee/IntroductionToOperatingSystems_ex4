#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */
#ifndef __NODE_H
#define __NODE_H

#define CHAR_ARRAY_SIZE 100
#include <winsock2.h>
typedef enum { ACTIVE , NOT_ACTIVE } ActiveStatus;

typedef struct Node
{
	struct Node *next;
	char* name;
	ActiveStatus activeStatus;
	SOCKET socket;
} Node;

typedef struct List
{
	Node *firstNode;
	Node *lastNode; 
	int size_of_list;
} List;

List* CreateList();
Node* CreateNode();
Node* ReturnElementByName(List* list, char* name);
Node* ReturnElementByIndex(List* list, int element_index, Node* previous_node);
void DeleteList(List* list);

//insert element I at end of list. Return 1 if succeed or 0 if List has bug. 
Node* AddElementAtEnd(List* list, char* name,SOCKET socket);

//insert element I at start of list
void AddElemntAtStart(List* list, char* name,SOCKET socket);

//return the first index where element I is found, -1 if not found.
//The first element is index 1, the second is index 2, etc.
int ReturnIndexOfElement(List* list, char* name);

//remove the element at the specified index.
//If index is too large return 0, else return 1.
void DeleteElementByIndex(List* list, int element_index);

//remove the first element with the specified data.
//If data does not exist return 0, else return 1.
int DeleteElementByData(List* list, char* name);

//print the entire list.An empty list is printed as “[]”.
//A list with one element whose value is 1 is printed as “[1]”.
//A list with 3 elements, 1 - 3, is printed as “[1, 2, 3]”, with a space after each comma
void PrintList(List* list);
//return pointer to list with all active user nodes

#endif /* __NODE_H */


