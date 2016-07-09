#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <ctype.h>
#include <assert.h>
#include <tchar.h>

#include "list.h"
#include "server.h"
#include "utils.h"
#include "SocketSendRecvTools.h"
#include "SocketExampleShared.h"

#define SEND_STR_SIZE 35
HANDLE* ThreadHandles;
SOCKET* ThreadInputs;
FILE *ServerLog;

int NUM_OF_WORKER_THREADS;
int MAX_LOOPS;
static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
static DWORD ServiceThread( SOCKET *t_socket );

LPCTSTR MutexName = _T( "MutexTop" );
HANDLE MutexHandle;
SOCKET MainSocket;

void InitParams(int maxClients);
List* Users;

HANDLE CreateMutexSimple( LPCTSTR MutexName )
{
	return CreateMutex( 
		NULL,              // default security attributes
		FALSE,             // initially not owned
		MutexName);             
}

//***************************************************************
// Main Server - open file, open socket, accept conections and
//               and sending worker thread to each connection
//****************************************************************

void MainServer(int portNumber,int maxClients)
{
	int Ind;
	int Loop;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	WSADATA wsaData;
	DWORD waitRes;
	// Initialize Winsock.
	int StartupRes = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );	           
	ServerLog = fopen("server_log.text","w+");
	if (ServerLog == NULL )
	{
		printf("Failed to create log file\n");
		exit(1);
	}
	MainSocket = INVALID_SOCKET;

	if ( StartupRes != NO_ERROR )
	{
		printf( "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError() );
		return;
	}
	InitParams(maxClients);
	MainSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	if ( MainSocket == INVALID_SOCKET ) 
	{
		printf( "Error at socket( ): %ld\n", WSAGetLastError( ) );
		exit(1);
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons( portNumber ); //The htons function converts a u_short from host to TCP/IP network byte order 
	
	bindRes = bind( MainSocket, ( SOCKADDR* ) &service, sizeof( service ) );
	if ( bindRes == SOCKET_ERROR ) 
	{
		printf( "bind( ) failed with error %ld. Ending program\n", WSAGetLastError( ) );
		exit(1);
	}

	ListenRes = listen( MainSocket, SOMAXCONN );
	if ( ListenRes == SOCKET_ERROR ) 
	{
		printf( "Failed listening on socket, error %ld.\n", WSAGetLastError() );
		exit(1);
	}

	MutexHandle = CreateMutexSimple( MutexName );
	if (MutexHandle == NULL) 
	{
		printf("CreateMutex error: %d\n", GetLastError());
	}

	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
		ThreadHandles[Ind] = NULL;

	printf( "Waiting for a client to connect...\n" );

	for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )
	{
		SOCKET AcceptSocket;
		AcceptSocket = accept( MainSocket, NULL, NULL );
		if ( AcceptSocket == INVALID_SOCKET )
		{
			printf( "Accepting connection with client failed, error %ld\n", WSAGetLastError() ) ; 
			CleanupWorkerThreads();
			exit(1);
		}

		Ind = FindFirstUnusedThreadSlot();

		if ( Ind == NUM_OF_WORKER_THREADS ) //no slot is available
		{ 
			fprintf(ServerLog,"SYSTEM:: No available socket at the moment. Try again later.\n");
			if (SendString( "No available socket at the moment. Try again later.", AcceptSocket ) == TRNS_FAILED ) 
			{
				printf( "Service socket error while writing, closing thread.\n" );
			}
			closesocket( AcceptSocket ); //Closing the socket, dropping the connection.
			Loop = Loop -1;
		} 
		else 	
		{
			printf( "Client Connected.\n" );
			if (SendString( "connected", AcceptSocket ) == TRNS_FAILED ) 
			{
				printf( "Service socket error while writing, closing thread.\n" );
			}
			Loop = Loop -1;
			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close 
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				( LPTHREAD_START_ROUTINE ) ServiceThread,
				&( ThreadInputs[Ind] ),
				0,
				NULL
				);
		}
	}

}

//********************************************************************
// FindFirstUnusedThreadSlot - bring back indication of unused thread
//********************************************************************

