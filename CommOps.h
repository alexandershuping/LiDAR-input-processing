#pragma once

#include <string>
#include <SerialStream.h>

using std::string;
using namespace LibSerial;

namespace cops{


/**
* ENUM DEFINITIONS
*/
enum CONNECTION_STATE{
  STATE_DISCONNECTED,  // SerialStream is closed, and microcontroller knows this
	STATE_CONNECTED,     // SerialStream is open,   and microcontroller knows this
	STATE_ERROR,         // SerialStream is open,   but microcontroller may not know this
	STATE_FORCED_CLOSED  // SerialStream is closed, but microcontroller may not know this

};

enum LIDAR_COMMAND{

  LIDAR_SYN    = 0x00, // SYNc for handshake                           -- Sent     to   microcontroller to establish a connection
	LIDAR_SYNACK = 0x01, // SYNc ACKnowledge for handshake               -- Received from microcontroller to indicate successful connection
	LIDAR_ACK    = 0x02, // ACKnowledge for handshake                    -- Sent     to   microcontroller to complete a connection
	LIDAR_GET    = 0x04, // GET data from microcontroller                -- Sent     to   microcontroller when data are requested
	LIDAR_RSP    = 0x05, // ReSPonse to data request                     -- Received from microcontroller, indicates that data are being sent
	LIDAR_DISCON = 0x06, // DISCONnect with microcontroller              -- Sent     to   microcontroller to indicate that data collection is finished and the connection is being terminated
	LIDAR_DSCACK = 0x07, // DiSConnect ACKnowledge from microcontroller  -- Received from microcontroller to confirm that disconnection was successful
  LIDAR_NUL    = 0x0A  // NULl packet                                  -- Dummy packet that shouldn't be sent either way

/*[DEPRECATEDLIDAR_PROBE  = 0x08, // connection PROBE                             -- Sent     to   unknown serial port to determine whether a compatible microcontroller is attached*/

/*[DEPRECATED	LIDAR_PRBACK = 0x09  // connection PRoBe ACKnowledge                 -- Received from microcontroller to confirm that it is connected over the tested serial port*/

/*[DEPRECATED] LIDAR_SET = 0x03, // SET microcontroller setting        -- Sent     to   microcontroller to send settings*/

};


/**
* CLASS DEFINITIONS
*/

// Abstract exception class
class Exception{

  public:
	  virtual int         getErrorCode()     = 0;
	  virtual std::string getExceptionType() = 0;
	  virtual std::string getErrorMessage()  = 0;

    virtual            ~Exception()        = 0;
};
									

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
	  virtual             ~PacketDataLengthException();
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
		const static int _DOUBLE_PRECISION = 1000;

    LIDAR_COMMAND _cmd;                                                     // Command sent or received (see struct LIDAR_COMMAND above for details)
		unsigned char _dat[_MAX_PACKET_SIZE];                                   // _MAX_PACKET_SIZE bytes of data
		
		unsigned long _recursiveByteifyInt(unsigned long, unsigned int, unsigned char[_MAX_PACKET_SIZE]); // INTERNAL function to deconstruct an int into a byte[]
		void          _recursiveByteifyInt(unsigned long, unsigned char[_MAX_PACKET_SIZE]);

	public:
	                packet(LIDAR_COMMAND l, char d[_MAX_PACKET_SIZE]); // Initialize packet with command and data
									packet(LIDAR_COMMAND l, std::string   d ); // Initialize packet with command and string data (string must be MAX 8 chars)
									packet(LIDAR_COMMAND l, long          d ); // Initialize packet with command and long data
									packet(LIDAR_COMMAND l, int           d ); // Initialize packet with command and int data
									packet(LIDAR_COMMAND l, unsigned long d ); // Initialize packet with command and unsigned long data
									packet(LIDAR_COMMAND l, unsigned int  d ); // Initialize packet with command and unsigned int data
									packet(LIDAR_COMMAND l, double        d ); // Initialize packet with command and double data
									packet();                                  // Blank constructor = meaningless packet. Data will be blank, and command will be LIDAR_NUL

                  packet(const packet& first              ); // Copy constructor
							 	 ~packet(                                 ); // Destructor
					packet& operator=(const packet& first           ); // Assignment operator

    static int    getMaxPacketSize();                        // Get the maximum amount of data that can be stored in a packet

		void          getData(char buf[_MAX_PACKET_SIZE]) const; // Get raw      data from packet, and store it in buf
		std::string   getString() const;                         // Get string   data from packet
		unsigned long getUnsignedLong() const;                   // Get uns long data from packet  
		long          getLong() const;                           // Get long     data from packet
		unsigned int  getUnsignedInt() const;                    // Get uns int  data from packet
		int           getInt() const;                            // Get int      data from packet
		double        getDouble() const;                         // Get double   data from packet
		LIDAR_COMMAND getCommand() const;                        // Get command       from packet

    std::string   toString() const;                          // Dump packet data to string
};

struct thread_io{
  bool released;
	bool dataAvailable;
	int port;
	SerialStream* ser;
	unsigned char data;
};

/**
* class connection
*
* A serial connection made with a microcontroller
*/

class connection{

  private:
	  string           _port;
		CONNECTION_STATE _state;
		int              _baud_rate;
		int              _timeout;
		SerialStream     _ser;

		bool             _doHandshake();

		static void      _serialThreadBlock(thread_io*);
    static void      _releaseHandle(thread_io*);

	public:
	                   connection(string port, int baudRate, int timeout);
		bool             open();                         // True = handshake successful, false = handshake unsuccessful
		bool             close(bool);                    // True = disconnect handshake successful, false = handshake unsuccessful
		CONNECTION_STATE getConnectionState();
		string           getPort();
		int              getBaudRate();
		bool             send(const packet&);      // True = packet sent and ack'ed, false = packet not ack'ed by microcontroller
		packet           get(int timeout, bool& success);// DO NOT TRY TO ACCESS THE PACKET WITHOUT CHECKING THAT SUCCESS IS TRUE. If the get operation times out, the packet will be meaningless.
};

/**
*
*/


/*
* FUNCTION PROTOTYPES
*/


/*End of namespace -- any code beyond this bracket is not part of namespace cops*/}
