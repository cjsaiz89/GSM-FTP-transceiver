// Convert raw values from ADC counts to physical units
// ADC values used here are averages, max or min
// Wiring: PSP thermopile(white A-  black B+)  
// Wiring: PIR thermopile(black A-  black C+) ; case thermistor( white D black E) ; dome thermistor (green F red G)

// Calibration constants need to be measured beforehand 
double V3ref = 3300.3; // mV measured at Aref pin coming from U4 (precision V ref)
double V165ref = 1651.2; // mV measured from voltage divider on U4 to rise v_pir voltage at half-Vref
double V5ref = 5001.1; // mV measured from U3 used to measure case and dome RTDs
double Rdiv_dome = 10.0033; // kohm measured on Rb1
double Rdiv_case = 10.0038; // kohm measured on Ra1
double cal_dome = 1.003812; // calibration constant for dome for R measured correction
double cal_case = 1.005318;
double c1 = 0.0010295, c2 = 0.0002391, c3 = 0.0000001568; // RTD to temperature logarithmic constants
double theta = 5.6704E-8 ; // Stefan-Boltzmann constant W.m-2.K-4 
double k = 3.5 ; // dome-case adjustment 3.5 or 4 according to PIR documentation
double max_adc = 4095.0; // adc counts according to resolution used

struct pir{ int a2,a3,a4; };

// A1 - PSP radiometer thermopile voltage - range 0-15 mV
// Amp gain measured: 
// Vin <= 4mV -> Vout=VA1 = 203.12 x Vin + 11.284 & Vin > 4mV -> Vout=VA1 = 201.88 x Vin + 18.181 w/ Vin=VBA=V_PSP in mV
int short_irradiance(double adc_val){
  double k_psp = 8.26E-6 ; // psp constant in V/Wm-2
  double v_psp; // value determined as input coming from psp
  double vread = adc_val * V3ref / max_adc ; // value read in mV  
  if(vread <= 700.0 ) v_psp = (vread - 12.901)/204.64;
  else v_psp = (vread - 5.4963)/205.54;
  
  int swi = v_psp / (k_psp * 1000) ; // short-wave irradiance in Wm-2 - final value as INT  
  return swi;  
}

// A2 - PIR radiometer thermopile - range -2.5 to +2.5 mV
// Amp gain measured: Vout - V165ref = 564.93 x Vin + 90.916 in mV
double net_long(double adc_val){
  double k_pir = 2.81E-6 ; // PIR constant in V/W.m-2
  double v_pir;
  double vread =  adc_val * V3ref / max_adc ; 
  if(vread <= 1365.0) v_pir = ((vread - 1642.7)/563.15)-0.03;
  else if((vread > 1365.0) && (vread <= 1632.0)) v_pir = ((vread - 1640.9)/554.78)-0.03;
  else if(vread > 1632.0 && vread <= 1911.0) v_pir = (vread - 1621.5)/580.49;
  else v_pir = (vread - 1628.6)/565.83;

  double lwi = v_pir / (k_pir * 1000.0); // net long wave irradiance in Wm-2
  return lwi;
}


// RTD measurement
double rtd(double adc_val, double rdiv, double cal){
  double vdiv = adc_val * V3ref / ( max_adc * 1000.0); // voltage divider output read in V
  double rtd = rdiv * cal * 1000.0 / ( V5ref/vdiv - 1); // resistance measured in ohms
  return rtd;
}

// Temperature from RTD in Kelvin
double temp(double rtd){
  return 1/( c1 + c2 * log(rtd*1000.0) + c3 * pow((log(rtd*1000.0)),3));
}


// Incoming long-wave irradiance
// A2: pir thermopile, A3: dome RTD, A4: case RTD
double long_irradiance(pir adc){
  double net = net_long(adc.a2); // irradiance in Wm-2
  double rtd_dome = rtd(adc.a3,Rdiv_dome,cal_dome); // rtd in ohm
  double rtd_case = rtd(adc.a4,Rdiv_case,cal_case);
  double td = temp(rtd_dome); // dome temperature in Kelvin
  double tc = temp(rtd_case);
  double lwi_in = net + theta * pow(tc,4) - k * theta * ( pow(td,4) - pow(tc,4));

  return lwi_in;
}
