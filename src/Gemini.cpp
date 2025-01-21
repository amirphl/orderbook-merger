#include "Gemini.hpp"

#include <stdexcept>

#include "HttpClient.hpp"
#include "Logger.hpp"
Gemini::Gemini() { name = "Gemini"; }

void Gemini::fetchOrderBook() {
  try {
    std::string response = HttpClient::get(API_ENDPOINT);

    Json::Value root;
    Json::Reader reader;

    if (reader.parse(response, root)) {
      bids.clear();
      asks.clear();

      LOG_DEBUG("Parsing Gemini order book");
      parseOrders(root["bids"], bids);
      parseOrders(root["asks"], asks);
      LOG_INFO("Successfully parsed Gemini order book - Bids: " + std::to_string(bids.size()) +
               ", Asks: " + std::to_string(asks.size()));
    } else {
      throw std::runtime_error("Failed to parse Gemini response");
    }
  } catch (const std::exception& e) {
    LOG_ERROR("Error fetching Gemini order book: " + std::string(e.what()));
    throw;
  }
}

void Gemini::parseOrders(const Json::Value& orders, std::multimap<cpp_dec_float_50, OrderBookEntry>& orderMap) {
  for (const auto& order : orders) {
    try {
      cpp_dec_float_50 price(order["price"].asString());
      cpp_dec_float_50 quantity(order["amount"].asString());

      // Gemini sometimes includes orders with zero quantity - skip them
      if (quantity > 0) {
        orderMap.emplace(price, OrderBookEntry(price, quantity, name));
      }
    } catch (const std::exception& e) {
      LOG_WARNING("Failed to parse Gemini order: " + std::string(e.what()));
    }
  }
}

const std::multimap<cpp_dec_float_50, OrderBookEntry>& Gemini::getBids() const { return bids; }

const std::multimap<cpp_dec_float_50, OrderBookEntry>& Gemini::getAsks() const { return asks; }

std::string Gemini::getName() const { return name; }
