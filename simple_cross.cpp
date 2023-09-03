#include <unordered_map>
#include <sstream>
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
  virtual results_t handle_action(order::BookMap& books) override final
  {
    return books.handle_order(&order).serialize();
  }
};

struct CancelOrderAction : public OrderAction {
  explicit CancelOrderAction(uint32_t oid) : OrderAction(oid) {}
  virtual results_t handle_action(order::BookMap& books) override final
  {
    return books.cancel_order(oid).serialize();
  }
};

struct PrintAction : public Action {
  virtual results_t handle_action(order::BookMap& books) override final
  {
    return books.serialize();
  }
};

/**
 * Allowable input formats for action_string
 * O 10001 IBM B 10 99.0
 * X 10002
 * P
 */
bool is_whitespace(const std::string& line)
{
  return std::all_of(line.cbegin(), line.cend(), [](const char& c) {
    return std::isspace(static_cast<unsigned char>(c));
  });
}

bool valid_symbol(const order::symbol_t& sym)
{
  return std::find_if(sym.cbegin(), sym.cend(), [](const auto& c) {
           return !std::isalnum(c);
         }) == sym.end();
}

std::unique_ptr<Action> Action::deserialize(const std::string& action_string)
{
  if (action_string.size() == 0 || is_whitespace(action_string)) {
    return std::make_unique<Action>();
  }
  std::stringstream astream(action_string);
  std::string type;
  astream >> type;
  if (!kAllowableActionTokens.count(type)) {
    LOG_ERROR("Invalid action: " + action_string);
  }

  if (type == "P") {
    if (action_string.size() == 1) {
      return std::make_unique<PrintAction>();
    }
    LOG_ERROR("Invalid Print Action request size: " + action_string);
    return std::make_unique<Action>();

  } else if (type == "O") {
    order::oid_t oid;
    astream >> oid;

    order::symbol_t symbol;
    astream >> symbol;
    if (!valid_symbol(symbol)) {
      LOG_ERROR("Invalid Symbol: " + symbol);
      return std::make_unique<Action>();
    }

    char side_char;
    astream >> side_char;
    if (!kAllowableSides.count(side_char)) {
      LOG_ERROR(std::to_string(oid) + " Invalid order side: " + type);
      return std::make_unique<Action>();
    }

    order::OrderSide side =
        (side_char == 'B') ? order::OrderSide::kBuy : order::OrderSide::kSell;

    order::qty_t qty;
    astream >> qty;
    if (qty == 0) {
      LOG_ERROR(std::to_string(oid) +
                " Invalid quantity: " + std::to_string(qty));
      return std::make_unique<Action>();
    }

    order::price_t price;
    astream >> price;
    return std::make_unique<PlaceOrderAction>(
        oid, symbol, order::Order{oid, symbol, side, qty, price});

  } else if (type == "X") {
    order::oid_t oid;
    astream >> oid;
    return std::make_unique<CancelOrderAction>(oid);
  }
  return std::make_unique<Action>();
}

results_t SimpleCross::action(const std::string& line)
{
  auto action = Action::deserialize(line);
  return action->handle_action(books_);
}
