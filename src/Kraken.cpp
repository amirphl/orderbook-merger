#include "Kraken.hpp"

#include <stdexcept>

#include "HttpClient.hpp"
#include "Logger.hpp"

Kraken::Kraken() { name = "Kraken"; }

void Kraken::fetchOrderBook() {
  try {
    std::string response = HttpClient::get(API_ENDPOINT);

    Json::Value root;
    Json::Reader reader;

    if (reader.parse(response, root)) {
      if (root.isMember("error") && !root["error"].empty()) {
        throw std::runtime_error("Kraken API error: " + root["error"][0].asString());
      }

      bids.clear();
      asks.clear();

      // Kraken uses XXBTZUSD as the pair name
      Json::Value& result = root["result"]["XXBTZUSD"];

      LOG_DEBUG("Parsing Kraken order book");
      parseOrders(result["bids"], bids);
      parseOrders(result["asks"], asks);
      LOG_INFO("Successfully parsed Kraken order book - Bids: " + std::to_string(bids.size()) +
               ", Asks: " + std::to_string(asks.size()));
    } else {
      throw std::runtime_error("Failed to parse Kraken response");
    }
  } catch (const std::exception& e) {
    LOG_ERROR("Error fetching Kraken order book: " + std::string(e.what()));
    throw;
  }
}

void Kraken::parseOrders(const Json::Value& orders, std::multimap<cpp_dec_float_50, OrderBookEntry>& orderMap) {
  for (const auto& order : orders) {
    try {
      // Kraken provides [price, volume, timestamp]
      cpp_dec_float_50 price(order[0].asString());
      cpp_dec_float_50 quantity(order[1].asString());

      orderMap.emplace(price, OrderBookEntry(price, quantity, name));
    } catch (const std::exception& e) {
      LOG_WARNING("Failed to parse Kraken order: " + std::string(e.what()));
    }
  }
}

const std::multimap<cpp_dec_float_50, OrderBookEntry>& Kraken::getBids() const { return bids; }

const std::multimap<cpp_dec_float_50, OrderBookEntry>& Kraken::getAsks() const { return asks; }

std::string Kraken::getName() const { return name; }
