#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <sys/stat.h>
#include <sys/types.h>
#pragma comment(lib, "IPHLPAPI.lib")

#include "utils.h"
#include "SocketSendRecvTools.h"
#include "client.h"

SOCKET m_socket;
FILE *UsernameErrorsFile;
FILE *UsernameLogFile;

//******************************************************************************
// CheckSessionAccess - send request and handle according to aproove/disapproove
//******************************************************************************

DWORD CheckSessionAccess(char* clientName)
{
	TransferResult_t sendRes;
	TransferResult_t recvRes;
	char *acceptedStr = NULL;
	
	// recived connection failed/successed
	recvRes = ReceiveString( &acceptedStr , m_socket );
	if ( recvRes == TRNS_FAILED )
	{
		printf("Socket error while trying to write data to socket\n");
		return 0x555;
	}
	if (STRINGS_ARE_EQUAL(acceptedStr,"No available socket at the moment. Try again later."))
	{
		fprintf(UsernameLogFile,"RECEIVED:: %s\n",acceptedStr);
		printf("%s\n",acceptedStr);
		return NO_ACCESS;
	}
	if (STRINGS_ARE_EQUAL(acceptedStr,"connected"))
	{
		acceptedStr = NULL;
		sendRes = SendString( clientName, m_socket);
		if ( sendRes == TRNS_FAILED ) 
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}

		recvRes = ReceiveString( &acceptedStr , m_socket );
		if ( recvRes == TRNS_FAILED )
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		else if ( recvRes == TRNS_DISCONNECTED )
		{
		printf("Server closed connection. Bye!\n");
		return 0x555;
		}

		if (STRINGS_ARE_EQUAL(acceptedStr,"already taken!"))
		{
			printf("%s %s\n",clientName,acceptedStr);
			return NO_ACCESS;
		}
	
		if (STRINGS_ARE_EQUAL(acceptedStr,"welcome to the session."))
		{
			printf("Hello %s, welcome to the session.\n",clientName);
			fprintf(UsernameLogFile,"RECEIVED:: Hello %s, welcome to the session.\n",clientName);
			return ACCESS;
		}
	return NO_ACCESS;
	}
 return NO_ACCESS;
}

//Reading data coming from the server
static DWORD RecvDataThread(void)
{
	TransferResult_t RecvRes;

	while (1) 
	{
		char *acceptedStr = NULL;
		RecvRes = ReceiveString( &acceptedStr , m_socket );

		if ( RecvRes == TRNS_FAILED )
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		else if ( RecvRes == TRNS_DISCONNECTED )
		{
			printf("Server closed connection. Bye!\n");
			return 0x555;
		}
		else
		{
			fprintf(UsernameLogFile,"RECEIVED:: %s\n",acceptedStr);
			printf("%s\n",acceptedStr);
		}
		
		free(acceptedStr);
	}

	return 0;
}

//Sending data to the server
static DWORD SendDataThread(void)
{
	char SendStr[256];
	TransferResult_t SendRes;
	
	while (1) 
	{
		gets(SendStr); //Reading a string from the keyboard
		
		SendRes = SendString( SendStr, m_socket);
		fprintf(UsernameLogFile,"SENT:: %s\n",SendStr);
		if ( SendRes == TRNS_FAILED ) 
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
	}
}
//********************************************************************
// CreateLogsFiles - open files by user_name
//********************************************************************
void CreateLogsFiles(char* clientName)
{
	char* usernameErrorsFile_string = (char*) malloc(MAX_USER_NAME_LENGTH *sizeof (*usernameErrorsFile_string));
	char* usernameLogFile_string = (char*) malloc(MAX_USER_NAME_LENGTH *sizeof (*usernameErrorsFile_string));
	
	if (usernameErrorsFile_string == NULL || usernameLogFile_string == NULL)
	{
		exit(1);
	}

	usernameErrorsFile_string =  ConcatString(clientName,"_errors" , ".txt");
	usernameLogFile_string = ConcatString (clientName,"_log" , ".txt");
	
	//Openfiles for create new files
	UsernameErrorsFile=fopen(usernameErrorsFile_string,"a");	
	UsernameLogFile   =fopen(usernameLogFile_string,"a");
	
	if (UsernameErrorsFile == NULL || UsernameLogFile == NULL )
	{
		printf("ERROR: FILES Were not be able to create\n");
		exit(1);
	}
	return;
}

//********************************************************************
// MainClient - open a socket, ask for connection ,and manage data 
//              thransportation in from of the server by 2 threads
//********************************************************************
void MainClient(char* serverIp,char* clientName,int serverPort)
{
	SOCKADDR_IN clientService;
	HANDLE hThread[2];
	struct sockaddr_in foo;
	int len = sizeof(struct sockaddr);
    // Initialize Winsock.
    WSADATA wsaData; //Create a WSADATA object called wsaData.
    int iResult = WSAStartup( MAKEWORD(2, 2), &wsaData );
    if ( iResult != NO_ERROR )
        printf("Error at WSAStartup()\n");
	
	// Create a socket.
    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	// Check for errors to ensure that the socket is a valid socket.
    if ( m_socket == INVALID_SOCKET ) {
        printf( "Error at socket(): %ld\n", WSAGetLastError() );
        WSACleanup();
        return;
    }

	CreateLogsFiles(clientName);

    clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr( serverIp ); //Setting the IP address to connect to
    clientService.sin_port = htons( serverPort ); //Setting the port to connect to.
	if ( connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) 
	{
		getsockname(m_socket, (struct sockaddr *) &foo, &len);
        fprintf(UsernameErrorsFile, "%s failed to connect to %s:%d - error number %d\n",
			 inet_ntoa(foo.sin_addr),serverIp,serverPort,WSAGetLastError() );
		fprintf(UsernameErrorsFile, "%s failed to connect to %s:%d - error number %d\n",
			 inet_ntoa(foo.sin_addr),serverIp,serverPort,WSAGetLastError() );
        WSACleanup();
        return;
    }

	// after connect established, check accsess to the session in front of the server
	if (CheckSessionAccess(clientName) == NO_ACCESS)
	{
		closesocket(m_socket);
		WSACleanup();
		exit(1);
	}

	hThread[0]=CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE) SendDataThread,
		NULL,
		0,
		NULL
	);
	hThread[1]=CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE) RecvDataThread,
		NULL,
		0,
		NULL
	);

	WaitForMultipleObjects(2,hThread,FALSE,INFINITE);

	TerminateThread(hThread[0],0x555);
	TerminateThread(hThread[1],0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	fclose(UsernameErrorsFile);
	fclose(UsernameLogFile);

	closesocket(m_socket);
	WSACleanup();
    
	return;
}

