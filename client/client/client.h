#ifndef CLIENT_H
#define CLIENT_H

#ifndef MAX_BUUFER
#define MAX_BUFFER 256
#endif

#ifndef MAX_USER_NAME_LENGTH
#define MAX_USER_NAME_LENGTH 16
#endif

typedef char* string;
typedef enum { NO_ACCESS , ACCESS } AccessResult;
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void MainClient(string serverIp,string clientName,int serverPort);

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif // CLIENT_H