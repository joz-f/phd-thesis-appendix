#ifndef __RFID_SERIAL_HPP__
#define __RFID_SERIAL_HPP__

#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <iostream>
#include <vector>

#include "string_utils.hpp"

namespace serial {
  int   set_interface_attribs (int fd, int speed, int parity);
  void  set_blocking (int fd, int should_block);
  int   open_port(std::string port_path);
  int   rfid_init(int fd);
  int   rfid_read(int fs, std::string & reply);
  std::string rfid_write(int fd, std::string data);
  std::string arduino_write(int fd, std::string data);
  int   close_port(int fd);
  int sensor_read(int fd, std::string & reply);
}
#endif
