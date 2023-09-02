cmake_minimum_required(VERSION 3.18.2)

set(order_book_src
  order.cpp
  order_book.cpp
)
add_library(order ${order_book_src})
