/**
* LiDAR data input and processing program
*
* Current Version: 0.1
*
* By Alexander Shuping
*
* First Write: 19  August     2016
* Last Change: 26  September  2016
*
* Version History:
*   0.1 -- First version
*
* Operation parameters are specified by command-line arguments
*/

#include <cstdlib>
#include <csignal>

#include <iostream>

#include "ControlComputer.h"
#include "StringOps.h"
#include "CommOps.h"


using std::string;
using std::cout;
using std::endl;

using strops::streq;

int main(int argc, char* argv[]){

	std::signal(SIGINT, handleSafeTermination); // Add a handler for signal-2 termination

	cout << "Parsing parameters and config...\n";
  
	params p;
	config c;
	
	try{
	p = strops::processParams(argc, argv);           // Process command-line parameters
	c = strops::processConfig(p);                    // Load a configuration file
	}catch(strops::MalformedInputException e){
    cout << "\nError found in input: Code " << e.getErrorCode() <<"\n" 
		     << e.getErrorMessage() << "\n\n"
				 << "Execution cannot continue. Halting...\n\n";

		exit(e.getErrorCode());
	}
	cout << "Done!\n";

	if(p.debugMode){
	  strops::dumpConfigAndParams(c, p);                          // If debug mode is enabled, dump all read options
		                                                            //  to verify functionality
	}

  for(int i = 2312; i < 8978; i+=17){
	  cops::packet pak(cops::LIDAR_RSP, i);
		cops::packet pak2(cops::LIDAR_SET, (double)i / 100.0);
		cout << "Assembled packet with command " << pak.getCommand() << " and data " << pak.getInt() << '\n'
		     << "Assembled packet with command " << pak2.getCommand() << " and data " << pak2.getDouble() << '\n'
		     << "   Input datum was " << i << "\n\n";
	}

	/*const int serialPort = establishSerial(p, c);                 // Establish serial communication with Arduino and
	                                                                //  send the config file

	if(serialPort == -1){
    cout << "Could not find a responsive serial port. "
		     << "Are you sure that the Arduino is plugged in "
				 << "and running a compatible program?" << std::endl;

		exit(1);
	}

	if(c.numberOfScans == -1){                                    // If config specifies infinite readings

		while(true){ doScan(p, c); }                                // Scan until signal 2 is caught

	}else{

		for(int i = 0; i < c.numberOfScans; i++){ doScan(p, c); }   // Perform the specified number of scans

	}
	cout << "Config: " << p.configFile;
	if(!p.toSTDOUT){std::cout << " | Output: " << p.outputFile;}
	cout << " | Output to STDOUT: " << p.toSTDOUT
	     << " | Serial port: " << p.serialPort << std::endl;
*/}


/**
* Blank destructor for Exception superclass for polymorphism reasons
*/
Exception::~Exception(){

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

