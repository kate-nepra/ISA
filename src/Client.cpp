#include "../include/Client.hpp"
#include "../include/ArgsParser.hpp"
#include "../include/CommunicationBase.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

const char *FILENAME = "login-token";
const int OK_HEADER_LENGTH = 3;
const int ERR_HEADER_LENGTH = 4;

/**
 * @brief  Extracts login token from the server message and saves it to a file
 * @param  message: message from the server
 * @retval None
 */
void Client::saveTokenToFile(std::string message) {
  std::string login_token = message.substr(0, message.find(")"));

  std::ofstream file(FILENAME);
  if (!file.is_open()) {
    std::cerr << "ERR: Login token could not be saved :(" << std::endl;
    exit(1);
  }
  file << login_token;
  file.close();
}

/**
 * @brief  Loads login token from file
 * @retval Login token
 */
std::string Client::getToken() {
  std::string login_token;
  std::ifstream file(FILENAME);
  if (file.is_open()) {
    while (getline(file, login_token)) {
      ;
    }
    file.close();
    return login_token;
  } else {
    std::cerr << "ERR: Login token could not be obtained :(" << std::endl;
    exit(1);
  }
}

/**
 * @brief  Finds a substring in the string and replaces it with another
 * @param  data: string data to be modified
 * @param  to_replace: substring that is searched for and replaced
 * @param  replace_by: substring with which we replace
 * @retval String data with replaced substrings
 */
std::string Client::findReplace(std::string data, std::string to_replace,
                                std::string replace_by) {
  size_t position = data.find(to_replace);
  while (position != std::string::npos) {
    data.replace(position, to_replace.size(), replace_by);
    position = data.find(to_replace, position + replace_by.size());
  }
  return data;
}

/**
 * @brief  Escapes special characters in the string given
 * @param  data: string in which the characters will be escaped
 * @retval String data with escaped characters
 */
std::string Client::escapeData(std::string data) {
  data = findReplace(data, "\\", "\\\\");
  data = findReplace(data, "\"", "\\\"");
  data = findReplace(data, "\\\\n", "\\n");
  return data;
}

/**
 * @brief  Reverts escaping of special characters in the string given
 * @param  data: string in which the characters will be un-escaped
 * @retval String data with un-escaped characters
 */
std::string Client::unEscapeData(std::string data) {
  data = findReplace(data, "\\\\", "\\");
  data = findReplace(data, "\\\"", "\"");
  return data;
}

/**
 * @brief  Formats data to be sent to the server according to the command type
 * @param  command: command type
 * @param  command_args: command arguments
 * @retval Formatted data
 */
std::string
Client::getFormattedData(CommandType command,
                         std::map<CommandArg, std::string> command_args) {

  std::string data = "(" + getCommandTypeEq(command);

  /* Sorting and processing by type of teh command */
  switch (command) {
  case CommandType::LIST:
  case CommandType::SEND:
  case CommandType::FETCH:
  case CommandType::LOGOUT:
    data = data + " " + getToken();
    break;

  default:
    break;
  }

  /* Assembling the data */
  for (auto m : command_args) {
    if (command == CommandType::FETCH) {
      data = data + " " + m.second;
      data = data + ")";
      return data;
    }
    m.second = escapeData(m.second);
    data = data + " \"" + m.second + "\"";
  }
  data = data + ")";

  return data;
}

/**
 * @brief  Identifies whether the server message status is okay
 * @param  message: message data from the server
 * @retval True: message status is ok | False: message status is not ok
 */
bool Client::isMessageOk(const std::string message) {
  int position = message.find("(ok ");
  return position == 0;
}

/**
 * @brief  Cuts off the header containing the message status from the server
 * @param  message: message data to be modified
 * @retval Message data without the header
 */
std::string Client::cutOffHeader(std::string message) {
  if (isMessageOk(message)) {
    return message.substr(OK_HEADER_LENGTH, message.size() - 1);
  } else {
    return message.substr(ERR_HEADER_LENGTH, message.size() - 1);
  }
}

