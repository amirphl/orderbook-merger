
#include "CoinbasePro.hpp"

#include <json/json.h>

#include <stdexcept>

#include "HttpClient.hpp"
#include "Logger.hpp"

CoinbasePro::CoinbasePro() { name = "Coinbase Pro"; }

void CoinbasePro::fetchOrderBook() {
  try {
    std::string response = HttpClient::get(API_ENDPOINT);

    Json::Value root;
    Json::Reader reader;

    if (reader.parse(response, root)) {
      bids.clear();
      asks.clear();

      LOG_DEBUG("Parsing Coinbase Pro order book");
      parseOrders(root["bids"], bids);
      parseOrders(root["asks"], asks);
      LOG_INFO("Successfully parsed Coinbase Pro order book - Bids: " + std::to_string(bids.size()) +
               ", Asks: " + std::to_string(asks.size()));
    } else {
      throw std::runtime_error("Failed to parse Coinbase Pro response");
    }
  } catch (const std::exception &e) {
    LOG_ERROR("Error fetching Coinbase Pro order book: " + std::string(e.what()));
    throw;
  }
}

void CoinbasePro::parseOrders(const Json::Value &orders, std::multimap<cpp_dec_float_50, OrderBookEntry> &orderMap) {
  for (const auto &order : orders) {
    try {
      // Kraken provides [price, volume, timestamp]
      cpp_dec_float_50 price(order[0].asString());
      cpp_dec_float_50 quantity(order[1].asString());

      orderMap.emplace(price, OrderBookEntry(price, quantity, name));
    } catch (const std::exception &e) {
      LOG_WARNING("Failed to parse Coinbase Pro order: " + std::string(e.what()));
    }
  }
}

const std::multimap<cpp_dec_float_50, OrderBookEntry> &CoinbasePro::getBids() const { return bids; }

const std::multimap<cpp_dec_float_50, OrderBookEntry> &CoinbasePro::getAsks() const { return asks; }

std::string CoinbasePro::getName() const { return name; }
