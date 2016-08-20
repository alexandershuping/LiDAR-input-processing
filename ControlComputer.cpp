/**
* LiDAR data input and processing program
* 
* Current Version: 0.1
* 
* By Alexander Shuping
* 
* First Write: 19 August 2016
* Last Change: 20 August 2016
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

bool streq(std::string a, std::string b){
  int len;
	if(a.length() == b.length()){
    len = a.length();
	}else{
    return false;
	}

	for(int i = 0; i < len; i++){
    if(a.at(i) != b.at(i)){
      return false;
		}
	}
	return true;
}

int main(int argc, char* argv[]){
  std::signal(SIGINT, handleSafeTermination); // Add a handler for signal-2 termination

  const params p = processParams(argc, argv);                   // Process command-line parameters
	/*const config c = loadConfig(&p);                              // Load a configuration file
	establishSerial(&p, &c);                                      // Establish serial communication with Arduino and send the config file
  
	if(c.numberOfScans == -1){                                    // If config specifies infinite readings
    
		while(true){ doScan(&p, &c); }                              // Scan until signal 2 is caught
	
	}else{
    
		for(int i = 0; i < c.numberOfScans; i++){ doScan(&p, &c); } // Perform the specified number of scans
	
	}*/
	std::cout << "Config: " << p.configFile;
	if(!p.toSTDOUT){std::cout << " | Output: " << p.outputFile;}
	std::cout << " | Output to STDOUT: " << p.toSTDOUT
	          << " | Serial port: " << p.serialPort << std::endl;
}

/**
* params processParams(int argc, char* argv[])
*
* Processes command-line arguments
* 
* PARAMS:
*  int argc     -- argument count
*  char* argv[] -- argument vector (array of arguments)
* 
* RETURN:
*  params       -- structure containing processed parameters
* 
* ERRORS:
*  Prints usage and force-exits if the program is given improper parameters
*/
params processParams(int argc, char* argv[]){
  if(argc < 3){
    std::cout << "Missing parameters.\n"
		          << "Usage: ControlComputer [-o <output>] [-s <serial>] -c config\n"
		          << "Run man ControlComputer for more information\n\n";
		exit(1);
	}

	params outParams;
	  outParams.configFile = "NULL";
		outParams.outputFile = "NULL";
		outParams.toSTDOUT   = true;
		outParams.serialPort = -1  ;

  for(int i = 1 /*Not a mistake, argv[0] is skipped because it is just the command name*/; i < argc; i++){
    if(streq(argv[i], "-s")){
      
			try{
      
			  outParams.serialPort = atoi(argv[i+1]);
				i++;
			
			}catch(...){
       
			  std::cout << "Paramater -s must be followed by an integer\n"
			            << "Usage: ControlComputer [-o <output>] [-s <serial>] -c config\n"
			            << "Run man ControlComputer for more information\n\n";
        exit(1);
        
			}
		
		}else if(streq(argv[i], "-c")){

			try{
      
			  outParams.configFile = argv[i+1];
				i++;
			
			}catch(...){
       
			  std::cout << "Paramater -c must be followed by a file path\n"
			            << "Usage: ControlComputer [-o <output>] [-s <serial>] -c config\n"
			            << "Run man ControlComputer for more information\n\n";
        exit(1);
        
			}
		}else if(streq(argv[i], "-o")){

			try{
      
			  outParams.outputFile = argv[i+1];
				outParams.toSTDOUT   = false;
				i++;
			
			}catch(...){
       
			  std::cout << "Paramater -o must be followed by a file path\n"
			            << "Usage: ControlComputer [-o <output>] [-s <serial>] -c config\n"
			            << "Run man ControlComputer for more information\n\n";
        exit(1);
        
			}
		}else{
      std::cout << argv[i] << " is not a valid parameter for this program.\n"
			          << "Usage: ControlComputer [-o <output>] [-s <serial>] -c config\n"
			          << "Trying to continue anyway...\n\n";
		}
	}

  if(!streq(outParams.configFile, "NULL") && (!streq(outParams.outputFile, "NULL") || outParams.toSTDOUT)){
    
		return outParams;
	
	}else{

			  std::cout << "Invalid or missing parameters.\n"
			            << "Usage: ControlComputer [-o <output>] [-s <serial>] -c config\n"
			            << "Run man ControlComputer for more information\n";
        exit(1);
	
	}
}

/**
* config processConfig(const params&)
* 
* Opens and processes the given config file
* 
* PARAMS:
*  const params& -- Const reference to the commandline parameters
* 
* RETURN:
*  const config& -- Const reference to the processed config options
* 
* ERRORS:
*  Prints usage and exits if config file is improperly-formatted
*/
config processConfig(const params&){
  
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
