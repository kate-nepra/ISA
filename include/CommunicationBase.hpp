#pragma once
#ifndef COMMUNICATION_BASE_HPP
#define COMMUNICATION_BASE_HPP

#include <arpa/inet.h>
#include <string>
#include <sys/socket.h>

/**
 * @brief  Class providing a connection to the server
 * @retval None
 */
class CommunicationBase {
private:
  std::string _address{};
  bool _is_v6{};
  int _port{};

  int _sockfd{};
  struct sockaddr_in _server_address;
  struct sockaddr_in6 _server6_address;

  void createSocket();
  void sinAssign();
  void connectToServer();

public:
  CommunicationBase(std::string address, bool isV6, int port);
  ~CommunicationBase() = default;

  void setConnection();
  std::string communicate(std::string data);
  void endConnection();
};

#endif