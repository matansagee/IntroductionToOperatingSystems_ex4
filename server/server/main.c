#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"server.h"

//---------------------------------------------------------------------------
// Main.c -  includes of handling input parameters and file creations, 
//           calling main server prog
//---------------------------------------------------------------------------

int main(int argc,char** argv)
{
	int portNumber, maxClients;
	if (argc != 3)
	{
		printf("Argument Prameters are not correct\n");
		return 0;
	}
	portNumber = atoi(argv[1]);
	maxClients = atoi(argv[2]);
	if (maxClients <= 0 && portNumber <= 0)
	{
		printf("Argument Prameters are not correct\n");
		return 0;
	}
	MainServer(portNumber,maxClients);
}