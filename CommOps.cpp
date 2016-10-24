#include "CommOps.h"

#include <cmath>
#include <string>
#include <iostream>
#include <thread>          // This is not going to go well.
#include <chrono>
#include <SerialStream.h>

#define DEBUG

using namespace cops;
using namespace LibSerial;
using std::string;
using std::cout;

namespace cops{

/**
* Blank destructor for Exception superclass for polymorphism reasons
*/
Exception::~Exception(){}

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
* Destroy a PacketDataLengthException 
*/
PacketDataLengthException::~PacketDataLengthException(){
}

/**
* Returns an error message corresponding with the error code provided in the constructor.
*
* If the error code does not correspond to any error message, the end-user is prompted to contact the devs, and given the offending error code.
*/
string PacketDataLengthException::getErrorMessage(){

	string s;

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
string PacketDataLengthException::getExceptionType(){return "PacketDataLengthExeption";}

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
packet::packet(LIDAR_COMMAND l, char d[_MAX_PACKET_SIZE]){     // Initialize packet with raw byte data -- array MUST be an exact fit
  _cmd = l;

	for(int i = 0; i > _MAX_PACKET_SIZE; i++){
    _dat[i] = d[i];
	}
}

/**
* Constructor for string data
* 
* string MUST BE <= _MAX_PACKET_SIZE chars long
*/
packet::packet(LIDAR_COMMAND l, string d){
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

/**
* Constructor for long data
*/
packet::packet(LIDAR_COMMAND l, long d){
  _cmd = l;

	packet::_recursiveByteifyInt((unsigned long)d, _dat);

	if(d >= 0){
    _dat[_MAX_PACKET_SIZE-1] = 1;
	}else{
    _dat[_MAX_PACKET_SIZE-1] = 0;
	}

	
}

/**
* Constructor for int data
*/
packet::packet(LIDAR_COMMAND l, int d){
  _cmd = l;
  packet::_recursiveByteifyInt((unsigned long)d, _dat);

	if(d >= 0){
    _dat[_MAX_PACKET_SIZE-1] = 1;
	}else{
    _dat[_MAX_PACKET_SIZE-1] = 0;
	}
}

/**
* Constructor for unsigned long data
*/
packet::packet(LIDAR_COMMAND l, unsigned long d){
  _cmd = l;
	packet::_recursiveByteifyInt(d, _dat);
}

/**
* Constructor for unsigned int data
*/
packet::packet(LIDAR_COMMAND l, unsigned int d){
  _cmd = l;
  packet::_recursiveByteifyInt((unsigned long)d, _dat);
}

/**
* Constructor for double data
* 
* Multiplies doubles by _DOUBLE_PRECISION to convert them to integer types
* 
* Currently inefficient. TODO: make more efficient.
*/
packet::packet(LIDAR_COMMAND l, double d){
  _cmd = l;

	unsigned long o = (unsigned long)(d * _DOUBLE_PRECISION);
	packet::_recursiveByteifyInt(o, _dat);
	if(d >= 0){
    _dat[_MAX_PACKET_SIZE-1] = 1;
	}else{
    _dat[_MAX_PACKET_SIZE-1] = 0;
	}

} 

/**
* Internal function to convert an int to a byte[].
*
* If calling elsewhere in a class function, call with index=0
*
* The return value will always be zero, as long as the function is called with index=0.
*/
unsigned long packet::_recursiveByteifyInt(unsigned long i, unsigned int index, unsigned char buf[_MAX_PACKET_SIZE]){

  unsigned char byteindex = floor((double)index / 8.0);
	unsigned char bitindex  = index % 8;

	if(pow(2,index) > i){
		for(int k = 0; k < _MAX_PACKET_SIZE; k++){buf[k] = 0;}
		return i;

	}

	unsigned long remaining = _recursiveByteifyInt(i, index+1, buf);

	if(remaining >= pow(2,index)){
    buf[byteindex] |= (unsigned char)pow(2,bitindex);
    remaining -= pow(2,index);
	}

	return remaining;

}
void packet::_recursiveByteifyInt(unsigned long i, unsigned char buf[_MAX_PACKET_SIZE]){
  _recursiveByteifyInt(i, 0, buf);
}

/**
* Dummy packet
*/
packet::packet(){
  _cmd = LIDAR_NUL;
	for(int i = 0; i < _MAX_PACKET_SIZE; i++){
    _dat[i] = 0;
	}
}

/**
* Copy constructor
*/
packet::packet(const packet& first){
  _cmd = first._cmd;
  for(int i = 0; i < _MAX_PACKET_SIZE; i++){
    _dat[i] = first._dat[i];
	}
}

/**
* Destructor
*/
packet::~packet(){
}

/**
* Assignment operator
*/
packet& packet::operator=(const packet& first){
  _cmd = first._cmd;
	for(int i = 0; i < _MAX_PACKET_SIZE; i++){
    _dat[i] = first._dat[i];
	}
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
void packet::getData(char buf[_MAX_PACKET_SIZE]) const{
  for(int i = 0; i < _MAX_PACKET_SIZE; i++){
    buf[i] = this -> _dat[i];
	}
}

/**
* Get string data from a packet
*/
string packet::getString() const{
    string s = "";
	for(int i = 0; i < _MAX_PACKET_SIZE; i++){
	  char c = (char) _dat[i];
		const char* ch = &c;
    s.append( ch );
	}

	return s;
}

/**
* Get unsigned long data from a packet
*/
unsigned long packet::getUnsignedLong() const{
  unsigned long out = 0;
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
* Get long data from a packet
*/
long packet::getLong() const{
  bool sign;
  
	if(_dat[_MAX_PACKET_SIZE - 1] == 1){
    sign = true;
	}else{
    sign = false;
	}

  char dat2[_MAX_PACKET_SIZE];
	for(int i = 0; i < _MAX_PACKET_SIZE; i++){
    dat2[i] = _dat[i];
	}

  dat2[_MAX_PACKET_SIZE - 1] = 0;
  
	unsigned long raw = packet(this -> _cmd, dat2).getUnsignedLong();

	return sign ? (long)raw : (long)(raw * -1);
}

/**
* Get unsigned int data from a packet
*/
unsigned int packet::getUnsignedInt() const{
  return (unsigned int)this -> getUnsignedLong();
}

/*
* Get int data from a packet
*/
int packet::getInt() const{
  bool sign;
  
	if(_dat[_MAX_PACKET_SIZE - 1] == 1){
    sign = true;
	}else{
    sign = false;
	}
  
	char dat2[_MAX_PACKET_SIZE];
	for(int i = 0; i < _MAX_PACKET_SIZE; i++){
    dat2[i] = _dat[i];
	}

  dat2[_MAX_PACKET_SIZE - 1] = 0;
  
	unsigned int raw = packet(this -> _cmd, dat2).getUnsignedInt();
  
	return sign ? (int) raw : (int)(raw * -1);
}

/**
* Get double data from a packet
*/
double packet::getDouble() const{
  long raw = this -> getLong();
  return ((double)raw / _DOUBLE_PRECISION);
}

/**
* Get LIDAR_COMMAND from packet
*/
LIDAR_COMMAND packet::getCommand() const{
  
  return this -> _cmd;
}

/**
* Dump packet to string
*/
std::string packet::toString() const{
  std::string s = "";
	s.append("---cops::packet---\n");
	s.append("  Command: ");
	  s += (char)(this -> _cmd+33);
	  s += '\n';
	s.append("  Data:\n");
	for(int i = 0; i < packet::getMaxPacketSize(); i++){
  	s.append("    [");
		  s += (i+48);
			s.append("]: ");
	    s += (this -> _dat[i]+33);
			s += '\n';
	}
	s.append("---END---\n");
	return s;
}

/**
* Class functions for class connection
*/
connection::connection(string port, int baudRate, int timeout){
   _port      = port;
	 _baud_rate = baudRate;
	 _state     = STATE_DISCONNECTED;
	 _timeout   = timeout;
 
}

/**
* Open a connection
*/
bool connection::open(){
  if( !this -> _state == STATE_CONNECTED ){
		_ser.Open( this -> _port );

  SerialStreamBuf::BaudRateEnum b;

	switch( this -> _baud_rate ){
    case 50:
		  b = SerialStreamBuf::BAUD_50;
			break;
		case 75:
		  b = SerialStreamBuf::BAUD_75;
			break;
		case 110:
		  b = SerialStreamBuf::BAUD_110;
			break;
		case 134:
		  b = SerialStreamBuf::BAUD_134;
			break;
		case 150:
		  b = SerialStreamBuf::BAUD_150;
			break;
	  case 200:
		  b = SerialStreamBuf::BAUD_200;
			break;
		case 300:
		  b = SerialStreamBuf::BAUD_300;
			break;
		case 600:
		  b = SerialStreamBuf::BAUD_600;
			break;
		case 1200:
		  b = SerialStreamBuf::BAUD_1200;
			break;
		case 1800:
		  b = SerialStreamBuf::BAUD_1800;
			break;
		case 2400:
		  b = SerialStreamBuf::BAUD_2400;
			break;
		case 4800:
		  b = SerialStreamBuf::BAUD_4800;
			break;
		case 9600:
		  b = SerialStreamBuf::BAUD_9600;
			break;
		case 19200:
		  b = SerialStreamBuf::BAUD_19200;
			break;
		case 38400:
		  b = SerialStreamBuf::BAUD_38400;
			break;
		case 57600:
		  b = SerialStreamBuf::BAUD_57600;
			break;
		case 115200:
		  b = SerialStreamBuf::BAUD_115200;
			break;
		case 230400:
		  b = SerialStreamBuf::BAUD_230400;
			break;
		default:
		  cout << "Invalid Baudrate";
			return 1;
	}

  _ser.SetBaudRate(b);
	_ser.SetCharSize(SerialStreamBuf::CHAR_SIZE_8);
	_ser.SetParity  (SerialStreamBuf::PARITY_NONE);

	if(_ser.IsOpen()){
	  
		if( this -> _doHandshake() ){
      this -> _state = STATE_CONNECTED;
		  return true;
		}else{
		  this -> _state = STATE_ERROR;
			return false;
		}
	
	}else{
    this -> _state = STATE_DISCONNECTED;
		return false;
	}

	}else{
    return false; // Already connected.
	}

}

bool connection::_doHandshake(){
  this -> _state = STATE_CONNECTED;
  packet pak(LIDAR_SYN, 0);
#ifdef DEBUG
cout << "Syn\n";
cout << pak.toString();
#endif
	this -> send(pak);
	bool success = false;
  packet rsp = this -> get(this -> _timeout, success);
#ifdef DEBUG
cout << "Synack\n";
cout << rsp.toString();
#endif
	if(!(success && rsp.getCommand() == LIDAR_SYNACK))
		return false;

	packet pak2(LIDAR_ACK, 0);
	this -> send(pak2);
#ifdef DEBUG
cout << "Handshake good.\n";
#endif

  return true;
}

/**
* Closes the serial port.
* 
* WARNING! THIS FUNCTION WILL CLOSE THE SERIALSTREAM, BUT ONLY IF THE DISCONNECT HANDSHAKE
*  COMPLETES!
* 
* If the serial device is non-responsive, and the SerialStream must be closed, this behavior
*  can be overwritten by setting the 'force' parameter to TRUE.
*/
bool connection::close(bool force){
  packet pak(LIDAR_DISCON, 0);
	this -> send(pak);
	bool success = false;
	packet rsp = this -> get(this -> _timeout, success);
	if(!(success && rsp.getCommand() == LIDAR_DSCACK)){
	  if(force){
		  this -> _ser.Close();               // SerialStream forced closed
			this -> _state = STATE_FORCED_CLOSED;
		}
	  return false;
	}

  this -> _ser.Close();
	this -> _state = STATE_DISCONNECTED;
	return true;
	
}

/**
* Gets conection state (see definitions in CommOps.h for details
*/
CONNECTION_STATE connection::getConnectionState(){
  return _state;
}

/**
* Gets port for current connection (may or may not be open, call getConnectionState to find
*  out
*/
string connection::getPort(){
  return _port;
}

/**
* Gets baud rate for current connection
*/
int connection::getBaudRate(){
  return _baud_rate;
}

/**
* Sends a packet down the wire, and waits for ack from microcontroller
* 
* Returns true if the packet is sent successfully (acknowledged by microcontroller), and false
*  if it is dropped (not acknowledged by microcontroller)
* 
* Note that send() and get() WILL REFUSE TO WORK (i.e. return false) if connection is in any
*  state other than STATE_CONNECTED. Call open() to re-establish the connection.
*/
bool connection::send(const packet& p){
  if(!(this -> _state == STATE_CONNECTED))
	  return false;                           // I'm not joking about this.

#ifdef DEBUG
cout << "Writing " << (char)(p.getCommand()+33) << '\n';
#endif
	(this -> _ser) << p.getCommand();
	char buf[packet::getMaxPacketSize()];
	p.getData(buf);
	for(int i = 0; i < packet::getMaxPacketSize(); i++){
#ifdef DEBUG
cout << "Writing " << (char)(buf[i]+33) << '\n';
#endif
    (this -> _ser) << buf[i];
	}

	bool success = false;
	packet rsp = get(this -> _timeout, success);
	if(!(success && rsp.getCommand() == LIDAR_ACK))
	  return false;
	return true;
}

/**
* Gets a packet from the microcontroller, blocking until packet is received.
*  NOTE: will time out if <timeout> milliseconds pass between bytes being sent.
* 
* Will change the value of <success> based on success state (true if packet is received, false
*  if timed out)
* 
* Note that send() and get() WILL REFUSE TO WORK (i.e. return false) if connection is in any 
*  state other than STATE_CONNECTED. Call open() to re-establish the connection.
* 
* Rules for multi-threading:
*   If the parent or the child is finished with the handle, they must set released to true.
*   This setting must be the LAST access to the handle. THIS IS IMPORTANT TO AVOID ACCESSING
*     NULL POINTERS
*   Before released is set, the thread MUST check whether the other thread has set released
*     to true
*   If the other thread's released is set to true, the first thread must delete() the handle
*   If the other thread's released is set to false, the first CANNOT delete() the handle.
*     (parent sees released == false, so they CANNOT delete(handle). Instead, they must set 
*      released to true, so that when the child process is done with the handle, the child 
*      process will delete(handle) itself)
* 
*   Send an angry email to alexandershuping@ufl.edu if this policy is violated.
*  
*/
packet connection::get(int timeout, bool& success){
  if(this -> _state != STATE_CONNECTED){
#ifdef DEBUG
cout << "Disconnected";
#endif
    success = false;
		return packet();
	}
  thread_io *handle = new thread_io;
	handle -> released      = false;
	handle -> dataAvailable = false;
	handle -> data          = 0x0;
	handle -> ser           = &(this -> _ser);

	std::thread t(_serialThreadBlock, handle);
	t.detach();                         // Avoid blocking main thread

  bool timedOut = true;

	LIDAR_COMMAND cmd = LIDAR_GET;

	for(int i = 0; i < timeout; i++){
    if(handle -> dataAvailable){
#ifdef DEBUG
cout << ">Data Got\n";
#endif
      cmd = (LIDAR_COMMAND)(handle -> data);
			timedOut = false;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
  
	if(timedOut){
#ifdef DEBUG
cout << "Timed out!\n";
#endif
		_releaseHandle(handle);
		success = false;
		return packet();
	}
	
  char dat[packet::getMaxPacketSize()];
	for(int i = 0; i < packet::getMaxPacketSize(); i++){
    for(int j = 0; j < timeout; j++){
      if(handle -> dataAvailable){
#ifdef DEBUG
cout << ">Data Got\n";
#endif
			  dat[i] = handle -> data;
			  handle -> dataAvailable = false;
			  timedOut = false;
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		if(timedOut){
#ifdef DEBUG
cout << "Timed out on the second one!\n";
#endif
	   	_releaseHandle(handle);
			success = false;
			return packet();
		}
	}

	_releaseHandle(handle);
	success = true;
	return packet(cmd, dat);
}

/**
* Called in a new thread
* Blocks for serial data, one byte at a time. When a byte is received, it is placed in the 
*  handle, and the dataAvailable flag is set. It will not put any more data into the handle
*  until the parent thread resets dataAvailable (presumably, after reading the data).
* This prevents the thread from writing more data than the parent thread can read.
*/
void connection::_serialThreadBlock(thread_io* handle){
// TODO TODO TODO TODO TODO TODO TODO TODO TODO
// THIS WILL BREAK ON TIMEOUT! FIX IT OR FIND ANOTHER LIBRARY!
//  TODO TODO TODO TODO TODO TODO TODO TODO TODO
  while(!(handle -> released)){
	  if(!(handle -> dataAvailable)){ // Don't overwrite data until it is used by parent.
		  handle -> data = (char)(handle -> ser) -> get();
			handle -> dataAvailable = true;

#ifdef DEBUG
cout << "---Data Put: " << (char)(handle -> data + 33) << '<';
#endif
		}
	}
#ifdef DEBUG
cout << "---Goodbye Cruel World---\n";
#endif
	_releaseHandle(handle);
}

/**
* Enforces serial policy. 
* If handle has not been released by either thread (i.e. released == false), it sets
*  released to true. If handle has been released by one thread (i.e. released == true), it
*  deletes the handle. Thus, the function must be called twice (once by each thread) to delete
*  the handle, avoiding depressing null pointer errors.
*/
void connection::_releaseHandle(thread_io* handle){
  if(handle == NULL)
	  return;
	
	if(handle -> released){
    delete(handle);
	}else{
    handle -> released = true;
	}
}


/*END OF NAMESPACE -- THERE SHOULD BE NO CODE BEYOND THIS BRACKET -->*/}
