#include "CommOps.h"
#include <cmath>
#include <string>
#include <iostream>

using namespace cops;
namespace cops{

/**
* Class functions for class PacketDataLengthException
*/

/**
* Initializes a PacketDataLengthExeption
*
* PARAMS:
*   int code     -- Error code, see class description in CommOps.h for description of codes
*   int length   -- Length of erroneous data
*   int expected -- Expected length for data
*/
PacketDataLengthException::PacketDataLengthException(int code, int length, int expected){
  _error_code = code;
	_length     = length;
	_expected   = expected;
}

/**
* Returns an error message corresponding with the error code provided in the constructor.
*
* If the error code does not correspond to any error message, the end-user is prompted to contact the devs, and given the offending error code.
*/
std::string PacketDataLengthException::getErrorMessage(){

	std::string s;

	switch(this -> _error_code){
    case 0:
		  s = "Expected packet data of at most ";
			s.append(std::to_string(this -> _expected));
			s.append(" bytes. Received ");
			s.append(std::to_string(this -> _length));
			s.append(" bytes.");
			break;

    case 1:
		  s = "Expected packet data of at least ";
			s.append(std::to_string(this -> _expected));
			s.append(" bytes. Received ");
			s.append(std::to_string(this -> _length));
			s.append(" bytes.");
			break;

    case 2:
		  s = "Expected packet data of exactly ";
			s.append(std::to_string(this -> _expected));
			s.append(" bytes. Received ");
			s.append(std::to_string(this -> _length));
			s.append(" bytes.");
			break;

		default:
		  s = "Error code ";
			s.append(std::to_string(this -> _error_code));
			s.append(" has no corresponding error message! This is bad! Contact the devs!");
			s.append("\n  Expected = ");
			s.append(std::to_string(this -> _expected));
			s.append("\n  Length   = ");
			s.append(std::to_string(this -> _length));
	}

	return s;
}

/**
* Returns the type of exception, which should be a string with the class name in it.
*/
std::string PacketDataLengthException::getExceptionType(){return "PacketDataLengthExeption";}

int PacketDataLengthException::getErrorCode()      {return this -> _error_code;}
int PacketDataLengthException::getLength()         {return this -> _length;}
int PacketDataLengthException::getExpectedLength() {return this -> _expected;}



/**
* Class Functions for class packet
*/

/**
* packet(LIDAR_COMMAND l, double d)
*
* Constructor for a packet
*/
packet::packet(LIDAR_COMMAND l, char d[_MAX_PACKET_SIZE], bool raw){     // Initialize packet with raw byte data -- array MUST be an exact fit
  _cmd = l;

	for(int i = 0; i > _MAX_PACKET_SIZE; i++){
    _dat[i] = d[i];
	}
}

packet::packet(LIDAR_COMMAND l, std::string d){ // Initialize packet with string data (string MUST BE <= _MAX_PACKET_SIZE CHARS LONG)
  if(d.length() > _MAX_PACKET_SIZE){
    throw PacketDataLengthException(0, d.length(), _MAX_PACKET_SIZE); // String length (d.length()) was longer (error code 0) than expected length (this._MAX_PACKET_SIZE)
	}

	_cmd = l;                                                // Set command
	unsigned int i;
	for(i = 0; i < d.length(); i++){                         // Count through string, setting array values
    _dat[i] = d.at(i);
	}

	for(unsigned int j = i; j < _MAX_PACKET_SIZE; j++){      // Fill the rest of the array with zeroes
    _dat[j] = 0;
	}

}

packet::packet(LIDAR_COMMAND l, int d){         // Initialize packet with int data
  _cmd = l;
  unsigned char b[_MAX_PACKET_SIZE];
  for(int i = 0; i < _MAX_PACKET_SIZE; i++){
    this -> _dat[i] = 0;
  }
  packet::_recursiveByteifyInt(d, 0, this -> _dat);
}

packet::packet(LIDAR_COMMAND l, double d){
  
} 

/**
* Internal function to convert an int to a byte[].
*
* If calling elsewhere in a class function, call with index=0
*
* The return value will always be zero, as long as the function is called with index=0.
*/
int packet::_recursiveByteifyInt(int i, unsigned int index, unsigned char buf[_MAX_PACKET_SIZE]){

  unsigned char byteindex = floor((double)index / 8.0);
	unsigned char bitindex  = index % 8;

	if(pow(2,index) > i){
		for(int k = 0; k < _MAX_PACKET_SIZE; k++){buf[k] = 0;}
		return i;

	}

	int remaining = _recursiveByteifyInt(i, index+1, buf);

	if(remaining >= pow(2,index)){
    buf[byteindex] |= (unsigned char)pow(2,bitindex);
    remaining -= pow(2,index);
	}



	return remaining;

}

/**
* Get the maximum amount of data that can be stored in a packet
*/
int packet::getMaxPacketSize(){
  return _MAX_PACKET_SIZE;
}

/**
* Get raw data from a packet, storing it in buf
*/
void packet::getData(char buf[_MAX_PACKET_SIZE]){
  for(int i = 0; i < _MAX_PACKET_SIZE; i++){
    buf[i] = this -> _dat[i];
	}
}

/**
* Get string data from a packet
*/
std::string packet::getString(){
    std::string s = "";
	for(int i = 0; i < _MAX_PACKET_SIZE; i++){
	  char c = (char) _dat[i];
		const char* ch = &c;
    s.append( ch );
	}

	return s;
}

/**
* Get int data from a packet
*/
int packet::getInt(){
  int out = 0;
  for(int byteindex = 0; byteindex < _MAX_PACKET_SIZE; byteindex++){
	  for(int bitindex = 0; bitindex < 8; bitindex++){
      if( (this -> _dat[byteindex] & (unsigned char)pow(2, bitindex)) != 0 ){
        out += pow(2, byteindex * 8 + bitindex);
			}
		}
	}

	return out;
}

/**
* Get LIDAR_COMMAND from packet
*/
LIDAR_COMMAND packet::getCommand(){
  
  return this -> _cmd;
}

/*END OF NAMESPACE -- THERE SHOULD BE NO CODE BEYOND THIS BRACKET -->*/}
