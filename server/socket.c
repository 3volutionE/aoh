#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define PORT 33333
#define SERVER "127.0.0.1"

int sock_fd;
int sizeof_clientaddr;
struct sockaddr_in cliaddr;

int sock_init(){
	printf ("--- Setup GPIO\n");

	//uint32_t tx_buff[] = {1,2,3,4,11,12,13,14,21,22,23,24,31,32,33,34,41,42,43,44,51,52,53,54,61,62,63,64};

    struct sockaddr_in servaddr; //, cliaddr;
    //struct sockaddr_in si_me, si_other, si;

	//char message[] = "Test sending data to ui-app";
	char message[] = "Hello Message to ui-app socket";

    //if ((sock_fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
	if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		printf("Error socket create\n");
		return -1;
	}

    sizeof_clientaddr = sizeof(cliaddr);
	memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof_clientaddr);
	   
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

	//bind socket to port
	if( bind(sock_fd , (struct sockaddr*)&servaddr, sizeof(servaddr) ) == -1)
	{
		printf("error binding\n");
	}
	return 0;
}


int socket_wait_client()
{
	// Wait Client to send hello message
	int byte_recv;
	char buff[100];
	byte_recv = recvfrom(sock_fd, (char *)buff, 1000, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &sizeof_clientaddr);
	return byte_recv;
}

int socket_receive(char *buff)
{
	// Wait Client to send hello message
	int byte_recv;
	//char buff[100];

	byte_recv = recvfrom(sock_fd, (char *)buff, 1000, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &sizeof_clientaddr);
	//printf("Receive from udp %d bytes\n", byte_recv);
	*(buff+byte_recv) = '\0';
	return byte_recv;
}

int socket_send(unsigned char *tx_buff, unsigned long nbyte)
{
	sendto(sock_fd, (const char *)tx_buff, nbyte, MSG_CONFIRM, (const struct sockaddr *) &cliaddr, sizeof_clientaddr);
}

