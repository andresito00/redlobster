#include <sstream>
#include <iomanip>
#include <list>
#include "order.h"

namespace order
{

Order::Order() : oid(kMaxOID) {}
Order::Order(uint32_t oid, std::string symbol, OrderSide side, uint16_t qty,
             double price)
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
  other.side = OrderSide::kNone;
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
  other.side = OrderSide::kNone;
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
  price_stream << std::setw(13) << std::setfill('0') << std::fixed
               << std::setprecision(5) << price;
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

};  // namespace order
