/*
	Author: Mengwen Li
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <crypt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define BUFSIZE 1024
#define USERCNT 5
#define PORT 9002

void usage(char** argv);
int isTerminate(char* recvMsg, int size);
char* concat(char* s1, char* s2);
int sendMsg(char* msg, int sock);
char* recvMsg(int sock);
char* getPwd(char* clientName);
int getCmdCnt(char* cmd);
void getCmdArr(char** cmdArr, char* cmd, int arrSize);
int isNameLegal(char* name);

// Define a list of users here.
char* legalNames[USERCNT] = {"a", "b", "c", "d", "e"};
char* legalPwd[USERCNT] = {"a", "b", "c", "d", "e"};
char* salt = "salt";

int main(int argc, char** argv)
{
	time_t t;
	int sock, servHostPort, cliLen, clientSock;
	struct sockaddr_in cliAddr, servAddr;
	// Set server port.
	servHostPort = PORT;
	if(argc != 1)
	{
		printf("Command line argument error!");
		usage(argv);
		exit(-1);
	}
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket()");
		exit(sock);
	}
	bzero((char*) &servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(servHostPort);
	if(bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
	{
		perror("bind()");
		exit(-1);
	}
	if(listen(sock, 5))
	{
		perror("listen()");
		exit(-1);
	}
	while(1)
	{
		// Wait for client connection.
		cliLen = sizeof(cliAddr);
		clientSock = accept(sock, (struct sockaddr *)&cliAddr, &cliLen);
		if(clientSock < 0)
		{
			perror("accept()");
			exit(clientSock);
		}
		// Fork a process here.
		int pid = fork();
		if(pid == 0)
		{
			char intBuf[1024];
			close(sock);
			srand((unsigned)time(&t));
			// Readin username.
			char* clientName = recvMsg(clientSock);
			// printf("Received username is %s\n", clientName);
			if(isNameLegal(clientName))
			{
				int randNum = rand();
				sprintf(intBuf, "%d", randNum);
				sendMsg(intBuf, clientSock);
			}
			else
			{
				sendMsg("usernotfound", clientSock);
				close(clientSock);
				return 0;
			}
			char* rcvdPwd = recvMsg(clientSock);
			// printf("Received password is %s\n", rcvdPwd);
			// Check whether the password is correct.
			char* corrPwd = getPwd(clientName);
			char* keyStr = concat(corrPwd, intBuf);
			char* cryptKey = crypt(keyStr, salt);
			// printf("Correct key should be: %s\n", cryptKey);
			if(strcmp(cryptKey, rcvdPwd) == 0)
			{
				sendMsg("keycorrect", clientSock);
			}
			else
			{
				sendMsg("keyerror", clientSock);
				close(clientSock);
				return 0;
			}
			// Receive command from client.
			char* cmd = recvMsg(clientSock);
			char* dupCmd = strdup(cmd);
			// printf("command received is %s\n", cmd);
			int cmdCnt = getCmdCnt(dupCmd);
			// printf("cmdcnt is %d\n", cmdCnt);
			char* cmdArr[cmdCnt + 1];
			getCmdArr(cmdArr, cmd, cmdCnt + 1);
			// printf("After function\n");
			cmdArr[cmdCnt] = (char*)NULL;
			// printf("After print\n");
			// Use dup2 to redirect output.
			dup2(clientSock, STDOUT_FILENO);
			dup2(clientSock, STDERR_FILENO);
			close(clientSock);
			int eRtn = execvp(*cmdArr, cmdArr);
			if(eRtn)
			{
				perror("execvp()");
				exit(-1);
			}
			// printf("Finish cmd\n");
			// close(clientSock);
			return 0;
		}
		else
		{
			// close clientSock.
			close(clientSock);
			int status;
			// call waitpid to free resources.
			int wRtn = waitpid(-1, &status, WNOHANG);
			// Free all finished processes.
			while(wRtn > 0)
			{
				wRtn = waitpid(-1, &status, WNOHANG);
			}
			if(wRtn < 0)
			{
				perror("waitpid()");
				exit(-1);
			}
			continue;
		}

	}
}
// Convert command to an array.
void getCmdArr(char** cmdArr, char* cmd, int arrSize)
{
	int i = 0;
	char* token;
	token = strtok(cmd, " ");
	cmdArr[i] = strdup(token);
	while(token != NULL)
	{
		i ++;
		token = strtok(NULL, " ");
		if(token == NULL)
		{
			break;
		}
		// cmdArr[i] = strdup(token);
		cmdArr[i] = token;
	}
	return;
}
// Get command count.
int getCmdCnt(char* cmd)
{
	int cmdCnt = 0;
	char* token;
	token = strtok(cmd, " ");
	while(token != NULL)
	{
		cmdCnt ++;
		token = strtok(NULL, " ");
	}
	return cmdCnt;
}
// Get the password corresponding to the given user name.
char* getPwd(char* clientName)
{
	int i;
	for(i = 0; i < USERCNT; i ++)
	{
		if(strcmp(clientName, legalNames[i]) == 0)
		{
			return legalPwd[i];
		}
	}
	return "";
}
// Check whether a given name is legal.
int isNameLegal(char* name)
{
	int i;
	for(i = 0; i < USERCNT; i ++)
	{
		if(strcmp(name, legalNames[i]) == 0)
		{
			return 1;
		}
	}
	return 0;
}
// Check whether the received message contains the terminate token (4).
int isTerminate(char* recvMsg, int size)
{
	int i;
	for(i = 0; i < size; i ++)
	{
		if(recvMsg[i] == 4)
		{
			return i;
		}
	}
	return -1;
}
// Send a message with ETX.
int sendMsg(char* msg, int sock)
{
	char termiStr[2] = {4, '\0'};
	if(write(sock, msg, strlen(msg)) == -1)
	{
		perror("write()");
		exit(-1);
	}
	if(write(sock, termiStr, strlen(termiStr)) == -1)
	{
		perror("write()");
		exit(-1);
	}
	return 0;
}
// Receive a message.
char* recvMsg(int sock)
{
	char msgBuf[BUFSIZE];
	char* msg = "";
	int bytes;
	while(1)
	{
		bytes = read(sock, msgBuf, BUFSIZE);
		if(bytes == 0)
		{
			printf("client disconnected!\n");
			exit(-1);
		}
		else if(bytes < 0)
		{
			perror("read()");
			exit(-1);
		}
		else
		{
			int termLoc = isTerminate(msgBuf, bytes);
			if(termLoc >= 0)
			{
				msgBuf[termLoc] = '\0';
				msg = concat(msg, msgBuf);
				return msg;
			}
			else
			{
				msgBuf[bytes] = '\0';
				msg = concat(msg, msgBuf);
			}
		}
	}	
}
char* concat(char* s1, char* s2)
{
    char* result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
void usage(char** argv)
{
	fprintf(stderr, "usage: %s\n", argv[0]);
}

