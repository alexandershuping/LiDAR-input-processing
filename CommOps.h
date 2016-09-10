#pragma once

#include "ControlComputer.h"
#include <string>

namespace cops{


/**
* STRUCT DEFINITIONS
*/
struct CONNECTION_STATE{
  STATE_DISCONNECTED,
	STATE_CONNECTED,
	STATE_ERROR

};

struct LIDAR_COMMAND{

  LIDAR_SYN    = 0x00, // SYNc for handshake                           -- Sent     to   microcontroller to establish a connection
	LIDAR_SYNACK = 0x01, // SYNc ACKnowledge for handshake               -- Received from microcontroller to indicate successful connection
	LIDAR_ACK    = 0x02, // ACKnowledge for handshake                    -- Sent     to   microcontroller to complete a connection
	LIDAR_GET    = 0x03, // GET data from microcontroller                -- Sent     to   microcontroller when data are requested
	LIDAR_RSP    = 0x04, // ReSPonse to data request                     -- Received from microcontroller, indicates that data are being sent
	LIDAR_DISCON = 0x05, // DISCONnect with microcontroller              -- Sent     to   microcontroller to indicate that data collection is finished and the connection is being terminated
	LIDAR_DSCACK = 0x06, // DiSConnect ACKnowledge from microcontroller  -- Received from microcontroller to confirm that disconnection was successful
  LIDAR_PROBE  = 0x07, // connection PROBE                             -- Sent     to   unknown serial port to determine whether a compatible microcontroller is attached
	LIDAR_PRBACK = 0x08  // connection PRoBe ACKnowledge                 -- Received from microcontroller to confirm that it is connected over the tested serial port

}


/**
* CLASS DEFINITIONS
*/

/**
* class packet
* 
* A packet of data, comprising a command and a set of 8 data bytes. Sent to and received from the microcontroller.
*/
class packet{

  private:
    LIDAR_COMMAND cmd;                                       // Command sent or received (see struct LIDAR_COMMAND above for details)
		char          dat[8];                                    // 8 bytes of data

	public:
	                packet(LIDAR_COMMAND l, char        d[8]); // Initialize packet with command and data
									packet(LIDAR_COMMAND l, std::string d   ); // Initialize packet with command and string data (string must be MAX 8 chars)
									packet(LIDAR_COMMAND l, int         d   ); // Initialize packet with command and int data
									packet(LIDAR_COMMAND l, double      d   ); // Initialize packet with command and double data
	  char[8]       getData();                                 // Get raw    data from packet
		std::string   getString();                               // Get string data from packet
		int           getInt();                                  // Get int    data from packet
		double        getDouble();                               // Get double data from packet
		LIDAR_COMMAND getCommand();                              // Get command from packet

}// ******************************************************************************************************************************************************************************************
//  ***************************************************************************PLACEHOLDER****************************************************************************************************
//  ******************************************************************************************************************************************************************************************

/**
* class connection
* 
* A serial connection made with a microcontroller
*/
class connection{

  private:
	  int              port;
		CONNECTION_STATE state;

	public:
	                   connection(const config&, const params&);
		CONNECTION_STATE getConnectionState();
		int              getPort();
		bool             send(packet);

}


/*
* FUNCTION PROTOTYPES
*/


/*End of namespace -- any code beyond this bracket is not part of namespace cops*/}
