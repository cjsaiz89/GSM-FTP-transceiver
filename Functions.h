#include "Libraries.h"
#include "SIM_FTP.h"
#include "convert_units.h"


// PARAMETERS
struct dts{  int m,d,y,hh,mm,ss;  } ;
struct data_line { dts date_time; int psp_avg, psp_min, psp_max, pir_avg, pir_min, pir_max; };
struct sampling_config { int stime, sfreq ; };
struct sample_period { int tstarth,  tendh, tstartm, tendm ; float tdelta, tstart, tend; };
const int chipSelect = 4; // SD card CS
String header = "MMDDYYhhmmss PSP_AVG PSP_MIN PSP_MAX PIR_AVG PIR_MIN PIR_MAX [W.m-2]";



// FUNCTIONS

void welcome(){
  Serial.println("---------- EPPLEY RADIOMETER DATA LOGGER SYSTEM ---------- ");
  Serial.println("Software version FTPv9.ino Mar-2021 ");
  Serial.println("Calibration according to EPPLEY PIR-PSP Radiometer r2 12-2020 PCB - S/N 37741F3 PSP & S/N 38446F3 PIR ");
  Serial.println("Data lines will be shown as follows:");
  Serial.println(header);
  Serial.println("-------------------- ENTER SETTINGS ---------------------- ");
}

