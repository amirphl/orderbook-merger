#pragma once
#include <memory>
#include <vector>

#include "IExchange.hpp"
#include "Logger.hpp"

class AggregatedOrderBook {
 public:
  void addExchange(std::shared_ptr<IExchange> exchange);
  void fetchAllOrderBooks();
  std::pair<cpp_dec_float_50, cpp_dec_float_50> calculateCosts(cpp_dec_float_50 quantity) const;
  void printOrderBook() const;
  cpp_dec_float_50 calculateAvailableLiquidity(bool isBid) const;
  void logOrderBookMerge(const std::string &exchangeName, const std::multimap<cpp_dec_float_50, OrderBookEntry> &orders,
                         bool isBids) const;

 private:
  std::vector<std::shared_ptr<IExchange>> exchanges;
  std::multimap<cpp_dec_float_50, OrderBookEntry> aggregatedBids;
  std::multimap<cpp_dec_float_50, OrderBookEntry> aggregatedAsks;

  void mergeOrderBooks();
};
