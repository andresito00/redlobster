#include <sstream>
#include <iomanip>
#include <list>
#include <numeric>
#include "order.h"

namespace order
{
constexpr uint8_t kPrintPrecision = 5U;
constexpr qty_t kMaxQuantity = std::numeric_limits<order::qty_t>::max();
constexpr oid_t kMaxOID = std::numeric_limits<oid_t>::max();
constexpr fifo_idx_t kMaxDQIdx = std::numeric_limits<fifo_idx_t>::max();
constexpr size_t kMaxSymbolSize = 8;
constexpr order::price_t kMaxPrice = 9999999.99999;

Order::Order()
    : oid(kMaxOID), side(OrderSide::kBuy), qty(0), price(0.0), idx(kMaxDQIdx)
{
}
Order::Order(oid_t oid, symbol_t symbol, OrderSide side, qty_t qty,
             price_t price)
    : oid(oid),
      symbol(symbol),
      side(side),
      qty(qty),
      price(price),
      idx(kMaxDQIdx)
{
}
Order::Order(const Order& other)
    : oid(other.oid),
      symbol(other.symbol),
      side(other.side),
      qty(other.qty),
      price(other.price),
      idx(other.idx)
{
}
Order::Order(Order&& other)
    : oid(other.oid),
      symbol(other.symbol),
      side(other.side),
      qty(other.qty),
      price(other.price),
      idx(other.idx)
{
  if (this == &other) {
    return;
  }
  other.oid = kMaxOID;
  other.symbol = "";
  other.side = OrderSide::kBuy;
  other.qty = 0;
  other.price = 0.0;
  other.idx = kMaxDQIdx;
}

Order& Order::operator=(const Order& other)
{
  if (this == &other) {
    return *this;
  }
  symbol = other.symbol;
  oid = other.oid;
  side = other.side;
  qty = other.qty;
  price = other.price;
  idx = other.idx;
  return *this;
}

Order& Order::operator=(Order&& other)
{
  if (this == &other) {
    return *this;
  }
  symbol = other.symbol;
  oid = other.oid;
  side = other.side;
  qty = other.qty;
  price = other.price;
  idx = other.idx;

  other.oid = kMaxOID;
  other.symbol = "";
  other.side = OrderSide::kBuy;
  other.qty = 0;
  other.price = 0.0;
  other.idx = kMaxDQIdx;
  return *this;
}

std::string Order::str(char prepend, bool print_side) const
{
  std::stringstream order_stream;
  if (prepend != '\0') {
    order_stream << prepend << ' ';
  }
  order_stream << oid << ' ' << symbol << ' ';
  if (print_side) {
    char side_char = (side == OrderSide::kBuy) ? 'B' : 'S';
    order_stream << side_char << ' ';
  }
  order_stream << qty << ' ';
  std::stringstream price_stream;
  price_stream << std::fixed << std::setprecision(kPrintPrecision) << price;
  return order_stream.str() + price_stream.str();
}

std::list<std::string> OrderResult::serialize() const
{
  std::list<std::string> result;
  if (type == ResultType::kFilled) {
    for (const auto& o : orders) {
      result.push_back(o.str('F', false));
    }
  } else if (type == ResultType::kError) {
    result.push_back("E " + error_msg);
  } else if (type == ResultType::kCancelled) {
    result.push_back("X " + std::to_string(orders.front().oid));
  }
  return result;
}

}  // namespace order
