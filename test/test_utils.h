#ifndef TEST_TEST_GENERATORS_H_
#define TEST_TEST_GENERATORS_H_
#include <vector>
#include <string>
#include <cassert>
#include <numeric>
#include <order.h>

#define assertm(exp, msg) assert(((void)msg, exp))

const extern order::qty_t kDefaultOrderQty;

order::Order generate_dummy_order(
    order::oid_t oid, order::qty_t qty,
    order::OrderSide side = order::OrderSide::kBuy, std::string symbol = "IBM");
std::vector<order::Order> generate_dummy_n_orders(size_t n, order::oid_t start,
                                                  order::qty_t qty = 10,
                                                  std::string symbol = "IBM");
std::vector<order::Order> generate_asc_desc_full_fills(
    size_t n, order::oid_t start, order::qty_t qty = kDefaultOrderQty);
std::vector<order::Order> generate_asc_asc_full_fills(
    size_t n, order::oid_t start, order::qty_t qty = kDefaultOrderQty);

#endif  // TEST_TEST_GENERATORS_H_
