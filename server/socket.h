

#ifndef __SOCKET_H__
#define __SOCKET_H__

    #define PORT 33333
    #define SERVER "127.0.0.1"
    
    int sock_init();
    int socket_wait_client(); //char *buff);
    int socket_send(unsigned char *tx_buff, unsigned long len);
    int socket_receive(char *buff);

#endif
