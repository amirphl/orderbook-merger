#pragma once
#include <json/json.h>

#include "IExchange.hpp"

class Gemini : public IExchange {
 public:
  Gemini();

  void fetchOrderBook() override;
  const std::multimap<cpp_dec_float_50, OrderBookEntry>& getBids() const override;
  const std::multimap<cpp_dec_float_50, OrderBookEntry>& getAsks() const override;
  std::string getName() const override;

 private:
  void parseOrders(const Json::Value& orders, std::multimap<cpp_dec_float_50, OrderBookEntry>& orderMap);
  const std::string API_ENDPOINT = "https://api.gemini.com/v1/book/btcusd?limit_bids=1000&limit_asks=1000";
};
