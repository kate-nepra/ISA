#include "../include/ArgsParser.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <getopt.h>
#include <netdb.h>
#include <regex>

/**
 * @brief  Base64 string encoder
 * @note  using code made by Megumi Tomita
 * @param  data: password data
 * @retval encoded password
 */
std::string ArgsParser::base64Encode(const std::string data) {
  /****************************************************************/
  /************************* Adopted code *************************/

  /*****************************************************************
  / * Title :  _MACARON_BASE64_H_ Base64.h
    * Author : Megumi Tomita
    * Date : 2016
    * Code version : 1.0
    * License : MIT
    * Availability :
  https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594
  ******************************************************************
   (Version 1.0)[Base64.h]
  https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594/ */

  static constexpr char sEncodingTable[] = {
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

  size_t in_len = data.size();
  size_t out_len = 4 * ((in_len + 2) / 3);
  std::string ret(out_len, '\0');
  size_t i;
  char *p = const_cast<char *>(ret.c_str());

  for (i = 0; i < in_len - 2; i += 3) {
    *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
    *p++ = sEncodingTable[((data[i] & 0x3) << 4) |
                          ((int)(data[i + 1] & 0xF0) >> 4)];
    *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2) |
                          ((int)(data[i + 2] & 0xC0) >> 6)];
    *p++ = sEncodingTable[data[i + 2] & 0x3F];
  }
  if (i < in_len) {
    *p++ = sEncodingTable[(data[i] >> 2) & 0x3F];
    if (i == (in_len - 1)) {
      *p++ = sEncodingTable[((data[i] & 0x3) << 4)];
      *p++ = '=';
    } else {
      *p++ = sEncodingTable[((data[i] & 0x3) << 4) |
                            ((int)(data[i + 1] & 0xF0) >> 4)];
      *p++ = sEncodingTable[((data[i + 1] & 0xF) << 2)];
    }
    *p++ = '=';
  }

  return ret;
  /****************************************************************/
}

/**
 * @brief  Returns server hostname or address to connect to
 * @retval address
 */
std::string ArgsParser::getAddress() const { return _address; }

/**
 * @brief  Returns IPv6 flag
 * @retval IPv6 flag
 */
bool ArgsParser::isV6() const { return _is_v6; }

/**
 * @brief  Returns server port to connect to
 * @retval port
 */
int ArgsParser::getPort() const { return _port; }

/**
 * @brief  Returns command type
 * @retval command type
 */
CommandType ArgsParser::getCommandType() const { return _command_type; }

/**
 * @brief  Returns command arguments
 * @retval command arguments
 */
std::map<CommandArg, std::string> ArgsParser::getCommandArgs() const {
  return _command_args;
}

/**
 * @brief Operator (<<) applied to an output stream
 * @param  &os: pointer to a streambuf object from whose controlled input
 * sequence the characters are copied.
 * @param  &ap: a function that takes and returns a stream object. It generally
 * is a manipulator function.
 * @retval streambuf object
 */
std::ostream &operator<<(std::ostream &os, const ArgsParser &ap) {
  os << "Address:" << ap.getAddress() << std::endl
     << "Port:" << ap.getPort() << std::endl
     << "Command:" << getCommandTypeEq(ap.getCommandType()) << std::endl;
  for (auto &item : ap.getCommandArgs()) {
    os << " + " << getCommandArgEq(item.first) << ":" << item.second
       << std::endl;
  }
  return os;
}

/**
 * @brief  Enum class to string 'converter' for command type
 * @param  command_type: command type to be 'converted'
 * @retval string value according to command type
 */
std::string getCommandTypeEq(const CommandType command_type) {
  switch (command_type) {
  case (CommandType::REGISTER):
    return "register";
  case (CommandType::LOGIN):
    return "login";
  case (CommandType::LIST):
    return "list";
  case (CommandType::SEND):
    return "send";
  case (CommandType::FETCH):
    return "fetch";
  case (CommandType::LOGOUT):
    return "logout";
  default:
    std::cerr << "Internal error occured :(" << std::endl;
    exit(1);
  }
}

