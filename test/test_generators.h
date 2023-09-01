#ifndef TEST_TEST_GENERATORS_H_
#define TEST_TEST_GENERATORS_H_
#include <order.h>
#include <vector>

std::vector<order::Order> generate_dummy_n_orders(size_t n, order::oid_t start,
                                                  order::qty_t qty = 10);

#endif  // TEST_TEST_GENERATORS_H_
