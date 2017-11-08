// ----------------------------------------------------------------------------
// (CSS 432) Computer Networking (Spring 2016) - Reza Naeemi
// University of Washington Bothell
// Professor: Robert Dimpsey
//
// Program 2 (Sliding Window) = hw2.cpp
// ----------------------------------------------------------------------------
// This program implements a Stop-and-wait algorithm and the sliding window
// algorithm. The program instantiates sock, (UdpSocket object), allocates a
// 1460-byte message[], and then evaluates the performance of UDP
// point-to-point communication using three different test cases:
//  1. Unreliable transfer
//  2. Stop-and-Wait
//  3. Sliding Window
// ----------------------------------------------------------------------------
// -- As server, usage: ./hw2
// -- As client, usage: ./hw2 [serverIP]
//
// Assumptions:
// This program has very different results when it is run on the localhost
// versus a gigabit LAN.  For proper timings, this program should be used on
// two seperate Linux lab computers.

#include <iostream>
#include <vector>
#include <unistd.h>		// (Added by Reza)

#include "UdpSocket.h"
#include "Timer.h"
using namespace std;

const int PORT = 12721;       // my (Reza Naeemi) UDP port
const int MAX = 20000;        // times of message transfer
const int MAX_WIN = 30;       // maximum window size
const int MIN_WIN = 1;		// (Added by Reza) Minimum window size
const int TIMEOUT = 1500;	// (Added by Reza) Default Timeout value
const bool verbose = false;   // use verbose mode for more information during run

// client packet sending functions
void ClientUnreliable(UdpSocket &sock, int max, int message[]);
int ClientStopWait(UdpSocket &sock, int max, int message[]);
int ClientSlidingWindow(UdpSocket &sock, int max, int message[],int windowSize);

// server packet receiving functions
void ServerUnreliable(UdpSocket &sock, int max, int message[]);
void ServerReliable(UdpSocket &sock, int max, int message[]);
void ServerEarlyRetrans(UdpSocket &sock, int max, int message[],int windowSize );

enum myPartType {CLIENT, SERVER} myPart;

int main( int argc, char *argv[] ) 
{
    int message[MSGSIZE/4]; 	  // prepare a 1460-byte message: 1460/4 = 365 ints;

    // Parse arguments
    if (argc == 1) 
    {
        myPart = SERVER;
    }
    else if (argc == 2)
    {
        myPart = CLIENT;
    }
    else
    {
        cerr << "usage: " << argv[0] << " [serverIpName]" << endl;
        return -1;
    }

    // Set up communication
    // Use different initial ports for client server to allow same box testing
    UdpSocket sock( PORT + myPart );  
    if (myPart == CLIENT)
    {
        if (! sock.setDestAddress(argv[1], PORT + SERVER)) 
        {
            cerr << "cannot find the destination IP name: " << argv[1] << endl;
            return -1;
        }
    }

    int testNumber;
    cerr << "Choose a testcase" << endl;
    cerr << "   1: unreliable test" << endl;
    cerr << "   2: stop-and-wait test" << endl;
    cerr << "   3: sliding windows" << endl;
    cerr << "--> ";
    cin >> testNumber;

    if (myPart == CLIENT) 
    {
        Timer timer;           
        int retransmits = 0;   

        switch(testNumber) 
        {
        case 1:
            timer.Start();
            ClientUnreliable(sock, MAX, message); 
            cout << "Elasped time = ";  
            cout << timer.End( ) << endl;
            break;
        case 2:
            timer.Start();   
            retransmits = ClientStopWait(sock, MAX, message); 
            cout << "Elasped time = "; 
            cout << timer.End( ) << endl;
            cout << "retransmits = " << retransmits << endl;
            break;
        case 3:
            for (int windowSize = 1; windowSize <= MAX_WIN; windowSize++ ) 
            {
	        timer.Start( );
	        retransmits = ClientSlidingWindow(sock, MAX, message, windowSize);
	        cout << "Window size = ";  
	        cout << windowSize << " ";
	        cout << "Elasped time = "; 
	        cout << timer.End( ) << endl;
	        cout << "retransmits = " << retransmits << endl;
            }
            break;
        default:
            cerr << "no such test case" << endl;
            break;
        }
    }
    if (myPart == SERVER) 
    {
        switch(testNumber) 
        {
            case 1:
                ServerUnreliable(sock, MAX, message);
                break;
            case 2:
                ServerReliable(sock, MAX, message);
                break;
            case 3:
                for (int windowSize = 1; windowSize <= MAX_WIN; windowSize++)
                {
	            ServerEarlyRetrans( sock, MAX, message, windowSize );
                }
                break;
            default:
                cerr << "no such test case" << endl;
                break;
        }

        // The server should make sure that the last ack has been delivered to client.
        
        if (testNumber != 1)
        {
            if (verbose)
            {
                cerr << "server ending..." << endl;
            }
            for ( int i = 0; i < 10; i++ ) 
            {
                sleep( 1 );
                int ack = MAX - 1;
                sock.ackTo( (char *)&ack, sizeof( ack ) );
            }
        }
    }
    cout << "finished" << endl;
    return 0;
}

