#pragma once

#include "ControlComputer.h"

namespace strops{

/**
* ENUMS
*/
enum BAD_TYPE{
  BAD_COMMANDLINE,
	BAD_CONFIG
};



/**
* CLASSES
*/

/**
* Class MalformedInputException
* 
* Thrown when input does not match an expected form
* 
* Error codes show what form was expected for the value:
*  6 -- General mismatch
*  1 -- Name of existing file expected
*  2 -- Expected config file to have more lines
*  3 -- Expected "TRUE" or "FALSE"
*  4 -- Expected an integer value
*  5 -- Invalid or missing parameters
*/
class MalformedInputException : public Exception{
  private:
    std::string _badText;
		int         _code;
	public:
	                      MalformedInputException(int code, std::string badText);
												~MalformedInputException();

	  virtual int         getErrorCode();
    virtual std::string getExceptionType();
		virtual std::string getErrorMessage();
};


/**
* FUNCTION PROTOS
*/
bool        streq               (std::string a, std::string b)       ;
std::string trimConfigLine      (std::string configLine)             ;
void        dumpConfigAndParams (const config&, const params&)       ;
void        printUsageInfo      (strops::BAD_TYPE, int, std::string) ;
void        printUsageInfo      ()                                   ;
params      processParams       (int argc, char* argv[])         ;
config      processConfig       (const params& p)                ;

}
