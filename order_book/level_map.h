#ifndef ORDER_BOOK_LEVEL_MAP_H_
#define ORDER_BOOK_LEVEL_MAP_H_
#include <cstddef>
#include <algorithm>
#include <map>
#include <memory>
#include "order.h"

namespace levelmap
{
#ifdef __linux__
template <typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;
template <arithmetic Key, typename Value,
#elif __APPLE__
template <typename Key, typename Value,
#endif  // OS
          template <typename, typename> class FifoContainer,
          template <typename> class Compare>
class LevelMap
{
 public:
  struct OQueue {
    OQueue() = default;
    size_t num_orders;
    FifoContainer<Value, std::allocator<Value>> fifo;
    decltype(auto) pop_front() { return fifo.pop_front(); }

    decltype(auto) front() { return fifo.front(); }

    decltype(auto) push_back(const Value& v) { return fifo.push_back(v); }

    template <typename... Args>
    decltype(auto) emplace_back(Args&&... args)
    {
      return fifo.emplace_back(std::forward<Args>(args)...);
    }
    size_t size() const { return fifo.size(); }
    bool fifo_empty() const { return fifo.empty(); }
    bool empty() const { return num_orders == 0; }
  };

  LevelMap() = default;

  size_t fifos_size() const noexcept
  {
    size_t result = 0;
    std::for_each(
        fifo_map_.begin(), fifo_map_.end(),
        [&result](const auto& pair) { result += pair.second.size(); });
    return result;
  }
  size_t order_count() const noexcept { return num_orders_; }
  decltype(auto) begin() { return fifo_map_.begin(); }

  decltype(auto) end() { return fifo_map_.end(); }

  decltype(auto) rbegin() { return fifo_map_.rbegin(); }

  decltype(auto) rend() { return fifo_map_.rend(); }

  decltype(auto) crbegin() const { return fifo_map_.crbegin(); }

  decltype(auto) crend() const { return fifo_map_.crend(); }

  bool empty() const { return num_orders_ == 0; }

  bool map_empty() const { return fifo_map_.empty(); }

  bool map_size() const { return fifo_map_.size(); }

  bool fifos_empty() const { return fifos_size() == 0; }

  void inc_counts(order::price_t price, size_t count)
  {
    fifo_map_[price].num_orders += count;
    num_orders_ += count;
  }

  void dec_counts(order::price_t price, size_t count)
  {
    fifo_map_[price].num_orders -= count;
    num_orders_ -= count;
  }

  template <typename... Args>
  decltype(auto) erase(Args&&... args)
  {
    return fifo_map_.erase(std::forward<Args>(args)...);
  }

  template <typename Arg>
  decltype(auto) operator[](Arg&& arg)
  {
    return fifo_map_.operator[](std::forward<Arg>(arg));
  }

  auto& get_first_level(const Value order)
  {
    if (order.side == order::OrderSide::kBuy) {
      return *begin();
    } else {
      return *rbegin();
    }
  }

 private:
  std::map<Key, OQueue, Compare<Key>> fifo_map_;
  size_t num_orders_;
};

using MinLevelMap =
    LevelMap<order::price_t, order::Order, std::deque, std::less>;

}  // namespace levelmap

#endif  // ORDER_BOOK_LEVEL_MAP_H_
