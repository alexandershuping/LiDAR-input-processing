#pragma once

enum DATA_STATE{
  DATA_NUL,       //No data in structure yet
	DATA_RCV,       //Data received, but not processed
	DATA_PRO,       //Data processed
	DATA_BAD        //Data corrupt
};

struct lidarData{
  
	DATA_STATE state; //State of data -- see enum DATA_STATE above for details
		
	//Unprocessed readings. Do not access unless state is DATA_RCV or DATA_PRO.
	double vHorz;     //Horizontal-axis micromirror voltage
	double vVert;     // Vertical -axis micromirror voltage
	double distance;  //Distance to target point

	//Processed readings. Do not access unless state is DATA_PRO. 
  double x;         //x-coordinate of target point
	double y;         //y-coordinate of target point
	double z;         //z-coordinate of target point

};

//Function Prototypes
void handleSafeTermination(int);
