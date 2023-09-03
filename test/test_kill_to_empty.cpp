#include <fstream>
#include <order_book.h>
#include <order.h>
#include <log.h>
#include "test_utils.h"

/**
 * test_kill_to_empty:
 * 1. Adds kNumOrders or the order book
 * 2. Kills them all
 *    Note: At this point we expect the order_count to be 0,
 *    but because of how I've chosen to design the order cancellation flow
 *    we expect the FIFO containing those orders to still have them, granted at .qty == 0.
 * 4. Cross an order that would otherwise have been filled by the cancelled orders.
 * 5. Verify that the cancelled orders were popped off their fifos.
 * 6. Cross one more order to fill the one placed in Step 4.
 * 7. Verify that all of our data structures and counts corroborate (0, empty).
*/
int main(int argc, char* argv[])
{
  (void)argc;
  std::string ofile = std::string(argv[0]) + ".log";
  std::ofstream ostream(ofile, std::ios::out);
  constexpr size_t num_orders = 1024;
  constexpr size_t order_quantity = 10LU;

  auto dummy_buys = generate_dummy_n_orders(num_orders, 0, order_quantity);
  std::vector<order::OrderResult> results;
  results.resize(num_orders);
  order::OrderBook test_book{};
  // * 1. Add num_orders or the order book
  test_book.place_orders(&dummy_buys, &results);
  // * 2. Kill them all
  for (auto& d : dummy_buys) {
    test_book.kill_order(d);
  }
  //  * Verify that the order_count to be 0,
  //  * but because of how I've chosen to design the order cancellation flow
  //  * we expect the FIFO containing those orders to still have them, granted at .qty == 0.
  std::string assert_str1("Expected order_count == 0. Found: " +
                          std::to_string(test_book.order_count()));
  assertm(test_book.empty(), assert_str1.c_str());
  std::string assert_str2("Expected FIFOs size == " +
                          std::to_string(num_orders));
  assertm(test_book.fifos_size() == num_orders, assert_str2.c_str());

  order::Order dummy_order{24, "IBM", order::OrderSide::kSell, 10, 99.00};
  order::OrderResult result{};

  //  * 4. Cross an order that would otherwise have been filled by the cancelled orders.
  test_book.place_order(&dummy_order, &result);

  std::string assert_str3("Expected FIFOs size == 1");
  //  * 5. Verify that the cancelled orders were popped off their fifos.
  assertm(test_book.fifos_size() == 1, assert_str3.c_str());

  //  * 6. Cross one more order to fill the one placed in Step 4.
  dummy_order.side = order::OrderSide::kBuy;
  test_book.place_order(&dummy_order, &result);

  //  * 7. Verify that all of our data structures and counts corroborate (0, empty).
  std::string assert_str4("Expected order_count == 0: " +
                          std::to_string(test_book.order_count()));
  std::string assert_str5("Expected FIFOs size == 0: " +
                          std::to_string(test_book.fifos_size()));
  std::string assert_str6("Expected FIFO maps size == 0: " +
                          std::to_string(test_book.fifos_size()));
  assertm(test_book.empty() && (test_book.order_count() == 0),
          assert_str4.c_str());
  assertm(test_book.fifos_empty() && (test_book.fifos_size() == 0),
          assert_str5.c_str());
  assertm(test_book.maps_empty() && ((test_book.maps_size() == 0)),
          assert_str6.c_str());

  return 0;
}
