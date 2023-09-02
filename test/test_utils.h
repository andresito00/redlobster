#ifndef TEST_TEST_GENERATORS_H_
#define TEST_TEST_GENERATORS_H_
#include <order.h>
#include <vector>
#include <cassert>
#include <numeric>

#define assertm(exp, msg) assert(((void)msg, exp))

const extern size_t kMaxOrders;

order::Order generate_dummy_order(
    order::oid_t oid, order::qty_t qty,
    order::OrderSide side = order::OrderSide::kBuy);
std::vector<order::Order> generate_dummy_n_orders(size_t n, order::oid_t start,
                                                  order::qty_t qty = 10);

#endif  // TEST_TEST_GENERATORS_H_
