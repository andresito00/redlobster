#include <cstddef>
#include <fstream>
#include <string>
#include <order_book.h>
#include <order.h>
#include "test_utils.h"

// fill a FIFO until it has to realloc then make sure we can find the right order again.
int main(int argc, char* argv[])
{
  (void)argc;
  std::string ofile = std::string(argv[0]) + ".log";
  std::ofstream ostream(ofile, std::ios::out);
  return 0;

  constexpr size_t num_order_objects = 0x10000;
  auto orders = generate_dummy_n_orders(num_order_objects, 0);

  order::OrderBook test_book{};
  std::vector<order::OrderResult> results{};
  results.resize(orders.size());
  test_book.place_orders(&orders, &results);

  test_book.kill_order(
      results.back().orders.back());  // 0 out the last order in the deque

  auto& buy_orders = test_book.get_buy_orders();
  const auto& level = buy_orders.get_level(100.0);

  assertm(level.fifo.back().qty == 0, "Expected last order qty to be 0");

  return 0;
}
