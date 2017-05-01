#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include "rf_util.hpp"

#define MYPORT 25341
#define BUFFER_SIZE 1024

int main()
{
    return 0;
}
#if 0
    ///....sockfd
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    ///....sockaddr_in
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MYPORT);                 ///.........
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); ///......ip

    ///..................0........-1
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    one_wire_onoff_s tmp_onoff;
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        switch (sendbuf[0])
        {
        case 'a':
            tmp_onoff.pin = 3;
            form_socket_message(sendbuf, 1, 'G', rf_one_wire_onoff, (char *)(&tmp_onoff));
            break;
        case 'b':
            tmp_onoff.pin = 2;
            tmp_onoff.data = 1;
            form_socket_message(sendbuf, 1, 'P', rf_one_wire_onoff, (char *)(&tmp_onoff));
            break;
        case 'c':
            tmp_onoff.pin = 2;
            tmp_onoff.data = 0;
            form_socket_message(sendbuf, 1, 'P', rf_one_wire_onoff, (char *)(&tmp_onoff));
            break;
        case 'd':
            tmp_onoff.pin = 3;
            tmp_onoff.data = 1;
            form_socket_message(sendbuf, 1, 'S', rf_one_wire_onoff, (char *)(&tmp_onoff));
            break;
        case 'e':
            tmp_onoff.pin = 3;
            tmp_onoff.data = 0;
            form_socket_message(sendbuf, 1, 'S', rf_one_wire_onoff, (char *)(&tmp_onoff));
            break;

        default:
            break;
        }
        send(sock_cli, sendbuf, 32, 0); ///....
        if (strcmp(sendbuf, "exit\n") == 0)
            break;
        int ret = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); ///....
        rf_payload *recvbuf_p = (rf_payload *)recvbuf;
        //one_wire_onoff_s* onoff_p = (one_wire_onoff_s*)(&(recvbuf_p->data));
        printf("got %d byte message form socket\n", ret);
        printf("receive message. protocol is %d, len is %d\n", recvbuf_p->protocol, recvbuf_p->len);
        //printf("int the receive data, pin is %d, data is %d\n", onoff_p->pin, onoff_p->data);
        //fputs(recvbuf, stdout);

        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
        memset(&tmp_onoff, 0, sizeof(one_wire_onoff_s));
    }

    close(sock_cli);
    return 0;
}
#endif