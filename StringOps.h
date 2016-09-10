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
* STRUCTS
*/




/**
* FUNCTION PROTOS
*/
bool        streq               (std::string a, std::string b)       ;
std::string trimConfigLine      (std::string configLine)             ;
void        dumpConfigAndParams (const config&, const params&)       ;
void        printUsageInfo      (strops::BAD_TYPE, int, std::string) ;
void        printUsageInfo      ()                                   ;

}
