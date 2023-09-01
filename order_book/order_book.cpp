#include <string>
#include <functional>
#include <stdexcept>
#include <iostream>
#include "order_book.h"
#include "order.h"

namespace order
{

// TODO: Further restrict this template type
template <typename T, typename U, typename Compare>
static fifo_idx_t update_book(T& search_level, U& book_level, Order& order,
                              OrderResult& result)
{
  auto meets_price_req = [](price_t level_price, price_t order_price) -> bool {
    return Compare()(level_price, order_price);
  };
  auto search_it = search_level.begin();
  auto remaining_qty = order.qty;
  while (search_it != search_level.end() && remaining_qty > 0) {
    auto& [price, level] = *search_it;
    if (!meets_price_req(price, order.price)) {
      // all following prices will exceed/fall below the req
      break;
    }

    while (!level.empty() && remaining_qty > 0) {
      auto& candidate = level.fifo.front();
      if (candidate.qty > 0) {
        auto min_fill = std::min(candidate.qty, remaining_qty);
        auto inbound_filled = order;
        auto outbound_filled = candidate;
        inbound_filled.qty = min_fill;
        inbound_filled.price = price;  // fill price might change for inbound
        outbound_filled.qty = min_fill;

        result.orders.emplace_back(inbound_filled);
        result.orders.emplace_back(outbound_filled);
        candidate.qty -= min_fill;
        remaining_qty -= min_fill;
        search_level.dec_counts(price, min_fill);
      }

      if (candidate.qty == 0) {
        // candidate order has been exhausted, or was previously cancelled
        level.pop_front();
      }
    }

    if (level.empty()) {
      search_level.erase(search_it->first);
      search_it = search_level.begin();
    }
  }

  if (remaining_qty) {
    order.qty = remaining_qty;
    book_level[order.price].push_back(order);
    book_level.inc_counts(order.price, remaining_qty);
    return static_cast<fifo_idx_t>(book_level[order.price].fifo.end() -
                                   book_level[order.price].fifo.begin() - 1);
  }
  return kMaxDQIdx;
}

fifo_idx_t OrderBook::execute_order(Order& order, OrderResult& result)
{
  if (order.side == OrderSide::kBuy) {
    return update_book<levelmap::MinLevelMap, levelmap::MaxLevelMap,
                       std::less_equal<price_t>>(
        this->sell_orders_, this->buy_orders_, order, result);
  } else {  // OrderSide::kSell
    return update_book<levelmap::MaxLevelMap, levelmap::MinLevelMap,
                       std::greater_equal<price_t>>(
        this->buy_orders_, this->sell_orders_, order, result);
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
  } else if (this->bmap_[order.symbol].empty()) {
    this->bmap_.erase(order.symbol);
    // TODO: erase from symbol registry
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
    if (this->bmap_[key.symbol].empty()) {
      this->bmap_.erase(key.symbol);
    }
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
    auto ofifo = it->second;
    sell_strings.splice(sell_strings.end(),
                        get_order_strings(ofifo.fifo, prepend));
  }
  auto buys = book.get_buy_orders();
  for (auto it = buys.begin(); it != buys.end(); ++it) {
    auto ofifo = it->second;
    buy_strings.splice(buy_strings.end(),
                       get_order_strings(ofifo.fifo, prepend));
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
