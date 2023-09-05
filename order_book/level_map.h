#ifndef ORDER_BOOK_LEVEL_MAP_H_
#define ORDER_BOOK_LEVEL_MAP_H_
#include <cstddef>
#include <algorithm>
#include <map>
#include <memory>
#include "order.h"

namespace levelmap
{
#ifdef __linux__ // TODO(andres): Use __has_feature instead, if possible.
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

    void pop_front() { fifo.pop_front(); }

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

  void dec_size() { --total_fifos_size_; }

  decltype(auto) front_with_key(const Key& k)
  {
    return fifo_map_.at(k).front();
  }

  decltype(auto) push_back_with_key(const Key& k, const Value& v)
  {
    OQueue* level = &fifo_map_[k];
    inc_counts(level, v.qty);
    ++total_fifos_size_;
    return level->push_back(v);
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

  void inc_counts(OQueue* level, size_t count)
  {
    level->num_orders += count;
    num_orders_ += count;
  }

  void dec_counts(OQueue& level, size_t count)
  {
    level.num_orders -= count;
    num_orders_ -= count;
  }

  void dec_counts(OQueue* level, size_t count)
  {
    level->num_orders -= count;
    num_orders_ -= count;
  }

  void zero_out_order(order::price_t price, order::fifo_idx_t idx)
  {
    OQueue* level = &fifo_map_[price];
    dec_counts(level, level->fifo[idx].qty);
    level->fifo[idx].qty = 0;
  }

  /**
   * Users MUST try/catch this in any usage where they
   * given key is either:
   * a) NOT first discovered from underlying map or,
   * b) is subject to change through some other out-of-band modification.
   *
   * Right now, this is only used in update_book with keys that were
   * found in the map.
  */
  decltype(auto) erase(const Key& k)
  {
    // We may be erasing a FIFO that reports 0-qty but has
    // Order objects inside, nevertheless.
    total_fifos_size_ -= fifo_map_.at(k).size();
    return fifo_map_.erase(k);
  }

  const auto& get_level(const Key k) const { return fifo_map_.at(k); }

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
