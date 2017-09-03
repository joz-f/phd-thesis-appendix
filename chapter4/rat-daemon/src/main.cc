#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <malloc.h>
#include <signal.h>
#include <syslog.h>
#include <getopt.h>

#include "rfid_serial.hpp"
#include "rfid_mysql.hpp"
#include "string_utils.hpp"
#include "email.hpp"

using namespace std;

struct RFIDOptions {
  string  device_name;
  int     device_descriptor;
  int     device_num;
  int     poll_sleep;
  bool    daemon;
  bool    arduino;
  bool    mysql;
  bool    mysql_remote;
  bool    verbose;
  string  mysql_con;
  string  mysql_remote_con;
  string  mysql_usr;
  string  mysql_pwd;
  string  mysql_db;
  string  email_addr;
  int     noise_level;
  float   noise_scale;
};

// We keep the global options on the heap so signals can access them
RFIDOptions *OPTIONS;

// A couple of global mysql connection data thingies

mysql::Data * MYSQL_LOCAL = nullptr;
mysql::Data * MYSQL_REMOTE = nullptr;

// Close the descriptor and log us out
int quit(int code) {
  cout  << s9::DateTimeStamp() << ":" << "Closing device: " << OPTIONS->device_name << endl;
  serial::close_port(OPTIONS->device_descriptor); 
  
  if (OPTIONS->mysql){
    mysql::Disconnect(MYSQL_LOCAL);
  }

  if (OPTIONS->mysql_remote){
    mysql::Disconnect(MYSQL_REMOTE);
  }
  
  cout  << s9::DateTimeStamp() << ":"  << OPTIONS->device_name << " closed with exit status " << s9::ToString(code) << endl;

  free(OPTIONS);
  exit(code);
  return code;
}


// Catch signals for a clean exit
void sig_term_handler(int signum, siginfo_t *info, void *ptr) {
  // Cast the pointer to our state 
  cout << s9::ToString(signum) << " received." << endl;
  // We return 1 because a sigterm should really not happen
  quit(1);
}

void sig_term_handler2(int signum) {
  // Cast the pointer to our state 
  cout << s9::ToString(signum) << " received." << endl; 
  // We return 1 because a sigterm should really not happen
  quit(1);
}

// Setup signal catching - pass in the state that we need
void catch_sigterm(void) {
  
  cout << "SIGTERM received." << endl;
  // We return 1 because a sigterm should really not happen
  quit(1);

/*  static struct sigaction _sigact;

  memset(&_sigact, 0, sizeof(_sigact));
  _sigact.sa_sigaction = sig_term_handler;
  _sigact.sa_flags = SA_SIGINFO;
  sigaction(SIGTERM, &_sigact, NULL);
*/
}

// Poll and parse the RFID Sensors

void _poll_rfid() {
  //std::string data = serial::rfid_write(OPTIONS->device_descriptor,"AVID 123*456*789\r");
  std::string data;
  int result = serial::rfid_read(OPTIONS->device_descriptor, data); 
  // We quit if the result is less than 0 so launchd can re-launch the process
  if (result < 0) {
    send_email(OPTIONS->email_addr, "molerat RFID read failure: " + s9::DateTimeStamp(), "The daemon running device " + OPTIONS->device_name + " has quit due to a failed read. Launchd will attempt to restart but do check to make sure it is running.\nCheers\n'lil daemon");
    quit(70);
  }

  if (data.length() > 0) {
    if (OPTIONS->verbose) {
      cout << OPTIONS->device_name << " data: " << data << endl; 
    }

    // Now parse the data 
    if (s9::StringContains(data,"NO ID FOUND") == false) {
      data = s9::rtrim(data);
      if (s9::StringContains(data,"AVID")) {
        data = s9::StringRemove(data,"AVID*");
        //vector<string> parsed = s9::SplitStringChars(data,"*");
        // Concat data
        //if (parsed.size() == 3) {
        //  data = parsed[0] + parsed[1] + parsed[2];   
        //}
         
        if (OPTIONS->mysql){
          if (mysql::WriteRFIDData(MYSQL_LOCAL, OPTIONS->device_num, data) == -1){
            cerr << "Failed to write data to mysql for sensor " << OPTIONS->device_name << endl; 
          
            send_email(OPTIONS->email_addr, "molerat mysql write failure: " + s9::DateTimeStamp(), "The daemon running device " + OPTIONS->device_name + " has quit due to not being able to write to mysql. Launchd will attempt to restart but do check to make sure it is running.\nCheers\n'lil daemon");
            quit(70);
          }
        }
     
        if (OPTIONS->mysql_remote){
          if (mysql::WriteRFIDData(MYSQL_REMOTE, OPTIONS->device_num, data) == -1){
            cerr << "Failed to write data to remote mysql for sensor " << OPTIONS->device_name << endl; 
          }
        }
      }
    }

    if (OPTIONS->verbose) {
      cout << s9::DateTimeStamp() << ":" << OPTIONS->device_name << " data parsed: " << data << endl; 
    }
  }
}

// Poll and parse the one Arduino we have

