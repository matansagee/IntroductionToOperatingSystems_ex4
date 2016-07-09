#define _CRT_SECURE_NO_WARNINGS /* to suppress Visual Studio 2010 compiler warning */
#ifndef SERVER_H
#define SERVER_H

#ifndef MAX_BUUFER
#define MAX_BUFFER 256
#endif

#ifndef MAX_USER_NAME_LENGTH
#define MAX_USER_NAME_LENGTH 16
#endif

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#include <ctype.h>
#include <assert.h>
#include <tchar.h>

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

typedef enum { NO_ACCESS , ACCESS } AccessResult;

typedef enum { 
    ISP_SUCCESS, 
	ISP_NO_SUCCESS,
    ISP_FILE_OPEN_FAILED, 
    ISP_FILE_SEEK_FAILED,
    ISP_FILE_READING_FAILED,
    ISP_FTELL_FAILED,
    ISP_MUTEX_OPEN_FAILED,
    ISP_MUTEX_CREATE_FAILED,
    ISP_MUTEX_WAIT_FAILED,
    ISP_MUTEX_ABANDONED,
    ISP_MUTEX_RELEASE_FAILED,
    ISP_ILLEGAL_LETTER_WAS_READ,
	ISP_EXIT_PROGRAM
} ErrorCode_t;

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void MainServer(int portNumber,int maxClients);
void CloseSession(SOCKET sourceSocket);

#endif // SERVER_H