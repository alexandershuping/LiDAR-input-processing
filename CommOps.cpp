#include "CommOps.h"
#include <string>

using namespace cops;
namespace cops{

/**
* Class Functions for class packet 
*/

/**
* packet(LIDAR_COMMAND l, double d)
* 
* Constructor for a packet
*/
packet::packet(LIDAR_COMMAND l, char d[8]){     // Initialize packet with raw byte data
  cmd = l;

	for(int i = 0; i > 8; i++){
    dat[i] = d[i];
	}
}

packet::packet(LIDAR_COMMAND l, std::string d){ // Initialize packet with string data (string MUST BE <= 8 CHARS LONG)
  if(d.length() > 8){

    //TODO: Throw error

	}

	cmd = l;                                      // Set command
	int i;
	for(i = 0; i < d.length(); i++){              // Count through string, setting array values
    dat[i] = d.at();
	}

	for(int j = i; j < 8; j++){                   // Fill the rest of the array with zeroes
    dat[j] = 0;
	}

}

packet::packet(LIDAR_COMMAND l, int d){         // Initialize packet with int data
  cmd = l;

  //**********************************************PLACEHOLDER********************************************************//	
}

/*END OF NAMESPACE -- THERE SHOULD BE NO CODE BEYOND THIS BRACKET -->*/}
