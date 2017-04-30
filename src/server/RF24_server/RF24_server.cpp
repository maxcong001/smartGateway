#include <ncurses.h>
#include "RF24Mesh.h"
#include <RF24.h>
#include <RF24Network.h>
#include <iostream>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
extern "C" {
#include <event.h>
//#include <sys/socket.h>
}

#include "rf_util.hpp"

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



#include <functional>

#include "loop.h"
#include "tcpServer.h"

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;
Logger logger = Logger::getInstance(LOG4CPLUS_TEXT("RF24_server: "));

RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

void printNodes(uint8_t boldID);
void pingNode(uint8_t listNo);
void *start_monitor(void *);

uint8_t nodeCounter;

uint16_t failID = 0;

int node_sockt_fd[256];

#define RF24_SERVER_PORT 25341
#define RF24_SERVER_IP "127.0.0.1"


//void push_frame_queue(uint8_t nodeID, uint16_t type, char *message, uint16_t len)
void push_frame_queue(uint16_t nodeID, char type, char *message)
{
    int16_t address_id;
    address_id = mesh.getAddress(nodeID);
    RF24NetworkHeader rf_header = RF24NetworkHeader(address_id, type);
    RF24NetworkFrame frame;

    frame.message_size = sizeof(rf24_msg);
    if (frame.message_size > 22 )
    {
        LOG4CPLUS_WARN(logger, " message size is larger than 22!");
        frame.message_size = 22;
    }
    memcpy(&frame, &rf_header, sizeof(RF24NetworkHeader));
    memcpy((frame.message_buffer), message, frame.message_size);
    network.frame_queue.push(frame);
}

class RF24_server : public translib::TcpServer
{
public:
	static RF24_server & instance()
	{
		static RF24_server ins;
		return ins;
	}
protected:
    virtual void onListenError()
    {
        LOG4CPLUS_ERROR(logger, "listen error!");
    }

	virtual void onSessionRead(translib::TcpSession *session)
	{
		uint32_t length = session->getInputBufferLength();
		char * buff = new char[length + 1];
		session->readInputBuffer((uint8_t *)buff, length);
		buff[length] = '\0';
        LOG4CPLUS_DEBUG(logger, "got message " << buff << ". Session id is "<< session->id());

        socket_message *payload_p = (rf24_msg *)buff;
        if ((payload_p->magic_num) != MAGIC_NUM)
        {
            LOG4CPLUS_WARN(logger, "magic num miss match! Got invalid message!");
            string ret_msg("magic num miss match! You sent invalid message!");
            session->send(ret_msg.c_str(), ret_msg.size());
            return;
        }
        push_frame_queue(payload_p->nodeID, payload_p->type, &(payload_p->msg))

        string ret_msg("push message to frame queue successfully");
        session->send(ret_msg.c_str(), ret_msg.size());

		delete [] buff;
		session->close();

	}

	virtual void onSessionDisconnected(translib::TcpSession *session)
	{
		LOG4CPLUS_WARN(logger, "session disconnected!");

        // to do. add logic to handle retry?
	}

	virtual void onNewSession(translib::TcpSession *session)
	{
		LOG4CPLUS_DEBUG(logger, "got new session");
	}

};


