#include <cstddef>
#include <fstream>
#include <string>
#include <order_book.h>
#include <order.h>
#include "test_utils.h"

/**
 * Creates 2^16 order instances: half offers, half asks.
 * Each offer has a corresponding ask that it expects to be matched with
 * such that it fills the entire requested `order.qty`.
 *
 * The offers are placed in order of ascending price.
 * The asks are placed in order descending price, starting with the last offer price.
 * Expected state at the end is an order count of 0 as every ask should be matched with
 * its corresponding best offer
 *
*/
int main(int argc, char *argv[])
{
  (void)argc;
  std::string ofile = std::string(argv[0]) + ".log";
  std::ofstream ostream(ofile, std::ios::out);

  constexpr size_t num_order_objects = 0x10000;
  auto orders = generate_asc_desc_full_fills(num_order_objects, 0);

  order::OrderBook test_book{};
  std::vector<order::OrderResult> results{};
  results.resize(orders.size());
  test_book.place_orders(&orders, &results);

  assertm(test_book.empty(), "Expected all orders to be filled...");
  assertm(test_book.fifos_size() == 0, "Expected all orders to be filled...");

  return 0;
}
