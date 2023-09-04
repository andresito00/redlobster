#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <cctype>
#include <functional>
#include <unordered_set>
#include <order_book.h>
#include <order.h>
#include <log.h>
#include "simple_cross.h"

static std::unordered_set<std::string> kAllowableActionTokens{"O", "X", "P"};
static std::unordered_set<char> kAllowableSides{'B', 'S'};
static size_t kInvalidSubstringSize = 10LU;

struct OrderAction : public Action {
  explicit OrderAction(uint32_t oid) : oid(oid) {}
  uint32_t oid;
};

struct PlaceOrderAction : public OrderAction {
  order::symbol_t symbol;
  order::Order order;

  PlaceOrderAction(uint32_t oid, order::symbol_t symbol, order::Order order)
      : OrderAction(oid), symbol(symbol), order(order)
  {
  }

  virtual results_t handle_action(order::BookMap* books) override final
  {
    return books->handle_order(&order).serialize();
  }
};

struct CancelOrderAction : public OrderAction {
  explicit CancelOrderAction(uint32_t oid) : OrderAction(oid) {}
  virtual results_t handle_action(order::BookMap* books) override final
  {
    return books->cancel_order(oid).serialize();
  }
};

struct PrintAction : public Action {
  virtual results_t handle_action(order::BookMap* books) override final
  {
    return books->serialize();
  }
};

/**
 * Allowable input formats for action_string
 * O 10001 IBM B 10 99.0
 * X 10002
 * P
 */
static inline bool is_whitespace(const std::string& line)
{
  return std::all_of(line.cbegin(), line.cend(), [](const char& c) {
    return std::isspace(static_cast<unsigned char>(c));
  });
}

static inline bool valid_symbol(const order::symbol_t& sym)
{
  if (sym.size() == 0 || sym.size() > order::kMaxSymbolSize) {
    return false;
  }
  return std::find_if(sym.cbegin(), sym.cend(), [](const auto& c) {
           return !std::isalnum(c);
         }) == sym.end();
}

static inline bool valid_qty_format(const std::string& qty_str)
{
  return std::find_if(qty_str.cbegin(), qty_str.cend(), [](const auto& c) {
           return !std::isdigit(c);
         }) == qty_str.end();
}

std::unique_ptr<Action> Action::deserialize(const std::string& action_string,
                                            results_t* err)
{
  if (action_string.size() == 0 || is_whitespace(action_string)) {
    return std::make_unique<Action>();
  }
  std::stringstream astream(action_string);
  std::string type;
  astream >> type;
  if (!kAllowableActionTokens.count(type)) {
    err->emplace_back("Invalid action: " +
                      action_string.substr(0, std::max(kInvalidSubstringSize,
                                                       action_string.size())) +
                      "...");
  }

  if (type == "P") {
    if (action_string.size() == 1) {
      return std::make_unique<PrintAction>();
    }
    err->emplace_back("Invalid Print Action request size: " +
                      action_string.substr(0, std::max(kInvalidSubstringSize,
                                                       action_string.size())) +
                      "...");
    return std::make_unique<Action>();

  } else if (type == "O") {
    order::oid_t oid;
    astream >> oid;

    order::symbol_t symbol;
    astream >> symbol;
    if (!valid_symbol(symbol)) {
      err->emplace_back("Invalid Symbol: " + symbol);
      return std::make_unique<Action>();
    }

    char side_char;
    astream >> side_char;
    if (!kAllowableSides.count(side_char)) {
      err->emplace_back(std::to_string(oid) + " Invalid order side: " + type);
      return std::make_unique<Action>();
    }

    order::OrderSide side =
        (side_char == 'B') ? order::OrderSide::kBuy : order::OrderSide::kSell;

    std::string qty_str;
    astream >> qty_str;
    if (!valid_qty_format(qty_str)) {
      err->emplace_back(
          std::to_string(oid) + " Invalid quantity format: " +
          qty_str.substr(0, std::max(qty_str.size(), kInvalidSubstringSize)));
      return std::make_unique<Action>();
    }
    size_t qty = std::stoul(qty_str);
    if (qty == 0 || qty > order::kMaxQuantity) {
      err->emplace_back(std::to_string(oid) +
                        " Quantity out of valid range: " + std::to_string(qty));
      return std::make_unique<Action>();
    }

    std::string price_str;
    astream >> price_str;
    order::price_t price = std::stod(price_str);
    if (price <= 0.0 || price > order::kMaxPrice) {
      err->emplace_back(std::to_string(oid) +
                        " Price <= 0 || > 9999999.99999 ");
      return std::make_unique<Action>();
    }
    return std::make_unique<PlaceOrderAction>(
        oid, symbol,
        order::Order{oid, symbol, side, static_cast<order::qty_t>(qty), price});

  } else if (type == "X") {
    order::oid_t oid;
    astream >> oid;
    return std::make_unique<CancelOrderAction>(oid);
  }
  return std::make_unique<Action>();
}

results_t SimpleCross::action(const std::string& line)
{
  results_t err;
  auto action = Action::deserialize(line, &err);
  if (err.size()) {
    return err;
  }
  return action->handle_action(&books_);
}
