# LiDAR-input-processing

Command syntax: ControlComputer [-d] [-o <output>] [-s <serial>] -c <config>

  -d
    Activates debug / dump mode, giving more detailed output
  
  -o <output>
    Sends processed points to file at <output>. Without this parameter set, output is sent to STDOUT
  
  -s <serial>
    Establishes connection over serial port <serial>. Without this parameter set, all serial ports are probed for a compatible microcontroller.
  
  -c <config>
    Config file at <config> is parsed for run settings. This parameter is required. See config_example.cfg for an example of a config file.