/**
 * @brief  Extracts the text of the message from the server data
 * @param  message: message data from the server
 * @retval Server message content without header and brackets
 */
std::string Client::cutOutServerMessageContent(std::string message) {

  message = cutOffHeader(message);
  message = cutOffQuotesWrapping(message);

  return message;
}

/**
 * @brief  Splits the message data by the char given
 * @param  message: message data to be sliced
 * @param  cut_by: char according to which the message data will be sliced
 * @retval Vector of splitted substrings
 */
std::vector<std::string> Client::splitByChar(std::string message, char cut_by) {
  std::vector<std::string> container;
  /* Construct stream from the message */
  std::stringstream s_msg(message);
  std::string piece;

  /* Sequential division and addition to the container */
  while (std::getline(s_msg, piece, cut_by)) {
    container.push_back(piece);
  }

  return container;
}

/**
 * @brief  Splits the message data by the substring given
 * @param  message: message data to be sliced
 * @param  cut_by: substring according to which the message data will be sliced
 * @retval Vector of splitted substrings
 */
std::vector<std::string> Client::splitByString(std::string message,
                                               std::string cut_by) {
  std::string piece;
  std::vector<std::string> container;
  while (message.find(cut_by) != std::string::npos) {
    int k = 1;

    /* Detection of splitting by new rows and k constant modification */
    if (cut_by == "\\n") {
      k = 0;
    }

    /* Sequential division and addition to the container */
    piece = message.substr(0, message.find(cut_by) + k);
    container.push_back(piece);
    message =
        message.substr(message.find(cut_by) +
                           floor(static_cast<double>(cut_by.size()) / 2) + 1,
                       message.size());
  }
  /* Last piece */
  piece = message.substr(0, message.size());
  container.push_back(piece);

  return container;
}

/**
 * @brief  Removes first left round bracket
 * @param  message: message data to be modified
 * @retval Message data without first left round bracket
 */
std::string Client::cutOffFirstLeftBracket(std::string message) {
  if (message.find_first_of("(") != std::string::npos)
    return message.substr(message.find_first_of("(") + 1, message.size());

  return message;
}

/**
 * @brief  Removes last right round bracket
 * @param  message: message data to be modified
 * @retval Message data without last right round bracket
 */
std::string Client::cutOffLastRightBracket(std::string message) {
  if (message.find_first_of(")") != std::string::npos)
    return message.substr(0, message.find_last_of(")"));

  return message;
}

/**
 * @brief  Removes status of server message and first layer of brackets
 * @param  message: message data to be modified
 * @retval Message data without first layer of brackets and header
 */
std::string Client::cutOffPrimaryWrapping(std::string message) {
  message = cutOffHeader(message);
  message = cutOffLastRightBracket(message);

  return message;
}

/**
 * @brief  Removes one layer of external brackets
 * @param  message: message data to be modified
 * @retval Message data without one layer of external brackets
 */
std::string Client::cutOffBracketWrapping(std::string message) {
  message = cutOffFirstLeftBracket(message);
  message = cutOffLastRightBracket(message);

  return message;
}

/**
 * @brief  Removes external quotes
 * @param  message: message data to be modified
 * @retval Message data without qoutes wrapping
 */
std::string Client::cutOffQuotesWrapping(std::string message) {
  if (message.find("\"") != std::string::npos) {
    message = message.substr(message.find_first_of("\"") + 1, message.size());
    message = message.substr(0, message.find_last_of("\""));
  }

  return message;
}

/**
 * @brief  Processes and write out login message & calls for token processing
 * @param  message: message data
 * @retval None
 */
void Client::processLogin(std::string message) {
  message = cutOffHeader(message);
  std::vector<std::string> container = splitByString(message, "\" \"");
  std::cout << cutOffQuotesWrapping(unEscapeData(container[0])) << std::endl;
  saveTokenToFile(container[1]);
}

