/*Author: Mengwen Li (mli2)*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <crypt.h>

#define BUFSIZE 1024
#define PORT 9002

void usage(char** argv);
int isTerminate(char* recvMsg, int size);
char* concat(char* s1, char* s2);
int sendMsg(char* msg, int sock);
char* recvMsg(int sock);
int recvCmdRtn(int sock);

char* salt = "salt";

int main(int argc, char** argv)
{
	unsigned long int inAddr;
	int bytes;
	int sock, servHostPort;
	struct sockaddr_in servAddr;
	char* servHostAddr;
	struct hostent *hp;

	char* username;
	char* pwd;

	if(argc < 5)
	{	
		usage(argv);
	    exit(-1);
	}
	username = argv[2];
	pwd = argv[3];
	char* cmd = "";
	int i;
	for(i = 0; i < argc - 4; i ++)
	{
		cmd = concat(cmd, argv[4 + i]);
		cmd = concat(cmd, " ");
		// printf("command is %s\n", cmd);
	}
	// Connect to server.
	servHostAddr = argv[1];
	servHostPort = PORT;
	bzero((void *)&servAddr, sizeof(servAddr));
	if((hp = gethostbyname(servHostAddr)) == NULL)
	{
		perror("gethostbyname()");
		exit(-1);
	}
	bcopy(hp->h_addr, (char *)&servAddr.sin_addr, hp->h_length);
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(servHostPort);
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket()");
		exit(sock);
	}
	// Connect to server.
	if(connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
	{
		perror("connect()");
		exit(-1);
	}
	// Send user name to server.
	sendMsg(username, sock);
	char* randNum = recvMsg(sock);
	// printf("Received message is %s\n", randNum);
	if(strcmp(randNum, "usernotfound") == 0)
	{
		printf("User name not found on server side.\n");
		close(sock);
		exit(-1);
	}
	// Encryption.
	char* keyStr = concat(pwd, randNum);
	// printf("key string is %s\n", keyStr);
	char* cryptKey = crypt(keyStr, salt);
	// printf("cryptKey is %s\n", cryptKey);
	// Send encrypted password to server.
	sendMsg(cryptKey, sock);
	// Free used space.
	free(randNum);
	free(keyStr);
	char* keyFeedback = recvMsg(sock);
	// printf("feed back message for key is: %s\n", keyFeedback);
	if(strcmp(keyFeedback, "keyerror") == 0)
	{
		free(keyFeedback);
		free(cmd);
		printf("User password does not match server record.\n");
		close(sock);
		exit(-1);
	}
	else
	{
		free(keyFeedback);
		// Send command to server.
		sendMsg(cmd, sock);
		free(cmd);

	}
	// printf("Start receiving: \n");
	recvCmdRtn(sock);
	// printf("Finish receiving!\n");
	close(sock);
}

void usage(char** argv)
{
	fprintf(stderr, "%s - client to try TCP connection to server", argv[0]);
	fprintf(stderr, "usage: %s <host> <username> <password> <command>\n", argv[0]);
	fprintf(stderr, "\t<host>\t- Internet name of server host\n");
}

// Check whether the received message contains the terminate token (4).
int isTerminate(char* recvMsg, int size)
{
	int i;
	for(i = 0; i < size; i ++)
	{
		// printf("recvMsg[%d] is %c\n", i, recvMsg[i]);
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
// Receive the returned string from a command executed by server.
int recvCmdRtn(int sock)
{
	char msgBuf[BUFSIZE];
	// char* msg = "";
	int bytes;
	while(1)
	{
		bytes = read(sock, msgBuf, BUFSIZE);
		if(bytes == 0)
		{
			return 0;
		}
		else if(bytes < 0)
		{
			perror("read()");
			return -1;
		}
		else
		{
			msgBuf[bytes] = '\0';
			printf("%s", msgBuf);
		}
	}
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
			// printf("In recvMsg funciton\n");
			// printf("bytes is %d\n", bytes);
			int termLoc = isTerminate(msgBuf, bytes);
			// printf("termLoc is %d\n", termLoc);
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
			// printf("%s", msg);
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

