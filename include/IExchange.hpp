#pragma once
#include <map>
#include <string>

#include "OrderBookEntry.hpp"

class IExchange {
 public:
  IExchange() = default;
  virtual ~IExchange() = default;
  virtual void fetchOrderBook() = 0;
  virtual const std::multimap<cpp_dec_float_50, OrderBookEntry> &getBids() const = 0;
  virtual const std::multimap<cpp_dec_float_50, OrderBookEntry> &getAsks() const = 0;
  virtual std::string getName() const = 0;

 protected:
  std::multimap<cpp_dec_float_50, OrderBookEntry> bids;
  std::multimap<cpp_dec_float_50, OrderBookEntry> asks;
  std::string name;

  // Move constructor and assignment operator
  IExchange(IExchange &&) noexcept = default;
  IExchange &operator=(IExchange &&) noexcept = default;

  // Prevent copying
  IExchange(const IExchange &) = delete;
  IExchange &operator=(const IExchange &) = delete;
};
