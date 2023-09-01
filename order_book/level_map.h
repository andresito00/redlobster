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
  struct OQueue {
    OQueue() = default;
    size_t num_orders;
    std::deque<Value> fifo;
    decltype(auto) pop_front() { return fifo.pop_front(); }

    decltype(auto) push_back(const Value& v) { return fifo.push_back(v); }

    bool fifo_empty() const { return fifo.empty(); }
    bool empty() const { return num_orders == 0; }
  };

  LevelMap() = default;

  // Should be tracking this size...
  size_t fifos_size() const noexcept
  {
    size_t result = 0;
    std::for_each(
        fifo_map.begin(), fifo_map.end(),
        [&result](const auto& pair) { result += pair.second.fifo.size(); });
    return result;
  }
  size_t order_count() const noexcept { return num_orders; }
  decltype(auto) begin() { return fifo_map.begin(); }

  decltype(auto) end() { return fifo_map.end(); }

  decltype(auto) rbegin() { return fifo_map.rbegin(); }

  decltype(auto) rend() { return fifo_map.rend(); }

  bool empty() const { return num_orders == 0; }

  bool map_empty() const { return fifo_map.empty(); }

  bool map_size() const { return fifo_map.size(); }

  bool fifos_empty() const { return fifos_size() == 0; }

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

  auto& get_first_level(const Value& order)
  {
    if (order.side == order::OrderSide::kBuy) {
      return *begin();
    } else {
      return *rbegin();
    }
  }

 private:
  std::map<Key, OQueue, Compare<Key>> fifo_map;
  size_t num_orders;
};

using MinLevelMap = LevelMap<order::price_t, order::Order, std::less>;

};  // namespace levelmap

#endif  // ORDER_BOOK_LEVEL_MAP_H_
