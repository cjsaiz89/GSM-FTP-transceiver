# GSM-FTP-transceiver

**Description**

Arduino MKR1400 GSM module - sends data from 2 Eppley Radiometers to an FTP server


**Details**
- Solar powered board (hardware and firmware) that sends data from 2 radiometers (Eppley long and short wave radiation) to an FTP server via GSM cellular network.
Even though this board was specifically designed to condition the signals coming out of the 2 sensors (analog voltages), it can be used as a low cost transmission method at any remote location with 3G coverage for any other sensor type.
- It has 3 modes of operation: *1-SD card + FTP upload* ; *2- SD card* ; *3- Read values to serial monitor*. 
- Sampling *frequency* and *time interval* of each sampled line can be set according to any particular need, as well as the sampling window (*start time* - *end time*).
- It stores the data during the sampling window, and once it is over, it sends the text file (named ddmmyyhh.txt) to the FTP server. Each file is named after the start time and date.
- A H2O data plan allows to send up to 100MB within 3 months, with a cost of (~$10).

- Sampling configuration needs to be specified at the beginning of the set up. GSM network configuration needs to be uploaded with the code.


**Files**
- ZIP file contains the .ino code, headers and instructions. Some standard libraries *.h* and *.c* need to be changed.
- Use *ADC_mode.h* to set ADC resolution, and sampling configuration.
- Use *SIM_FTP.h* to set FTP and SIM card parameters.
- Use *convert_units.h* to change calibration settings. These may change according to the used sensor and PCB components.
