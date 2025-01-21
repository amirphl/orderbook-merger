#pragma once
#include <json/json.h>

#include "IExchange.hpp"

class Kraken : public IExchange {
 public:
  Kraken();

  void fetchOrderBook() override;
  const std::multimap<cpp_dec_float_50, OrderBookEntry>& getBids() const override;
  const std::multimap<cpp_dec_float_50, OrderBookEntry>& getAsks() const override;
  std::string getName() const override;

 private:
  void parseOrders(const Json::Value& orders, std::multimap<cpp_dec_float_50, OrderBookEntry>& orderMap);
  const std::string API_ENDPOINT = "https://api.kraken.com/0/public/Depth?pair=XBTUSD&count=1000";
};