void _poll_arduino() {
  std::string data;
  int result = serial::sensor_read(OPTIONS->device_descriptor, data);
  if (data.length() > 0) {
    
    if (OPTIONS->verbose) {
      cout  << s9::DateTimeStamp() << ":" << OPTIONS->device_name << " data: " << data << endl;
    }

    if (OPTIONS->mysql){
      if (mysql::WriteSensorData(MYSQL_LOCAL, data, OPTIONS->noise_level, OPTIONS->noise_scale) == -1){
        cerr << "Failed to write data to mysql for sensor " << OPTIONS->device_name << endl; 
       
        send_email(OPTIONS->email_addr, "molerat mysql write failure: " + s9::DateTimeStamp(), "The daemon running device " + OPTIONS->device_name + " has quit due to not being able to write to mysql. Launchd will attempt to restart but do check to make sure it is running.\nCheers\n'lil daemon");
        quit(70);

      }
    }
   
    if (OPTIONS->mysql_remote){
      if (mysql::WriteSensorData(MYSQL_REMOTE, data, OPTIONS->noise_level, OPTIONS->noise_scale) == -1){
        cerr << "Failed to write data to remote mysql for sensor " << OPTIONS->device_name << endl; 
      }
    }
  }
}


// An RFID / Sensor polling script (one daemon for each sensor?)
// Poll the serial devices - the RFID in this case
void poll() {
  while (true) {
    // While testing with our arduino, we send data and see what we get back     
    // TODO - replace with a proper read 
    if (OPTIONS->arduino){
      _poll_arduino();
    } else {
      _poll_rfid();
    }

    usleep(OPTIONS->poll_sleep);
  }
  quit(0);
}

int ParseCommandOptions(int argc, const char * argv[]) {
  int c;
  int digit_optind = 0;
  static struct option long_options[] = {
      {"device_name", 1, 0, 0}, 
      {"device_name", 0, 0, 0},
      {NULL, 0, NULL, 0}
  };
  int option_index = 0; 

  while ((c = getopt_long(argc, (char **)argv, "vad:f:c:u:p:b:n:r:x:s:z:?h", long_options, &option_index)) != -1) {
  	int this_option_optind = optind ? optind : 1;
  	switch (c) {
      case 0 :
        break;
      case 'f':       
        OPTIONS->device_name = std::string(optarg);
        break;
      case 'd':
        OPTIONS->daemon = true;
        break;
      case 'h':
        cout << "Usage: molerat_rfid [-f <device path>] [-n <device number>] [-d daemonize] [-a use arduino instead of rfid] [-c <mysql connect string>] [-u <mysql user>] [-p <mysql password>] [-b <mysql db name>] [-r <remote mysql connect string>] [-v verbose] [-x poll_delay in usec] [-e email address] [-z noise level] [-s noise scale]" << endl;
        return -1;
        break;
      case 'c':
        OPTIONS->mysql_con = std::string(optarg);
        OPTIONS->mysql = true;
        break;
      case 'u':
        OPTIONS->mysql_usr = std::string(optarg);
        break;
      case 'p':
        OPTIONS->mysql_pwd = std::string(optarg);
        break;
      case 'b':
        OPTIONS->mysql_db = std::string(optarg);
        break;
      case 'r':
        OPTIONS->mysql_remote = true;
        OPTIONS->mysql_remote_con = std::string(optarg);  
        break;
      case 'v':
        OPTIONS->verbose = true;
        break;
      case 'n':
        OPTIONS->device_num = s9::FromString<int>(std::string(optarg));
        break;
      case 'x':
        OPTIONS->poll_sleep = s9::FromString<int>(std::string(optarg));
        break;
      case 'a':
        OPTIONS->arduino = true;
        break;
      case 'e':
        OPTIONS->email_addr = std::string(optarg);
        break;
      case 'z':
        OPTIONS->noise_level = s9::FromString<int>(std::string(optarg)); 
        break;
      case 's':
        OPTIONS->noise_scale = s9::FromString<float>(std::string(optarg)); 
        break;
     default : 
        break;
    }
  }
  return 0;
}

