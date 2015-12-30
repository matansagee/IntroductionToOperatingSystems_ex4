#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"server.h"

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
	printf("Shani is need to go buy Bamba %d %d\n",portNumber,maxClients);
	MainServer(portNumber,maxClients);

	while(1);
}