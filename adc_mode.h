// ADC register modes ------------------------------------------------
// averaging AVGCTRL - byte size
#define sample1 B0 // default if not set
#define sample2 B00010001
#define sample4 B00100010
#define sample8 B00110011
#define sample16 B01000100
#define sample32 B01000101
#define sample64 B01000110  // std deviation < 0.0003
#define sample128 B01000111 // std deviation < 0.0001 - USE THIS
#define sample256 B01001000
#define sample512 B01001001
#define sample1024 B01001010

//ADC Resolution CTRLB - word size (same as analogReadResolution(xx) )
#define res16 (( ADC->CTRLB.reg & ~(1<<5) ) | (1<<4)) // 16 bit only for ovs and averaging with more than 1 sample
#define res12 ( ADC->CTRLB.reg & ~(1<<5) )  // 12 bit 
#define res10 (( ADC->CTRLB.reg & ~(1<<5) ) | (1<<5)) // 10 bit
#define res8  (( ADC->CTRLB.reg & ~(1<<5) ) | (1<<4) | (1<<5)) // 8 bit

void configure_adc( byte avgctrl, word ctrlb){
  //choose ADC configuration
  //Results with sampleXX are 8, 10 or 12 bits, but resolution must be set to 16. 
  //Oversampling gives up to 16 bit resolution result but is not accurate
  // avgctrl = sample128; // sampleXX or ovsXX
  // ctrlb = res16; // resXX - set to 16 for ovs and sampleXX>1, if sample1 set res8, 10 or 12
  
  ADC->CTRLB.reg = ctrlb;  
  ADC->AVGCTRL.reg |= avgctrl;
}
