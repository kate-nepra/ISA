#include "../include/CommunicationBase.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @brief  CommunicationBase class constructor
 * @param  address: server address
 * @param  isV6: IPv6 flag
 * @param  port: destination port
 * @retval Constructed object
 */
CommunicationBase::CommunicationBase(std::string address, bool isV6, int port)
    : _address(address), _is_v6(isV6), _port(port) {}

/**
 * @brief  Create socket for communication with the server
 * @retval None
 */
void CommunicationBase::createSocket() {

  if (_is_v6) {
    _sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
  } else {
    _sockfd = socket(AF_INET, SOCK_STREAM, 0);
  }

  if (_sockfd == -1) {
    std::cerr << "ERR: Unable to create socket :(" << std::endl;
    exit(1);
  }
}

/**
 * @brief  Based on the type of connection assigns address and port
 * @retval None
 */
void CommunicationBase::sinAssign() {

  if (_is_v6) {
    _server6_address.sin6_family = AF_INET6;
    _server6_address.sin6_port = htons(_port);
    inet_pton(AF_INET6, _address.c_str(), &_server6_address.sin6_addr);
  } else {
    _server_address.sin_family = AF_INET;
    _server_address.sin_port = htons(_port);
    _server_address.sin_addr.s_addr = inet_addr(_address.c_str());
  }
}

/**
 * @brief  Creates connection to the server
 * @retval None
 */
void CommunicationBase::connectToServer() {
  if (_is_v6) {
    if (connect(_sockfd, (struct sockaddr *)&_server6_address,
                sizeof(_server6_address)) < 0) {
      std::cerr << "ERR: Unable to connect to server :(" << std::endl;
      exit(1);
    }
  } else {
    if (connect(_sockfd, (struct sockaddr *)&_server_address,
                sizeof(_server_address)) < 0) {
      std::cerr << "ERR: Unable to connect to server :(" << std::endl;
      exit(1);
    }
  }
}

/**
 * @brief  Completely arranges the connection to the server
 * @retval None
 */
void CommunicationBase::setConnection() {
  createSocket();
  sinAssign();
  connectToServer();
}

/**
 * @brief  Provides communication with the server within the connection
 * @param  data: message to be send to the sevrer
 * @retval message from the server
 */
std::string CommunicationBase::communicate(std::string data) {
  char buffer[4096];

  /* Send message */
  auto comm = write(_sockfd, data.c_str(), data.size());
  if (comm == -1) {
    std::cerr << "ERR: Unable to send data to server :(" << std::endl;
    exit(1);
  }

  /* Receive message */
  long unsigned int tmp_size{};
  std::string message;
  while (true) {
    bzero(&buffer, sizeof(buffer));
    comm = read(_sockfd, buffer, sizeof(buffer));
    message = message + std::string(buffer);
    if (tmp_size == message.size()) {
      break;
    }
    tmp_size = message.size();
  }

  if (comm == -1) {
    std::cerr << "ERR: Unable to process data from server :(" << std::endl;
    exit(1);
  }
  return message;
}

void CommunicationBase::endConnection() { close(_sockfd); }
