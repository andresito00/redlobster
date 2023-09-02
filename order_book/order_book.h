#ifndef ORDER_BOOK_ORDER_BOOK_H_
#define ORDER_BOOK_ORDER_BOOK_H_
#include <cstddef>
#include <map>
#include <unordered_map>
#include <queue>
#include <numeric>
#include <list>
#include <string>
#include "order.h"
#include "level_map.h"

namespace order
{

class OrderBook
{
 public:
  OrderBook() = default;
  fifo_idx_t place_order(Order* order, OrderResult* result);
  std::vector<fifo_idx_t> place_orders(std::vector<Order>* orders,
                                       std::vector<OrderResult>* results);
  void kill_order(Order& order);
  inline const levelmap::MinLevelMap& get_sell_orders() const
  {
    return sell_orders_;
  }
  inline const levelmap::MinLevelMap& get_buy_orders() const
  {
    return buy_orders_;
  }
  inline size_t buy_order_count() const noexcept
  {
    return buy_orders_.order_count();
  }
  inline size_t sell_order_count() const noexcept
  {
    return sell_orders_.order_count();
  }
  inline size_t order_count() const noexcept
  {
    return buy_order_count() + sell_order_count();
  }
  inline size_t fifos_size() const noexcept
  {
    return buy_orders_.fifos_size() + sell_orders_.fifos_size();
  }
  inline size_t maps_size() const noexcept
  {
    return buy_orders_.map_size() + sell_orders_.map_size();
  }
  inline bool empty() const noexcept { return buys_empty() && sells_empty(); }
  inline bool maps_empty() const noexcept
  {
    return buy_map_empty() && sell_map_empty();
  }
  inline bool fifos_empty() const noexcept
  {
    return buy_fifos_empty() && sell_fifos_empty();
  }
  inline bool buys_empty() const noexcept { return buy_orders_.empty(); }
  inline bool sells_empty() const noexcept { return sell_orders_.empty(); }
  inline bool buy_fifos_empty() const noexcept
  {
    return buy_orders_.fifos_empty();
  }
  inline bool sell_fifos_empty() const noexcept
  {
    return sell_orders_.fifos_empty();
  }
  inline bool buy_map_empty() const noexcept { return buy_orders_.map_empty(); }
  inline bool sell_map_empty() const noexcept
  {
    return sell_orders_.map_empty();
  }

 private:
  levelmap::MinLevelMap buy_orders_;
  levelmap::MinLevelMap sell_orders_;
};

class BookMap
{
 public:
  OrderResult handle_order(Order* order);
  OrderResult cancel_order(const oid_t oid);
  std::list<std::string> serialize();

 private:
  std::unordered_map<std::string, OrderBook> book_map_;
  std::unordered_map<oid_t, Order> order_lut_;
};

std::list<std::string> get_order_strings(std::deque<Order>& fifo, bool prepend);
std::list<std::string> serialize_book(OrderBook& book, char prepend);

}  // namespace order

#endif  // ORDER_BOOK_ORDER_BOOK_H_
