#include <string>
#include <functional>
#include <stdexcept>
#include <iostream>
#include "order_book.h"
#include "order.h"

namespace order
{

// templating this function on different types: min vs. max level map --> 2023 bytes with O(s)/clang 12
// runtime changes to implement get_first_level and meets_price_req:  --> 1245 bytes with O(s)/clang 12
static qty_t update_book(levelmap::MinLevelMap& search_levels,
                         std::function<bool(price_t, price_t)> meets_price_req,
                         Order& order, OrderResult& result)
{
  auto remaining_qty = order.qty;
  while (!search_levels.fifos_empty() && remaining_qty > 0) {
    auto& [price, level] = search_levels.get_first_level(order);
    if (!meets_price_req(price, order.price)) {
      // all following prices will exceed/fall below the req
      break;
    }

    while (!level.fifo_empty() && remaining_qty > 0) {
      auto& candidate = level.front();
      if (candidate.qty > 0) {
        auto min_fill = std::min(candidate.qty, remaining_qty);
        auto inbound_filled = order;
        auto outbound_filled = candidate;
        inbound_filled.qty = min_fill;
        inbound_filled.price = price;  // fill price might change for inbound
        outbound_filled.qty = min_fill;

        result.orders.emplace_back(std::move(inbound_filled));
        result.orders.emplace_back(std::move(outbound_filled));
        candidate.qty -= min_fill;
        remaining_qty -= min_fill;
        search_levels.dec_counts(price, min_fill);
      }

      if (candidate.qty == 0) {
        // candidate order has been exhausted, or was previously cancelled
        level.pop_front();
      }
    }

    if (level.empty()) {
      search_levels.erase(price);
    }
  }
  return order.qty = remaining_qty;
}

fifo_idx_t OrderBook::place_order(Order& order, OrderResult& result)
{
  // TODO(andres): check for optimal branch assembly...
  std::function<bool(price_t, price_t)> compare_fn;
  if (order.side == order::OrderSide::kBuy) {
    compare_fn = std::less_equal<price_t>();
  } else {
    compare_fn = std::greater_equal<price_t>();
  }
  auto& search_levels =
      (order.side == order::OrderSide::kBuy) ? sell_orders_ : buy_orders_;
  auto& book_levels =
      (order.side == order::OrderSide::kBuy) ? buy_orders_ : sell_orders_;
  if (auto remaining_qty =
          update_book(search_levels, compare_fn, order, result)) {
    auto price = order.price;
    auto next_idx = static_cast<fifo_idx_t>(book_levels[price].size());
    order.idx = next_idx;
    book_levels[price].push_back(order);
    book_levels.inc_counts(price, remaining_qty);
    return next_idx;
  }
  return kMaxDQIdx;
}

std::vector<fifo_idx_t> OrderBook::place_orders(
    std::vector<Order>& orders, std::vector<OrderResult>& results)
{
  size_t n = orders.size();
  if (n != results.size()) {
    return {};
  }
  std::vector<fifo_idx_t> result;
  result.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    result.push_back(place_order(orders[i], results[i]));
  }
  return result;
}

void OrderBook::kill_order(Order& order)
{
  auto& levels = (order.side == OrderSide::kBuy) ? buy_orders_ : sell_orders_;
  levels.dec_counts(order.price, levels[order.price].fifo[order.idx].qty);
  levels[order.price].fifo[order.idx].qty = 0;
}

OrderResult BookMap::handle_order(Order& order)
{
  // check for dups
  auto curr_oid = order.oid;
  if (order_lut_.count(curr_oid)) {
    return OrderResult{ResultType::kError,
                       std::to_string(curr_oid) + " Duplicate order id",
                       {}};
  }
  // reserve while in flight...
  order_lut_[curr_oid];
  OrderResult result{ResultType::kFilled, "", {}};
  auto dq_idx = book_map_[order.symbol].place_order(order, result);
  if (dq_idx != kMaxDQIdx) {
    order_lut_[curr_oid] = order;
    return result;
  } else if (book_map_[order.symbol].empty()) {
    book_map_.erase(order.symbol);
    // TODO(andres): erase from symbol registry
  }
  order_lut_.erase(curr_oid);
  return result;
}

/**
 * Cancel order is a bit weird. We will 0-out the qty for the
 * order to be cancelled with an O(1) lookup.
 * Because the order FIFOs are implemented
 * with deque, not linked lists,
*/
OrderResult BookMap::cancel_order(const oid_t oid)
{
  if (order_lut_.count(oid)) {
    auto order = order_lut_[oid];
    // Copy out useful metadata before we erase the K,V pair.
    auto result = OrderResult{ResultType::kCancelled, "", {order}};
    book_map_[order.symbol].kill_order(order);
    // Erase this oid, so incoming orders may now use it.
    // The Order instance will be destroyed lazily.
    order_lut_.erase(oid);
    if (book_map_[order.symbol].empty()) {
      book_map_.erase(order.symbol);
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
  for (auto it = sells.crbegin(); it != sells.crend(); ++it) {
    auto ofifo = it->second;
    sell_strings.splice(sell_strings.end(),
                        get_order_strings(ofifo.fifo, prepend));
  }
  auto buys = book.get_buy_orders();
  for (auto it = buys.crbegin(); it != buys.crend(); ++it) {
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
  for (auto& [symbol, book] : book_map_) {
    result.splice(result.end(), order::serialize_book(book, 'P'));
  }
  return result;
}

}  // namespace order
