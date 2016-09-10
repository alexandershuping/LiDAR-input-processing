#ifndef STRINGOPS
#define STRINGOPS

#include <string>
#include <iostream>
#include <fstream>

#include "StringOps.h"
#include "ControlComputer.h"

#define NUMBER_OF_CONFIG_OPTIONS 5 // How many lines does the config file have? (used for config-file processing loops)

//Begin Lib Code

using namespace strops;
using std::cout;
using std::string;
namespace strops{

/**
* bool streq(std::string a, std::string b)
* 
* Checks whether two strings are equal
* 
* PARAMS:
*  std::string a, b -- Strings to check for equality
* 
* RETURN:
*  bool             -- Whether the strings are equal
*/
bool streq(std::string a, std::string b){
  int len;
	if(a.length() == b.length()){  // Check whether the lengths match
    len = a.length();            // If they do, we need to check that many characters.
	}else{
    return false;                // If not, reject. The strings are clearly not equal if they aren't even the same length.
	}

	for(int i = 0; i < len; i++){  // For each character
    if(a.at(i) != b.at(i)){      // Check that it is equal in both strings
      return false;              // If not, reject.
		}
	}
	return true;                   // If the strings have not been rejected at this point, they are equal.
}

/**
* string trimConfigLine(string configLine)
* 
* Trims out all characters before '=' and then any whitespace between '=' and the first non-whitespace character
* Used to parse values from a config file, while leaving the file human-readable
* 
* PARAMS:
*  string configLine -- line of text to parse
* 
* RETURN:
*  string            -- processed string
*/
std::string trimConfigLine(std::string configLine){

  int i = 0;
	int len = configLine.length();
	while(configLine.at(i) != '=' && i+1 < len){ i++; } // Search through the string until '=' is found
	i++;
	while(configLine.at(i) == ' ' && i+1 < len){ i++; } // Discard whitespace after '='
	i++;

  return configLine.substr(i-1, len - (i-1));         // Return the resulting string

}

/**
* void dumpConfigAndParams(config c, params p)
* 
* Debug function to dump parsed config and parameters to STDOUT
* 
* PARAMS:
*  config c -- configuration file to dump to STDOUT
*  params p -- parameters    file to dump to STDOUT
* 
* RETURN:
*  NONE
*/
void dumpConfigAndParams(const config& c, const params& p){
  using std::string;
	using std::cout;

	cout << "Beginning configuration and parameter dump...\n"
	     << "CONFIG:\n"
			 << "  maxVoltage      = " << c.maxVoltage     << '\n'
			 << "  manualReadings  = " << c.manualReadings << '\n'
			 << "  scanType        = " << c.scanType;

			 switch(c.scanType){
         case SCAN_ZIGZAG:
				   cout << " (SCAN_ZIGZAG)\n";
					 break;
			   
				 case SCAN_HORZ_ONLY_ZIGZAG:
				   cout << " (SCAN_HORZ_ONLY_ZIGZAG)\n";
					 break;

				 case SCAN_VERT_ONLY_ZIGZAG:
				   cout << " (SCAN_VERT_ONLY_ZIGZAG)\n";
					 break;

				 default:
				   cout << " (SOMETHING_HAS_GONE_HORRIBLY_WRONG_CALL_THE_DEVS)\n";
			 }

	cout << "  scanResolution  = " << c.scanResolution  << '\n'
	     << "  continuousScans = " << c.continuousScans << '\n'
			 << '\n'
			 << "PARAMS:\n"
			 << "  configFile      = " << p.configFile      << '\n'
			 << "  outputFile      = " << p.outputFile      << '\n'
			 << "  toSTDOUT        = " << p.toSTDOUT        << '\n'
			 << "  serialPort      = " << p.serialPort      << '\n'
			 << '\n'
			 << "End of configuration and parameter dump...\n";
}

/**
* void printUsageInfo()
* 
* Prints a short message to STDOUT explaining the proper command syntax
*/
void printUsageInfo(BAD_TYPE s, int i, std::string str){
  if(s == BAD_COMMANDLINE){ 
    std::cout << "Usage: ControlComputer [-d] [-o <output>] [-s <serial>] -c <config>\n"
		          << "Error code: " << i << '\n'
							<< str << '\n'
              << "Consult the manual for more information.\n\n";
  }else if(s == BAD_CONFIG){
    std::cout << "Bad config file (line " << i << ")!\n"
		          << "Parsed incorrect value \"" << str << "\" from file\n"
		          << "Consult the manual!\n\n";
	}
}

void printUsageInfo(){
  std::cout << "Usage: ControlComputer [-d] [-o <output>] [-s <serial>] -c <config>\n"
            << "Consult the manual for more information.\n\n";
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
  if(par.debugMode){
   	cout << "Opened \"" << par.configFile << "\". Parsing...\n";     
	}

  string configLines[NUMBER_OF_CONFIG_OPTIONS];                    // Define array for config options

	for(int i = 0; i < NUMBER_OF_CONFIG_OPTIONS; i++){               // For every potential config option
    string s;
		
		if(!getline(configFileStream, s)){                             // Take a line from the config file
			printUsageInfo(BAD_CONFIG, -1, "N/A");                       // If there are no more lines, show error and exit
			exit(1);
		}
		
		configLines[i] = trimConfigLine(s);                            // Trim config option to value only
	
  }

  /**
	* PLACE NEW CONFIG OPTIONS HERE
	* DON'T FORGET TO UPDATE NUMBER_OF_CONFIG_OPTIONS AT TOP OF FILE
	*/

	out.maxVoltage = atof(configLines[0].c_str());                   // Assign max voltage value
	
	if(streq(configLines[1], "TRUE")){                               // Assign manual readings value
    out.manualReadings = true;
	}else if(streq(configLines[1], "FALSE")){
    out.manualReadings = false;
	}else{
	printUsageInfo(BAD_CONFIG, 1, configLines[1]);
		exit(1);
	}

	if(streq(configLines[2], "SCAN_ZIGZAG")){                        // Assign scan type value
    out.scanType = SCAN_ZIGZAG;
	}else if(streq(configLines[2], "SCAN_HORZ_ONLY_ZIGZAG")){
    out.scanType = SCAN_HORZ_ONLY_ZIGZAG;
	}else if(streq(configLines[2], "SCAN_VERT_ONLY_ZIGZAG")){
    out.scanType = SCAN_VERT_ONLY_ZIGZAG;
	}else{
    printUsageInfo(BAD_CONFIG, 2, configLines[2]);
		exit(1);
	}

	out.scanResolution = atof(configLines[3].c_str());               // Assign scan resolution value

  if(streq(configLines[4], "TRUE")){                               // Assign continuous scans value
    out.continuousScans = true;
	}else if(streq(configLines[4], "FALSE")){
    out.continuousScans = false;
	}else{
    printUsageInfo(BAD_CONFIG, 4, configLines[4]);
	}

	/* END CONFIG OPTIONS */

	return out;

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
    outParams.debugMode  = false;  // If this is not set (by -d parameter), program will not run in debug mode

  for(int i = 1 /*Not a mistake, argv[0] is skipped because it is just the command name*/; i < argc; i++){
    if(streq(argv[i], "-s")){                         // Parameter to set serial port
      
			try{
        
				if(argv[i+1][0] > '9' || argv[i+1][0] < '0'){ // Sanity check -- First character must be a digit if 
				                                              //  input is a positive integer
          
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
				i++; 
			
			}catch(...){
      
			  cout << "Paramater -o must be followed by a file path\n";
			  printUsageInfo();
        exit(1);
        
			}
		}else if(streq(argv[i], "-d")){                  // Parameter to set debug mode (verbose logging)

		  outParams.debugMode = true;
		
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

//End Lib Code
}

#endif
