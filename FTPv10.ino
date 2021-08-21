#include "adc_mode.h"
#include "Functions.h" // includes Libraries and SIM_FTP

int ftime; // time per file in minutes
sample_period myhours; // start hour, end hour and delta time
sampling_config myconfig;
String fname; // file name to be saved into sd card
dts tnow; // struct to retrieve current date and time m,d,y,hh,mm,ss
bool gsm_on = false, rtc_on = false;
byte log_mode = 3; // 1: only transmission to ftp, 2: only store in SD card, 3: both
data_line this_sample; // struct of values dates and times
String sdline; // string read from SD card
int prev_min = 100;


void setup() {
  // LED indicator on  
  pinMode(LED_BUILTIN, OUTPUT); // blink while writing a line to the sd card
  blinkled(1,3);
  reduce_pin_power(); // connects pullup resistor to decrease not-in-use pin current
  // Start serial comms
  Serial.begin(9600);
  while (!Serial) ;
  Serial.setTimeout(50);
  welcome();
  
  // main server
  myftp1.host = SECRET_FTP_HOST_1;
  myftp1.user = SECRET_FTP_USER_1;
  myftp1.psw = SECRET_FTP_PASSWORD_1;
  myftp1.port = SECRET_FTP_PORT_1;
  myftp1.dir = SECRET_FTP_REMOTE_DIR_1; 
  // backup server
  myftp2.host = SECRET_FTP_HOST_2;
  myftp2.user = SECRET_FTP_USER_2;
  myftp2.psw = SECRET_FTP_PASSWORD_2;
  myftp2.port = SECRET_FTP_PORT_2;
  myftp2.dir = SECRET_FTP_REMOTE_DIR_2;
  
  // Configure log mode, file time, sampling freq and time time per file, current date-time, and log mode
  log_mode = get_log_mode(); // 1 2 3

  myconfig = config_sampling(); // frequency of sampling and sampling time per lines of data

  //if(log_mode == 1 ) tnow = set_time_mode(rtc_on,gsm_on); else tnow = manual_time(); // get time from network or type manually
  tnow = set_time_mode(rtc_on,gsm_on); 

  // file upload test
  if(log_mode == 1){
    // connect gsm if it wasn't connected
    bool test = ask4test();
    if(test){ if(!gsm_on) gsm_connect(gsm_on); test_upload(myftp1);  test_upload(myftp2); }
  }
  
  // start,end and sampling time per file . Ex. For a 12hs dt -> 720 min. If 1 sample/min -> 720 lines
  if (log_mode == 1 || log_mode == 2) myhours = get_filetime();
  
  // Start GSM module
  //if(log_mode == 1 ) { if(!gsm_on) gsm_connect(gsm_on); } // if it was not connected setting the time, connect now

  // Start SD card
  if(log_mode == 1 || log_mode == 2) { if(!SD.begin(chipSelect)) Serial.println("Card failed, or not present"); else  Serial.println("Card initialized."); }
  
  // Set ADC parameters
  analogReference(AR_EXTERNAL); // V3REF = 3.3V
  configure_adc(sample128,res16); // average mode or oversampling, resolution bits (set to 16 for averaging) 
  
  // shut down until ftp connection
  if(gsmAccess.shutdown()){ Serial.println("GSM disconnected"); gsm_on = false; } else Serial.println("GSM NOT disconnected");
  
  delay(100);  
  
}


