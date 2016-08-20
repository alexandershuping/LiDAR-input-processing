#pragma once
#include <string>


/**
* ENUMS
*/


//Type of scan to perform
enum SCAN_TYPE{
  SCAN_ZIGZAG,           // 
	SCAN_HORZ_ONLY_ZIGZAG,
	SCAN_VERT_ONLY_ZIGZAG
};

// State of a data packet
enum DATA_STATE{
  DATA_NUL,       // No data in structure yet
	DATA_RCV,       // Data received, but not processed
	DATA_PRO,       // Data processed
	DATA_BAD        // Data corrupt
};


/**
* STRUCTURES
*/


// Data recieved from a single point LiDAR scan
struct lidarData{
  
	DATA_STATE state; // State of data -- see enum DATA_STATE above for details
		
	// Unprocessed readings. Do not access unless state is DATA_RCV or DATA_PRO.
	double vHorz;     // Horizontal-axis micromirror voltage
	double vVert;     //  Vertical -axis micromirror voltage
	double distance;  // Distance to target point

	// Processed readings. Do not access unless state is DATA_PRO. 
  double x;         // x-coordinate of target point
	double y;         // y-coordinate of target point
	double z;         // z-coordinate of target point

};

// Command-line parameters
struct params{
  std::string configFile; // Location of config file
  std::string outputFile; // Location of output file
	bool        toSTDOUT;   // Output to a file or to STDOUT
	int         serialPort; // Serial port to connect over
};

struct config{
  SCAN_TYPE scanType;       // Type of scan to perform
	int       scanResolution; // Resolution of scan (in delta-volts per scan). In config file, this is specified in delta-angle per scan.
	int       numberOfScans;  // Number of complete scans to perform, or -1 to scan until signal 2 is caught
};


/**
* FUNCTION PROTOTYPES
*/


void   handleSafeTermination (int sig)                      ;
params processParams         (int argc, char* argv[])       ;
config processConfig         (const params&)                ;
void   doScan                (const params&, const config&) ;
