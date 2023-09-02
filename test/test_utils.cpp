#include <order_book.h>
#include <order.h>
#include <vector>
#include "test_utils.h"

constexpr size_t kMaxOrders = std::numeric_limits<uint16_t>::max();

order::Order generate_dummy_order(order::oid_t oid, order::qty_t qty,
                                  order::OrderSide side)
{
  return order::Order{oid, "IBM", side, qty, 100.0};
}

std::vector<order::Order> generate_dummy_n_orders(size_t n, order::oid_t start,
                                                  order::qty_t qty)
{
  std::vector<order::Order> result;
  result.resize(n);
  order::oid_t count = start;
  std::for_each(result.begin(), result.end(), [&](order::Order& order) {
    order.oid = count++;
    order.side = order::OrderSide::kBuy;
    order.symbol = "IBM";
    order.qty = qty;
    order.price = 100.00;
  });
  return result;
}
