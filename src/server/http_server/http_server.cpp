#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include "rf_util.hpp"

#define MYPORT 25341
#define BUFFER_SIZE 1024
// the format is /x/x/x/x
void parse_url(char *url, int &nodeID, int &pin, int &protocol, int &get_set, int &data)
{
    char *tmp_url = (char *)url + 1;
    char *ret = strstr(tmp_url, "/");

    if (ret)
    {
        *ret = 0;
    }
    else
    {
        printf("ret is %p\n", ret);
        printf("invalid pointer!");
        return;
    }

    nodeID = atoi(tmp_url);
    printf("nodeID = %d\n", nodeID);

    tmp_url = ret + 1;
    ret = strstr(tmp_url, "/");

    if (ret)
    {
        *ret = 0;
    }
    else
    {
        printf("ret is %p\n", ret);
        printf("invalid pointer!");
        return;
    }

    pin = atoi(tmp_url);
    printf("pin = %d\n", pin);

    tmp_url = ret + 1;
    ret = strstr(tmp_url, "/");
    if (ret)
    {
        *ret = 0;
    }
    else
    {
        printf("ret is %p\n", ret);
        printf("invalid pointer!");
        return;
    }

    protocol = atoi(tmp_url);
    printf("protocol = %d\n", protocol);

    tmp_url = ret + 1;
    ret = strstr(tmp_url, "/");
    if (ret)
    {
        *ret = 0;
    }
    else
    {
        printf("ret is %p\n", ret);
        printf("invalid pointer!");
        return;
    }

    get_set = atoi(tmp_url);
    printf("get_set = %d\n", get_set);

    tmp_url = ret + 1;
    printf("tmp_url is %s\n", tmp_url);
    data = atoi(tmp_url);
    printf("data = %d\n", data);
    return;
}
char get_message_type(int type)
{
    switch (type)
    {
    case 0:
        return 'G';
    case 1:
        return 'P';
    case 2:
        return 'S';
    default:
        //need log here
        //printf("error, wrong message type form outside");
        return 0;
    }
}
void send_message(int nodeID, int pin, int protocol, int get_set, int data, int &out_data)
{
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    ///....sockaddr_in
    struct timeval timeout = {3, 0}; // send/recv timeout
    int ret = setsockopt(sock_cli, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
    // need log here
    ret = setsockopt(sock_cli, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
    // need log here
    if (!ret)
    {
        printf("setsockopt return fail!\n");
        //return;
    }
    


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
    // to do, now noly support one protocol
    one_wire_onoff_s tmp_onoff;
    tmp_onoff.pin = pin;
    tmp_onoff.data = data;
    char tmp_type = get_message_type(get_set);
    if (tmp_type)
    {
        form_socket_message(sendbuf, nodeID, tmp_type, protocol, (char *)(&tmp_onoff));
    }
    else
    {
        // need log here
        return;
    }

    send(sock_cli, sendbuf, 32, 0); ///....

    ret = recv(sock_cli, recvbuf, sizeof(recvbuf), 0);

    rf_payload *recvbuf_p = (rf_payload *)recvbuf;

    one_wire_onoff_s *tmp_onoff_p = (one_wire_onoff_s *)(&(recvbuf_p->data));

    out_data = tmp_onoff_p->data;
    printf("Max:::got data %d\n", out_data);

    close(sock_cli);
}

void generic_cb(struct evhttp_request *request, void *arg)
{
    const struct evhttp_uri *evhttp_uri = evhttp_request_get_evhttp_uri(request);
    char url[8192];
    evhttp_uri_join(const_cast<struct evhttp_uri *>(evhttp_uri), url, 8192);

    printf("accept request url:%s\n", url);

    int nodeID;
    int pin;
    int protocol;
    int get_set;
    int data;
    int recv_data;

    parse_url(url, nodeID, pin, protocol, get_set, data);
    send_message(nodeID, pin, protocol, get_set, data, recv_data);

    // need to form html data here#################################################################

    struct evbuffer *evbuf = evbuffer_new();
    if (!evbuf)
    {
        printf("create evbuffer failed!\n");
        return;
    }

    evbuffer_add_printf(evbuf, "Server response. Your request url is %d", recv_data);
    evhttp_send_reply(request, HTTP_OK, "OK", evbuf);
    evbuffer_free(evbuf);
}

void test_cb(struct evhttp_request *request, void *arg)
{
}

int main()
{
    short http_port = 8081;
    std::string http_addr("135.252.143.76");
    

    struct event_base *base = event_base_new();
    struct evhttp *http_server = evhttp_new(base);
    if (NULL == http_server)
    {
        return -1;
    }

    int ret = evhttp_bind_socket(http_server, http_addr.c_str(), http_port);
    if (ret != 0)
    {
        return -1;
    }

    evhttp_set_cb(http_server, "/test", test_cb, NULL);
    evhttp_set_gencb(http_server, generic_cb, NULL);
    printf("http server start OK!\n");
    event_base_dispatch(base);
    evhttp_free(http_server);

    return 0;
}