void reduce_pin_power(){
  // internal pullup resistor on not used pins
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  digitalWrite(0,HIGH);
  digitalWrite(1,HIGH);
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(7,HIGH);  
  pinMode(A0, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  pinMode(A6, INPUT_PULLUP);
  digitalWrite(A0,HIGH);
  digitalWrite(A5,HIGH);
  digitalWrite(A6,HIGH); 
}

byte get_log_mode(){
  a:
  Serial.println("1) SELECT DATALOGGER MODE:\n[1]-SD card & transmission to ftp, [2]-Only store in SD card, [3]-Print to serial only");
  Serial.println("Note: if [1], data is stored in SD card and sent to the ftp server at the end of each day");
  Serial.print("Enter data logger mode : ");
  while(!Serial.available());
  byte c = Serial.parseInt();
  byte residue = Serial.parseInt();
  Serial.println(c);
  Serial.println("");
  if(c < 1 || c > 3) { Serial.println("Value out of range: re-enter 1, 2, 3 or 4"); goto a; }
  
  return c;
}


// Returns start, end and delta hours of sampling
sample_period get_filetime(){
  sample_period mytime;
  byte residue;
  a:
  Serial.println("2) SELECT START-END TIME FOR SAMPLING \nNote: Consider 1 min of dead time before start sampling if manual time was selected");
  Serial.print("Enter START hour in 24h format: ");
  while(!Serial.available()) ;  // wait for input
  mytime.tstarth = Serial.parseInt(); // int 0-24
  residue = Serial.parseInt();
  Serial.println(mytime.tstarth);
  if(mytime.tstarth < 0 || mytime.tstarth > 23){ Serial.println("Value out of range"); goto a; }
  b:
  Serial.print("Enter START minute: ");
  while(!Serial.available()) ;  // wait for input
  mytime.tstartm = Serial.parseInt(); // int 0-23
  residue = Serial.parseInt();
  Serial.println(mytime.tstartm);
  if(mytime.tstartm < 0 || mytime.tstartm > 59){ Serial.println("Value out of range"); goto b; }
  c:
  Serial.print("Enter END hour in 24h format: ");
  while(!Serial.available()) ;  // wait for input
  mytime.tendh = Serial.parseInt(); // int 0-23
  residue = Serial.parseInt();
  Serial.println(mytime.tendh);
  if(mytime.tendh < 0 || mytime.tendh > 23){ Serial.println("Value out of range"); goto c; }
  d:
  Serial.print("Enter END minute: ");
  while(!Serial.available()) ;  // wait for input
  mytime.tendm = Serial.parseInt(); // int 0-23
  residue = Serial.parseInt();
  Serial.println(mytime.tendm);
  if(mytime.tendm < 0 || mytime.tendm > 59){ Serial.println("Value out of range"); goto d; }

  mytime.tstart = mytime.tstarth + mytime.tstartm/60.0 ;
  mytime.tend = mytime.tendh + mytime.tendm/60.0 ;
  
  if( mytime.tstart > mytime.tend ){
    mytime.tdelta = 24.0 - mytime.tstart + mytime.tend ; }
  else {
    mytime.tdelta = - mytime.tstart + mytime.tend ;
  }
  
  if(mytime.tdelta < 1.0){ 
    Serial.println("One file with " +  String(mytime.tdelta * 60.0) + " minutes of data will be created every day"); }
  else {
    Serial.println("One file with " +  String(mytime.tdelta) + " hours of data will be created every day"); }

  e:
  Serial.print("Correct? [y/n]: ");
  while(!Serial.available()) ;
  char key = Serial.read();
  residue = Serial.parseInt();
  Serial.println(key);
  switch(key){
    case 'n': Serial.println(""); goto a; 
    case 'y': Serial.println(""); break;
    default: Serial.println("Incorrect key"); goto e;
  }  
  return mytime; // return start,end,delta time to sample
}


sampling_config config_sampling(){
  sampling_config mysample;
  Serial.println("3) ENTER SAMPING TIME: Sampling time in seconds to average and store per line of data");
  Serial.println(" --> ex.: 60 sec will create a line of data every 60 seconds");
  a:
  Serial.print("Enter sampling time: ");
  while(!Serial.available()) ;  // wait until serial is sent
  mysample.stime = Serial.parseInt();
  byte residue = Serial.parseInt();
  Serial.println(mysample.stime);
  Serial.flush();
  Serial.println("");
  Serial.println("4) ENTER SAMPLING FREQUENCY: Sampling frequency in Hz to determine averages, min and max within the sampling time");
  Serial.print("Enter sampling frequency: ");
  while(!Serial.available()) ;
  mysample.sfreq = Serial.parseInt();
  residue = Serial.parseInt();
  Serial.println(mysample.sfreq);
  b:
  Serial.print(String(mysample.stime) + " seconds and " + String(mysample.sfreq) + " Hz. Correct? [y/n]:");
  while(!Serial.available()) ;
  char c = Serial.read();
  residue = Serial.parseInt();
  Serial.println(c);
  switch(c){
    case 'n': Serial.println(""); goto a; 
    case 'y': Serial.println(""); break;
    default: Serial.println("Incorrect key"); goto b;
  
  return mysample;  
  }
}


// Formats the date and time in 2 digit format
String two_digit(int number) {
  String n = "";
  if (number < 10) n = "0" + String(number); 
  else  n = String(number);
  
  return n;
}

// Prints a date and time in 2 digit format
void print_date(dts t){
  Serial.println(two_digit(t.d)+"/"+two_digit(t.m)+"/"+two_digit(t.y)+" "+two_digit(t.hh)+":"+two_digit(t.mm)+":"+two_digit(t.ss));
}

// Return date and time from GSM network
dts gsm_time(bool &rtc_on){
  dts t;
  Serial.println("Retrieving time from network...");
  rtc.begin();
  rtc_on = true;
  rtc.setEpoch(gsmAccess.getTime());
  t.d = rtc.getDay();
  t.m = rtc.getMonth();
  t.y = rtc.getYear();
  t.hh = rtc.getHours();
  t.mm = rtc.getMinutes();
  t.ss = rtc.getSeconds();
  Serial.print("UTC time retrieved from network: ");
  print_date(t);
  
  return t;
}


// Connect to the network, show carrier and signal
void gsm_connect( bool &gsm_on){
  Serial.print("Connecting to GSM network... ");
  bool connected = false;
  scannerNetworks.begin();
  while (!connected) {
    if ((gsmAccess.begin(card.pin) == GSM_READY) && (gprs.attachGPRS(card.apn, card.login, card.psw) == GPRS_READY)) {
      connected = true;
      Serial.println("Connected to GSM Network");
      gsm_on = true;
    } else {
      Serial.println("Not connected");
    }
  }
  if(connected){
    Serial.print("Current carrier: ");
    Serial.println(scannerNetworks.getCurrentCarrier());
    Serial.print("Signal Strength: ");
    String str = scannerNetworks.getSignalStrength();
    Serial.print(str.toInt()*100.0/31.0);
    Serial.println(" %");
  }
  GSMFileSystemElem localFile;
  GSMFTPElem remoteFile;
}

// Get integers input from user - used for date and time input
int enterVal() {
  while(!Serial.available()); 
  byte x = Serial.parseInt();
  Serial.println(x);
  byte garbage = Serial.parseInt(); 
  return (int)(x);
}

// Get date and time from user input and not the network
dts manual_time(){
  dts t;
  a:
  do {  Serial.print("Enter day DD: ");   t.d = enterVal();  } while ( t.d > 31 );
  do {  Serial.print("Enter Month MM: ");   t.m = enterVal();  } while ( t.m > 12 );
  do {  Serial.print("Enter Year YY: ");   t.y = enterVal(); } while ( t.y > 99 );
  do {  Serial.print("Enter hour hh [24h]: ");   t.hh = enterVal();  } while ( t.hh > 24 );
  do {  Serial.print("Enter minute mm: ");   t.mm = enterVal();  } while ( t.mm > 59 );
  do {  Serial.print("Enter second ss: ");   t.ss = enterVal();  } while ( t.ss > 59 );
  Serial.print("Time entered: ");
  print_date(t);

  return t;
}

// Set RTC with selected date and time
void set_rtc(dts t,bool rtc_on){
  if(!rtc_on) rtc.begin(); 
  rtc.setTime(t.hh, t.mm, t.ss); 
  rtc.setDate(t.d, t.m, t.y); 
  Serial.println("Clock started...\n");
}


// Select how to get date and time (manual/automatic)
dts set_time_mode(bool &rtc_on, bool &gsm_on){
  dts t;
  a:
  Serial.print("Retrieve UTC time automatically from network [a] or enter manually [m]?: ");
  while(!Serial.available());
  char c = Serial.read();
  byte residue = Serial.parseInt();
  Serial.println(c);
  switch (c){
    case 'a': gsm_connect(gsm_on); t = gsm_time(rtc_on); break;
    case 'm': t = manual_time(); break;
    default: Serial.println("\nWrong key "); goto a;
  }
  set_rtc(t,rtc_on); // set date-time with these values and ask if correct later to avoid delays
  b:
  Serial.print("Correct? [y/n]: ");
  while(!Serial.available());
  c = Serial.read();
  residue = Serial.parseInt();
  Serial.println(c);
  switch (c){
    case 'y': break;
    case 'n': goto a;
    default: Serial.println("\nWrong key "); goto b;
  }
  return t;
}



// Generate file name with date and time of creation 
// 8.3 Format length, otherwise it won't open!
String file_name() {
  String fname = two_digit(rtc.getMonth()) + two_digit(rtc.getDay()) + two_digit(rtc.getYear()) + two_digit(rtc.getHours()) + ".txt";
                                                   
  return fname;
}

bool check_time_window(sample_period myhours){
  bool sample_on = false;
  float tnow = rtc.getHours() + rtc.getMinutes()/60.0 ;
  
  if(myhours.tstart < myhours.tend){
    sample_on = ( (tnow >= myhours.tstart) && (tnow < myhours.tend) ); }
  else {
    sample_on = ( (tnow >= myhours.tstart)&&(tnow < 24.0) || (tnow < myhours.tend) ); } // to cover overnight sampling

    return sample_on;
}

bool check_time_end(sample_period myhours){
  bool sample_on = false;
  float tnow = rtc.getHours() + rtc.getMinutes()/60.0;

  if(myhours.tstart < myhours.tend){
    sample_on = ( tnow < myhours.tend ); }
  else {
    sample_on = ( ((tnow >= myhours.tend)&&(tnow >= myhours.tstart)) || (tnow < myhours.tend)  ); } // to cover overnight sampling

  return sample_on;
}

// Returns a string with all the values of a given data_line type array
String datastring(data_line line){
  return two_digit(line.date_time.m) + two_digit(line.date_time.d) + two_digit(line.date_time.y) +
  two_digit(line.date_time.hh) + two_digit(line.date_time.mm) + two_digit(line.date_time.ss) + " " +
  String(line.psp_avg) + " " + String(line.psp_min) + " " + String(line.psp_max) + " " +
  String(line.pir_avg) + " " + String(line.pir_min) + " " + String(line.pir_max) ;
}

// Get the next round second at which sampling should start.
// This corrects an offset between sample time lines that was being added 
// since wait_time is checked only every 1/freq and can't be precise for low frequencies like 1hz
int next_sec( int prev, int st){
  int next;
  int k = st/60; // 0,1,2...

  if( k < 2){
    if( prev + st >= 60){
      next = prev + st - 60; }
    else {
      next = prev + st;
    } 
  } 
  else{
    next = prev + st - k * 60; }

  return next; 
}

void blinkled(int t, int n){
  for( int i = 0 ; i < n ; i++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(t*1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(t*1000);
  }
}

// ** --------------------------------------------------------------- SAMPLE SECTION -----------------------------------------------------

// Read values at a certain frequency, get max, min, average, and save to a string with date and time
// n: number of samples = freq x stime
data_line get_line(sampling_config myconfig, int prev_sec ) {  

  // wait here until next corresponding second -> tn = tn-1+stime
  while( rtc.getSeconds() != next_sec(prev_sec, myconfig.stime) ); // to avoid offset in seconds wait here until next round integer second
  
  unsigned long t0 = millis(); // start timer
  data_line line; // struct to store line values
  int snum = myconfig.sfreq * myconfig.stime ; // sample number eg.-> 1hz x 60 sec = 60 samples 
  int wait_time = myconfig.stime*1000.0 / ( snum - 1 ); // ms to wait between samples
  
// Get current date and time from RTC BEFORE start sampling  
  line.date_time.d = rtc.getDay();
  line.date_time.m = rtc.getMonth();
  line.date_time.y = rtc.getYear()%100; // YY format
  line.date_time.hh = rtc.getHours();
  line.date_time.mm = rtc.getMinutes();
  line.date_time.ss = rtc.getSeconds();
  
  // average, max, min
  int a1 = 0;
  double sum_lwi = 0.0, max_lwi = 0.0, min_lwi = 99999.0; // pir values
  double sum_swi = 0.0, max_swi = 0.0, min_swi = 99999.0; // psp values
  pir mypir ; // a2,a3,a4
  mypir.a2 = 0;  mypir.a3 = 0;  mypir.a4 = 0;
  
  // loop waiting between samples
  for(int i = 1 ; i < snum ; i++ ){ // skip last sample to adjust time between lines
      
    unsigned long t0_sample = millis();
    // get 1 set of samples and store them
    a1 = analogRead(A1); // psp radiometer
    mypir.a2 = analogRead(A2);
    mypir.a3 = analogRead(A3);
    mypir.a4 = analogRead(A4);
        
    double inst_lwi = long_irradiance(mypir); // PIR instant value Wm-2 , needs to be converted to get max,min,ave
    double inst_swi = short_irradiance(a1); // PSP instant value Wm-2
    // add values to average later
    sum_swi += inst_swi;
    sum_lwi += inst_lwi;
    // Max value    
    if(inst_swi > max_swi) max_swi = inst_swi;
    if(inst_lwi > max_lwi) max_lwi = inst_lwi;

    // Min value
    if(inst_swi < min_swi) min_swi = inst_swi;
    if(inst_lwi < min_lwi) min_lwi = inst_lwi;

    // Show instant values
    Serial.println(String(i) + " -> PSP: "+String(int(inst_swi))+"  PIR: "+String(int(inst_lwi))+" W/m2");
    
    
    // Adjust delay for fixed frequency    
    int dtime = millis() - t0_sample ; // time that the sampling process took
    if( i != snum  ) delay(wait_time - dtime); // wait between samples, substract time spent processing samples. Don't wait on the last sample
    
  } 
  
  // Get average values
  int avg_swi = sum_swi*1.0/(snum-1); 
  int avg_lwi = sum_lwi*1.0/(snum-1); 

  // Assign values to struct and return values  
  line.psp_avg = avg_swi;
  line.psp_min = min_swi;
  line.psp_max = max_swi;
  line.pir_avg = avg_lwi;
  line.pir_min = min_lwi;
  line.pir_max = max_lwi;
  Serial.println( datastring(line) );
  
  return line;
}

void print_time(){
  int hnow = rtc.getHours();
  int mnow = rtc.getMinutes();
  int snow = rtc.getSeconds();
  Serial.println("Now: " + two_digit(hnow) + ":" + two_digit(mnow) + ":" + two_digit(snow) );
}



// ** --------------------------------------------------------------- SD CARD SECTION -----------------------------------------------------
File open_w_file(String fname){
  myfile = SD.open(fname, FILE_WRITE); // create file object
  if( myfile ) Serial.println(fname + " file opened to write"); else Serial.println(fname + " file to write didn't open") ;
  myfile.println(header); 
  return myfile;
}

File open_r_file(String fname, bool &read_opened){
  read_opened = false;
  myfile = SD.open(fname, FILE_READ); // create file object
  if( myfile ){ Serial.println(fname + " file re-opened to read"); read_opened = true;} else Serial.println(fname + " file to read didn't re-open");
  return myfile;
}

String read_SD_line(){
  String txt;
  char a;
  
  for(int i = 0; i < 500; i++){ // set a limit to avoid hanging up
    a = myfile.read();
    txt = txt + a;
    if( a == '\n' ) break; 
  }
  return txt ;
}



// ** --------------------------------------------------------------- TRANSMISSION SECTION -----------------------------------------------------

// Connect to ftp, select directory and disconnect
bool connect_ftp(server a){
  delay(10);
  return ftp.connect(a.host, a.user, a.psw, a.port); ;
}
bool change_dir(server a){  return ftp.cd(a.dir); }
bool close_ftp(){  return ftp.disconnect(); }


// Upload file to server
bool StreamOut(String fname, String data){ //(file name, time per file, n of samples or lines per file )
    int dataLen = data.length();
    char a[data.length() + 1] ; // define the char array used to send data with ftp.streamout
    data.toCharArray( a, dataLen + 1 ); // convert data string to char array and stores it in "a" ,length + '\0'
    int k = strlen(a);
    
    if( ftp.streamOut( a, k ) ) return true; else return false; // ( String, String length )--> string includes "\n" and "\0" for string NULL termination
}

bool send_data(String fname, String data){
  return StreamOut(fname, data ); 
}

// Connect to ftp server
bool connect_ftp_loop(int n, server myftp){
  for(int i = 0 ; i < n ; i++){ // try 5 times 
    if(connect_ftp(myftp)){ 
      Serial.println(String(i+1) + " - Connected to ftp " + myftp.host); 
      return true; }
    else {
      Serial.println(String(i+1) + " - Not connected to ftp " + myftp.host + " - check simultaneous sessions");
      close_ftp(); // try to close it if there was an opened session 
      bool gsm_on;
      if(gsmAccess.shutdown()){ Serial.println("GSM disconnected\n"); gsm_on = false; } else Serial.println("GSM NOT disconnected\n");
      gsm_connect(gsm_on); } // modem on  
      delay(100);
  }
  //gsmAccess.shutdown(); // if it couldn't connect, leave it off
  return false;
}

// Open/create directory on server
bool directory_ftp_loop(int n, server myftp){
  for(int i = 0 ; i < n ; i++){ // try 5 times
    if(change_dir(myftp)){
      Serial.println(String(i+1)+ " - " + myftp.dir + " directory created/opened");
      return true; }
    else {
      Serial.println(String(i+1)+ " - " + myftp.dir + " directory couldn't be created/opened");
      delay(100);
    }
  }
  return false;
}

bool start_stream_loop(int n, String fname){
  for(int i = 0 ; i < n ; i++){ // try 5 times
    if(ftp.streamOutStart(fname)){
      Serial.println(fname + " file created/opened on server ");
      return true; }
    else {
      Serial.println(fname + " file couldn't be created/opened on server");
      delay(100); } 
  }
  return false;
}

bool read_send(int sdcount, String fname, bool &errflag){
  String sdline;
  int ftpcount = 0;
  errflag = false;
  while( myfile.available() ){ // while there's data on the file, read it
    //Read line from SD
    sdline = read_SD_line();
    Serial.println( String(sdline.length()) + " bytes - " + "Line " + String(ftpcount) + "/" + String(sdcount) + ":");
    Serial.print(sdline);
    // send line to file in ftp
    if( send_data( fname, sdline) ){
      Serial.println("OK -> Sent"); } 
    else { 
      Serial.println("ERROR -> NOT sent");
      errflag = true; }        
    ftpcount++;
  } // end of while
  while(true){ if(ftp.streamOutReady()) return true;   } // close file on ftp
}

void send_loop(int sdcount, String fname){
  int attempt = 0;
  restart_upload:
  bool line_error = false;
  
  bool ftp_file_closed = read_send(sdcount, fname, line_error); // read each line from sd card and send it to the ftp
  
  if(line_error){
    attempt++;
    Serial.println("Some lines were not uploaded");
    delay(1000); // wait 10 seconds and retry
    if(attempt < 5) goto restart_upload; } // try to upload completely 5 times
  
    if(close_ftp()) Serial.println("ftp closed"); else Serial.println("ftp not closed ");  
//  unsigned long tf = millis();
//  Serial.println(String((tf-t0)/1000.0) + " seconds to upload");

  if(line_error) Serial.println("Finished incomplete file"); else Serial.println("Finish complete file");
}

void test_upload( server myftp){
  Serial.println("Starting test file upload to " + myftp.host);
  
  String file = "test.txt";
  SD.begin(chipSelect);
   
  myfile = SD.open(file, FILE_READ);
  if(myfile){
    Serial.println(file + " Opened");
    if(connect_ftp_loop(2, myftp))
      if(directory_ftp_loop(2, myftp))
        if(start_stream_loop(2, file)) send_loop(6, file);
    myfile.close();
  } else Serial.println("Couldn't open the test file");
  
  Serial.println("End of test upload to" + myftp.host + "\n");
  
}

bool ask4test(){
  e:
  Serial.print("\nUpload test file? [y/n]: ");
  while(!Serial.available()) ;
  char key = Serial.read();
  byte residue = Serial.parseInt();
  Serial.println(key);
  Serial.println("");
  switch(key){
    case 'n': return false; 
    case 'y': return true;
    default: Serial.println("Incorrect key"); goto e;
  } 
}
