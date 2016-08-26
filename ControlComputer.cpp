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
#include <csignal>

#include <iostream>
#include <fstream>

#include "ControlComputer.h"
#include "StringOps.cpp"

#define NUMBER_OF_CONFIG_OPTIONS 5 // How many lines does the config file have? (used for config-file processing loops)


using std::string;
using std::cout;
using std::endl;

using strops::streq;


int main(int argc, char* argv[]){
	
	std::signal(SIGINT, handleSafeTermination); // Add a handler for signal-2 termination

  const params p = processParams(argc, argv);                   // Process command-line parameters
	const config c = processConfig(p);                            // Load a configuration file
	/*const int serialPort = establishSerial(p, c);                 // Establish serial communication with Arduino and send the config file
  
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

	if(argc < 3){                      // ControlComputer + -c + <config> = at least 3 parameters
    cout << "Missing parameters.\n"; // Call cannot possibly be valid if there are <3 parameters, so exit immediately
		printUsageInfo();
		exit(1);
	}

	params outParams;
	  outParams.configFile = "NULL"; // If this is not set (by -c parameter) after all options have been processed, program will exit
		outParams.outputFile = "NULL"; // This only needs to be set if toSTDOUT is false
		outParams.toSTDOUT   = true;   // This is unset if -o is present
		outParams.serialPort = -1  ;   // If this is not set (by -s parameter), program will attempt to auto-detect serial port

  for(int i = 1 /*Not a mistake, argv[0] is skipped because it is just the command name*/; i < argc; i++){
    if(streq(argv[i], "-s")){                         // Parameter to set serial port
      
			try{
        
				if(argv[i+1][0] > '9' || argv[i+1][0] < '0'){ // Sanity check -- First character must be a digit if a positive integer is to be input
          
			    cout << "Paramater -s must be followed by a positive integer\n";
			    printUsageInfo();
          exit(1);
        
				}

			  outParams.serialPort = atoi(argv[i+1]);      // Serial port set
				i++;
			
			}catch(...){                                   // Exception thrown if argv[i+1] is not an int
       
			  cout << "Paramater -s must be followed by an integer\n";
			  printUsageInfo();
        exit(1);
        
			}
		
		}else if(streq(argv[i], "-c")){                  // Parameter to set config file location

			try{
      
			  outParams.configFile = argv[i+1];
				i++;
			
			}catch(...){
       
			  cout << "Paramater -c must be followed by a file path\n";
			  printUsageInfo();
        exit(1);
        
			}
		}else if(streq(argv[i], "-o")){                  // Parameter to set output file location

			try{
      
			  outParams.outputFile = argv[i+1];
				outParams.toSTDOUT   = false;
				i++; // *** *** *** *** *** *** *** *** PLACEHOLDER *** *** *** *** *** *** *** *** *** *** *** *** *** ***
			
			}catch(...){
      
			  cout << "Paramater -o must be followed by a file path\n";
			  printUsageInfo();
        exit(1);
        
			}
		}else{                                           // Parameter is invalid
      cout << argv[i] << " is not a valid parameter for this program.\n";
			printUsageInfo();
			cout	<< "Trying to continue anyway...\n\n";
		}
	}

  if(!streq(outParams.configFile, "NULL") && (!streq(outParams.outputFile, "NULL") || outParams.toSTDOUT)){// Check that all necessary values have been set
    
		return outParams;
	
	}else{

			  cout << "Invalid or missing parameters.\n";
        printUsageInfo();
				exit(1);
	
	}
}

string trimConfigLine(string configLine){

  int i = 0;
	int len = configLine.length();
	while(configLine.at(i) != '=' && i+1 < len){ i++; } // Search through the string until '=' is found
	i++;
	while(configLine.at(i) == ' ' && i+1 < len){ i++; } // Discard whitespace after '='
	i++;

  

  return configLine.substr(i, len - i);               // Return the resulting string

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
config processConfig(const params& par){
  using std::ifstream;

  config out;                                                      // Struct for config options
	ifstream configFileStream;                                       // File stream for config
	
	configFileStream.open(par.configFile.c_str());                   // Open stream, point to specified file
	if(!configFileStream.is_open()){
    cout << "Could not open file \"" << par.configFile << "\"\n";  // Check if file exists, exit if not
		printUsageInfo();
		exit(1);
	}
  
	cout << "Opened \"" << par.configFile << "\". Parsing...\n";     

  string configLines[NUMBER_OF_CONFIG_OPTIONS];                    // Define array for config options

	for(int i = 0; i < NUMBER_OF_CONFIG_OPTIONS; i++){               // For every potential config option
    string s;
		
		if(!getline(configFileStream, s)){                             // Take a line from the config file
			printUsageInfo(strops::BAD_CONFIG);                          // If there are no more lines, show error and exit
			exit(1);
		}
		
		configLines[i] = trimConfigLine(s);                            // Trim config option to value only
	
  }

	out.maxVoltage = atoi(configLines[0].c_str());                   // Assign max voltage value
	
	if(streq(configLines[1], "TRUE")){                               // Assign manual readings value
    out.manualReadings = true;
	}else if(streq(configLines[1], "FALSE")){
    out.manualReadings = false;
	}else{
		printUsageInfo(strops::BAD_CONFIG);
		exit(1);
	}

	if(streq(configLines[2], "SCAN_ZIGZAG")){                        // Assign scan type value
    out.scanType = SCAN_ZIGZAG;
	}else if(streq(configLines[2], "SCAN_HORZ_ONLY_ZIGZAG")){
    out.scanType = SCAN_HORZ_ONLY_ZIGZAG;
	}else if(streq(configLines[2], "SCAN_VERT_ONLY_ZIGZAG")){
    out.scanType = SCAN_VERT_ONLY_ZIGZAG;
	}else{
    printUsageInfo(strops::BAD_CONFIG);
		exit(1);
	}


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

/**
* void printUsageInfo()
* 
* Prints a short message to STDOUT explaining the proper command syntax
*/
void printUsageInfo(strops::BAD_TYPE s){
  if(s == strops::BAD_COMMANDLINE){ 
    cout << "Usage: ControlComputer [-o <output>] [-s <serial>] -c <config>\n"
         << "Consult the manual for more information.\n\n";
  }else if(s == strops::BAD_CONFIG){
    cout << "Bad config file!\n"
		     << "Consult the manual!\n\n";
	}
}

void printUsageInfo(){
  cout << "Usage: ControlComputer [-o <output>] [-s <serial>] -c <config>\n"
       << "Consult the manual for more information.\n\n";
}
