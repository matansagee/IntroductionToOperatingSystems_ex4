#include"list.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <ctype.h>
#include <assert.h>
#include <tchar.h>

#include"SocketSendRecvTools.h"

//*******************************************************
// List.c - include generic function to handle a list
//          the name of the function indicates its action
//*******************************************************


List* CreateList()
{
	List* new_list;
	new_list =(List*)malloc(sizeof(List));

	if (new_list == NULL)
		return NULL;

	new_list->firstNode = NULL;
	new_list->lastNode = NULL;
	new_list->size_of_list = 0;
	return new_list;
}

Node* AddElementAtEnd(List* list, char* name,SOCKET socket)
{
	Node *new_node;
	do{
		new_node = CreateNode();
	} while (new_node == NULL);

	new_node->socket=socket;
	new_node->name = name;
	new_node->next = NULL;

	if (IsListEmpty(list))
	{
		list->firstNode = new_node;
		list->lastNode = list->firstNode;
	}
	else
	{
		list->lastNode->next = new_node;
		list->lastNode = new_node;
	}
	list->size_of_list++;
	return new_node;
}

void AddElemntAtStart(List* list, char* name,SOCKET socket)
{
	Node *TemporaryNode;
	Node *node_new;
	do{
		node_new = CreateNode();
	} while (node_new == NULL);

	node_new->socket=socket;
	node_new->name = name;
	node_new->next = NULL;

	if (IsListEmpty(list))
	{
		list->firstNode = node_new;
		list->lastNode = node_new;
	}
	else 
	{
		TemporaryNode = list->firstNode;
		list->firstNode = node_new;
		list->firstNode->next = TemporaryNode;
	}
	list->size_of_list++;
}



int ReturnIndexOfElement(List* list, char* name)
{
	Node* current_node = list->firstNode;
	int index_of_element = 0;

	while (current_node != NULL)
	{
		if (STRINGS_ARE_EQUAL(current_node->name,name))
		{
			return index_of_element;
		}
		else
		{
			current_node = current_node->next;
			index_of_element++;
		}
	}
	return -1;
}

void DeleteElementByIndex(List* list, int element_index)
{
	Node* previous_node=NULL;
	Node* current_node = list->firstNode;
	int i;
	if (element_index > list->size_of_list)
	{
		printf("ERROR!! index is to large");
		exit;
	}
	if (current_node == NULL)
		return;
	if (current_node->next == NULL)
	{
		free(current_node);
		list->firstNode = NULL;
		return;
	}
	for (i = 0; i < element_index; i++)
	{
		previous_node = current_node;
		current_node = current_node->next;
	}
	previous_node->next = current_node->next;
	free(current_node);

}

void DeleteList(List* list)
{
	Node* current_node;
	Node* next_node;

	if (IsListEmpty(list))
		return;

	current_node = list->firstNode;
	next_node = current_node->next;

	while (current_node != NULL)
	{
		free(current_node);
		current_node = next_node;
		next_node = next_node->next;
	}

	free(list);
	printf("List has been deleted");
}

void PrintList(List* list)
{
	Node* current_node;
	current_node = list->firstNode;

	if (IsListEmpty(list))
	{
		printf("[]\n");
		return;
	}

	printf ("[");
	while (current_node->next != NULL)
	{
		printf("%s, ",current_node->name);
		current_node = current_node->next;
	}
	printf("%s]\n", current_node->name);

}

// Local functions
int IsListEmpty(List *list)
{
	return (list->firstNode == NULL)? 1 : 0;
}

Node* CreateNode()
{
	Node* new_node;
	new_node =(Node*)malloc(sizeof(Node));

	if (new_node == NULL)
		return NULL;

	new_node->next = NULL;
	new_node->activeStatus = ACTIVE;
	new_node->name = NULL;

	return new_node;
}

Node* ReturnElementByName(List* list, char* name)
{
	Node* current_node;
	if (IsListEmpty(list))
	{
		return NULL;
	}
	current_node = list->firstNode;
	if (STRINGS_ARE_EQUAL(current_node->name,name))
	{
		return current_node;
	}

	while (current_node != NULL)
	{
		if (STRINGS_ARE_EQUAL(current_node->name,name))
		{
			return current_node;
		}
		else
		{
			current_node = current_node->next;
		}
	}
	return NULL;
}

Node* ReturnElementByIndex(List* list, int element_index, Node* previous_node)
{
	Node* current_node;
	int index;
	previous_node = NULL;
	current_node = list->firstNode;

	if (element_index < 0)
	{
		printf("ERROR! Input index is illegal!");
		exit;
	}

	if (element_index == 0)
		return current_node;

	for (index = 1; index <= element_index; index++)
	{
		previous_node = current_node;
		current_node = current_node->next;

		if (current_node != NULL)
		{
			continue;
		}
		else
		{
			printf("ERROR!! Element index does not exist!");
			exit;
		}
	}
	return current_node;
}

// another funtions
int DeleteElementByData(List* list, char* name)
{
	Node* node_to_del;
	Node* previous_node=NULL;

	node_to_del = ReturnElementByName(list, name);
	if (node_to_del != NULL)
	{
		previous_node->next = node_to_del->next;
		free(node_to_del);
	}
	else
	{
		printf("Element does not exist!");
		return 0;
	}
	return 1;	 
}


