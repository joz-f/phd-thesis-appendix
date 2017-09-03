#ifndef __EMAIL_HPP__
#define __EMAIL_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <CoreFoundation/CoreFoundation.h>

int send_email(std::string recipient, std::string subject, std::string message);

#endif
