#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <ctype.h>
#include <assert.h>
#include <tchar.h>

#include "utils.h"
#include "list.h"
#include "server.h"
#include "SocketSendRecvTools.h"

//*******************************************************
// ConcatString - concat 3 stings
//*******************************************************

char* ConcatString(	char* source_a, char* source_b,char* source_c)
{
	int total_size = strlen(source_a)+strlen(source_b)+strlen(source_c);
	char* string = (char*) malloc(total_size* sizeof(char));
	if (string == NULL)
	{
		exit(1);
	}
	strcpy(string,source_a);
	strcat(string,source_b);
	strcat(string,source_c);
	return string;
}

//*******************************************************
// RequestAccessFlow - if user name is valid- add it
//                     to list and send others a message
//*******************************************************

BOOL RequestAccessFlow(List* users, char* name,SOCKET socket,FILE *serverLog)
{
	int i;
	Node* node = ReturnElementByName(users,name);
	Node* currentNode;
	BOOL statusRequest = FALSE;
	char* message;
	if (STRINGS_ARE_EQUAL(name,"server"))
	{
		return statusRequest;
	}
	if (node == NULL)
	{
		node = AddElementAtEnd(users,name,socket);
		statusRequest= TRUE;
	}
	else if (node->activeStatus == NOT_ACTIVE)
	{
		node->activeStatus = ACTIVE;
		node->socket = socket;
		statusRequest= TRUE;
	}
	if (statusRequest == TRUE)
	{
		currentNode = users->firstNode;
		while(currentNode != NULL)
		{
			if(currentNode->activeStatus== ACTIVE && !STRINGS_ARE_EQUAL(currentNode->name,node->name))
			{
				message = ConcatString("*** ",name," has joined the session ***");
				SendString(message,currentNode->socket);
			}
			currentNode = currentNode->next;
		}
		fprintf(serverLog,"SYSTEM:: *** %s has joined the session ***\n",name);
	}
	return statusRequest;
}
//******************************************************************
// LeaveSessionFlow - update list of users and send others a message
//                    **handle corner case of last user leaving 
//******************************************************************

ErrorCode_t LeaveSessionFlow(char* clientName,SOCKET socket,List* users,HANDLE mutex,FILE *serverLog)
{
	DWORD WaitRes;
	Node* userNode;
	Node* currentNode;
	TransferResult_t sendRes;
	int activeUserCounter = 0;
	char* message;
	fprintf(serverLog,"SYSTEM:: *** %s has left the session ***\n",clientName);
	__try 
	{
		WaitRes = WaitForSingleObject( mutex, INFINITE );
		switch (WaitRes) 
		{
		case WAIT_OBJECT_0: 
			{
				userNode=ReturnElementByName(users, clientName);
				userNode->activeStatus=NOT_ACTIVE;
				userNode->socket=NULL;
				currentNode = users->firstNode;
				while(currentNode != NULL)
				{
					if(currentNode->activeStatus== ACTIVE)
					{
						activeUserCounter++;
						message = ConcatString("*** ",clientName," has left the session ***");
						sendRes = SendString(message,currentNode->socket);
						if ( sendRes == TRNS_FAILED ) 
						{
							printf( "Service socket error while writing, closing thread.\n" );
							currentNode->activeStatus= NOT_ACTIVE;
							closesocket( currentNode->socket );
							currentNode->socket= NULL;

						}
					}
					currentNode = currentNode->next;
				}
				if (activeUserCounter == 0)
				{
					CloseSession(socket);
					return ISP_EXIT_PROGRAM;
				}
			} 
			break; 
		case WAIT_ABANDONED: 
			return ISP_MUTEX_ABANDONED; 
		}
	}
	__finally 
	{ 
		if ( !ReleaseMutex(mutex)) {
			return ( ISP_MUTEX_RELEASE_FAILED );
		} 
		//printf("%d is releasing mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
		return ( ISP_SUCCESS );
	}
}

//**************************************************
// SendActiveUsers - handle active users request
//**************************************************
ErrorCode_t SendActiveUsers(SOCKET *sd, List* users, HANDLE mutex,char* clientNameStr,char** systemMessage,FILE *serverLog)
{
	DWORD WaitRes;
	Node* currentNode = users->firstNode;
	Node* userNode;
	char* activeUsersNameList;
	BOOL firstActiveUser= FALSE;
	TransferResult_t sendRes;
	activeUsersNameList = (char*) malloc (MAX_USER_NAME_LENGTH*sizeof(*activeUsersNameList));
	if (activeUsersNameList == NULL)
	{
		exit(1);
	}
	__try 
	{
		WaitRes = WaitForSingleObject( mutex, INFINITE );
		switch (WaitRes) 
		{
		case WAIT_OBJECT_0: 
			{
				while(currentNode != NULL)
				{
					if(currentNode->activeStatus== ACTIVE && ! firstActiveUser)
					{
						strcpy(activeUsersNameList,currentNode->name);
						firstActiveUser=TRUE;
					}
					else if (currentNode->activeStatus== ACTIVE)
					{
						activeUsersNameList = ConcatString(activeUsersNameList," ",currentNode->name);
					}
					currentNode = currentNode->next;
				}
				*systemMessage = activeUsersNameList;
				sendRes = SendString(activeUsersNameList,*sd);
				if ( sendRes == TRNS_FAILED ) 
				{
					printf( "Service socket error while writing, closing thread.\n" );
					userNode = ReturnElementByName(users,clientNameStr);
					userNode->activeStatus = NOT_ACTIVE;
					closesocket( userNode->socket );
					userNode->socket=NULL;
					return ISP_NO_SUCCESS;
				}
				fprintf(serverLog,"SYSTEM:: sent to %s: %s\n",clientNameStr,activeUsersNameList);
			}
			break; 
		case WAIT_ABANDONED: 
			return ISP_MUTEX_ABANDONED; 
		}
	}
	__finally 
	{ 
		if ( !ReleaseMutex(mutex)) 
		{
			return ( ISP_MUTEX_RELEASE_FAILED );
		} 

		//printf("%d is releasing mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
		return ( ISP_SUCCESS );
	}
}


