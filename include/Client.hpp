#pragma once
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "ArgsParser.hpp"
#include <string>
#include <vector>

/**
 * @brief  Class encapsulating client functionality
 * @retval None
 */
class Client {
private:
  static std::string findReplace(std::string data, std::string to_replace,
                                 std::string replace_by);
  static std::string escapeData(std::string data);
  static std::string unEscapeData(std::string data);
  static std::string
  getFormattedData(CommandType command,
                   std::map<CommandArg, std::string> command_args);

  static void saveTokenToFile(std::string message);
  static std::string getToken();

  static bool isMessageOk(const std::string message);
  static std::string cutOffHeader(std::string message);
  static std::string cutOutServerMessageContent(std::string message);
  static std::string cutOffFirstLeftBracket(std::string message);
  static std::string cutOffLastRightBracket(std::string message);
  static std::string cutOffPrimaryWrapping(std::string message);
  static std::string cutOffBracketWrapping(std::string message);
  static std::string cutOffQuotesWrapping(std::string message);
  static std::vector<std::string> splitByChar(std::string message, char cut_by);
  static std::vector<std::string> splitByString(std::string message,
                                                std::string cut_by);
  static void processLogin(std::string message);
  static void printList(std::string message);
  static void printFetch(std::string message);
  static void processServerMessage(const CommandType command,
                                   std::string message);

public:
  Client(ArgsParser args);
  ~Client() = default;
};

#endif