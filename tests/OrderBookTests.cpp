#include <gtest/gtest.h>

#include "AggregatedOrderBook.hpp"
#include "Exceptions.hpp"
#include "Gemini.hpp"
#include "Kraken.hpp"

class MockExchange : public IExchange {
 public:
  MockExchange(const std::string &name) { this->name = name; }

  void setMockData(const std::multimap<cpp_dec_float_50, OrderBookEntry> &mockBids,
                   const std::multimap<cpp_dec_float_50, OrderBookEntry> &mockAsks) {
    this->bids = mockBids;
    this->asks = mockAsks;
  }

  void fetchOrderBook() override {}
  const std::multimap<cpp_dec_float_50, OrderBookEntry> &getBids() const override { return bids; }
  const std::multimap<cpp_dec_float_50, OrderBookEntry> &getAsks() const override { return asks; }
  std::string getName() const override { return name; }
};

class OrderBookTest : public ::testing::Test {
 protected:
  void SetUp() override { Logger::getInstance().setLogFile("test_orderbook.log"); }

  AggregatedOrderBook aggregator;
};

TEST_F(OrderBookTest, TestInsufficientLiquidity) {
  auto mockExchange = std::make_shared<MockExchange>("MockExchange");

  // Setup mock data with limited liquidity
  std::multimap<cpp_dec_float_50, OrderBookEntry> mockBids{{50000.0, OrderBookEntry(50000.0, 1.0, "MockExchange")},
                                                           {49900.0, OrderBookEntry(49900.0, 1.0, "MockExchange")}};

  std::multimap<cpp_dec_float_50, OrderBookEntry> mockAsks{{50100.0, OrderBookEntry(50100.0, 1.0, "MockExchange")},
                                                           {50200.0, OrderBookEntry(50200.0, 1.0, "MockExchange")}};

  mockExchange->setMockData(mockBids, mockAsks);
  aggregator.addExchange(mockExchange);

  // Try to calculate costs for more BTC than available
  EXPECT_THROW(
      {
        try {
          aggregator.calculateCosts(3.0);  // Try to trade 3 BTC when only 2 available
        } catch (const InsufficientLiquidityException &e) {
          EXPECT_EQ(e.getRequestedAmount(), 3.0);
          EXPECT_EQ(e.getAvailableAmount(), 2.0);
          throw;
        }
      },
      InsufficientLiquidityException);
}

TEST_F(OrderBookTest, TestOrderBookMerge) {
  auto exchange1 = std::make_shared<MockExchange>("Exchange1");
  auto exchange2 = std::make_shared<MockExchange>("Exchange2");

  // Setup mock data for first exchange
  std::multimap<cpp_dec_float_50, OrderBookEntry> bids1{{50000.0, OrderBookEntry(50000.0, 1.0, "Exchange1")}};

  std::multimap<cpp_dec_float_50, OrderBookEntry> asks1{{50100.0, OrderBookEntry(50100.0, 1.0, "Exchange1")}};

  // Setup mock data for second exchange
  std::multimap<cpp_dec_float_50, OrderBookEntry> bids2{{50050.0, OrderBookEntry(50050.0, 1.0, "Exchange2")}};

  std::multimap<cpp_dec_float_50, OrderBookEntry> asks2{{50150.0, OrderBookEntry(50150.0, 1.0, "Exchange2")}};

  exchange1->setMockData(bids1, asks1);
  exchange2->setMockData(bids2, asks2);

  aggregator.addExchange(exchange1);
  aggregator.addExchange(exchange2);
  aggregator.fetchAllOrderBooks();

  // Test merged order book properties
  auto costs = aggregator.calculateCosts(1.0);
  EXPECT_EQ(costs.first, 50100.0);   // Should get best ask price
  EXPECT_EQ(costs.second, 50050.0);  // Should get best bid price
}

// TODO
// TEST_F(OrderBookTest, TestGeminiParsing) {
//   auto gemini = std::make_shared<Gemini>();

//   // Test handling of zero quantity orders
//   Json::Value mockResponse;
//   mockResponse["bids"][0]["price"] = "50000.00";
//   mockResponse["bids"][0]["amount"] = "0.00";  // Should be skipped
//   mockResponse["bids"][1]["price"] = "49900.00";
//   mockResponse["bids"][1]["amount"] = "1.00";

//   // ... Add more test cases for Gemini parsing
// }

// TEST_F(OrderBookTest, TestKrakenErrorHandling) {
//   auto kraken = std::make_shared<Kraken>();

//   // Test handling of Kraken API errors
//   Json::Value mockResponse;
//   mockResponse["error"].append("EAPI:Rate limit exceeded");

//   // ... Add more test cases for Kraken error handling
// }
