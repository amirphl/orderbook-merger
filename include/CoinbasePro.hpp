#pragma once
#include <json/json.h>

#include "IExchange.hpp"
#include "OrderBookEntry.hpp"

class CoinbasePro : public IExchange {
 public:
  CoinbasePro();

  void fetchOrderBook() override;
  const std::multimap<cpp_dec_float_50, OrderBookEntry> &getBids() const override;
  const std::multimap<cpp_dec_float_50, OrderBookEntry> &getAsks() const override;
  std::string getName() const override;

 private:
  void parseOrders(const Json::Value &orders, std::multimap<cpp_dec_float_50, OrderBookEntry> &orderMap);
  // const std::string API_ENDPOINT = "https://api.pro.coinbase.com/products/BTC-USD/book?level=2";
  const std::string API_ENDPOINT = "https://api.exchange.coinbase.com/products/BTC-USDT/book?level=2";
};
