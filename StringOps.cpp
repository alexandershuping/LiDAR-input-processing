#ifndef STRINGOPS
#define STRINGOPS

#include <string>
#include <iostream>

#include "StringOps.h"
#include "ControlComputer.h"

//Begin Lib Code


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
bool strops::streq(std::string a, std::string b){
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
std::string strops::trimConfigLine(std::string configLine){

  int i = 0;
	int len = configLine.length();
	while(configLine.at(i) != '=' && i+1 < len){ i++; } // Search through the string until '=' is found
	i++;
	while(configLine.at(i) == ' ' && i+1 < len){ i++; } // Discard whitespace after '='
	i++;

  return configLine.substr(i-1, len - (i-1));               // Return the resulting string

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
void strops::dumpConfigAndParams(const config& c, const params& p){
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
				   cout << " (SOMETHING_HAS_GONE_HORRIBLY_WRONG)\n";
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
void strops::printUsageInfo(strops::BAD_TYPE s, int i, std::string str){
  if(s == strops::BAD_COMMANDLINE){ 
    std::cout << "Usage: ControlComputer [-o <output>] [-s <serial>] -c <config>\n"
		          << "Error code: " << i << '\n'
							<< str << '\n'
              << "Consult the manual for more information.\n\n";
  }else if(s == strops::BAD_CONFIG){
    std::cout << "Bad config file (line " << i << ")!\n"
		          << "Parsed incorrect value \"" << str << "\" from file\n"
		          << "Consult the manual!\n\n";
	}
}

void strops::printUsageInfo(){
  std::cout << "Usage: ControlComputer [-o <output>] [-s <serial>] -c <config>\n"
            << "Consult the manual for more information.\n\n";
}

//End Lib Code


#endif
