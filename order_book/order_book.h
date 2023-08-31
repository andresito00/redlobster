#ifndef ORDER_BOOK_ORDER_BOOK_H_
#define ORDER_BOOK_ORDER_BOOK_H_
#include <map>
#include <unordered_map>
#include <unordered_set>
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
  fifo_idx_t execute_order(Order& order, OrderResult& result);
  inline levelmap::MinLevelMap& get_sell_orders() { return this->sell_orders_; }

  inline levelmap::MaxLevelMap& get_buy_orders() { return this->buy_orders_; }

  inline void kill_buy_order(price_t price, fifo_idx_t idx)
  {
    this->buy_orders_[price].total -= this->buy_orders_[price].fifo[idx].qty;
    this->buy_orders_[price].fifo[idx].qty = 0;
  }

  inline void kill_sell_order(price_t price, fifo_idx_t idx)
  {
    this->sell_orders_[price].total -= this->sell_orders_[price].fifo[idx].qty;
    this->sell_orders_[price].fifo[idx].qty = 0;
  }

  inline bool empty() const noexcept
  {
    return this->sell_orders_.empty() && this->buy_orders_.empty();
  }

 private:
  levelmap::MaxLevelMap buy_orders_;
  levelmap::MinLevelMap sell_orders_;
};

class BookMap
{
 public:
  OrderResult handle_order(Order& order);
  OrderResult cancel_order(const oid_t oid);
  std::list<std::string> serialize();

 private:
  std::unordered_map<std::string, OrderBook> bmap_;
  std::unordered_map<oid_t, Order> lut_;
};

std::list<std::string> get_order_strings(std::deque<Order>& fifo, bool prepend);
std::list<std::string> serialize_book(OrderBook& book, char prepend);

};  // namespace order

#endif  // ORDER_BOOK_ORDER_BOOK_H_
