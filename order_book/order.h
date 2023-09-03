#ifndef ORDER_BOOK_ORDER_H_
#define ORDER_BOOK_ORDER_H_
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <numeric>
#include <cstdint>

namespace order
{

enum class OrderSide {
  kSell = 'S',
  kBuy = 'B',
};

using oid_t = uint32_t;
using qty_t = uint16_t;
using price_t = double;
using fifo_idx_t = uint32_t;
using symbol_t = std::string;

const extern qty_t kMaxOrders;
const extern oid_t kMaxOID;
const extern fifo_idx_t kMaxDQIdx;

struct Order {
  oid_t oid;
  symbol_t symbol;
  OrderSide side;
  qty_t qty;
  price_t price;
  fifo_idx_t idx;
  Order();
  Order(oid_t oid, symbol_t symbol, OrderSide side, qty_t qty, price_t price);
  ~Order() = default;
  Order(const Order& other);
  Order(Order&& other);
  Order& operator=(const Order& other);
  Order& operator=(Order&& other);
  std::string str(char prepend, bool print_side = true) const;
};

enum class ResultType {
  kNop,
  kError,
  kFilled,
  kCancelled,
};

struct OrderResult {
  ResultType type;
  std::string error_msg;
  std::deque<Order> orders;
  std::list<std::string> serialize() const;
};

}  // namespace order

#endif  // ORDER_BOOK_ORDER_H_
