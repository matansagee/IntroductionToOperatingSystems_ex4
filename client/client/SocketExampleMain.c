/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering, Winter 2011, 
 by Amnon Drory.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#include <stdio.h>
#include <conio.h>

#include "SocketExampleClient.h"
#include "SocketExampleServer.h"

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void main()
{
	char c = '0';

	while( ( c != '1' ) && ( c != '2' ) )
	{
		printf("Enter 1 to run Server, 2 to run Client: ");

		c = _getch();
		printf("%c\n",c);

		if ( c == '1' )
		{
			printf("\n\n\n\n\n\t\t\tSERVER\n\n\n");
			MainServer();
		}
		else if ( c == '2' )
		{
			printf("\n\n\n\n\n\t\t\tClient\n\n\n");
			MainClient();
		}
	}
}
