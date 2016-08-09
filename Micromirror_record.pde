import processing.serial.*;
import javax.swing.*;
import static javax.swing.JOptionPane.*;

final int SERIAL_TIMEOUT = 2000;

final float pi = 3.1415926535;

final boolean throwOutRepeats = false;


final char CHAR_SYN    = 'a';  // Connection test from control computer
final char CHAR_SYNACK = 'b';  // Response to connection test from microcontroller
final char CHAR_ACK    = 'c';  // Confirmation from control computer
final char CHAR_NAK    = 'd';  // Invalid response from control computer
final char CHAR_DAT    = 'e';  // Data request from control computer
final char CHAR_DRYRUN = 'f';  // Request to take dry-run measurements of microcontroller


enum state{
  STATE_DISCONNECTED,
  STATE_STOPPED,
  STATE_INITIALIZING,
  STATE_RUNNING
};

PGraphics win;
state cur_state;
Serial arduinoComms;

PrintWriter logOut;
String logLoc;
long readings = 0;

float[] lastReading = {0,0,0};

final JFileChooser fc = new JFileChooser(); 

int lastVal = 0;
int numRep = 0;


void setup(){
  cur_state = state.STATE_DISCONNECTED;
  System.out.println("Available serial ports: ");
  printArray(Serial.list());
 
  while(true){
		int port = Integer.parseInt(showInputDialog("Choose Serial Port (see output console)"));
		int baud = Integer.parseInt(showInputDialog("Baudrate?"));
  
		arduinoComms = new Serial(this, Serial.list()[port], baud);
  
		if(connect(arduinoComms)){
			break;
		}else{
      arduinoComms.stop();
			System.out.println("Connection timed out. Re-prompting for connection");
		}
  }
  
  int returnVal = fc.showSaveDialog(fc); 
  if (returnVal == JFileChooser.APPROVE_OPTION) { 
    File file = fc.getSelectedFile();
    if(file.exists()){
      if(!showInputDialog("File exists. Overwrite? [Y/N]").equals("Y")){
        exit();
      }
    }
    logLoc = file.getAbsolutePath();
    logOut = createWriter(logLoc);
  } 
  else { 
    println("Cancelled.");
  }
  
  System.out.println("Activating GUI");
  size(200,200);
  win = createGraphics(200,200);
  cur_state = state.STATE_STOPPED;
}

void draw(){
  win.beginDraw();
  System.out.println(cur_state);
  win.background(102);
  win.stroke(255);
  win.fill(cur_state==state.STATE_STOPPED ? 255 : 0);
  win.rect(25,20,150,50);
  win.textSize(20);
  
  win.fill(255,0,0);
  win.rect(180,180,10,10);
  
  String s;
    if(cur_state == state.STATE_STOPPED){
      win.fill(0);
      s = " STOPPED ";
    }else if(cur_state == state.STATE_INITIALIZING){
      win.fill(255);
      s = " INIT... ";
    }else if(cur_state == state.STATE_RUNNING){
      win.fill(255);
      s = "RECORDING";
    }else{
      win.fill(255,0,0);
      s = "ERROR...";
    }
  win.text(s, 30, 55);
  if(cur_state==state.STATE_RUNNING){
    delay(10);
    arduinoFlush(arduinoComms);
    float[] dat_p = getData();
    double[] dat = getCoordsFromAngles(angleFromVolts(dat_p[0]), angleFromVolts(dat_p[1]), dat_p[2]);
    if(!(dat.equals(lastReading)) || !throwOutRepeats){
    win.textSize(10);
    win.text("X="+dat[0]+"\nY="+dat[1]+"\nZ="+dat[2],0,100);
    win.text("Received Vh="+dat_p[0]+",Vv="+dat_p[1]+"Dis="+dat_p[2],0,160);
    String st = dat[0]+","+dat[1]+","+dat[2];
    readings+=st.length();
    win.text(readings+" Bytes Written",0,170);
    win.text("Logging output to: ",0,180);
    win.textSize(5);
    win.text(logLoc,0,190);
    logOut.println(st);
    }
  }else if(cur_state==state.STATE_INITIALIZING){
    
    
    float[] dat_pi;
    double[] dati;
    
      dat_pi = getData();
      dati = getCoordsFromAngles(angleFromVolts(dat_pi[0]), angleFromVolts(dat_pi[1]), (int)dat_pi[2]);
      win.textSize(10);
      win.text("X="+dati[0]+",Y="+dati[1]+"Z="+dati[2],0,100);
      win.text("Received Vh="+dat_pi[0]+",Vv="+dat_pi[1]+"Dis="+dat_pi[2],0,110);
      win.text("Currently initializing... No output yet.",0,125);
    if(abs((int)dat_pi[2] - lastVal)<2){
       numRep++;
       if(numRep>=5);
       win.text("Initialization complete!",0,135);
       cur_state = state.STATE_RUNNING;
    }
    lastVal = (int)dat_pi[2];
    
    
  }
  win.endDraw();
  image(win, 0, 0);
}

