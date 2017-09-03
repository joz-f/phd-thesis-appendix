#include "email.hpp"

using namespace std;

void write_smtp(int sockfd, std::string msg, bool read=true) {
  int len, bytes_sent;
  msg = msg + "\n";
  len = msg.length();
  bytes_sent = send(sockfd, msg.c_str(), len, 0);
  if (read) {
// read a reply from server
    char outbuf[1024];
    len=recv(sockfd,outbuf,1024,0);
    outbuf[len]='\0';
    //cout <<outbuf;
  }
}

int send_email(std::string recipient, std::string subject, std::string message) {
  int status;
  struct addrinfo hints, *res;
  struct sockaddr_in remote;
  int sockfd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  memset(&remote, 0, sizeof(remote));

  // get ready to connect
  status = getaddrinfo("mail1.qmul.ac.uk", "25", &hints, &res);

  //sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  int result = connect(sockfd, res->ai_addr, res->ai_addrlen);
 
  char buffer [4096]; // Classic!
  memset(&buffer, 0, sizeof buffer);
  int len = 0;
  // Wait for 220 message from the server as per standard
  while(strstr(buffer,"220") == nullptr){
    len = recv(sockfd, buffer, 4096, 0);
    buffer[len] = '\0';
  }

  write_smtp(sockfd, "EHLO mail1.qmul.ac.uk");
  write_smtp(sockfd, "MAIL FROM:<noreply@qmul.ac.uk>");
  write_smtp(sockfd, "RCPT TO:<"+ recipient +">");
  write_smtp(sockfd, "DATA");
  write_smtp(sockfd, "SUBJECT:" + subject, false );
  write_smtp(sockfd, "From: noreply@qmul.ac.uk", false);
  write_smtp(sockfd, "To:" + recipient, false);
  write_smtp(sockfd, "", false);
  write_smtp(sockfd, message, false );
  write_smtp(sockfd, ".");
 
  write_smtp(sockfd, "QUIT");
  //closesocket(sockfd);

}
