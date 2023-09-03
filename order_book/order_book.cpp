#include <string>
#include <functional>
#include <stdexcept>
#include "order_book.h"
#include "order.h"

namespace order
{

// templating this function on different types: min vs. max level map --> 2023 bytes with O(s)/clang 12
// runtime changes to implement get_first_level and meets_price_req:  --> 1245 bytes with O(s)/clang 12
static qty_t update_book(levelmap::MinLevelMap *search_levels,
                         std::function<bool(price_t, price_t)> meets_price_req,
                         Order *order, OrderResult *result)
{
  auto remaining_qty = order->qty;
  while (!search_levels->fifos_empty() && remaining_qty > 0) {
    auto &[price, level] = search_levels->get_first_level(order);
    if (!meets_price_req(price, order->price)) {
      // all following prices will exceed/fall below the req
      break;
    }

    while (level.size() != 0 && remaining_qty > 0) {
      auto &candidate = level.front();
      if (candidate.qty > 0) {
        auto min_fill = std::min(candidate.qty, remaining_qty);
        result->orders.emplace_back(order->oid, order->symbol, order->side,
                                    min_fill, price);
        result->orders.emplace_back(candidate.oid, candidate.symbol,
                                    candidate.side, min_fill, price);
        candidate.qty -= min_fill;
        remaining_qty -= min_fill;
        search_levels->dec_counts(level, min_fill);
      }

      if (candidate.qty == 0) {
        // candidate order has been exhausted, or was previously cancelled
        level.pop_front();
        // The price we pay for not looping over our FIFOs to determine
        // element count...
        search_levels->dec_size();
      }
    }

    if (level.empty()) {
      // This is where we do our clean-up.
      // If the total QTY COUNTS are 0 we can erase the price level.
      //
      // We don't necesssarily have to wait for the FIFO to be empty of objects.
      //
      // Normally map.erase does not throw.
      // However, in most cases (except this one) we SHOULD
      // be catching an exception here because of the .at check for the size
      // tally update in the wrapper implementation.
      search_levels->erase(price);
    }
  }
  return order->qty = remaining_qty;
}

fifo_idx_t OrderBook::place_order(Order *order, OrderResult *result)
{
  // TODO(andres): check for optimal branch assembly...
  std::function<bool(price_t, price_t)> compare_fn;
  if (order->side == order::OrderSide::kBuy) {
    compare_fn = std::less_equal<price_t>();
  } else {
    compare_fn = std::greater_equal<price_t>();
  }
  auto search_levels =
      (order->side == order::OrderSide::kBuy) ? &sell_orders_ : &buy_orders_;
  auto book_levels =
      (order->side == order::OrderSide::kBuy) ? &buy_orders_ : &sell_orders_;
  result->type = ResultType::kFilled;
  if (auto remaining_qty =
          update_book(search_levels, compare_fn, order, result)) {
    auto price = order->price;
    auto next_idx =
        static_cast<fifo_idx_t>(book_levels->fifo_size_with_key(price));
    order->idx = next_idx;
    book_levels->push_back_with_key(price, *order);
    return next_idx;
  }
  return kMaxDQIdx;
}

std::vector<fifo_idx_t> OrderBook::place_orders(
    std::vector<Order> *orders, std::vector<OrderResult> *results)
{
  size_t n = orders->size();
  if (n != results->size()) {
    throw std::runtime_error(
        "Parameters `orders` and `results` must have equal sizes.");
  }
  std::vector<fifo_idx_t> result;
  result.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    // TODO(andres): This is hideous. Consider just using arrays as this is pretty
    // much only test code at this point.
    result.push_back(place_order(&(*orders)[i], &(*results)[i]));
  }
  return result;
}

/**
 * Locates the order we need to kill then 0s out the quantity.
*/
void OrderBook::kill_order(const Order &reference_order_data)
{
  auto &levels = (reference_order_data.side == OrderSide::kBuy) ? buy_orders_
                                                                : sell_orders_;
  levels.zero_out_order(reference_order_data.price, reference_order_data.idx);
}

/**
 * handle_order dispatches an inbound order
 * Returns a kError
*/
OrderResult BookMap::handle_order(Order *order)
{
  // check for dups
  auto curr_oid = order->oid;
  if (order_lut_.count(curr_oid)) {
    return OrderResult{ResultType::kError,
                       std::to_string(curr_oid) + " Duplicate order id",
                       {}};
  }
  if (order->price == 0.0) {
    return OrderResult{ResultType::kError,
                       std::to_string(curr_oid) + " Invalid limit price: 0",
                       {}};
  }
  OrderResult result{};
  auto dq_idx = book_map_[order->symbol].place_order(order, &result);
  if (dq_idx != kMaxDQIdx) {
    order_lut_.emplace(curr_oid, std::move(*order));
  } else if (book_map_[order->symbol].empty()) {
    book_map_.erase(order->symbol);
    // TODO(andres): erase from symbol registry if/when I implement
  }
  return result;
}

/**
 * Cancel order is a bit weird. We will 0-out the qty for the
 * order to be cancelled with an O(log(n)) lookup instead of destroying it.
 * (Unless the STL is doing something fancier to make .begin() of
 * a (std::map) red-black tree O(1).)
 *
 * We pay this penalty by searching through non-empty FIFOs in update_book
 * and finally popping them off if we encounter total order counts of 0.
 *
 * We do this to support the use of deques and HOPEFULLY a nicer
 * memory reference profile instead of linked list nodes, (which
 * would otherwise offer a nicer O(1) erase on paper).
 *
 * I'd like to try with linked lists instead of deques to see how
 * that affects performance if I have time.
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

std::list<std::string> get_order_strings(std::deque<Order> *fifo, char prepend)
{
  std::list<std::string> result;
  for (auto &o : *fifo) {
    if (o.qty) {
      result.emplace_back(o.str(prepend));
    }
  }
  return result;
}

std::list<std::string> serialize_book(OrderBook &book, char prepend)
{
  std::list<std::string> result{};
  std::list<std::string> sell_strings{};
  std::list<std::string> buy_strings{};
  auto sells = book.get_sell_orders();
  for (auto it = sells.crbegin(); it != sells.crend(); ++it) {
    auto ofifo = it->second;
    sell_strings.splice(sell_strings.end(),
                        get_order_strings(&ofifo.fifo, prepend));
  }
  auto buys = book.get_buy_orders();
  for (auto it = buys.crbegin(); it != buys.crend(); ++it) {
    auto ofifo = it->second;
    buy_strings.splice(buy_strings.end(),
                       get_order_strings(&ofifo.fifo, prepend));
  }
  result.splice(result.end(), sell_strings);
  result.splice(result.end(), buy_strings);
  return result;
}

std::list<std::string> BookMap::serialize()
{
  std::list<std::string> result;
  for (auto &[symbol, book] : book_map_) {
    result.splice(result.end(), order::serialize_book(book, 'P'));
  }
  return result;
}

}  // namespace order
