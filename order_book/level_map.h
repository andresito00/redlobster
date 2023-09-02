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

    void pop_front() { return fifo.pop_front(); }

    decltype(auto) front() { return fifo.front(); }

    template <typename Arg>
    void push_back(Arg&& arg)
    {
      return fifo.push_back(std::forward<Arg>(arg));
    }

    template <typename... Args>
    void emplace_back(Args&&... args)
    {
      return fifo.emplace_back(std::forward<Args>(args)...);
    }

    size_t size() const { return fifo.size(); }
    bool empty() const { return num_orders == 0; }
  };

  LevelMap() = default;

  size_t fifos_size() const noexcept { return total_fifos_size_; }

  bool level_empty(const Key& k) const { return fifo_map_.at(k).empty(); }

  size_t fifo_size_with_key(const Key& k)
  {
    if (fifo_map_.count(k)) {
      return fifo_map_.at(k).size();
    }
    return 0;
  }

  decltype(auto) pop_front_with_key(const Key& k)
  {
    --total_fifos_size_;
    return fifo_map_.at(k).pop_front();
  }

  decltype(auto) front_with_key(const Key& k)
  {
    return fifo_map_.at(k).front();
  }

  decltype(auto) push_back_with_key(const Key& k, const Value& v)
  {
    ++total_fifos_size_;
    return fifo_map_[k].push_back(v);
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

  void kill_order(order::price_t price, size_t idx)
  {
    dec_counts(price, fifo_map_.at(price).fifo[idx].qty);
    fifo_map_[price].fifo[idx].qty = 0;
  }

  decltype(auto) erase(const Key& k)
  {
    total_fifos_size_ -= fifo_map_.at(k).size();
    return fifo_map_.erase(k);
  }

  auto& get_first_level(const Value* order)
  {
    if (order->side == order::OrderSide::kBuy) {
      return *begin();
    } else {
      return *rbegin();
    }
  }

 private:
  std::map<Key, OQueue, Compare<Key>> fifo_map_;
  size_t num_orders_;
  size_t total_fifos_size_;
};

using MinLevelMap =
    LevelMap<order::price_t, order::Order, std::deque, std::less>;

}  // namespace levelmap

#endif  // ORDER_BOOK_LEVEL_MAP_H_