// Main entry point. Deals mostly with setting up the daemon
int main(int argc, const char **argv) {
  
  // Parse the command line options
  OPTIONS = new RFIDOptions();

  // Defaults
  OPTIONS->daemon = false;
  OPTIONS->device_name = "/dev/ttyUSB0";
  OPTIONS->mysql_remote = false;
  OPTIONS->mysql_db = "test";
  OPTIONS->mysql_usr = "root";
  OPTIONS->mysql_pwd = "none";
  OPTIONS->mysql_con = "tcp://127.0.0.1:3306";
  OPTIONS->poll_sleep = 100000;
  OPTIONS->verbose = false;
  OPTIONS->email_addr = "julie@translatingnature.org";
  OPTIONS->noise_level = 20;
  OPTIONS->noise_scale = 0.5;

  if (ParseCommandOptions(argc, argv) != 0 ) {
    return 0;
  }

  // Firstly, open the log
  openlog("molerat_rfid",LOG_PID|LOG_CONS|LOG_PERROR, LOG_USER);
  syslog(LOG_INFO, "Launching molerat_rfid process");
  // Email to say we are starting up
  send_email(OPTIONS->email_addr, "Molerat process is starting up:  " + s9::DateTimeStamp(), "The daemon running device " + OPTIONS->device_name + " is starting up. This was either user caused or the mac restarted. Please check teamviewer.\nCheers\n'lil daemon");
     
  // Open the port we've got from the command line params
  int fd = serial::open_port(OPTIONS->device_name);
  
  serial::set_interface_attribs(fd,B9600,0);

  if (fd > -1){
    OPTIONS->device_descriptor = fd;
  } else {
    cerr << "molerat_rfid process failed to launch. Error opening device: " << OPTIONS->device_name  << endl; 
    send_email(OPTIONS->email_addr, "molerat RFID find failure: " + s9::DateTimeStamp(), "The daemon running device " + OPTIONS->device_name + " has quit due to failing to open the device. Launchd will attempt to restart but do check to make sure it is running.\nCheers\n'lil daemon");
    return 70;
  }
  
  cout << s9::DateTimeStamp() << ":" << "Opened device " << OPTIONS->device_name << endl; 

  if (!OPTIONS->arduino) {
    if (serial::rfid_init(fd) != 0){
      cerr << "Failed to initialise RFID device" << endl;
      send_email(OPTIONS->email_addr, "molerat RFID init failure: " + s9::DateTimeStamp(), "The daemon running device " + OPTIONS->device_name + " has quit due to a failed RFID initialisation. Launchd will attempt to restart but do check to make sure it is running.\nCheers\n'lil daemon");
      return 70; 
    }
  
    string logmsg = "Initialized device " + OPTIONS->device_name;
    cout << s9::DateTimeStamp() << ":"  << logmsg << endl;
    syslog(LOG_INFO, logmsg.c_str());
  }

  //catch_sigterm();
  signal(SIGINT, sig_term_handler2); 

  // Now fire up the connection to the mysql db
  if (OPTIONS->mysql){ 
    MYSQL_LOCAL = new mysql::Data();
    if ( mysql::Connect(MYSQL_LOCAL, OPTIONS->mysql_con, OPTIONS->mysql_usr, OPTIONS->mysql_pwd, OPTIONS->mysql_db) != 0){
      cerr  << s9::DateTimeStamp() << ":" << "Failed to connect to local mysql db!" << endl;
     
      send_email(OPTIONS->email_addr, "molerat mysql connect failure: " + s9::DateTimeStamp(), "The daemon running device " + OPTIONS->device_name + " has quit due to failing to connect to local mysql. Launchd will attempt to restart but do check to make sure it is running.\nCheers\n'lil daemon");
      return 70; 
    }
    mysql::HelloWorld(MYSQL_LOCAL);
  }
  
  if (OPTIONS->mysql_remote){ 
    MYSQL_REMOTE = new mysql::Data();
    if ( mysql::Connect(MYSQL_REMOTE, OPTIONS->mysql_remote_con, OPTIONS->mysql_usr, OPTIONS->mysql_pwd, OPTIONS->mysql_db) != 0){
      cerr  << s9::DateTimeStamp() << ":" << "Failed to connect to remote mysql db!" << endl;
      
      send_email(OPTIONS->email_addr, "molerat remote mysql connect failure: " + s9::DateTimeStamp(), "The daemon running device " + OPTIONS->device_name + " has quit due to failing to connect to the remote sql. Launchd will attempt to restart but do check to make sure it is running.\nCheers\n'lil daemon");
      return 70; 
    }
    mysql::HelloWorld(MYSQL_REMOTE);
  }

  if( signal( SIGINT, SIG_IGN ) != SIG_IGN ) {
    signal( SIGINT, sig_term_handler2 );
  }

  if( signal( SIGKILL, SIG_IGN ) != SIG_IGN ) {
    signal( SIGKILL, sig_term_handler2 );
  }

  if( signal( SIGTERM, SIG_IGN ) != SIG_IGN ) {
    signal( SIGTERM, sig_term_handler2);
  }
  
 
  if (OPTIONS->daemon) {
    // Run Daemonized
 
    if( fork() == 0 ) {
      fclose( stdin );
      fclose( stdout );
      poll(); // Start the proper looping FOREVER!

    } else {
      /* Exit parent process */
      if( signal( SIGINT, SIG_DFL ) != SIG_DFL ) {
        signal( SIGINT, SIG_DFL );
      }
      
      if( signal( SIGKILL, SIG_DFL ) != SIG_DFL ) {
        signal( SIGKILL, SIG_DFL );
      }

      if( signal( SIGTERM, SIG_DFL ) != SIG_DFL ) {
        signal( SIGTERM, SIG_DFL );
      }

      send_email(OPTIONS->email_addr, "molerat SIGKILL: " + s9::DateTimeStamp(), "The daemon running device " + OPTIONS->device_name + " has quit due to a SIGKILL. Launchd will attempt to restart but do check to make sure it is running.\nCheers\n'lil daemon");
      quit(1);
    
    } /* End if( fork() ) */
  } else {
    // Run normally so we can see the output properly
    poll();
  }

  exit(0);
  return(0);

}
