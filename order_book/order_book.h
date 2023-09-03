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

/**
 * There will be one OrderBook per symbol
*/
class OrderBook
{
 public:
  OrderBook() = default;

  /**
   * Attempts to match an inbound Order (buy or sell),
   * if a viable candidate is not found the order is placed
   * in one of buy_orders_ or sell_orders_ to be matched later.
  */
  fifo_idx_t place_order(Order* order, OrderResult* result);

  /**
   * Not really used outside of tests, but should be able to batch orders.
  */
  std::vector<fifo_idx_t> place_orders(std::vector<Order>* orders,
                                       std::vector<OrderResult>* results);

  /**
   * 0-out the quantity for an order in the book using its information
   * from the order_lut_
  */
  void kill_order(const Order& order);
  inline const levelmap::MinLevelMap& get_sell_orders() const
  {
    return sell_orders_;
  }
  inline const levelmap::MinLevelMap& get_buy_orders() const
  {
    return buy_orders_;
  }

  /**
   * Below are the various ways we can probe
   * an order book's physical size in memory as
   * well as our count of the internal Order.qty values
  */
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

/**
 * A BookMap owns one OrderBook per Symbol
*/
class BookMap
{
 public:
  OrderResult handle_order(Order* order);
  OrderResult cancel_order(const oid_t oid);
  std::list<std::string> serialize();

 private:
  // book_map_ is where we find the real orders that are in flight
  std::unordered_map<symbol_t, OrderBook> book_map_;
  /**
   * order_lut_ is the rosetta stone for finding an order in the
   * book_map_, basically using all of that order's internal information.
   * order_lut_ may be used to query information about the order EXCEPT
   * QUANTITY. QUANTITY IS STALE FROM THE MOMENT THE ORDER WAS PLACED
   * IN THE LIMIT ORDER QUEUE.
  */
  std::unordered_map<oid_t, Order> order_lut_;
};

std::list<std::string> get_order_strings(std::deque<Order>& fifo, bool prepend);
std::list<std::string> serialize_book(OrderBook& book, char prepend);

}  // namespace order

#endif  // ORDER_BOOK_ORDER_BOOK_H_
