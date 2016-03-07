/*
 * Mark Claypool
 * WPI
 * copyright 2002
 * 
 * Basic multicast socket wrappers.
 */

#include "msock.h"

#define NUM_SOCK 8		/* maximum sockets wrapers supports */

static struct sockaddr_in adr[NUM_SOCK];
static int is_valid[NUM_SOCK] ={0,0,0,0,0,0,0,0};

/* msockcreate -- Create socket from which to read.
   return socket descriptor if ok, -1 if not ok.  */
int msockcreate(int type, char *address, int port) {
  int sock;
  int ret, on=1;
  struct ip_mreq mreq;

  /* set up socket */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(1);
  }

  bzero((char *)&adr[sock], sizeof(adr[sock]));
  adr[sock].sin_family = AF_INET;
  adr[sock].sin_addr.s_addr = htonl(INADDR_ANY);
  adr[sock].sin_port = htons(port);
  if (type == SEND) {

    adr[sock].sin_addr.s_addr = inet_addr(address);
    is_valid[sock] = 1;

  } else {

    if ((ret=setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 
                   &on, sizeof(on))) < 0) {
      perror("setsockopt reuseaddr");
      return ret;
    }

    if ((ret=bind(sock, (struct sockaddr *)&adr[sock], 
		  sizeof(adr[sock]))) < 0) {        
      perror("bind");
      return ret;
    }
    
    mreq.imr_multiaddr.s_addr = inet_addr(address);         
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);         
    if ((ret=setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			&mreq, sizeof(mreq))) < 0) {
      perror("setsockopt mreq");
      return ret;
    }         
    
    is_valid[sock] = 1;
  }

  return sock;
}

/* msockdestroy -- Destroy socket by closing.
   return socket descriptor if ok, -1 if not ok.  */
int msockdestroy(int sock) {

  if (!is_valid[sock]) {
    fprintf(stderr, "sock %d not valid\n", sock);
    return -1;
  }

  is_valid[sock] = 0;
  return (close(sock));
}

/* msend -- send multicast essage to given address. 
   return number of bytes sent, -1 if error. */
int msend(int sock, char *message, int len) {

  int addrlen;
  
  if (!is_valid[sock]) {
    fprintf(stderr, "sock %d not valid\n", sock);
    return -1;
  }

  addrlen = sizeof(adr[sock]);
  return (sendto(sock, message, len, 0,
		 (struct sockaddr *) &adr[sock], addrlen));
}

/* mrecv -- receive message on given mcast address. Will block.
   return bytes received, -1 if error. */
int mrecv(int sock, char *message, int max_len) {
  int addrlen;

  if (!is_valid[sock]) {
    fprintf(stderr, "sock %d not valid\n", sock);
    return -1;
  }
  addrlen = sizeof(adr[sock]);
  
  return (recvfrom(sock, message, max_len, 0, 
		   (struct sockaddr *) &adr[sock], &addrlen));
}
