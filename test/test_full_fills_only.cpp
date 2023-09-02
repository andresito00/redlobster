#include <cstddef>
#include <fstream>
#include <string>
#include <order_book.h>
#include <order.h>
#include "test_utils.h"

int main(int argc, char *argv[])
{
  (void)argc;
  std::string ofile = std::string(argv[0]) + ".log";
  std::ofstream ostream(ofile, std::ios::out);

  auto orders = generate_asc_desc_full_fills(65536, 0);

  order::OrderBook test_book{};
  std::vector<order::OrderResult> results{};
  results.resize(orders.size());
  test_book.place_orders(&orders, &results);

  assertm(test_book.empty(), "Expected all orders to be filled...");

  return 0;
}