/**
 * @brief  Enum class to string 'converter' for command arguments
 * @param  command_arg: command argument type to be 'converted'
 * @retval string value according to command argument type
 */
std::string getCommandArgEq(const CommandArg command_arg) {
  switch (command_arg) {
  case (CommandArg::USERNAME):
    return "USERNAME";
  case (CommandArg::PASSWORD):
    return "PASSWORD";
  case (CommandArg::RECIPIENT):
    return "RECIPIENT";
  case (CommandArg::SUBJECT):
    return "SUBJECT";
  case (CommandArg::BODY):
    return "BODY";
  case (CommandArg::ID):
    return "ID";
  default:
    std::cerr << "Internal error occured :(" << std::endl;
    exit(1);
  }
}

/**
 * @brief  Prints program help
 * @retval None
 */
void ArgsParser::printHelp() {
  std::cout << "usage: client [ <option> ... ] <command> [<args>] ..."
            << std::endl
            << "Options:" << std::endl
            << "[-h | --help]" << std::endl
            << "  Show this help" << std::endl
            << "[-a | --adress]  <address>" << std::endl
            << "  Server hostname or address to connect to (default localhost)"
            << std::endl
            << "[-p | --port]    <port>" << std::endl
            << "  Server port to connect to (default 32323)" << std::endl
            << "--" << std::endl
            << "Do not treat any remaining argument as a switch (at this level)"
            << std::endl
            << "Supported commands:" << std::endl
            << " register <username> <password>" << std::endl
            << " login <username> <password>" << std::endl
            << " list" << std::endl
            << " send <recipient> <subject> <body>" << std::endl
            << " fetch <id>" << std::endl
            << " logout" << std::endl;
}

/**
 * @brief  Prints program problem
 * @param  problem: type of problem
 * @param  problem_arg: argument to the problem
 * @retval None
 */
void ArgsParser::printProblem(const std::string problem,
                              std::string problem_arg) {
  if (problem_arg != "") {
    problem_arg = ": " + problem_arg;
  }
  std::cerr << "Invalid " << problem << problem_arg
            << " , see help {-h | --help} for more info." << std::endl;
}

/**
 * @brief  IPv4 validator
 * @param  address: address to be validated
 * @retval True: address is IPv4 | False: address is not IPv4
 */
bool ArgsParser::IPv4Check(const std::string address) {
  if (std::regex_match(
          address, std::regex("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}"))) {
    for (auto c : address) {
      return std::isdigit(static_cast<unsigned char>(c)) || c == '.';
    }
    return false;
  } else {
    return false;
  }
}

/**
 * @brief  IPv6 validator
 * @param  address: address to be validated
 * @retval True: address is IPv6 | False: address is not IPv6
 */
bool ArgsParser::IPv6Check(const std::string address) {
  int segments{};
  int segment_size{};
  if ((address[0] == ':' && address[1] != ':') ||
      (address[address.size() - 1] == ':' &&
       address[address.size() - 2] != ':'))
    return false;
  for (auto c : address) {
    if (c == ':') {
      segment_size = 0;
      segments++;
    } else if (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') ||
               ('A' <= c && c <= 'F'))
      segment_size++;
    else
      return false;
    if (segment_size > 4)
      return false;
  }
  if (segments > 7)
    return false;
  return true;
}

/**
 * @brief  Checks if string is a number
 * @param  str: string to be checked
 * @retval True: string is a number | False: string is not a number
 */
