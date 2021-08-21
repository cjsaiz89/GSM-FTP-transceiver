
// SIM card info lycaMobile
//#define SECRET_PINNUMBER     "0000"
//#define SECRET_GPRS_APN      "data.lycamobile.com" // replace your GPRS APN
//#define SECRET_GPRS_LOGIN    "" // replace with your GPRS login
//#define SECRET_GPRS_PASSWORD "" // replace with your GPRS password

// SIM card info speedTalk
//#define SECRET_PINNUMBER     ""
//#define SECRET_GPRS_APN      "mnet" // replace your GPRS APN
//#define SECRET_GPRS_LOGIN    "" // replace with your GPRS login
//#define SECRET_GPRS_PASSWORD "" // replace with your GPRS password

// SIM card info h2o wireless
#define SECRET_PINNUMBER     ""
#define SECRET_GPRS_APN      "prodata" // replace your GPRS APN att.mvno
#define SECRET_GPRS_LOGIN    "" // replace with your GPRS login
#define SECRET_GPRS_PASSWORD "" // replace with your GPRS password



// FTP info1
#define SECRET_FTP_HOST_2             "f19-preview.royalwebhosting.net"  // replace with your FTP host server
#define SECRET_FTP_USER_2            "3391543"  // replace with your FTP user
#define SECRET_FTP_PASSWORD_2         "123q789q456q"  // replace with your FTP password
#define SECRET_FTP_PORT_2             21  // replace with your FTP port (optional)
#define SECRET_FTP_REMOTE_DIR_2       "/Radiometer" // replace with your FTP default remote directory

// FTP info2 - IMPORTANT: change 'uint32_t c_connectionTimeout' to 30000 in GSMFTP.h file. Increase other timeouts
#define SECRET_FTP_HOST_1             "ftp.aoml.noaa.gov"  // replace with your FTP host server
#define SECRET_FTP_USER_1             "christian.saiz"  // replace with your FTP user
#define SECRET_FTP_PASSWORD_1         "Kircchoff.072019"  // replace with your FTP password
#define SECRET_FTP_PORT_1             21  // replace with your FTP port (optional)
#define SECRET_FTP_REMOTE_DIR_1       "/phod/incoming/christian.saiz/radiometer01" // replace with your FTP default remote directory


// Assign SIM card info
struct sim{
  const char pin[sizeof(SECRET_PINNUMBER)] = SECRET_PINNUMBER;
  const char apn[sizeof(SECRET_GPRS_APN)] = SECRET_GPRS_APN; // change
  const char login[sizeof(SECRET_GPRS_LOGIN)] = SECRET_GPRS_LOGIN;
  const char psw[sizeof(SECRET_GPRS_PASSWORD)] = SECRET_GPRS_PASSWORD;
} ;
sim card;

// Assign FTP info
struct server{ String host, user, psw, dir; int port; } myftp1,myftp2;
