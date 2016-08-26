#ifndef STRINGOPS
#define STRINGOPS

#include <string>

namespace strops{
//Begin Lib Code


enum BAD_TYPE{
  BAD_COMMANDLINE,
	BAD_CONFIG
};

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


//End Lib Code
}

#endif
