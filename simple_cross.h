#ifndef SIMPLE_CROSS_H_
#define SIMPLE_CROSS_H_

#include <list>
#include <functional>
#include <memory>
#include <order_book.h>
#include <order.h>

using results_t = std::list<std::string>;

class SimpleCross
{
 public:
  results_t action(const std::string& line);

 private:
  // Consider hashing on symbol and process per symbol group...
  order::BookMap books_;
};

struct Action {
  /* ACTION: single character value with the following definitions
      O - place order, requires OID, SYMBOL, SIDE, QTY, PX
      X - cancel order, requires OID
      P - print sorted book (see example below)
    OID: positive 32-bit integer value which must be unique for all orders
    SYMBOL: alpha-numeric string value. Maximum length of 8.
    SIDE: single character value with the following definitions (B - buy, S -
    sell) QTY: positive 16-bit integer value PX: positive double precision value
    (7.5 format)*/
  static std::unique_ptr<Action> deserialize(const std::string& action_string);
  Action() = default;
  virtual ~Action() = default;
  virtual results_t handle_action(order::BookMap& books)
  {
    (void)books;
    return {};
  }
};

#endif  // SIMPLE_CROSS_H_