// Test 1 Client
void ClientUnreliable(UdpSocket &sock, int max, int message[]) 
{
    // transfer message[] max times; message contains sequences number i
    for ( int i = 0; i < max; i++ ) 
    {
        message[0] = i;                            
        sock.sendTo( ( char * )message, MSGSIZE ); 
        if (verbose)
        {
            cerr << "message = " << message[0] << endl;
        }
    }
    cout << max << " messages sent." << endl;
}

// Test1 Server
void ServerUnreliable(UdpSocket &sock, int max, int message[]) 
{
    // receive message[] max times and do not send ack
    for (int i = 0; i < max; i++) 
    {
        sock.recvFrom( ( char * ) message, MSGSIZE );
        if (verbose)
        {  
            cerr << message[0] << endl;
        }                    
    }
    cout << max << " messages received" << endl;
}



int ClientStopWait(UdpSocket &sock, int max, int message[])
{
    //Implement this function
	int retransmissionCount = 0;

	Timer ackTimer;

	// Loop max number of times, so that all frames are sent and acks received.
	for (int sequenceNum = 0; sequenceNum < max; sequenceNum++)
	{
		message[0] = sequenceNum;
		sock.sendTo((char*)message, MSGSIZE);

		ackTimer.Start();

		// While there is no data to be read,
		// prepare for the current frame to time out.
		// Otherwise, skip the current loop.
		while (sock.pollRecvFrom() < 1)
		{
			// If timeout occurs,
			// resend the frame and then restart the timer,
			// so it can be used for the next frame.
			if (ackTimer.End() > TIMEOUT)
			{
				sock.sendTo((char*)message, MSGSIZE);
				
				if (verbose)
				{
					cout << "Message #" <<
					sequenceNum <<
					" resent." << 
					endl;
				}

				ackTimer.Start();

				retransmissionCount++;
			}
		}	

		// The while loop scope has ended.
		// So, ack is for sure available to read.
		sock.recvFrom((char*)message, MSGSIZE);

	}


	return retransmissionCount;

}

void ServerReliable(UdpSocket &sock, int max, int message[])
{
   //Implement this function
	for (int sequenceNum = 0; sequenceNum < max; sequenceNum++)
	{
		do
		{
			sock.recvFrom((char*)message, MSGSIZE);
		
			// If the frames are in order, send an ack back.
			// Otherwise, ignore the current frame and allow
			// the client to time out.
			if (message[0] == sequenceNum)
			{
				sock.ackTo((char*)&sequenceNum, sizeof(sequenceNum));
			}
			else if (verbose)
			{
				cout << "Got message #" <<
				message[0] << " from the client." <<
				endl;
			}


			// Note: Need to keep track of the sequence order 				
		
		}while (message[0] != sequenceNum);
 			
	}
	
}

