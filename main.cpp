#include <boost/program_options.hpp>
#include <iostream>

#include "AggregatedOrderBook.hpp"
#include "CoinbasePro.hpp"
#include "Gemini.hpp"
#include "Kraken.hpp"
#include "LiveDataManager.hpp"

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
  try {
    // Command line options
    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")("live", "enable live data mode")(
        "interval", po::value<int>()->default_value(1000), "update interval in milliseconds")(
        "quantity", po::value<double>()->default_value(10.0), "quantity of BTC to calculate");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    // Setup logging
    Logger::getInstance().setLogFile("orderbook.log");

    // Create order book aggregator
    AggregatedOrderBook aggregator;
    aggregator.addExchange(std::make_shared<CoinbasePro>());
    aggregator.addExchange(std::make_shared<Gemini>());
    aggregator.addExchange(std::make_shared<Kraken>());

    if (vm.count("live")) {
      // Live data mode
      LiveDataManager liveManager(aggregator);
      int interval = vm["interval"].as<int>();

      LOG_INFO("Starting live data mode with " + std::to_string(interval) + "ms interval");
      liveManager.start(interval);

      // Wait for signal
      while (!LiveDataManager::received_signal) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      liveManager.stop();

    } else {
      // Single fetch mode
      double quantity = vm["quantity"].as<double>();

      aggregator.fetchAllOrderBooks();
      auto costs = aggregator.calculateCosts(quantity);

      std::cout << std::fixed << std::setprecision(2);
      std::cout << "Buy cost (" << quantity << " BTC): $" << costs.first << std::endl;
      std::cout << "Sell proceeds (" << quantity << " BTC): $" << costs.second << std::endl;
    }

  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
