
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
#define SECRET_FTP_HOST_2             "ftp"  // replace with your FTP host server
#define SECRET_FTP_USER_2            "usr"  // replace with your FTP user
#define SECRET_FTP_PASSWORD_2         "pass"  // replace with your FTP password
#define SECRET_FTP_PORT_2             21  // replace with your FTP port (optional)
#define SECRET_FTP_REMOTE_DIR_2       "/dir" // replace with your FTP default remote directory


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