int ClientSlidingWindow(UdpSocket &sock, int max, int message[], int windowSize)
{
    //Implement this function
	int retransmissionCount = 0;
	int sequenceNextFrame = 0;	// Next frame to be transmitted
	int ackSequenceLastFrame = 0;	// Last frame in the sequence,
					// that is waiting for acknowledgement.

	Timer ackTimer;
	int lastAckReceived = -1;

	
	// Loop while ther are frames to be sent, 
	// and acks waiting for be received.
	while (
		(sequenceNextFrame < max) || 
		(ackSequenceLastFrame < max))
	{
		// Each time that loop occurs,
		// send another frame and advance the sequence,
		// while making sure the number of frames currently being trasmitted
		// does not exceed the size of the window.
		// As the ackSequenceLastFrame falls behind the sequenceNextFrame,
		// the window will shink or close.
		// The acks must be received in oder to open again,
		// inorder to advance the sending sequence.
		if (
			(sequenceNextFrame < max) &&
			((ackSequenceLastFrame + windowSize) > sequenceNextFrame)
		)
		{
			message[0] = sequenceNextFrame;
			sock.sendTo((char*)message, MSGSIZE);
			
			sequenceNextFrame++;
      
      // cout << "Message #" << message[0] << " was sent." << endl;
      
		} 


		usleep(1);
			
		


		// If the data in the socket is available to read,
		// read the ack from the server.
		// If the ack is in the ackSequenceLastFrame order that is expected,
		// then open the send window on the next loop.
		if (sock.pollRecvFrom() > 0)
		{
			sock.recvFrom((char*)&lastAckReceived,
						sizeof(lastAckReceived));
                                            
      // cout << "Ack #" << lastAckReceived << " was received." << endl;
   

			if (lastAckReceived == ackSequenceLastFrame)
			{
				ackSequenceLastFrame++;
			}
		}
		else
		{
			// Note: Since we got here, no data is available to read.
			
			ackTimer.Start();

			while (sock.pollRecvFrom() < 1)
			{
				if (ackTimer.End() > TIMEOUT)
				{
					if (verbose)
					{
						cout << "Frame #: " <<
							ackSequenceLastFrame <<
							" timed out," << 
							" for sequence #: " <<
							sequenceNextFrame <<
						endl;
					}

					retransmissionCount +=
					(sequenceNextFrame - ackSequenceLastFrame);


					

					// The frame arrived late
					if (
						(lastAckReceived <=
							sequenceNextFrame) &&
						(lastAckReceived >=
							ackSequenceLastFrame)
					)			
					{
						ackSequenceLastFrame =
							1 + lastAckReceived;
					}
					else	// The frame never arrived,
						// and is considered lost.
					{
						sequenceNextFrame =
							ackSequenceLastFrame;
					}

					// Break out of the while loop
					break;

					
				}
			}
		}
		
	}

	
	return retransmissionCount;

}

void ServerEarlyRetrans(UdpSocket &sock, int max, int message[],int windowSize )
{
   //Implement this function
	int lastFrameReceived = 0;
	int lastAckFrame = 0;
	int lastSequence = -1;

	vector<bool> array(max, false);

	
	do
	{
		// Wait until there is data available to be read from the client.
		if (sock.pollRecvFrom() > 0)
		{
			sock.recvFrom((char*)message, MSGSIZE);
			lastFrameReceived = message[0];

			// If the frame happens to be outside the window size,
			// then it should be ignored.
			if ((lastFrameReceived - lastAckFrame) > windowSize)
			{
				// Ignore frame (drop it!)
				continue;
			}
			else if (lastFrameReceived > lastAckFrame)
			{
				array[lastFrameReceived] = true;

				// Make should the frames received are continuous
				// for cumulatative acks.
				while (array[lastAckFrame])
				{
					lastSequence = lastAckFrame++;
				}				

			}
			else
			{
				// Acknowledge the last frame that was received.
				array[lastFrameReceived] = true;
				lastSequence = lastAckFrame;
			}

			

			sock.ackTo((char*)&lastSequence, sizeof(lastSequence));
			// cout << "Ack #" << lastSequence << " was sent." << endl;			

		}


	} while (lastAckFrame < max); 	


}
