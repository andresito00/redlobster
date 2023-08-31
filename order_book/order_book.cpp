#include <string>
#include <functional>
#include <stdexcept>
#include <iostream>
#include "order_book.h"
#include "order.h"

namespace order
{

// TODO: Further restrict this template type
template <typename T, typename U>
static dq_idx_t update_book(T& search_level, U& book_level,
                            std::function<bool(double, double)> meets_price_req,
                            Order& order, OrderResult& result)
{
  // for every level starting at get_start, search for possible matches
  auto search_it = search_level.begin();
  auto remaining_qty = order.qty;
  while (search_it != search_level.end() && remaining_qty > 0) {
    auto& [price, order_queue] = *search_it;
    if (!meets_price_req(price, order.price)) {
      // all following prices will exceed/fall below the req
      break;
    }

    while (!order_queue.empty() && remaining_qty > 0) {
      // req_quantity can never be 0 here
      auto& candidate = order_queue.front();
      if (candidate.qty >= remaining_qty) {
        candidate.qty -= remaining_qty;

        auto inbound_filled = Order(order);
        inbound_filled.qty = remaining_qty;
        inbound_filled.price = price;
        result.orders.push_back(inbound_filled);

        auto outbound_filled = Order(candidate);
        outbound_filled.qty = remaining_qty;
        result.orders.push_back(outbound_filled);

        remaining_qty = 0;

      } else {
        remaining_qty -= candidate.qty;
        // report filled orders
        if (candidate.qty > 0) {
          auto inbound_filled = Order(order);
          inbound_filled.qty = candidate.qty;
          inbound_filled.price = price;
          result.orders.push_back(inbound_filled);

          auto outbound_filled = Order(candidate);
          result.orders.push_back(outbound_filled);

          candidate.qty = 0;
        }
      }

      if (candidate.qty == 0) {
        // candidate order has been exhausted, or was previously cancelled
        order_queue.pop_front();
        if (order_queue.empty()) {
          // Erase invalidates the current iterators, reset them.
          search_level.erase(search_it->first);
          search_it = search_level.begin();
          break;
        }
      }
    }
  }

  if (remaining_qty) {
    order.qty = remaining_qty;
    // TODO: Currently considering using a shared pointer between map and LUT
    // When references are removed from both, the Order will be freed.
    book_level[order.price].push_back(order);
    return static_cast<dq_idx_t>(book_level[order.price].end() -
                                 book_level[order.price].begin() - 1);
  }
  return kMaxDQIdx;
}

oid_t OrderBook::execute_order(Order& order, OrderResult& result)
{
  if (order.side == OrderSide::kBuy) {
    auto lte_compare = [](double level_price, double order_price) -> bool {
      return level_price <= order_price;
    };
    return update_book<MinLevelMap, MaxLevelMap>(
        this->sell_orders_, this->buy_orders_, lte_compare, order, result);
  } else {  // OrderSide::kSell
    auto gte_compare = [](double level_price, double order_price) -> bool {
      return level_price >= order_price;
    };
    return update_book<MaxLevelMap, MinLevelMap>(
        this->buy_orders_, this->sell_orders_, gte_compare, order, result);
  }
}

OrderResult BookMap::handle_order(Order& order)
{
  // check for dups
  auto curr_oid = order.oid;
  if (this->lut_.count(curr_oid)) {
    return OrderResult{ResultType::kError,
                       std::to_string(curr_oid) + " Duplicate order id",
                       {}};
  }
  // reserve while in flight...
  this->lut_[curr_oid];
  OrderResult result{ResultType::kFilled, "", {}};
  auto dq_idx = this->bmap_[order.symbol].execute_order(order, result);
  order.idx = dq_idx;
  if (dq_idx != kMaxDQIdx) {
    this->lut_[curr_oid] = order;
    return result;
  }
  this->lut_.erase(curr_oid);
  return result;
}

OrderResult BookMap::cancel_order(const oid_t oid)
{
  if (this->lut_.count(oid)) {
    auto key = this->lut_[oid];
    // Copy out useful metadata before we erase the K,V pair.
    auto result = OrderResult{ResultType::kCancelled, "", {key}};
    if (key.side == OrderSide::kBuy) {
      this->bmap_[key.symbol].kill_buy_order(key.price, key.idx);
    } else {
      this->bmap_[key.symbol].kill_sell_order(key.price, key.idx);
    }
    // Erase this oid, so incoming orders may now use it.
    // The Order instance will be destroyed lazily.
    this->lut_.erase(oid);
    return result;
  }
  return OrderResult{
      ResultType::kError, "Invalid OID: " + std::to_string(oid), {}};
}

std::list<std::string> get_order_strings(std::deque<Order>& fifo, char prepend)
{
  std::list<std::string> result;
  for (auto& o : fifo) {
    if (o.qty) {
      result.emplace_back(o.str(prepend));
    }
  }
  return result;
}

std::list<std::string> serialize_book(OrderBook& book, char prepend)
{
  std::list<std::string> result{};
  std::list<std::string> sell_strings{};
  std::list<std::string> buy_strings{};
  auto sells = book.get_sell_orders();
  for (auto it = sells.rbegin(); it != sells.rend(); ++it) {
    auto order_fifo = it->second;
    sell_strings.splice(sell_strings.end(),
                        get_order_strings(order_fifo, prepend));
  }
  auto buys = book.get_buy_orders();
  for (auto it = buys.begin(); it != buys.end(); ++it) {
    auto order_fifo = it->second;
    buy_strings.splice(buy_strings.end(),
                       get_order_strings(order_fifo, prepend));
  }
  result.splice(result.end(), sell_strings);
  result.splice(result.end(), buy_strings);
  return result;
}

std::list<std::string> BookMap::serialize()
{
  std::list<std::string> result;
  for (auto& [symbol, book] : bmap_) {
    result.splice(result.end(), order::serialize_book(book, 'P'));
  }
  return result;
}

};  // namespace order
