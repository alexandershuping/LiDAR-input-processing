/**
* LiDAR data input and processing program
* 
* Current Version: 0.1
* 
* By Alexander Shuping
* 
* First Write: 19 August 2016
* Last Change: 19 August 2016
* 
* Version History:
*   0.1 -- First version
* 
* Operation parameters are specified by command-line arguments
*/

#include <cstdlib>
#include <iostream>
#include <csignal>

#include "ControlComputer.h"

int main(){
  int number_of_readings = -1;
  
  std::signal(SIGINT, handleSafeTermination);
  
	while(true){std::cout << "Testing... terminate this program now...\n";}
  //activateUI();
  //establishSerial();     //Establish serial communication with Arduino 
}

/**
* void handleSafeTermination(int sig)
* 
* Called when signal 2 is caught (by Ctrl-C)
* Closes the program safely (saves and closes any files that are currently being edited)
* This function is required for the program to save data when running in "infinite" mode.
*/
void handleSafeTermination(int sig){
  std::cout << "Signal " << sig << " Caught! Aborting!\n";
	exit(sig);
}