//****************************************************
// SendPrivateMessage - handle private message request
//****************************************************
ErrorCode_t SendPrivateMessage(char* userDestName, char* userSourceName, char* message, HANDLE mutex, List* users,SOCKET *sourceSocket,FILE *serverLog)
{
	DWORD WaitRes;
	Node* userDestNode;
	Node* userSourceNode;
	TransferResult_t sendRes;
	char* messageConcat;
	char* messageConcatPrivate;
	__try 
	{
		WaitRes = WaitForSingleObject( mutex, INFINITE );
		switch (WaitRes) 
		{
		case WAIT_OBJECT_0: 
			{
				userDestNode = ReturnElementByName(users,userDestName);
				if (userDestNode != NULL && userDestNode->activeStatus == ACTIVE)
				{
					messageConcatPrivate = ConcatString("/private_message from",userSourceName,"");
					if (message != NULL)
					messageConcat = ConcatString(messageConcatPrivate,": ",message);
					else
					messageConcat = ConcatString(messageConcatPrivate,": ","");
					sendRes = SendString(messageConcat,userDestNode->socket);
					if ( sendRes == TRNS_FAILED ) 
					{
						printf( "Service socket error while writing, closing thread.\n" );
						userDestNode->activeStatus = NOT_ACTIVE;
						closesocket( userDestNode->socket );
						userDestNode->socket=NULL;
						return ISP_NO_SUCCESS;
					}
				}
				else
				{
					fprintf(serverLog,"SYSTEM:: sent to %s: No such user %s\n",userSourceName,userDestName);
					messageConcat = ConcatString("No such user ","",userDestName);
					sendRes = SendString(messageConcat,*sourceSocket);
					if ( sendRes == TRNS_FAILED ) 
					{
						printf( "Service socket error while writing, closing thread.\n" );
						userSourceNode = ReturnElementByName(users,userSourceName);
						userSourceNode->activeStatus= NOT_ACTIVE;
						closesocket( userSourceNode->socket );
						userSourceNode->socket= NULL;
						return ISP_NO_SUCCESS;
					}
				}
			}
			break; 
		case WAIT_ABANDONED: 
			return ISP_MUTEX_ABANDONED; 
		}
	}
	__finally 
	{ 
		if ( !ReleaseMutex(mutex)) 
		{
			return ( ISP_MUTEX_RELEASE_FAILED );
		} 
		//printf("%d is releasing mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
		return ( ISP_SUCCESS );
	}

	return ISP_SUCCESS;
}

//****************************************************
// SendPublicMessage - handle puplic message request
//****************************************************
ErrorCode_t SendPublicMessage(char* arg1,char* clientNameStr,HANDLE mutexHandle,List* users,SOCKET sourceSocket)
{
	DWORD WaitRes;
	Node* sourceNode;
	Node* currentNode;
	TransferResult_t sendRes;
	char* messageConcat;
	__try 
	{
		WaitRes = WaitForSingleObject( mutexHandle, INFINITE );
		switch (WaitRes) 
		{
		case WAIT_OBJECT_0: 
			{
				sourceNode = ReturnElementByName(users,clientNameStr);
				currentNode = users->firstNode;
				while(currentNode != NULL)
				{
					if(currentNode->activeStatus== ACTIVE && !STRINGS_ARE_EQUAL(currentNode->name,sourceNode->name))
					{
						messageConcat = ConcatString(clientNameStr,": ",arg1);
						sendRes = SendString(messageConcat,currentNode->socket);
						if ( sendRes == TRNS_FAILED ) 
						{
							printf( "Service socket error while writing, closing thread.\n" );
							currentNode->activeStatus= NOT_ACTIVE;
							closesocket( currentNode->socket );
							currentNode->socket= NULL;
							return ISP_NO_SUCCESS;
						}
					}
					currentNode = currentNode->next;
				}
			}
			break; 
		case WAIT_ABANDONED: 
			return ISP_MUTEX_ABANDONED; 
		}
	}
	__finally 
	{ 
		if ( !ReleaseMutex(mutexHandle)) 
		{
			return ( ISP_MUTEX_RELEASE_FAILED );
		} 
		//printf("%d is releasing mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
		return ( ISP_SUCCESS );
	}
}

//******************************************************
// HandleAccessRequest - get the mutex on the list and call
//                       RequestAccessFlow func.
//******************************************************
ErrorCode_t HandleAccessRequest(char* AcceptedStr,AccessResult* accessResult,SOCKET socket,HANDLE mutexHandle,List* users,FILE *serverLog)
{
	DWORD WaitRes;
	__try 
	{
		WaitRes = WaitForSingleObject( mutexHandle, INFINITE );
		switch (WaitRes) 
		{
		case WAIT_OBJECT_0: 
			{
				if (RequestAccessFlow(users,AcceptedStr,socket,serverLog))
				{
					*accessResult = ACCESS;
				}
				else
				{
					*accessResult = NO_ACCESS;
				}
			} 
			break; 
		case WAIT_ABANDONED: 
			return ISP_MUTEX_ABANDONED; 
		}
	}
	__finally 
	{ 
		if ( !ReleaseMutex(mutexHandle)) {
			return ( ISP_MUTEX_RELEASE_FAILED );
		} 
		//printf("%d is releasing mutex %d\n",GetCurrentThreadId(),MutexHandleTop);
		return ( ISP_SUCCESS );
	}
}