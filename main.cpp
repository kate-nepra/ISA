#include "./include/ArgsParser.hpp"
#include "./include/Client.hpp"
#include "./include/CommunicationBase.hpp"

/**
 * @brief  Main function
 * @param  argc: number of strings pointed to by argv
 * @param  **argv: array of arguments
 * @retval 0
 */
int main(int argc, char **argv) {

  Client client(ArgsParser(argc, argv));

  return 0;
}