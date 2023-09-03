#include <cstddef>
#include <order_book.h>
#include <order.h>
#include <vector>
#include <string>
#include "test_utils.h"

extern const order::qty_t kMaxQuantity;
constexpr size_t kDefaultTestPrice = 100.0;
constexpr order::qty_t kDefaultOrderQty = 10;

order::Order generate_dummy_order(order::oid_t oid, order::qty_t qty,
                                  order::OrderSide side, order::symbol_t symbol)
{
  return order::Order{oid, symbol, side, qty, kDefaultTestPrice};
}

std::vector<order::Order> generate_dummy_n_orders(size_t n, order::oid_t start,
                                                  order::qty_t qty,
                                                  order::symbol_t symbol)
{
  std::vector<order::Order> result;
  result.resize(n);
  order::oid_t count = start;
  std::for_each(result.begin(), result.end(), [&](order::Order& order) {
    order.oid = count++;
    order.side = order::OrderSide::kBuy;
    order.symbol = symbol;
    order.qty = qty;
    order.price = kDefaultTestPrice;
  });
  return result;
}

std::vector<order::Order> generate_asc_desc_full_fills(size_t n,
                                                       order::oid_t start,
                                                       order::qty_t qty)
{
  assert(n % 2 == 0);
  std::vector<order::Order> result;
  result.resize(n);
  order::oid_t count = start;
  order::price_t curr_price{1.0};

  size_t i = 0;
  for (; i < n / 2; ++i) {
    result[i].oid = count++;
    result[i].side = order::OrderSide::kBuy;
    result[i].symbol = "IBM";
    result[i].qty = qty;
    result[i].price = curr_price;
    curr_price += 1.0;
  }

  curr_price -= 1.0;

  for (; i < n; ++i) {
    result[i].oid = count++;
    result[i].side = order::OrderSide::kSell;
    result[i].symbol = "IBM";
    result[i].qty = qty;
    result[i].price = curr_price;
    curr_price -= 1.0;
  }

  return result;
}

std::vector<order::Order> generate_asc_asc_full_fills(size_t n,
                                                      order::oid_t start,
                                                      order::qty_t qty)
{
  assert(n % 2 == 0);
  std::vector<order::Order> result;
  result.resize(n);
  order::oid_t count = start;
  order::price_t curr_price{1.0};

  size_t i = 0;
  for (; i < n / 2; ++i) {
    result[i].oid = count++;
    result[i].side = order::OrderSide::kBuy;
    result[i].symbol = "IBM";
    result[i].qty = qty;
    result[i].price = curr_price;
    curr_price += 1.0;
  }
  curr_price = 1.0;
  for (; i < n; ++i) {
    result[i].oid = count++;
    result[i].side = order::OrderSide::kSell;
    result[i].symbol = "IBM";
    result[i].qty = qty;
    result[i].price = curr_price;
    curr_price += 1.0;
  }

  return result;
}
