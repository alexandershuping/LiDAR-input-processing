#include "CommOps.h"
#include <iostream>
#include <string>

using std::string;
using std::cout;

bool streq(std::string a, std::string b){
  if(a.length() != b.length())
	  return false;

  for(int i = 0; i < a.length(); i++){
    if(a.at(i) != b.at(i))
      return false;
  }
 
  return true;
}

void printUsage();
											

int main(int argc, char* argv[]){
  if(argc < 7){
    printUsage();
	    return 1;
	}

  int    baud_rate   = 0 ;
  string serial_port = "";
	int    timeout     = 0 ;

  for(int i = 1; i+1 < argc; i+=2){
    if(streq(argv[i], "-b")){
      baud_rate = atoi(argv[i+1]);
		}else if(streq(argv[i], "-p")){
      serial_port = argv[i+1];
		}else if(streq(argv[i], "-t")){
      timeout = atoi(argv[i+1]);
		}
	}
   
  cout << serial_port << '\n';

	cops::connection arduino(serial_port, baud_rate, timeout);
	if(arduino.open()){
    cout << "Connection up!\n";
	}else{
	  cout << "Connection down!\n";
	}

	switch(arduino.getConnectionState()){
    case cops::STATE_DISCONNECTED:
		  cout << "STATE_DISCONNECTED";
			break;
		case cops::STATE_CONNECTED:
		  cout << "STATE_CONNECTED";
			break;
		case cops::STATE_ERROR:
		  cout << "STATE_ERROR";
			break;
		case cops::STATE_FORCED_CLOSED:
		  cout << "STATE_FORCED_CLOSED";
			break;
		default:
		  cout << "invalid state";
	}

	cout << '\n';
  cout << "Trying to disconnect...\n";
	if(arduino.getConnectionState() == cops::STATE_CONNECTED){
    if(arduino.close(false)){
      cout << "Disconnected safely.\n";
			return 0;
		}else{
		  arduino.close(true);
			cout << "Forced disconnection\n";
	}

  }
}
void printUsage(){
  cout << "Usage: PaktTest -b <baudrate> -p <path_to_serial_port> -t <timeout>\n";
}

