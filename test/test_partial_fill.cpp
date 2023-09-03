#include <cstddef>
#include <fstream>
#include <order_book.h>
#include <order.h>
#include <log.h>
#include "test_utils.h"

/**
 * Executes a partial fill and asserts the expected quantities and order sides.
*/
int main(int argc, char *argv[])
{
  (void)argc;
  std::string ofile = std::string(argv[0]) + ".log";
  std::ofstream ostream(ofile, std::ios::out);

  constexpr size_t kOrderQty = 10LU;
  auto dummy_buy = generate_dummy_order(0, kOrderQty, order::OrderSide::kBuy);
  auto dummy_sell =
      generate_dummy_order(1, kOrderQty / 2, order::OrderSide::kSell);
  order::OrderResult result;

  order::OrderBook test_book{};
  test_book.place_order(&dummy_buy, &result);
  test_book.place_order(&dummy_sell, &result);

  assertm(result.type == order::ResultType::kFilled, "Expected filled result");
  assertm(result.orders.size() == 2, "Expected two filled orders");
  assertm(result.orders[0].side == order::OrderSide::kSell,
          "Expected sell order first");
  assertm(result.orders[1].side == order::OrderSide::kBuy,
          "Expected buy order second");
  assertm(result.orders[0].qty == kOrderQty / 2, "Expected half filled order");
  assertm(result.orders[1].qty == kOrderQty / 2, "Expected half filled order");

  return 0;
}