void loop() {
  
  // Get file name
  fname = file_name(); // current MMDDYYhh
  int sdcount = 0;
  int prev_sec = 0; // previous second of minute in secuence 
  bool sampled = false, ftp_connected = false, dir_created = false, stream_started = false, ftp_file_closed = false, read_opened = false; 

  // wait and start day sampling at the start of the next minute mm:00
  while( rtc.getSeconds() != 0) ;
  
  // print time in the meantime every minute
  int min_now = rtc.getMinutes();
  if(prev_min != min_now ){ print_time(); prev_min = min_now; }

  if(log_mode == 1 ){
    
    if( check_time_window(myhours) ){ // if within the sampling hours, sample and save
      // Start sampling and saving to SD card. Open to write -> sample -> save
      myfile = open_w_file(fname);// Open file and write header 
      sampled = true;
      
      while( check_time_end(myhours) ){ // Sample and save while within hours. Repeat this n times, with n = myhours.tdelta * 60 / myconfig.stime
        this_sample = get_line( myconfig, prev_sec ); // get one sample with frequency and sample time
        myfile.println( datastring( this_sample ) ); // write string with header format to file, add '\n'
        sdcount++; // keep tabs of number of lines written
        prev_sec = this_sample.date_time.ss;
      }
      myfile.close(); // close file once sampling hours finish. Now start new loop to read and send to ftp
    } // end of IF within hours

    if( sampled == true ){ // if there's something sampled, send it

      gsm_connect(gsm_on); // connect again to start upload
      
      // Upload to ftp-1
      // Open file to read
      Serial.print("Opening " + fname + " ...");
      myfile = open_r_file(fname,read_opened);
      if(read_opened){
        Serial.println("-> " + String(myfile.size()) + " bytes will be uploaded - Connecting to server 1...");
        unsigned long t0 = millis();
        Serial.println("Server 1 - start");
        ftp_connected = connect_ftp_loop(5, myftp1); // try to connect n times      
        if(ftp_connected) dir_created = directory_ftp_loop(5, myftp1); // try to open directory n times      
        if(dir_created) stream_started = start_stream_loop(5, fname); // try to create file and start streaming n times
        if(stream_started) send_loop(sdcount, fname); // file upload loop
        Serial.println(String((millis()-t0)/1000.0) + " seconds to upload");  
        myfile.close(); // close file
        Serial.println("Server 1 - end\n");
      }

      
      // Upload to ftp-2
      ftp_connected = false; dir_created = false; stream_started = false, read_opened = false;
      Serial.print("Opening " + fname + " ...");
      myfile = open_r_file(fname,read_opened);
      if(read_opened){
        Serial.println("-> " + String(myfile.size()) + " bytes will be uploaded - Connecting to server 2...");
        unsigned long t0 = millis();
        Serial.println("Server 2 - start");
        ftp_connected = connect_ftp_loop(5, myftp2); // try to connect n times
        if(ftp_connected) dir_created = directory_ftp_loop(5, myftp2); // try to open directory n times      
        if(dir_created) stream_started = start_stream_loop(5, fname); // try to create file and start streaming n times
        if(stream_started) send_loop(sdcount, fname); // file upload loop
        Serial.println(String((millis()-t0)/1000.0) + " seconds to upload");
        myfile.close(); // close file
        Serial.println("Server 2 - end\n");
      }

      if(gsmAccess.shutdown()) Serial.println("GSM disconnected\n"); else Serial.println("GSM NOT disconnected\n");
      blinkled(2,3);
      Serial.println("Waiting for next time window...");
    }
  } // end of log mode 1
    
  
  // Only save to SD card
  if(log_mode == 2 ){
    // if within the sampling hours, sample and save
    if( check_time_window(myhours) ){
      // Start sampling and saving to SD card. Open to write -> sample -> save
      sampled = true;
      // Open file and write header
      myfile = open_w_file(fname);
      
      // Sample and save while within hours. Repeat this n times, with n = myhours.tdelta * 60 / myconfig.stime 
      while( check_time_end(myhours) ){
        this_sample = get_line( myconfig, prev_sec ); // get one sample string with frequency and sample time
        myfile.println( datastring( this_sample ) ); // write string with header format to file, add '\n'
        sdcount++; // keep tabs of number of lines written
        prev_sec = this_sample.date_time.ss;
      }
            
      // close file once sampling hours finish. Now start new loop to read and send to ftp
      myfile.close();
      blinkled(2,3);
    } // end of IF within hours
  }


  // Only print on serial monitor
  if(log_mode == 3){
    while(1){
      this_sample = get_line( myconfig, prev_sec );
      prev_sec = this_sample.date_time.ss;
    }
  }

  if( sampled ) reduce_pin_power();
  
}
