#ifndef ORDER_H_
#define ORDER_H_
#include <string>
#include <vector>
#include <queue>
#include <numeric>
#include <cstdint>

namespace order
{

enum class OrderSide {
  kNone = 'U',
  kSell = 'S',
  kBuy = 'B',
};

using oid_t = uint32_t;
using qty_t = uint16_t;
using price_t = double;
using dq_idx_t = uint32_t;
static constexpr oid_t kMaxOID = std::numeric_limits<oid_t>::max();
static constexpr oid_t kMaxDQIdx = std::numeric_limits<dq_idx_t>::max();

struct Order {
  oid_t oid;
  std::string symbol;
  OrderSide side;
  qty_t qty;
  price_t price;
  dq_idx_t idx;
  Order();
  Order(oid_t oid, std::string symbol, OrderSide side, qty_t qty,
        price_t price);
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

};  // namespace order

#endif  // ORDER_H_