/**
 * @brief  Processes and writes to the output message of the list type
 * @param  message: message data to be printed
 * @retval None
 */
void Client::printList(std::string message) {
  std::cout << std::endl;
  /* Breakdown into individual items */
  std::vector<std::string> container = splitByString(message, ") (");

  /* Processing of individual items */
  for (auto item : container) {
    item = cutOffBracketWrapping(item);
    /* Extraction of item number */
    std::string number = item.substr(0, item.find_first_of(" "));
    item = item.substr(item.find_first_of(" ") + 1, item.size() - 1);
    if (item.empty()) {
      return;
    }
    std::cout << number << ": " << std::endl;
    /* Distribution and listing of sender and subject */
    std::vector<std::string> i_container = splitByString(item, "\" \"");
    int cnt{};
    for (auto p : i_container) {
      if (p != " " && !p.empty()) {
        cnt++;
        if (cnt == 1) {
          std::cout << "  From: ";
        } else if (cnt == 2) {
          std::cout << "  Subject: ";
        } else {
          break;
        }
        std::cout << unEscapeData(cutOffQuotesWrapping(p)) << std::endl;
      }
    }
  }
}

/**
 * @brief  Processes and writes to the output message of the fetch type
 * @param  message: message data to be printed
 * @retval None
 */
void Client::printFetch(std::string message) {

  std::cout << std::endl << std::endl;

  /* Distribution and listing of sender and subject */
  std::vector<std::string> container = splitByString(message, "\" \"");
  int cnt{};
  for (auto item : container) {
    cnt++;
    if (cnt == 1) {
      std::cout << "From: ";
    } else if (cnt == 2) {
      std::cout << "Subject: ";
    } else if (cnt == 3) {
      std::cout << std::endl;
      item = (cutOffQuotesWrapping(item));
      std::vector<std::string> i_container = splitByString(item, "\\n");

      for (auto p : i_container) {
        std::cout << unEscapeData(p);
        if (p != i_container.back()) {
          std::cout << std::endl;
        }
      }
    } else {
      break;
    }

    if (cnt != 3) {
      std::cout << unEscapeData(cutOffQuotesWrapping(item));
      std::cout << std::endl;
    }
  }
}

/**
 * @brief  Identifies the type of message from the server and processes its
 * content
 * @param  command: type of the message
 * @param  message: message data to be processed
 * @retval None
 */
void Client::processServerMessage(const CommandType command,
                                  std::string message) {
  if (isMessageOk(message)) {
    std::cout << "SUCCESS: ";
    switch (command) {
    case CommandType::REGISTER:
    case CommandType::SEND:
      std::cout << unEscapeData(cutOutServerMessageContent(message))
                << std::endl;
      break;
    case CommandType::LOGIN:
      processLogin(message);
      break;
    case CommandType::LIST:
      printList(cutOffBracketWrapping(cutOffPrimaryWrapping(message)));
      break;
    case CommandType::FETCH:
      printFetch(cutOffBracketWrapping(cutOffPrimaryWrapping(message)));
      break;
    case CommandType::LOGOUT:
      std::cout << unEscapeData(cutOutServerMessageContent(message))
                << std::endl;
      std::remove(FILENAME);
      break;

    default:
      break;
    }
  } else {
    std::cout << "ERROR: " << unEscapeData(cutOutServerMessageContent(message))
              << std::endl;
  }
}

/**
 * @brief  Client constructor (and server communication launcher)
 * @param  args: parsed program arguments
 * @retval Client
 */
Client::Client(ArgsParser args) {
  CommunicationBase c(args.getAddress(), args.isV6(), args.getPort());
  c.setConnection();
  auto message = c.communicate(
      getFormattedData(args.getCommandType(), args.getCommandArgs()));
  processServerMessage(args.getCommandType(), message);
  c.endConnection();
}