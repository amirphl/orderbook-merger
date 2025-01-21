#include "AggregatedOrderBook.hpp"

#include <iostream>

#include "Exceptions.hpp"

void AggregatedOrderBook::addExchange(std::shared_ptr<IExchange> exchange) { exchanges.push_back(exchange); }

void AggregatedOrderBook::fetchAllOrderBooks() {
  for (auto &exchange : exchanges) {
    exchange->fetchOrderBook();
  }
  mergeOrderBooks();
}

void AggregatedOrderBook::mergeOrderBooks() {
  LOG_INFO("Starting order book merge");
  aggregatedBids.clear();
  aggregatedAsks.clear();

  for (const auto &exchange : exchanges) {
    LOG_INFO("Merging order book from " + exchange->getName());

    // Merge and log bids
    logOrderBookMerge(exchange->getName(), exchange->getBids(), true);
    for (const auto &bid : exchange->getBids()) {
      aggregatedBids.insert(bid);
    }

    // Merge and log asks
    logOrderBookMerge(exchange->getName(), exchange->getAsks(), false);
    for (const auto &ask : exchange->getAsks()) {
      aggregatedAsks.insert(ask);
    }
  }

  LOG_INFO("Order book merge completed. Total entries - Bids: " + std::to_string(aggregatedBids.size()) +
           ", Asks: " + std::to_string(aggregatedAsks.size()));
}

void AggregatedOrderBook::logOrderBookMerge(const std::string &exchangeName,
                                            const std::multimap<cpp_dec_float_50, OrderBookEntry> &orders,
                                            bool isBids) const {
  std::stringstream ss;
  ss << "Merging " << (isBids ? "bids" : "asks") << " from " << exchangeName << ": " << orders.size() << " entries";

  if (!orders.empty()) {
    auto first = orders.begin();
    auto last = std::prev(orders.end());
    ss << " (Price range: " << first->first << " - " << last->first << ")";
  }

  LOG_DEBUG(ss.str());
}

cpp_dec_float_50 AggregatedOrderBook::calculateAvailableLiquidity(bool isBuy) const {
  cpp_dec_float_50 totalLiquidity = 0.0;

  if (isBuy) {
    for (const auto &ask : aggregatedAsks) {
      totalLiquidity += ask.second.quantity;
    }
  } else {
    for (const auto &bid : aggregatedBids) {
      totalLiquidity += bid.second.quantity;
    }
  }

  return totalLiquidity;
}

std::pair<cpp_dec_float_50, cpp_dec_float_50> AggregatedOrderBook::calculateCosts(cpp_dec_float_50 quantity) const {
  cpp_dec_float_50 buyCost = 0.0;
  cpp_dec_float_50 sellCost = 0.0;
  cpp_dec_float_50 remainingQuantity;

  // Check available liquidity first
  cpp_dec_float_50 availableBuyLiquidity = calculateAvailableLiquidity(true);
  cpp_dec_float_50 availableSellLiquidity = calculateAvailableLiquidity(false);

  if (quantity > availableBuyLiquidity) {
    throw InsufficientLiquidityException("Insufficient liquidity for buy order", quantity, availableBuyLiquidity);
  }

  if (quantity > availableSellLiquidity) {
    throw InsufficientLiquidityException("Insufficient liquidity for sell order", quantity, availableSellLiquidity);
  }

  // Calculate buy cost (from asks)
  remainingQuantity = quantity;
  for (const auto &ask : aggregatedAsks) {
    const auto &entry = ask.second;
    if (remainingQuantity <= 0) break;

    cpp_dec_float_50 fillQuantity = std::min(remainingQuantity, entry.quantity);
    buyCost += fillQuantity * entry.price;
    remainingQuantity -= fillQuantity;
  }

  // Calculate sell cost (from bids)
  remainingQuantity = quantity;
  for (auto it = aggregatedBids.rbegin(); it != aggregatedBids.rend(); ++it) {
    if (remainingQuantity <= 0) break;

    const auto &entry = it->second;
    cpp_dec_float_50 fillQuantity = std::min(remainingQuantity, entry.quantity);
    sellCost += fillQuantity * entry.price;
    remainingQuantity -= fillQuantity;
  }

  return {buyCost, sellCost};
}
