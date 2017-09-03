/**
 * RFID Serial functions for molerat.
 * http://www.cmrr.umn.edu/~strupp/serial.html#2_5_1
 * http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c#6947758
 * https://github.com/todbot/arduino-serial/blob/master/arduino-serial-lib.c
 */


#include "rfid_serial.hpp"

using namespace std;

namespace serial {

/**
 * Set the attributes of this RFID port
 * @param fd an integer file handle for the rfid device
 * @param speed the speed of this port
 * @param parity
 */

// Speed values B115200, B230400, B9600, B19200, B38400, B57600, B1200, B2400, B4800
// Parity values 0 (meaning no parity), PARENB|PARODD

// This works best with the Arduino. The RFID thing proper may be harder
int set_interface_attribs (int fd, int speed, int parity) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0) {
    // TODO - think about logging to file and timing
    cerr << "error " << errno << " from tcgetattr" << endl;
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

  // 8N1
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  // no flow control
  tty.c_cflag &= ~CRTSCTS;

  tty.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

  tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
  tty.c_oflag &= ~OPOST; // make raw

  // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
  tty.c_cc[VMIN]  = 0;
  tty.c_cc[VTIME] = 0;

  tcsetattr (fd, TCSANOW, &tty); 
  if (tcsetattr (fd, TCSAFLUSH, &tty) != 0) {
		cerr << "error " << errno << " from tcsetattr" << endl;
		return -1;
	}
	return 0;
}

/**
 * Set whether or not this port should block?
 * @param fd an integer file handle for the rfid device
 * @param should_block an int that is 0 or 1
 */

void set_blocking (int fd, int should_block) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	
	if (tcgetattr (fd, &tty) != 0) {
		cerr << "error " << errno << " from tggetattr" << endl;
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		cerr << "error " << errno << " setting term attributes" << endl;
	}
}

/**
 * Open the serial device port
 * @param port_path the full name of the device (e.g /dev/ttyUSB0)
 */

int open_port(string port_path) {
  int fd;
  // Open non-blocking in this version
  fd = open(port_path.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
  //fd = open(port_path.c_str(), O_RDWR | O_NOCTTY);

  if (fd == -1) {
    string msg = "open_port: Unable to open";
    msg += " " + port_path; 
    perror(msg.c_str());
  } else {
    fcntl(fd, F_SETFL, 0);
  }

  // Wait for the bootloader
  sleep(3);
  return (fd);
}

/**
 * Initialise the RFID Reader
 * @param fd an integer file handle for the rfid device
 */

int rfid_init(int fd) {
 
  int n = write(fd, "bC$", 3);
  // Double check this redirects when in daemon mode :S
  if (n < 0){
    fputs("write() of 3 bytes failed in init!\n", stderr);
    return -1;
  }

	//usleep ((n + 25) * 1000);	// sleep enough to transmit the 7 plus
                            // receive 25:  approx 100 uS per char transmit string buffer = "";
	
	//char buffer[3];  
  //int nr = read (fd, buffer, 3);   
  //if (nr != 3){
  //  cerr << "Unable to read during init" << endl;
  //  return -1;
  //}

  //std::string reply(buffer,3);
  //cout << "Init read from buffer: " << reply << endl;

	return 0;

}

/**
 * Perform a basic write and read back the result
 * @param fd an integer file handle for the rfid device
 * @param data a std::string we break down and send as bytes
 */

std::string rfid_write(int fd, std::string data) {

  int n = write(fd, data.c_str(), data.length());
  // Double check this redirects when in daemon mode :S
  if (n < 0){
    fputs("rfid_write failed", stderr);
  }
	
	usleep (data.length() * 100);	// sleep enough to transmit the 7 plus
                            // receive 25:  approx 100 uS per char transmit string buffer = "";

  // Read one byte by one
  char buffer[1];
 	int nb;
  std::string reply = "";
  do {
    nb = read (fd, buffer, 1);  
    if (nb < 0) {
		  cerr << "Failed to read from device during write/read" << endl;
		  return std::string("");
    }
    if (nb == 1) {
      reply = reply + buffer[0];
    }
  } while (nb > 0 && buffer[0] != '\r');
  
	return reply;

}



/**
 * Perform a basic write and read back the result
 * @param fd an integer file handle for the rfid device
 * @param data a std::string we break down and send as bytes
 */
std::string arduino_write(int fd, std::string data) {

  int n = write(fd, data.c_str(), data.length());
  // Double check this redirects when in daemon mode :S
  if (n < 0){
    fputs("arduino_write failed", stderr);
  }
	
	usleep (data.length() * 100);	// sleep enough to transmit the 7 plus
                            // receive 25:  approx 100 uS per char transmit string buffer = "";

  // Read one byte by one
  char buffer[1];
 	int nb;
  std::string reply = "";
  do {
    nb = read (fd, buffer, 1);  
    if (nb < 0) {
		  cerr << "Failed to read from device during write/read" << endl;
		  return std::string("");
    }
    if (nb == 1) {
      reply = reply + buffer[0];
    }
  } while (nb > 0 && buffer[0] != '\n');
  
	return reply;

}


/**
 * We wait for carriage return apparently - we will need to keep reading
 * @param fs an integer file handle for the rfid device
 * @param reply the data read
 * returns an int reflecting if the read worked
 */

int rfid_read(int fd, std::string & reply) {

 // Read one byte by one
  char buffer[1];
 	int nb;
  do {
    nb = read (fd, buffer, 1);  
    if (nb < 0) {
		  cerr << "Failed to read from device during read" << endl;
		  return nb;
    }
    if (nb == 1) {
      reply = reply + buffer[0];
    }
  } while ( buffer[0] != '\n');
 
	return 0;

}


// Possibly not needed but unsure as yet
int close_port(int fd) {
  close(fd);
  return 0;
}


/**
 * Read sensor data from the Arduino
 * @param fd an integer file handle for the rfid device
 * @param reply the data read
 * returns an int reflecting if the read worked 
 */

int sensor_read(int fd, std::string &reply) {

  // Read one byte by one until the newline char (apparently)
  char buffer[1];
 	int nb;
 
  do {
    nb = read (fd, buffer, 1);  
    if (nb < 0) {
		  cerr << "Failed to read from sensor device during read" << endl;
		  return nb;
    }
    if (nb == 1) {
      reply = reply + buffer[0];
    }
  } while (nb > 0 && buffer[0] != '\n' );
  
  return 0; 

}



} // end namespace serial
