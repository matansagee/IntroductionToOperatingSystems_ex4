#ifndef SERVER_H
#define SERVER_H

#ifndef MAX_BUUFER
#define MAX_BUFFER 256
#endif

#ifndef MAX_USER_NAME_LENGTH
#define MAX_USER_NAME_LENGTH 16
#endif

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void MainServer(int portNumber,int maxClients);

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif // SERVER_H