int main()
{
    // init log
    log4cplus::initialize();
    try
    {
        SharedObjectPtr<Appender> append_1(new FileAppender("RF24_server.log"));
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

    // create a new monitor thread 
    pthread_t ntid;
    pthread_create(&ntid, NULL, start_monitor, NULL);

    RF24_server &RF24_tcpServer = RF24_server::instance();

	if (!RF24_tcpServer.listen(RF24_SERVER_IP, RF24_SERVER_PORT))
	{
		LOG4CPLUS_ERROR(logger, "listen retrun fail!");
		return 0;
	}

    RF24_tcpServer.wait();

}
void handle_message(rf_payload *payload_p, RF24NetworkHeader *header_p)
{

    if (header_p->from_node == 0)
    {
        LOG4CPLUS_DEBUG(logger, 
                                    "got node id equals to 0 in the node, it is inserted while received message via socket");
        // send message to node
        network.write(*header_p, payload_p, payload_p->len + 2);
    }
    else
    {
        LOG4CPLUS_DEBUG(logger, 
                                    "receive data from node"
                                    << header_p->from_node
                                    << ". pin is "
                                    << int((reinterpret_cast<one_wire_onoff_s *>(&(payload_p->data)))->pin)
                                    << ". payload data is "
                                    << int((reinterpret_cast<one_wire_onoff_s *>(&(payload_p->data)))->data));
        int sock = node_sockt_fd[mesh.getNodeID(header_p->from_node)];
        if (sock)
        {
            int ret;
            ret = send(sock, payload_p, payload_p->len + 2, 0);
            LOG4CPLUS_DEBUG(logger, "send return " << ret);
        }
    }
}
void handle_G_message(rf_payload *payload_p, RF24NetworkHeader *header_p)
{
    static string _func = "handle_G_message() ";
    LOG4CPLUS_TRACE(logger, 
                                "entered. header_p->from_node is " << header_p->from_node);
    handle_message(payload_p, header_p);
}
void handle_P_message(rf_payload *payload_p, RF24NetworkHeader *header_p)
{
    static string _func = "handle_P_message() ";
    LOG4CPLUS_TRACE(logger, 
                                "entered. header_p->from_node is " << header_p->from_node);
    handle_message(payload_p, header_p);
}

void handle_S_message(rf_payload *payload_p, RF24NetworkHeader *header_p)
{
    static string _func = "handle_S_message() ";
    LOG4CPLUS_TRACE(logger, 
                                "entered. header_p->from_node is " << header_p->from_node);
    handle_message(payload_p, header_p);
}
void handle_R_message(rf_payload *payload_p, RF24NetworkHeader *header_p)
{
    static string _func = "handle_R_message() ";
    LOG4CPLUS_TRACE(logger, 
                                "entered. header_p->from_node is " << header_p->from_node);
    handle_message(payload_p, header_p);
}

void handle_incoming_message(char *buf, RF24NetworkHeader *header_p)
{
    static string _func = "handle_incoming_message() ";
    LOG4CPLUS_TRACE(logger, 
                                "entered! header_p->type is "
                                << header_p->type);
    rf_payload *payload_p = (rf_payload *)buf;
    switch (header_p->type)
    {
    case 'G':
        LOG4CPLUS_TRACE(logger, 
                                    "receive a 'G' message!");
        handle_G_message(payload_p, header_p);
        break;
    case 'P':
        LOG4CPLUS_TRACE(logger, 
                                    "receive a 'P' message!");
        handle_P_message(payload_p, header_p);
        break;
    case 'S':
        LOG4CPLUS_TRACE(logger, 
                                    "receive a 'S' message!");
        handle_S_message(payload_p, header_p);
        break;
    case 'R':
        LOG4CPLUS_TRACE(logger, 
                                    "receive a 'R' message!");
        handle_R_message(payload_p, header_p);
        break;
    case 'M':
        LOG4CPLUS_TRACE(logger, 
                                    "receive a 'M' message!");
        break;
    default:
        LOG4CPLUS_ERROR(logger, 
                                    "receive an invalid  message!");
        break;
    }
}
void *start_monitor(void *)
{
    static string _func = "start_monitor() ";
    printf("Establishing mesh...\n");
    mesh.setNodeID(0);
    mesh.begin();
    radio.printDetails();

    initscr(); /* Start curses mode 		  */
    start_color();
    curs_set(0);
    //keypad(stdscr, TRUE); //Enable user interaction
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    printw("RF24Mesh Master Node Monitoring Interface by Max Cong - 2016\n");
    attroff(COLOR_PAIR(1));
    refresh(); /* Print it on to the real screen */

    uint32_t kbTimer = 0, kbCount = 0, pingTimer = millis();
    //std::map<char,uint16_t>::iterator it = mesh.addrMap.begin();
    unsigned long totalPayloads = 0;

    while (1)
    {

        // Call mesh.update to keep the network updated
        mesh.update();
        // In addition, keep the 'DHCP service' running on the master node so addresses will
        // be assigned to the sensor nodes
        mesh.DHCP();
        // Wait until a sensor node is connected
        if (sizeof(mesh.addrList) < 1)
        {
            continue;
        }

        // Check for incoming data from the sensors
        while (network.available())
        {
            RF24NetworkHeader header;
            network.peek(header);

            uint8_t boldID = 0;

            // Print the total number of received payloads
            mvprintw(9, 0, " Total: %lu\n", totalPayloads++);

            kbCount++;

            attron(A_BOLD | COLOR_PAIR(1));
            mvprintw(2, 0, "[Last Payload Info]\n");
            attroff(A_BOLD | COLOR_PAIR(1));

            // Read the network payload
            //network.read(header,0,0);
            char message_buf[32 - sizeof(RF24NetworkHeader)];
            memset(message_buf, 0, 32 - sizeof(RF24NetworkHeader));
            uint16_t payload_len;
            payload_len = network.read(header, message_buf, 32 - sizeof(RF24NetworkHeader));
            LOG4CPLUS_TRACE(logger, 
                                        "got "
                                        << payload_len
                                        << " byte message form RF24");
            // handle coming message
            handle_incoming_message(message_buf, &header);

            // Display the header info
            mvprintw(3, 0, " HeaderID: %u  \n Type: %d  \n From: 0%o  \n", header.id, header.type, header.from_node);

            //refresh();
            //for (std::map<char,uint16_t>::iterator _it=mesh.addrMap.begin(); _it!=mesh.addrMap.end(); _it++){
            for (uint8_t i = 0; i < mesh.addrListTop; i++)
            {
                if (header.from_node == mesh.addrList[i].address)
                {
                    boldID = mesh.addrList[i].nodeID;
                }
            }
            printNodes(boldID);
        }
        //refresh();

        if (millis() - kbTimer > 1000 && kbCount > 0)
        {
            kbTimer = millis();
            attron(A_BOLD | COLOR_PAIR(1));
            mvprintw(7, 0, "[Data Rate (In)]");
            attroff(A_BOLD | COLOR_PAIR(1));
            mvprintw(8, 0, " Kbps: %.2f", (kbCount * 32 * 8) / 1000.00);
            kbCount = 0;
        }

        // Ping each connected node, one per second
        if (millis() - pingTimer > 1003 && sizeof(mesh.addrList) > 0)
        {
            pingTimer = millis();
            if (nodeCounter == mesh.addrListTop)
            { // if(mesh.addrMap.size() > 1){ it=mesh.addrMap.begin(); } continue;}
                nodeCounter = 0;
            }
            pingNode(nodeCounter);
            nodeCounter++;
        }

        /*uint32_t nOK,nFails;
	network.failures(&nFails,&nOK);
	attron(A_BOLD | COLOR_PAIR(1));
    mvprintw(2,24,"[Transmit Results] ");
    attroff(A_BOLD | COLOR_PAIR(1));
	mvprintw(3,25," #OK: %u   ",nOK);
	mvprintw(4,25," #Fail: %u   ",nFails);*/

        refresh();
        delay(2);
    } //while 1

    endwin(); /* End curses mode		  */
}

void printNodes(uint8_t boldID)
{

    uint8_t xCoord = 2;
    attron(A_BOLD | COLOR_PAIR(1));
    mvprintw(xCoord++, 27, "[Address Assignments]\n");
    attroff(A_BOLD | COLOR_PAIR(1));
    //for (std::map<char,uint16_t>::iterator it=mesh.addrMap.begin(); it!=mesh.addrMap.end(); ++it){
    for (uint8_t i = 0; i < mesh.addrListTop; i++)
    {
        //if( failID == it->first){
        if (failID == mesh.addrList[i].nodeID)
        {
            attron(COLOR_PAIR(2));
        }
        else if (boldID == mesh.addrList[i].nodeID)
        {
            attron(A_BOLD | COLOR_PAIR(1));
        }
        mvprintw(xCoord++, 28, "ID: %d  Network: 0%o   ", mesh.addrList[i].nodeID, mesh.addrList[i].address);
        attroff(A_BOLD | COLOR_PAIR(1));
        attroff(COLOR_PAIR(2));
    }
    mvprintw(xCoord++, 28, "                   ");
    mvprintw(xCoord++, 28, "                   ");
    mvprintw(xCoord++, 28, "                   ");
}

void pingNode(uint8_t listNo)
{

    attron(A_BOLD | COLOR_PAIR(1));
    mvprintw(11, 0, "[Ping Test]\n");
    attroff(A_BOLD | COLOR_PAIR(1));

    RF24NetworkHeader headers(mesh.addrList[listNo].address, NETWORK_PING);
    uint32_t pingtime = millis();
    bool ok = false;
    if (headers.to_node)
    {
        ok = network.write(headers, 0, 0);
        if (ok && failID == mesh.addrList[listNo].nodeID)
        {
            failID = 0;
        }
        if (!ok)
        {
            failID = mesh.addrList[listNo].nodeID;
        }
    }
    pingtime = millis() - pingtime;
    mvprintw(12, 0, " ID:%d", mesh.addrList[listNo].nodeID);
    mvprintw(13, 0, " Net:0%o", mesh.addrList[listNo].address);
    mvprintw(14, 0, " Time:%ums", pingtime);

    if (ok || !headers.to_node)
    {
        mvprintw(15, 0, " OK  ");
    }
    else
    {
        attron(A_BOLD);
        mvprintw(15, 0, " FAIL");
        attron(A_BOLD);
    }
}
