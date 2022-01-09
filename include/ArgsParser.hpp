#pragma once
#ifndef ARGS_PARSER_HPP
#define ARGS_PARSER_HPP

#include <arpa/inet.h>
#include <getopt.h>
#include <iostream>
#include <map>
#include <netdb.h>
#include <string>
#include <sys/socket.h>

enum class CommandType { REGISTER, LOGIN, LIST, SEND, FETCH, LOGOUT };
enum class CommandArg { USERNAME, PASSWORD, RECIPIENT, SUBJECT, BODY, ID };

/**
 * @brief  Class for parsing program arguments
 * @retval None
 */
class ArgsParser {
public:
  ArgsParser(int argc, char **argv);
  ~ArgsParser() = default;

  void printHelp();
  static bool IPv4Check(const std::string address);
  static bool IPv6Check(const std::string address);
  bool isNumber(const std::string str);

  std::string getAddress() const;
  bool isV6() const;
  int getPort() const;
  CommandType getCommandType() const;
  std::map<CommandArg, std::string> getCommandArgs() const;

private:
  std::string _address{"::1"};
  bool _is_v6{true};
  int _port{32323};
  static option _long_options[];
  CommandType _command_type;
  std::map<CommandArg, std::string> _command_args;

  void printProblem(const std::string problem, std::string problem_arg);
  std::string base64Encode(const std::string data);
};

std::string getCommandTypeEq(const CommandType _command_type);
std::string getCommandArgEq(const CommandArg _command_arg);
std::ostream &operator<<(std::ostream &os, const ArgsParser &ap);

#endif