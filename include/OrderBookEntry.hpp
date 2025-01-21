#pragma once
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <string>

// Define a fixed-precision decimal type with 50 decimal places
using boost::multiprecision::cpp_dec_float_50;

struct OrderBookEntry {
  cpp_dec_float_50 price;
  cpp_dec_float_50 quantity;
  std::string exchange;

  // Constructor using const references
  OrderBookEntry(const cpp_dec_float_50 &p, const cpp_dec_float_50 &q, const std::string &e)
      : price(p), quantity(q), exchange(e) {}

  // Move constructor for decimals and strings
  OrderBookEntry(cpp_dec_float_50 &&p, cpp_dec_float_50 &&q, std::string &&e)
      : price(std::move(p)), quantity(std::move(q)), exchange(std::move(e)) {}

  // Default constructor
  OrderBookEntry() = default;

  // Copy constructor and copy assignment operator
  OrderBookEntry(const OrderBookEntry &) = default;
  OrderBookEntry &operator=(const OrderBookEntry &) = default;

  // Move constructor and move assignment operator
  OrderBookEntry(OrderBookEntry &&) noexcept = default;
  OrderBookEntry &operator=(OrderBookEntry &&) noexcept = default;

  // For map comparison
  bool operator<(const OrderBookEntry &other) const { return price < other.price; }
};
