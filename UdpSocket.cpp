// Project:      CSS432 UDP Socket Class
// Professor:    Munehiro Fukuda
// Organization: CSS, University of Washington, Bothell
// Date:         March 5, 2004
// Updated 	 2016, Prof. Dimpsey

#include "UdpSocket.h"

UdpSocket::UdpSocket( int port ) 
{
    this->port = port;
    this->sd = NULL_SD;
    // Open a UDP socket (a datagram socket )
    if((sd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 ) 
    {
        cerr << "Cannot open a UDP socket." << endl;
    }

    // Bind our local address
    bzero((char*)&myAddr, sizeof( myAddr )); 
    myAddr.sin_family = AF_INET;         
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    myAddr.sin_port        = htons( port );    
    
    if( bind( sd, (sockaddr*)&myAddr, sizeof( myAddr ) ) < 0 ) 
    {
        cerr << "Cannot bind the local address to the UDP socket." << endl;
    }
}

UdpSocket::~UdpSocket( ) 
{
    if (sd != NULL_SD)
    {
        close(sd );
    }
}

bool UdpSocket::setDestAddress( char ipName[] ) 
{

    // Get the host entry corresponding to this destination ipName
    struct hostent* host = gethostbyname(ipName);
    if( host == NULL ) 
    {
        cerr << "Cannot find hostname." << endl;
        return false; 
    }

     // Fill in the structure "sendSockAddr" with this destination host entry
    bzero( (char*)&destAddr, sizeof(destAddr)); 
    destAddr.sin_family = AF_INET;  
    destAddr.sin_addr.s_addr =                     // set the destination IP addr
        inet_addr( inet_ntoa( *(struct in_addr*)*host->h_addr_list ) );
    destAddr.sin_port = htons(port);   

    return true;                                 
}


// Set the IP addr given a destination IP name and port number
bool UdpSocket::setDestAddress(char ipName[], int destPort) 
{
    // Get the host entry corresponding to this destination ipName
    struct hostent* host = gethostbyname( ipName );
    if( host == NULL ) 
    {
        cerr << "Cannot find hostname." << endl;
        return false;  
    }

    bzero( (char*)&destAddr, sizeof( destAddr ) ); 
    destAddr.sin_family      = AF_INET;            
    destAddr.sin_addr.s_addr = 
         inet_addr( inet_ntoa( *(struct in_addr*)*host->h_addr_list ) );
    destAddr.sin_port = htons( destPort );  // set the destination port
    return true;                                   
}

// Check if this socket has data to receive 
int UdpSocket::pollRecvFrom( ) 
{
    struct pollfd pfd[1];
    pfd[0].fd = sd;             // declare I'll check the data availability of sd
    pfd[0].events = POLLRDNORM; // declare I'm interested in only reading from sd

    // check it immediately and return a positive number if sd is readable,
    // otherwise return 0 or a negative number
    return poll( pfd, 1, 0 );
}

int UdpSocket::sendTo(char msg[], int length) 
{
    return sendto(sd, msg, length, 0, (sockaddr *)&destAddr, sizeof(destAddr));
}

int UdpSocket::recvFrom(char msg[], int length) 
{
    // zero-initialize the srcAddr structure so that it can be filled out with
    // the address of the source computer that has sent msg[]
    socklen_t addrlen = sizeof(srcAddr);
    bzero( (char *)&srcAddr, sizeof( srcAddr ) );

    return recvfrom(sd, msg, length, 0, &srcAddr, &addrlen);
}

// Send through the sd socket an acknowledgment in msg[] whose size is length
int UdpSocket::ackTo( char msg[], int length ) 
{
  // assume that srcAddress has be filled out upon the previous recvFrom( )
  // method.

    return sendto(sd, msg, length, 0, &srcAddr, sizeof( srcAddr ) );
}