void mouseClicked(){
  if((mouseX > 25) & (mouseY > 20) & (mouseX < 175) & (mouseY < 70)){
    if(cur_state == state.STATE_RUNNING){
      cur_state = state.STATE_STOPPED;
    }else{
			cur_state  = state.STATE_INITIALIZING;
    }
  }
  if((mouseX > 180) & (mouseY > 180) & (mouseX < 190) & (mouseY < 190)){
      logOut.flush();
      logOut.close();
      exit();
  }
}

/**
* bool connect(Serial s)
* 
* Establishes connection to the microcontroller
* Handshake structure is as follows:
*  1. Control computer sends "syn" signal to microcontroller
*  2. Microcontroller sends "synack" signal to control computer
*  3. Control computer sends "ack" to microcontroller
* This ensures that both components are responding and can communicate data.
* 
* PARAMS:
*   Serial s -- serial port to connect over
* 
* RETURN:
*   bool     -- Connection state (Successful / timed out)
*/
boolean connect(Serial s){
	s.write(CHAR_SYN);
	for(int i = 0; i < SERIAL_TIMEOUT; i++){
		char c = (char)s.read();
		if(c==CHAR_SYNACK){
			s.write(CHAR_ACK);
			return true;
		}else{
			delay(1);
		}
	}
	return false;
}


float[] getData(){
  arduinoFlush(arduinoComms);
  float[] arr = {0,0,0};
  for(int i = 0; i < 3; i++){
    System.out.println("Waiting for Arduino response");
    while(arduinoComms.available()==0){
      if(cur_state == state.STATE_RUNNING){
        arduinoFlush(arduinoComms);
        arduinoComms.write(CHAR_DAT);
        //delay(100);
      }else if(cur_state == state.STATE_INITIALIZING){
        arduinoFlush(arduinoComms);
        arduinoComms.write(CHAR_DRYRUN);
        //System.out.println(ini);
      }else{
        float[] f = {-1,-1,-1};
        return f;
      }
    }
    
    delay(10);
    System.out.println("Received response. Parsing.");
    String s = arduinoComms.readStringUntil('\n');
    /*if(s == null){
      System.out.println("Erroneous data packet received.");
      arr[i] = -1;
    }else if(s.length()>0){
    arr[i] = Float.parseFloat(s);
    }else{
      System.out.println("Erroneous data packet received.");
      arr[i] = -1;
    }*/
    try{
      arr[i] = Float.parseFloat(s);
    }catch(Exception e){
      System.out.println("Erroneous data packet received.");
      arr[i] = -1;
      
    }
    
  }
  return arr;
  
}

float angleFromVolts(float volts){
  System.out.println(volts+"->"+volts * (pi / 180.0));
  return volts * (pi / 180.0);
}

double[] getCoordsFromAngles(float yaw, float pitch, float distance){
  double x = (distance)*cos(pitch)*sin(yaw);
  double y = (distance)*cos(pitch)*cos(yaw);
  double z = (distance)*sin(pitch);
  double[] arr = {x,y,z};
  return arr;
}

void arduinoFlush(Serial ser){
  while(ser.available()==1){
    ser.read();
  }
}
