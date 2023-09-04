#include <string>
#include <fstream>
#include <iostream>
#include "simple_cross.h"

#if !defined(STDIN)
#define WHILE_GETLINE(x)                              \
  std::ifstream actions("actions.txt", std::ios::in); \
  while (std::getline(actions, x))
#else  // STDIN
#define WHILE_GETLINE(x) while (std::getline(std::cin, x))
#endif  // STDIN

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  SimpleCross scross;
  std::string line;
  WHILE_GETLINE(line)
  {
    results_t results = scross.action(line);
    for (results_t::const_iterator it = results.begin(); it != results.end();
         ++it) {
      std::cout << *it << '\n';
    }
  }
  return 0;
}
