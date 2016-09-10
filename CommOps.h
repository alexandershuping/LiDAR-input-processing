#pragma once

#include "ControlComputer.h"
#include <string>

namespace cops{


/**
* ENUM DEFINITIONS
*/
enum CONNECTION_STATE{
  STATE_DISCONNECTED,
	STATE_CONNECTED,
	STATE_ERROR

};

enum LIDAR_COMMAND{

  LIDAR_SYN    = 0x00, // SYNc for handshake                           -- Sent     to   microcontroller to establish a connection
	LIDAR_SYNACK = 0x01, // SYNc ACKnowledge for handshake               -- Received from microcontroller to indicate successful connection
	LIDAR_ACK    = 0x02, // ACKnowledge for handshake                    -- Sent     to   microcontroller to complete a connection
	LIDAR_GET    = 0x03, // GET data from microcontroller                -- Sent     to   microcontroller when data are requested
	LIDAR_RSP    = 0x04, // ReSPonse to data request                     -- Received from microcontroller, indicates that data are being sent
	LIDAR_DISCON = 0x05, // DISCONnect with microcontroller              -- Sent     to   microcontroller to indicate that data collection is finished and the connection is being terminated
	LIDAR_DSCACK = 0x06, // DiSConnect ACKnowledge from microcontroller  -- Received from microcontroller to confirm that disconnection was successful
  LIDAR_PROBE  = 0x07, // connection PROBE                             -- Sent     to   unknown serial port to determine whether a compatible microcontroller is attached
	LIDAR_PRBACK = 0x08  // connection PRoBe ACKnowledge                 -- Received from microcontroller to confirm that it is connected over the tested serial port

};


/**
* CLASS DEFINITIONS
*/

/**
* class PacketDataLengthException
*
* Exception thrown when something attempts to assign an invalid amount of data to a packet
*
* Exception type (string returned from getExceptionType) is "PacketDataLengthException"
*
* Error codes:
*  0 -- Packet is longer than the maximum allowed
*  1 -- Packet is shorter than the minimum allowed
*  2 -- Packet length is not equal to the only allowed length
*/
class PacketDataLengthException : public Exception{
  private:
		int _error_code;
		int _length;
		int _expected;

	public:

                         PacketDataLengthException(int code, int length, int expected); // length -- length of erroneous data, expected -- expected length
	  virtual             ~PacketDataLengthException(){}
		virtual std::string getErrorMessage();
		virtual std::string getExceptionType();
    virtual int         getErrorCode();
						int         getLength();
						int         getExpectedLength();
};

/**
* class packet
*
* A packet of data, comprising a command and a set of 8 data bytes. Sent to and received from the microcontroller.
*/
class packet{

  private:
    const static int _MAX_PACKET_SIZE = 8;

    LIDAR_COMMAND _cmd;                                                     // Command sent or received (see struct LIDAR_COMMAND above for details)
		unsigned char _dat[_MAX_PACKET_SIZE];                                   // _MAX_PACKET_SIZE bytes of data
		int           _recursiveByteifyInt(int, unsigned int, unsigned char[_MAX_PACKET_SIZE]); // INTERNAL function to deconstruct an int into a byte[]

	public:
	                packet(LIDAR_COMMAND l, char d[_MAX_PACKET_SIZE], bool raw); // Initialize packet with command and data
									packet(LIDAR_COMMAND l, std::string d   ); // Initialize packet with command and string data (string must be MAX 8 chars)
									packet(LIDAR_COMMAND l, int         d   ); // Initialize packet with command and int data
									packet(LIDAR_COMMAND l, double      d   ); // Initialize packet with command and double data

    //              packet(const packet& first              ); // Copy constructor
		//					 	 ~packet(                                 ); // Destructor
		//			packet& operator=(const packet& first           ); // Assignment operator

    int           getMaxPacketSize();                        // Get the maximum amount of data that can be stored in a packet

		void          getData(char buf[_MAX_PACKET_SIZE]);       // Get raw    data from packet, and store it in buf
		std::string   getString();                               // Get string data from packet
		int           getInt();                                  // Get int    data from packet
		//double        getDouble();                               // Get double data from packet
		LIDAR_COMMAND getCommand();                              // Get command from packet

};

/**
* class connection
*
* A serial connection made with a microcontroller
*/
/*
class connection{

  private:
	  int              port;
		CONNECTION_STATE state;

	public:
	                   connection(const config&, const params&);
		CONNECTION_STATE getConnectionState();
		int              getPort();
		bool             send(packet);

};
*/
/**
*
*/


/*
* FUNCTION PROTOTYPES
*/


/*End of namespace -- any code beyond this bracket is not part of namespace cops*/}
