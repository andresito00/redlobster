#include <unordered_map>
#include <sstream>
#include <memory>
#include <cctype>
#include <functional>
#include <unordered_set>
#include <order_book.h>
#include "simple_cross.h"

static std::unordered_set<std::string> kAllowableActionTokens{"O", "X", "P"};
static std::unordered_set<char> kAllowableSides{'B', 'S'};

struct OrderAction : public Action {
  explicit OrderAction(uint32_t oid) : oid(oid) {}
  uint32_t oid;
};

struct PlaceOrderAction : public OrderAction {
  std::string symbol;
  order::Order order;
  PlaceOrderAction(uint32_t oid, std::string symbol, order::Order order)
      : OrderAction(oid), symbol(symbol), order(order)
  {
  }
  virtual results_t handle_action(order::BookMap& books) override final
  {
    return books.handle_order(this->order).serialize();
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

std::unique_ptr<Action> Action::deserialize(const std::string& action_string)
{
  if (action_string.size() == 0 || is_whitespace(action_string) ||
      action_string.starts_with('#')) {
    return std::make_unique<Action>();
  }
  std::stringstream astream(action_string);
  std::string type;
  astream >> type;
  if (!kAllowableActionTokens.count(type)) {
    throw std::runtime_error("Invalid action: " + type);
  }

  if (type == "P") {
    return std::make_unique<PrintAction>();

  } else if (type == "O") {
    uint32_t oid;
    astream >> oid;

    std::string symbol;
    astream >> symbol;

    char side_char;
    astream >> side_char;
    if (!kAllowableSides.count(side_char)) {
      throw std::runtime_error("Invalid order side: " + type);
    }
    order::OrderSide side =
        (side_char == 'B') ? order::OrderSide::kBuy : order::OrderSide::kSell;

    uint16_t qty;
    astream >> qty;

    double price;
    astream >> price;
    return std::make_unique<PlaceOrderAction>(
        oid, symbol, order::Order{oid, symbol, side, qty, price});

  } else if (type == "X") {
    uint32_t oid;
    astream >> oid;
    return std::make_unique<CancelOrderAction>(oid);
  }
  return std::make_unique<Action>();
}

results_t SimpleCross::action(const std::string& line)
{
  auto action = Action::deserialize(line);
  return action->handle_action(this->books_);
}
