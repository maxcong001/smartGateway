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
#include <iostream>
#include "rf_util.hpp"

#include "loop.h"
#include "tcpClient.h"
#include "timer.h"

#include <ncurses.h>
#include "RF24Mesh.h"
#include <RF24.h>
#include <RF24Network.h>

#include <log4cplus/logger.h>
//#include <log4cplus/consoleappender.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
//#include <log4cplus/ndc.h>
//#include <log4cplus/mdc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/thread/threads.h>
//#include <log4cplus/helpers/sleep.h>
#include <log4cplus/loggingmacros.h>


using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("RF24_client_test: "));


class RF24TcpClient : public translib::TcpClient
{
public:
	RF24TcpClient(const translib::Loop &loop):
		translib::TcpClient(loop)
	{}

protected:
	virtual void onRead()
	{
	}

	virtual void onDisconnected()
	{
		cout << "RF24TcpClient::" << __FUNCTION__ << endl;
	}

	virtual void onConnected(int error)
	{
		cout << "RF24TcpClient::" << __FUNCTION__ << endl;
#if 0
void form_rf_payload(char *buf, rf24_protocol _protocol, protocol_detail _protocol_detail);
void form_socket_message_from_rf_payload(char *buf, char *rf_payload, uint16_t nodeID, char type);
void form_socket_message(char *buf, uint16_t nodeID, char type, rf24_protocol _protocol, protocol_detail _protocol_detail);
#endif  

        for (uint16_t nodeID = 0; nodeID < 10; nodeID++)
        {
            char buf[32] = {0};
            pin_detail pin_det= {1,1};
            protocol_detail proto_detail;
            memcpy(&(proto_detail._pin), &pin_det, sizeof(pin_detail));
            protocol_detail* tmp_proto_p = &proto_detail;
            form_socket_message(buf, nodeID, 65, rf24_protocol_pin, tmp_proto_p); 
		    send(buf, sizeof(socket_message));
            cout << "send to Node "<<nodeID << endl;
        }

	}
};

class RF24TcpClientManager : public translib::Loop
{
public:
	RF24TcpClientManager():
		_client(*this),
		_timer(*this)
	{
		_timer.startForever(1000, std::bind(&RF24TcpClientManager::tick, this));
	}

protected:
	void tick()
	{
		cout << "round " << _timer.curRound() << endl;
		if (!_client.isConnected())
		{
			_client.connect(RF24_SERVER_IP, RF24_SERVER_PORT);
		}

		if (_timer.curRound() >= 30)
		{
			stop();
		}
	}
private:
	RF24TcpClient _client;
	translib::Timer _timer;
};


int main()
{
    // init log
    log4cplus::initialize();
    try
    {
        SharedObjectPtr<Appender> append_1(new FileAppender("RF24_client_test.log"));
        append_1->setName(LOG4CPLUS_TEXT("First"));

        log4cplus::tstring pattern = LOG4CPLUS_TEXT("[%d{%m/%d/%y %H:%M:%S,%Q}] %c %-5p - %m [%l]%n");
        //  std::tstring pattern = LOG4CPLUS_TEXT("%d{%c} [%t] %-5p [%.15c{3}] %%%x%% - %m [%l]%n");
        append_1->setLayout(std::auto_ptr<Layout>(new PatternLayout(pattern)));
        Logger::getRoot().addAppender(append_1);

        logger.setLogLevel(DEBUG_LOG_LEVEL);
    }
    catch (...)
    {
        Logger::getRoot().log(FATAL_LOG_LEVEL, LOG4CPLUS_TEXT("Exception occured..."));
    }

    LOG4CPLUS_DEBUG(logger, "set logger done!");

	RF24TcpClientManager clientManager;
	clientManager.start(false);
	clientManager.wait();

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