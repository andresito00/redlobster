#ifndef ORDER_BOOK_LEVEL_MAP_H_
#define ORDER_BOOK_LEVEL_MAP_H_

#include <algorithm>
#include <map>
#include "order.h"

namespace levelmap
{
template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

template <arithmetic Key, typename Value, template <typename> class Compare>
class LevelMap
{
 public:
  decltype(auto) begin() { return fifo_map.begin(); }

  decltype(auto) end() { return fifo_map.end(); }

  decltype(auto) rbegin() { return fifo_map.rbegin(); }

  decltype(auto) rend() { return fifo_map.rend(); }

  bool empty() const { return num_orders == 0 || fifo_map.empty(); }

  void inc_counts(order::price_t price, size_t count)
  {
    fifo_map[price].num_orders += count;
    num_orders += count;
  }

  void dec_counts(order::price_t price, size_t count)
  {
    fifo_map[price].num_orders -= count;
    num_orders -= count;
  }

  template <typename... Args>
  decltype(auto) erase(Args&&... args)
  {
    return fifo_map.erase(std::forward<Args>(args)...);
  }

  template <typename Arg>
  decltype(auto) operator[](Arg&& arg)
  {
    return fifo_map.operator[](std::forward<Arg>(arg));
  }

 private:
  struct OQueue {
    size_t num_orders;
    std::deque<Value> fifo;
    decltype(auto) pop_front() { return fifo.pop_front(); }

    decltype(auto) push_back(const Value& v) { return fifo.push_back(v); }

    bool empty() const { return num_orders == 0 || fifo.empty(); }
  };
  std::map<Key, OQueue, Compare<Key>> fifo_map;
  size_t num_orders;
};

using MaxLevelMap = LevelMap<order::price_t, order::Order, std::greater>;
using MinLevelMap = LevelMap<order::price_t, order::Order, std::less>;

};  // namespace levelmap

#endif  // ORDER_BOOK_LEVEL_MAP_H_
