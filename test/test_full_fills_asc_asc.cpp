#include <cstddef>
#include <fstream>
#include <string>
#include <order_book.h>
#include <order.h>
#include "test_utils.h"

// same thing as asc desc except the offers come in ascending order now too
// my expectation is that we will end up with half of the orders above the mid price point will be unfilled
// the low half of the asks will reap the benefits of high offers on the table
// the second half of the offers will not see asks at or better than they need
int main(int argc, char *argv[])
{
  (void)argc;
  std::string ofile = std::string(argv[0]) + ".log";
  std::ofstream ostream(ofile, std::ios::out);

  constexpr size_t num_order_objects = 0x10000;
  size_t num_buys, num_sells;
  num_buys = num_sells = num_order_objects / 2;
  auto orders = generate_asc_asc_full_fills(num_order_objects, 0);

  order::OrderBook test_book{};
  std::vector<order::OrderResult> results{};
  results.resize(orders.size());
  test_book.place_orders(&orders, &results);

  assertm(test_book.sell_order_count() == kDefaultOrderQty * num_sells / 2,
          "Expected half of the orders to be filled...");
  assertm(test_book.buy_order_count() == kDefaultOrderQty * num_buys / 2,
          "Expected half of the orders to be filled...");
  assertm(test_book.get_sell_orders().fifos_size() == num_sells / 2,
          "Expected half of the orders to be filled...");
  assertm(test_book.get_buy_orders().fifos_size() == num_buys / 2,
          "Expected half of the orders to be filled...");

  return 0;
}