bool ArgsParser::isNumber(const std::string str) {
  for (auto c : str) {
    if (!std::isdigit(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  return true;
}

/**
 * @brief  ArgsParser constructor
 * @param  argc: number of strings pointed to by argv
 * @param  **argv: array of arguments
 * @retval constructed ArgsParser
 */
ArgsParser::ArgsParser(int argc, char **argv) {
  int c;
  char *arg_long{};

  /* Process options */
  static struct option _long_options[] = {
      {"address", required_argument, 0, 'a'},
      {"port", required_argument, 0, 'p'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}};

  int option_index;
  while ((c = getopt_long(argc, argv, "a:p:h", _long_options, &option_index)) !=
         -1) {
    switch (c) {

    case 0: {
      arg_long = optarg;
      break;
    }

    case 'a': {
      if (std::string(optarg) == "localhost") {
        /* Default IPv6 localhost settings */
        _address = "::1";
        break;
      }
      _address = std::string(optarg);
      if (!IPv4Check(_address) && !IPv6Check(_address)) {
        if (gethostbyname(_address.c_str()) == NULL) {
          printProblem("address", "");
          exit(1);
        }
        _address =
            inet_ntoa(*((struct in_addr *)(gethostbyname(_address.c_str())
                                               ->h_addr_list[0])));
      }
      if (IPv4Check(_address))
        _is_v6 = false;
      break;
    }
    case 'p': {
      if (!isNumber(std::string(optarg))) {
        printProblem("port", "");
        exit(1);
      }
      _port = std::stoi(std::string(optarg));
      if (_port > 65535) {
        printProblem("port", "");
        exit(1);
      }
      break;
    }
    case 'h': {
      printHelp();
      exit(0);
    }
    case '?': {
      printProblem("option", "");
      exit(1);
    }
    default: {
      printProblem("option", "");
      exit(1);
    }
    }
  }

  /* Process commands */

  int i = optind;
  if (i >= argc) {
    printProblem("command", "");
    std::cerr << "client: expects <command> [<args>] ... on the command line, "
                 "given 0 arguments"
              << std::endl;
    exit(1);
  }
  while (i < argc) {
    if (strcmp(argv[i], getCommandTypeEq(CommandType::REGISTER).c_str()) == 0) {
      _command_type = CommandType::REGISTER;
      if (i + 2 == argc - 1) {
        ++i;
        _command_args[CommandArg::USERNAME] = std::string(argv[i]);
        ++i;
        _command_args[CommandArg::PASSWORD] =
            base64Encode(std::string(argv[i]));
        ++i;
        break;
      } else {
        printProblem("arguments", "");
        exit(1);
      }
    }
    if (strcmp(argv[i], getCommandTypeEq(CommandType::LOGIN).c_str()) == 0) {
      _command_type = CommandType::LOGIN;
      if (i + 2 == argc - 1) {
        ++i;
        _command_args[CommandArg::USERNAME] = std::string(argv[i]);
        ++i;
        _command_args[CommandArg::PASSWORD] =
            base64Encode(std::string(argv[i]));
        ++i;
        break;
      } else {
        printProblem("arguments", "");
        exit(1);
      }
    }
    if (strcmp(argv[i], getCommandTypeEq(CommandType::LIST).c_str()) == 0) {
      _command_type = CommandType::LIST;
      ++i;
      break;
    }
    if (strcmp(argv[i], getCommandTypeEq(CommandType::SEND).c_str()) == 0) {
      _command_type = CommandType::SEND;
      if (i + 3 == argc - 1) {
        ++i;
        _command_args[CommandArg::RECIPIENT] = std::string(argv[i]);
        ++i;
        _command_args[CommandArg::SUBJECT] = std::string(argv[i]);
        ++i;
        _command_args[CommandArg::BODY] = std::string(argv[i]);
        ++i;
        break;
      } else {
        printProblem("arguments", "");
        exit(1);
      }
    }
    if (strcmp(argv[i], getCommandTypeEq(CommandType::FETCH).c_str()) == 0) {
      _command_type = CommandType::FETCH;
      if (i + 1 == argc - 1) {
        ++i;
        _command_args[CommandArg::ID] = std::string(argv[i]);
        ++i;
        break;
      } else {
        printProblem("arguments", "");
        exit(1);
      }
    }
    if (strcmp(argv[i], getCommandTypeEq(CommandType::LOGOUT).c_str()) == 0) {
      _command_type = CommandType::LOGOUT;
      ++i;
      break;
    } else {
      printProblem("command", std::string(argv[i]));
      exit(1);
    }
  }
}