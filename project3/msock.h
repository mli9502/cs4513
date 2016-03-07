/*
 * Mark Claypool
 * WPI
 * copyright 2002
 * 
 * Basic multicast socket wrappers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SEND 1
#define RECV 2

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff /* should be in <netinet/in.h> */
#endif

/* msockcreate -- Create socket from which to read.
   return socket descriptor if ok, -1 if not ok.  */
int msockcreate(int type, char *address, int port);

/* msockdestroy -- Destroy socket by closing.
   return socket descriptor if ok, -1 if not ok.  */
int msockdestroy(int sock);

/* msend -- send multicast essage to given address. 
   return number of bytes sent, -1 if error. */
int msend(int sock, char *message, int len);

/* mrecv -- receive message on given mcast address. Will block.
   return bytes received, -1 if error. */
int mrecv(int sock, char *message, int max_len);









