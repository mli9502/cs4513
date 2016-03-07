/*
	Author: Mengwen Li (mli2)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "msock.h"
#include "linkedList.h"

#define QIP "239.0.0.1"
#define QPORT 9000
#define RIP "239.0.0.2"
#define RPORT 9001

#define FREQSEC 0
#define FREQNSEC 100000000

int gpid = 0;

char* getMovieName(char* fileName);
void getMovieList(pNode* head, pNode* tail, char* dirLoc);
int existsMovie(pNode head, char* movie);
void initArr(char* arr, int size);
int strToInt(char* str);
char* consResMsg(char* movie, char* port);
char* concat(char* s1, char* s2);
void getResArr(char** resArr, char* res, int arrSize);
int sendMsg(char* msg, int sock);
char* recvMsg(int sock, int, int);
int isTerminate(char* recvMsg, int size);
int recvFrame(int sock);
int sendFrames(char* file, int sock, int, int, int);
void usage();
// Handle SIGINT (ctrl c).
void sig_handler(int sig)
{
	if(sig == SIGINT)
	{
		printf("received sigint! %d\n", gpid);

		if(gpid != 0)
		{
			kill(gpid, SIGKILL);
			exit(-1);
		}
	}
}

int main(int argc, char** argv)
{
	// usage: ./nutella -p <port number> -d <directory containing movies>
	// Movies in the directory should end with ".txt".
	int c;
	extern int optind, opterr;
	extern char* optarg;
	int errFlg = 0;
	// Default location for movies is in "./nutMovies".
	char* dirLoc = "nutMovies";
	// Default port number.
	char* portStr = "9002";
	int tcpPort;
	// Parse command line argument to get port for TCP and movie directory.
	while((c = getopt(argc, argv, "p:d:")) != -1)
	{
		switch(c)
		{
			case 'p':
				portStr = optarg;
				break;
			case 'd':
				dirLoc = optarg;
				break;
			default:
			{
				usage();
				exit(-1);
			}	
		}
	}
	if(optind < argc)
	{
		printf("Too many command line args.\n");
		usage();
		exit(-1);
	}
	// Convert the input port string to int.
	tcpPort = strToInt(portStr);
	// Initialize a linked list here to store all the files.
	pNode head, tail;
	// Get the movie list into a linked list with header node "head".
	getMovieList(&head, &tail, dirLoc);
	// Create process for client and server functionality.
	int pid = fork();
	// Child process, client functionality.
	if(pid == 0)
	{	
		// Create multicast socket to send query and receive response.
		int qsockSend = msockcreate(SEND, QIP, QPORT);
		int rsockRecv = msockcreate(RECV, RIP, RPORT);
		// Set rsockRecv to non-blocking socket.
		long on = 1L;
		// Set this socket to non-blocking to receive incoming frames.
		if(ioctl(rsockRecv, (int)FIONBIO, (char*)&on))
		{
			perror("ioctl()");
			exit(-1);
		}
		// Loop to readin movie name.
		while(1)
		{
			// Host name and port for client to contact.
			char* contHostname = NULL;
			int contPort = 0;
			// int rFlg = 0;
			char mvName[1024];
			char msg[1024];
			printf("Enter a movie (-r for repeatly play): ");
			fgets(mvName, 1024, stdin);
			mvName[strlen(mvName) - 1] = '\0';
			// Flush the socket before sending.
			int tmpCnt = mrecv(rsockRecv, msg, 1024);
			while(tmpCnt != -1)
			{
				tmpCnt = mrecv(rsockRecv, msg, 1024);
			}
			// Flush the socket. Clear what is left before.
			int cnt = msend(qsockSend, mvName, strlen(mvName) + 1);
			if(cnt != strlen(mvName) + 1)
			{
				exit(-1);
			}
			// Wait for 5 seconds if not getting the correct response.
			time_t start = time(NULL);
			while(1)
			{
				memset(msg, 0, 1024);
				if(time(NULL) > start + 5)
				{
					break;
				}
				cnt = mrecv(rsockRecv, msg, 1024);
				if(cnt > 0)
				{
					char* resMsg[3];
					getResArr(resMsg, msg, 3);
					char* newName;
					// Remove -r flag.
					if(strstr(mvName, " -r") != NULL)
					{
						mvName[strlen(mvName) - 3] = '\0';
						newName = mvName;
					}
					else
					{
						newName = mvName;
					}
					if(strcmp(resMsg[0], newName) == 0)
					{
						contHostname = resMsg[1];
						contPort = strToInt(resMsg[2]);
						break;
					}
				}
			}
			// If contacting hostname is NULL, movie is not found.
			if(contHostname == NULL)
			{
				printf("Movie not found.\n");
				continue;
			}
			// Set address and connect to the given server.
			struct sockaddr_in contAddr;
			struct hostent *hp;
			bzero((void *)&contAddr, sizeof(contAddr));
			if((hp = gethostbyname(contHostname)) == NULL)
			{
				perror("gethostbyname()");
				exit(-1);
			}
			bcopy(hp->h_addr, (char *)&contAddr.sin_addr, hp->h_length);
			contAddr.sin_family = AF_INET;
			contAddr.sin_port = htons(contPort);
			int contSock = socket(AF_INET, SOCK_STREAM, 0);
			if(contSock < 0)
			{
				perror("socket()");
				exit(-1);
			}
			// Connect to server.
			if(connect(contSock, (struct sockaddr *)&contAddr, sizeof(contAddr)) < 0)
			{
				perror("connect()");
				exit(-1);
			}
			// Send request message.
			sendMsg("transfer", contSock);
			long on = 1L;
			// Set this socket to non-blocking to receive incoming frames.
			if(ioctl(contSock, (int)FIONBIO, (char*)&on))
			{
				perror("ioctl()");
				exit(-1);
			}
			while(1)
			{
				if(recvFrame(contSock) == -2)
				{
					break;
				}
			}
			close(contSock);
		}	
		return 0;
	}
	else
	{
		// Set the global pid variable.
		// This is used to kill the child process when exception happens.
		gpid = pid;
		// Set signal handler to listen to SIGINT (ctrl-c).
		if(signal(SIGINT, sig_handler) == SIG_ERR)
		{
			printf("can't catch!\n");
			kill(pid, SIGKILL);
			exit(-1);
		}
		// Construct query socket and response UDP multicast socket.
		char msg[1024];
		int qsockRecv = msockcreate(RECV, QIP, QPORT);
		int rsockSend = msockcreate(SEND, RIP, RPORT);
		// Construct TCP socket for streaming movie,
		// bind the socket to address and listen.
		struct sockaddr_in servAddr;
		int servHostPort = tcpPort;
		int sock = socket(AF_INET, SOCK_STREAM, 0);
		if(sock < 0)
		{
			perror("socket()");
			kill(pid, SIGKILL);
			exit(-1);
		}
		bzero((char*)&servAddr, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servAddr.sin_port = htons(servHostPort);
		if(bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
		{
			perror("bind()");
			kill(pid, SIGKILL);
			exit(-1);
		}
		if(listen(sock, 5))
		{
			perror("listen()");
			kill(pid, SIGKILL);
			exit(-1);
		}
		// Set sock and qsockRecv to non-blocking socket in order to loop between them.
		long on = 1L;
		// Set this socket to non-blocking to receive incoming frames.
		if(ioctl(sock, (int)FIONBIO, (char*)&on))
		{
			perror("ioctl()");
			exit(-1);
		}
		if(ioctl(qsockRecv, (int)FIONBIO, (char*)&on))
		{
			perror("ioctl()");
			exit(-1);
		}
		// Replay flag. Used to replay video.
		int rFlg = 0;
		while(1)
		{
			// Collect finished child process.
			int status;
			waitpid(-1, &status, WNOHANG);
			char movie[1024];
			struct sockaddr_in clientAddr;
			int cliLen = sizeof(clientAddr);
			int clientSock = accept(sock, (struct sockaddr *)&clientAddr, &cliLen);
			if(clientSock < 0 && (errno != EAGAIN || errno != EWOULDBLOCK))
			{
				perror("accept()");
				kill(pid, SIGKILL);
				exit(-1);
			}
			// If client established a unicast connection.
			if(clientSock > 0)
			{
				// Receive starting message.
				char* tmpMsg = recvMsg(clientSock, pid, 1);
				// If starting message not correct.
				if(strcmp(tmpMsg, "transfer") != 0)
				{
					continue;
				}
				// Open the movie file.
				char* movieFile = concat(movie, ".txt");
				char* tmpDir = concat(dirLoc, "/");
				movieFile = concat(tmpDir, movieFile);
				free(tmpDir);
				// Send frames to client.
				sendFrames(movieFile, clientSock, pid, 1, rFlg);
			}
			// Check multicast socket.
			int cnt = mrecv(qsockRecv, msg, 1024);
			if(cnt <= 0 && (errno != EAGAIN || errno != EWOULDBLOCK))
			{
				perror("recvfrom()");
				kill(pid, SIGKILL);
				exit(-1);
			}
			if(cnt > 0 && strlen(msg) > 0)
			{
				// Check for -r argument.
				if(strstr(msg, " -r") != NULL)
				{
					rFlg = 1;
					msg[strlen(msg) - 3] = '\0';
				}
				else if(strlen(msg) > 0)
				{
					rFlg = 0;
				}
				if(existsMovie(head, msg))
				{
					// Movie name backup.
					strcpy(movie, msg);
					char* resMsg;
					// Construct the response message from movie name, host name and port number.
					resMsg = consResMsg(msg, portStr);
					// Multicast the response message back.
					cnt = msend(rsockSend, resMsg, strlen(resMsg) + 1);
					if(cnt < 0)
					{
						perror("msend()");
						kill(pid, SIGKILL);
						exit(-1);
					}
				}
			}
		}
		return 0;
	}
	
}
/* Reference: man7.org/linux/man-pages/man3/strtol.3.html */
/* Convert a string to integer. */
int strToInt(char* str)
{
	long val;
	char* endPtr;
	val = strtol(str, &endPtr, 10);
	if((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || 
		(errno != 0 && val == 0))
	{
		perror("strtol()");
		exit(-1);
	}
	if(endPtr == str)
	{
		printf("No digits found.\n");
		exit(-1);
	}
	if(*endPtr != '\0')
	{
		printf("Not a number.\n");
		exit(-1);
	}
	return (int)val;
}
/* Construct response array from response message. */
void getResArr(char** resArr, char* res, int arrSize)
{
	int i = 0;
	char* token;
	token = strtok(res, "`");
	resArr[i] = token;
	while(token != NULL)
	{
		i ++;
		token = strtok(NULL, "`");
		if(token == NULL)
		{
			break;
		}
		resArr[i] = token;
	}
	return;
}
/* Function used to check whether a movie exists. */
int existsMovie(pNode head, char* movie)
{
	while(head != NULL)
	{
		if(strcmp(head->name, movie) == 0)
		{
			return 1;
		}
		head = head->next;
	}
	return 0;
}
/* Function used to get movie names from the directory into a linked list. */
void getMovieList(pNode* head, pNode* tail, char* dirLoc)
{
	initList(head, tail);
	DIR* dp;
	struct dirent* d;
	dp = opendir(dirLoc);
	if(dp == NULL)
	{
		// perror("Can't open directory: %s\n", dirLoc);
		printf("Can't open directory: %s\n", dirLoc);
		exit(-1);
	}
	d = readdir(dp);
	while(d)
	{
		// Check wheter the name contains ".txt".
		if(strstr(d->d_name, ".txt") != NULL)
		{
			printf("movie is %s\n", d->d_name);
			addNode(head, tail, getMovieName(d->d_name));
		}
		d = readdir(dp);
	}
	travList(*head);
}
/* Get movie name from file name by removeing .txt. */
char* getMovieName(char* fileName)
{
	char* rtn = (char*)malloc(strlen(fileName) * sizeof(char));
	int i;
	for(i = 0; i < strlen(fileName) - 4; i ++)
	{
		rtn[i] = fileName[i];
	}
	rtn[i] = '\0';
	return rtn;
}
/* Construct response message for client query. */
char* consResMsg(char* movie, char* port)
{
	char hostname[128];
	int gRtn = gethostname(hostname, 128);
	if(gRtn)
	{
		perror("gethostname()");
		exit(gRtn);
	}
	char* rtn;
	rtn = concat(movie, "`");
	rtn = concat(rtn, hostname);
	rtn = concat(rtn, "`");
	rtn = concat(rtn, port);
	return rtn;
}
/* Concatinate two strings. */
char* concat(char* s1, char* s2)
{
    char* result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
/* Send a frame to client */
int sendFrame(char* frame, int sock)
{
	if(write(sock, frame, strlen(frame)) == -1)
	{
		perror("write()");
		if(gpid != 0)
		{
			kill(gpid, SIGKILL);
		}
		exit(-1);
	}
	return 0;
}
/* Read file and send frams to client. If exception happens, kill child process. */
int sendFrames(char* file, int sock, int pid, int isParent, int rFlg)
{
	FILE* fp;
	int read;
	char* line = NULL;
	char* frame = "";
	size_t len = 0;
	
	if(rFlg)
	{
		while(1)
		{
			fp = fopen(file, "r");
			if(fp == NULL)
			{
				perror("fopen()");
				return -1;
			}
			while((read = getline(&line, &len, fp)) != -1)
			{
				if(strcmp(line, "end\n") != 0)
				{
					frame = concat(frame, line);
				}
				else
				{
					// Send frame to client.
					sendFrame(frame, sock);
					sendFrame("frameend", sock);
					frame = "";
					// Sleep.
					struct timespec ts, fTs;
					ts.tv_sec = FREQSEC;
					ts.tv_nsec = FREQNSEC;
					if(nanosleep(&ts, &fTs) < 0)
					{
						perror("nanosleep()");
						if(isParent)
						{
							kill(pid, SIGKILL);
						}
						exit(-1);
					}
				}
			}
			fclose(fp);
		}
	}
	else
	{
		fp = fopen(file, "r");
		if(fp == NULL)
		{
			perror("fopen()");
			return -1;
		}
		// Read in lines from file.
		while((read = getline(&line, &len, fp)) != -1)
		{
			// If it is in one frame, concat them together.
			if(strcmp(line, "end\n") != 0)
			{
				frame = concat(frame, line);
			}
			else
			{
				// Send frame to client.
				sendFrame(frame, sock);
				sendFrame("frameend", sock);
				frame = "";
				// Sleep.
				struct timespec ts, fTs;
				ts.tv_sec = FREQSEC;
				ts.tv_nsec = FREQNSEC;
				if(nanosleep(&ts, &fTs) < 0)
				{
					perror("nanosleep()");
					if(isParent)
					{
						kill(pid, SIGKILL);
					}
					exit(-1);
				}
			}
		}
		// Close file.
		fclose(fp);
		// Send finishing mark.
		sendFrame("finishsending", sock);
		return 0;
	}
	
}
/* Receive frame from server. */
int recvFrame(int sock)
{
	char msgBuf[1024];
	char* frame = "";
	char* msg = "";
	int bytes;
	int clrFlg = 1;
	while(1)
	{
		// Read 1024 bytes at a time.
		bytes = read(sock, msgBuf, 1024);
		if(bytes == 0)
		{
			printf("client disconnected!\n");
			exit(-1);
		}
		else if(bytes < 0)
		{
			// If timeout, return -1.
			if(errno == EWOULDBLOCK)
			{
				continue;
			}
			else
			{
				perror("read()");
				exit(-1);
			}	
		}
		else
		{
			// If clear flag, clear the screen.
			if(clrFlg)
			{
				printf("\033[2J");
				printf("\033[0;0f");
				clrFlg = 0;
			}
			msgBuf[bytes] = '\0';
			char* sRtn;
			if((sRtn = strstr(msgBuf, "frameend")) != NULL && 
				strstr(msgBuf, "finishsending") == NULL)
			{
				msgBuf[sRtn - msgBuf] = '\0';
				frame = concat(frame, msgBuf);
				printf("%s", frame);
				frame = "";
				clrFlg = 1;
			}
			else if((sRtn = strstr(msgBuf, "finishsending")) != NULL)
			{
				msgBuf[sRtn - msgBuf] = '\0';
				frame = concat(frame, msgBuf);
				printf("%s", frame);
				return -2;
			}
			else
			{
				frame = concat(frame, msgBuf);
			}	
			int i = 0;
			for(i = 0; i < 1024; i ++)
			{
				msgBuf[i] = 0;
			}
		}
	}
}
/* Send a message with ETX. */
int sendMsg(char* msg, int sock)
{
	char termiStr[2] = {4, '\0'};
	if(write(sock, msg, strlen(msg)) == -1)
	{
		perror("write()");
		if(gpid != 0)
		{
			kill(gpid, SIGKILL);
		}
		exit(-1);
	}
	if(write(sock, termiStr, strlen(termiStr)) == -1)
	{
		perror("write()");
		if(gpid != 0)
		{
			kill(gpid, SIGKILL);
		}
		exit(-1);
	}
	return 0;
}
/* Receive message from socket. */
char* recvMsg(int sock, int pid, int isParent)
{
	char msgBuf[1024];
	char* msg = "";
	int bytes;
	while(1)
	{
		bytes = read(sock, msgBuf, 1024);
		if(bytes == 0)
		{
			printf("client disconnected!\n");
			if(isParent)
			{
				kill(pid, SIGKILL);
			}
			exit(-1);
		}
		else if(bytes < 0)
		{
			perror("read()");
			if(isParent)
			{
				kill(pid, SIGKILL);
			}
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

void usage()
{
	printf("usage: ./nutella -d <directory containing movies> -p <port number for streaming movie>\n");
	printf("\t\t-d: optional, default directory is ./nutMovie\n");
	printf("\t\t-p: optional, default port is 9002\n");
	return;
}

// int getTokenCnt(char* arr)
// {
// 	int tCnt = 0;
// 	char termiStr[2] = {4, '\0'};
// 	char* token;
// 	token = strtok(arr, termiStr);
// 	while(token != NULL)
// 	{
// 		tCnt ++;
// 		token = strtok(NULL, termiStr);
// 	}
// 	return tCnt;
// }

// void getTokenArr(char** resArr, char* res, int arrSize)
// {
// 	// printf("!!!!!!!!!!!!!!!!!!!!!!!!!1res is %s\n", res);
// 	int i = 0;
// 	char* token;
// 	char termiStr[2] = {4, '\0'};
// 	token = strtok(res, termiStr);
// 	// printf("token is %s\n", token);
// 	resArr[i] = token;
// 	while(token != NULL)
// 	{
// 		i ++;
// 		token = strtok(NULL, termiStr);
// 		// printf("token is %s\n", token);
// 		if(token == NULL)
// 		{
// 			break;
// 		}
// 		resArr[i] = token;
// 	}
// 	return;
// }



// Receive the returned string from a command executed by server.
// int recvFrame(int sock)
// {
// 	char msgBuf[1024];
// 	char* msg = "";
// 	int bytes;
// 	while(1)
// 	{
// 		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
// 		bytes = read(sock, msgBuf, 1024);
// 		printf("bytes is %d\n", bytes);
// 		if(bytes == 0)
// 		{
// 			printf("client disconnected!\n");
// 			exit(-1);
// 		}
// 		else if(bytes < 0)
// 		{
// 			perror("read()");
// 			exit(-1);
// 		}
// 		else
// 		{
// 			char* firstHalf = 
// 			msgBuf[bytes] = '\0';
// 			// printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ %c\n", msgBuf[bytes - 1]);
// 			// using strtok to split msgBuf.
// 			printf("msgBuf is \n");
// 			printf("%s\n", msgBuf);
// 			char* dupMsg = strdup(msgBuf);
// 			char* dupArrMsg = strdup(msgBuf);
// 			int frameCnt = getTokenCnt(dupMsg);
// 			// printf("frame cnt is %d\n", frameCnt);
// 			char* tokenArr[frameCnt + 1];
// 			getTokenArr(tokenArr, dupArrMsg, frameCnt + 1);
// 			int i;
// 			printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~BEFORE FOR LOOP\n");
// 			for(i = 0; i < frameCnt; i ++)
// 			{
// 				printf("%s\n", tokenArr[i]);
// 			}
// 			printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~After FOR LOOP\n");
// 			if(msgBuf[bytes - 1] != 4)
// 			{
// 				// Hold this frame to connect with next part on next read.

// 				printf("+++++++++++++++++++++++++ Last frame not complete\n");
// 			}
// 			else
// 			{
// 				printf("+++++++++++++++++++++++++ Last frame complete\n");
// 			}
// 			// printf("In recvMsg funciton\n");
// 			// printf("bytes is %d\n", bytes);
// 			// int termLoc = isTerminate(msgBuf, bytes);
// 			// // printf("termLoc is %d\n", termLoc);
// 			// if(termLoc >= 0)
// 			// {
// 			// 	msgBuf[termLoc] = '\0';
// 			// 	printf("%s", msgBuf);
// 			// 	return 0;
// 			// }
// 			// else
// 			// {
// 			// 	msgBuf[bytes] = '\0';
// 			// 	printf("%s", msgBuf);
// 			// }
// 			// printf("%s", msg);
// 		}
// 	}
// }
