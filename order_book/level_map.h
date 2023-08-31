#ifndef ORDER_BOOK_LEVEL_MAP_H_
#define ORDER_BOOK_LEVEL_MAP_H_

#include <algorithm>
#include <map>
#include "order.h"

namespace levelmap
{
template <typename Key, typename Value, template <typename> class Compare>
class LevelMap
{
 public:
  decltype(auto) begin() { return fifo_map.begin(); }

  decltype(auto) end() { return fifo_map.end(); }

  decltype(auto) rbegin() { return fifo_map.rbegin(); }

  decltype(auto) rend() { return fifo_map.rend(); }

  bool empty() const { return fifo_map.empty() || total == 0; }

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
  std::map<Key, std::deque<Value>, Compare<Key>> fifo_map;
  size_t total;
};

using MaxLevelMap = LevelMap<order::price_t, order::Order, std::greater>;
using MinLevelMap = LevelMap<order::price_t, order::Order, std::less>;

};  // namespace levelmap

#endif  // ORDER_BOOK_LEVEL_MAP_H_