static int FindFirstUnusedThreadSlot()
{ 
	int Ind;

	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
	{
		if ( ThreadHandles[Ind] == NULL )
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject( ThreadHandles[Ind], 0 ); 

			if ( Res == WAIT_OBJECT_0 ) // this thread finished running
			{				
				CloseHandle( ThreadHandles[Ind] );
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}

//*****************************************************************************
// CleanupWorkerThreads - exit flow- close socket and handlers for all threads
//*****************************************************************************
static void CleanupWorkerThreads()
{
	int Ind; 

	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
	{
		if ( ThreadHandles[Ind] != NULL )
		{
			DWORD Res = WaitForSingleObject( ThreadHandles[Ind], INFINITE ); 

			if ( Res == WAIT_OBJECT_0 ) 
			{
				closesocket( ThreadInputs[Ind] );
				CloseHandle( ThreadHandles[Ind] );
				ThreadHandles[Ind] = NULL;
				break;
			}
			else
			{
				printf( "Waiting for thread failed. Ending program\n" );
				return;
			}
		}
	}
}

//*****************************************************
// CleanupWorkerThreads - check recv string from client
//*****************************************************
int ValidateReceivingString(TransferResult_t recvRes)
{
	if ( recvRes == TRNS_FAILED )
	{
		printf( "Service socket error while reading, closing thread.\n" );
		return 0;
	}
	else if ( recvRes == TRNS_DISCONNECTED )
	{
		printf( "Connection closed while reading, closing thread.\n" );
		return 0;
	}
	return 1;
}

//********************************************************
// CleanupWorkerThreads - translate command from client
//                        and execute handler for each cmd
//********************************************************
DWORD HandleClientCommand(char* str, SOCKET *sourceSocket,BOOL* Done,char* clientNameStr)
{

	char* pch;
	char* command;
	char* newCommand;
	char* arg1;
	char* arg2;
	ErrorCode_t errorCode;
	char* systemMessage;
	char* strCopy = (char*) malloc (strlen(str)*sizeof(char));
	if (strCopy == NULL)
	{
		exit(1);
	}
	strcpy(strCopy,str);
	command = strtok (str," ");
	newCommand = command +1;
	if (STRINGS_ARE_EQUAL(command,"/quit"))
	{
		fprintf(ServerLog,"REQUEST::from %s: %s\n",clientNameStr,newCommand);

		*Done = TRUE;
		return ISP_SUCCESS;
	}
	else if (STRINGS_ARE_EQUAL(command,"/active_users"))
	{
		errorCode = SendActiveUsers(sourceSocket, Users, MutexHandle,clientNameStr,&systemMessage,ServerLog);
		if (errorCode == ISP_SUCCESS)
		{
			fprintf(ServerLog,"REQUEST::from %s: %s\n",clientNameStr,newCommand);
		}
		return errorCode;
	}

	else if (strlen(command)>0 && command[0] != '/')
	{
		errorCode = SendPublicMessage(strCopy,clientNameStr,MutexHandle,Users,*sourceSocket);
		if (errorCode == ISP_SUCCESS)
		{
			fprintf(ServerLog,"CONVERSATION:: %s: %s\n",clientNameStr,strCopy);
		}
		return errorCode;
	}

	else if (STRINGS_ARE_EQUAL(command,"/private_message"))
	{
		arg1 = strtok (NULL," ");
		arg2 = strtok (NULL,"\0");
		if (arg1 != NULL ) 
		{
			errorCode = SendPrivateMessage(arg1,clientNameStr,arg2, MutexHandle, Users,sourceSocket,ServerLog);
			if (errorCode == ISP_SUCCESS)
			{
				fprintf(ServerLog,"CONVERSATION:: private message from %s to %s: %s\n",clientNameStr,arg1,arg2);
			}
			return errorCode;
		}
		else
		{
			return ISP_NO_SUCCESS;
		}
	}

	return ISP_NO_SUCCESS;
}
//*****************************************************************
// Service thread - thread that opens for each successful
//                  client connection and "talks" to the client.
//                  handle send/rcvd actions
//*****************************************************************
DWORD ServiceThread( SOCKET *t_socket ) 
{
	char SendStr[SEND_STR_SIZE];
	char *clientNameStr = NULL;
	AccessResult accessResult = NO_ACCESS;
	BOOL Done = FALSE;
	TransferResult_t sendRes;
	TransferResult_t recvRes;
	recvRes = ReceiveString( &clientNameStr , *t_socket );
	if (HandleAccessRequest(clientNameStr,&accessResult,*t_socket,MutexHandle,Users,ServerLog) != ISP_SUCCESS)
	{
		closesocket( *t_socket );
		return 1;
	}
	if (accessResult == NO_ACCESS)
	{
		fprintf(ServerLog,"SYSTEM:: %s already taken!\n",clientNameStr);
		if (SendString("already taken!", *t_socket) == TRNS_FAILED)
		{
			closesocket( *t_socket );
			return 1;
		}
		closesocket( *t_socket );
		return 0;
	}
	//Session connected
	sendRes = SendString("welcome to the session.", *t_socket );
	fprintf(ServerLog,"SYSTEM:: sent to %s: Hello %s, welcome to the session\n",clientNameStr,clientNameStr); 
	if ( sendRes == TRNS_FAILED ) 
	{
		if (LeaveSessionFlow(clientNameStr,*t_socket,Users,MutexHandle,ServerLog) == ISP_EXIT_PROGRAM)
		{
			return ISP_EXIT_PROGRAM;
		}
		closesocket( *t_socket );
		return ISP_NO_SUCCESS;
	}
	while ( !Done ) 
	{		
		char* sessionStr = NULL;
		recvRes = ReceiveString( &sessionStr , *t_socket );
		if (!ValidateReceivingString(recvRes))
		{
			if (LeaveSessionFlow(clientNameStr,*t_socket,Users,MutexHandle,ServerLog) == ISP_EXIT_PROGRAM)
			{
				return ISP_EXIT_PROGRAM;
			}
			closesocket( *t_socket );
			return ISP_NO_SUCCESS;
		}
		if (HandleClientCommand( sessionStr, t_socket ,&Done,clientNameStr) != ISP_SUCCESS)
		{
			char* message = ConcatString("No such command: ",sessionStr + 1,"");
			fprintf(ServerLog,"SYSTEM:: sent to %s: No such command: %s\n",clientNameStr,sessionStr);
			sendRes = SendString(message, *t_socket );
			if ( sendRes == TRNS_FAILED ) 
			{
				printf( "Service socket error while writing, closing thread.\n" );
				if (LeaveSessionFlow(clientNameStr,*t_socket,Users,MutexHandle,ServerLog) == ISP_EXIT_PROGRAM)
				{
					return ISP_EXIT_PROGRAM;
				}
				closesocket( *t_socket );
				return ISP_NO_SUCCESS;
			}
		}
		free( sessionStr );		
	}

	if (LeaveSessionFlow(clientNameStr,*t_socket,Users,MutexHandle,ServerLog) == ISP_EXIT_PROGRAM)
	{
		return ISP_EXIT_PROGRAM;
	}

	closesocket( *t_socket );
	return ISP_SUCCESS;
}
//*****************************************************
// InitParams - init is a short is a short of initialization
//              in Hebrew - "ithul"
//*******************************************************
void InitParams(int maxClients)
{
	NUM_OF_WORKER_THREADS = maxClients;
	MAX_LOOPS = NUM_OF_WORKER_THREADS +1;

	ThreadHandles =(HANDLE*) malloc(NUM_OF_WORKER_THREADS*sizeof(*ThreadHandles));
	if (ThreadHandles == NULL){
		exit(1);
	}
	ThreadInputs =(SOCKET*) malloc(NUM_OF_WORKER_THREADS*sizeof(*ThreadInputs));
	if (ThreadInputs == NULL){
		exit(1);
	}
	Users = CreateList();
}
//*******************************************************
// CloseSession - last thread will close the main socket
//                and close server file
//*******************************************************
void CloseSession(SOCKET sourceSocket)
{
	closesocket(sourceSocket);
	closesocket(MainSocket);
	CloseHandle(MutexHandle);
	fclose(ServerLog);
}
