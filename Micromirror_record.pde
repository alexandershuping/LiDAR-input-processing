import processing.serial.*;
import javax.swing.*;
import static javax.swing.JOptionPane.*;


final char eot = 60; //ASCII character for End of Transmission -- sent by Arduino at end of data packet
final char enq = 'e'; //ASCII character for Enquiry             -- sent to test connection
final char ack = 'a'; //ASCII character for Acknowledge         -- Received in response to enq
final char nln = 'd';//ADCII character for line feed           -- sent to get data 

final float pi = 3.1415926535;

final boolean throwOutRepeats = true;

PGraphics win;
boolean recording;
Serial arduinoComms;

PrintWriter logOut;
String logLoc;
long readings = 0;

float[] lastReading = {0,0,0};

final JFileChooser fc = new JFileChooser(); 


void setup(){
  System.out.println("Available serial ports: ");
  printArray(Serial.list());
 
  while(true){
  int port = Integer.parseInt(showInputDialog("Choose Serial Port (see output console)"));
  int baud = Integer.parseInt(showInputDialog("Baudrate?"));
  
  arduinoComms = new Serial(this, Serial.list()[port], baud);
  
  arduinoComms.write(enq);
  do{
    arduinoComms.write(enq);
    delay(500);
  }while(arduinoComms.available()==0);
  char c = (char)arduinoComms.read();
  System.out.println(c);
  if(c == ack){
    System.out.println("Communication established.");
    break;
  }
  System.out.println("Communication with board failed.");
  arduinoComms.stop();
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
  recording = false;
}

void draw(){
  win.beginDraw();
  win.background(102);
  win.stroke(255);
  win.fill(recording ? 255 : 0);
  win.rect(25,20,150,50);
  win.textSize(20);
  
  String s = recording? "RECORDING" : " STOPPED";
  win.fill(255,0,0);
  win.rect(180,180,10,10);
  win.fill(recording ? 0 : 255);
  win.text(s, 30, 55);
  if(recording){
    float[] dat_p = getData();
    double[] dat = getCoordsFromAngles(angleFromVolts(dat_p[0]), angleFromVolts(dat_p[1]), (int)dat_p[2]);
    if(!(dat.equals(lastReading)) || !throwOutRepeats){
    win.textSize(10);
    win.text("X="+dat[0]+",Y="+dat[1]+"Z="+dat[2],0,100);
    win.text("Received Vh="+dat_p[0]+",Vv="+dat_p[1]+"Dis="+dat_p[2],0,110);
    String st = dat[0]+","+dat[1]+","+dat[2];
    readings+=st.length();
    win.text(readings+" Bytes Written",0,125);
    win.text("Logging output to: ",0,150);
    win.textSize(5);
    win.text(logLoc,0,160);
    logOut.println(st);
    }else{
    }
  }
  win.endDraw();
  image(win, 0, 0);
}

void mouseClicked(){
  if((mouseX > 25) & (mouseY > 20) & (mouseX < 175) & (mouseY < 70)){
    if(recording){
      //logOut.flush();
      //logOut.close();
      //exit();
      recording = false;
    }else{
    recording = true;
    }
  }
  if((mouseX > 180) & (mouseY > 180) & (mouseX < 190) & (mouseY < 190)){
      logOut.flush();
      logOut.close();
      exit();
  }
}

float[] getData(){
  arduinoComms.write(nln);
  float[] arr = {0,0,0};
  for(int i = 0; i < 3; i++){
    while(arduinoComms.available()==0){arduinoComms.write(nln);};
    delay(10);
    String s = arduinoComms.readStringUntil('\n');
    s = s.substring(0,s.length()-1);
    arr[i] = (float)(Integer.parseInt(s) / 10.0);
    
  }
  return arr;
  
}

float angleFromVolts(float volts){
  System.out.println(volts+"->"+volts * (pi / 180.0));
  return volts * (pi / 180.0);
}

double[] getCoordsFromAngles(float incl, float xy, int distance){
  double x = ((double)distance)*cos(incl)/**sin(xy)*/;
  double y = ((double)distance)*sin(incl)/**sin(xy)*/;
  double z = ((double)distance)*cos(incl);
  double[] arr = {x,y,z};
  return arr;
}