// Project:      CSS432 UDP Socket Class
// Professor:    Munehiro Fukuda
// Organization: CSS, University of Washington, Bothell
// Date:         March 5, 2004
// Updated: 	 2016, Prof. Dimpsey

#ifndef _UDPSOCKET_H_
#define _UDPSOCKET_H_

#include <sys/types.h>    // for sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>        // for gethostbyname( )
#include <unistd.h>       // for close( )
#include <string.h>       // for bzero( )
#include <sys/poll.h>     // for poll( )
#include <iostream>
using namespace std;

const int MSGSIZE = 1460;    // UDP message size in bytes
const int NULL_SD = -1;       

class UdpSocket 
{
    public:
        UdpSocket(int port);       
        ~UdpSocket( );
        bool setDestAddress(char ipName[]); 
        bool setDestAddress(char ipName[], int port); 
        int pollRecvFrom();           
        int sendTo(char msg[], int length);  
        int recvFrom(char msg[], int length);   
        int ackTo(char msg[], int lenghth ); 
    private:
        int port;        
        int sd;                       
        struct sockaddr_in myAddr;     // my socket address for internet
        struct sockaddr_in destAddr;   // a destination socket address for internet
        struct sockaddr srcAddr;       // a source socket address for internet
};  

#endif  
