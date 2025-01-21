#include "LiveDataManager.hpp"

#include <csignal>
#include <iomanip>
#include <iostream>

std::atomic<bool> LiveDataManager::received_signal(false);

LiveDataManager::LiveDataManager(AggregatedOrderBook& ob) : orderBook(ob), running(false) { setupSignalHandlers(); }

void LiveDataManager::setupSignalHandlers() {
  std::signal(SIGINT, [](int signal) { received_signal = true; });
  std::signal(SIGTERM, [](int signal) { received_signal = true; });
}

void LiveDataManager::start(int updateIntervalMs) {
  running = true;
  display_thread = std::thread([this, updateIntervalMs]() { displayLoop(); });

  LOG_INFO("Live data mode started with " + std::to_string(updateIntervalMs) + "ms interval");
}

void LiveDataManager::stop() {
  running = false;
  cv.notify_all();
  if (display_thread.joinable()) {
    display_thread.join();
  }
  LOG_INFO("Live data mode stopped");
}

void LiveDataManager::displayLoop() {
  while (running && !received_signal) {
    try {
      orderBook.fetchAllOrderBooks();
      auto costs = orderBook.calculateCosts(10.0);  // Default quantity

      // Clear screen (cross-platform)
      std::cout << "\033[2J\033[1;1H";

      // Display timestamp
      auto now = std::chrono::system_clock::now();
      auto now_c = std::chrono::system_clock::to_time_t(now);
      std::cout << "Last update: " << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << std::endl;

      // Display prices
      std::cout << std::fixed << std::setprecision(2);
      std::cout << "Buy cost (10 BTC): $" << costs.first << std::endl;
      std::cout << "Sell proceeds (10 BTC): $" << costs.second << std::endl;

      // Display controls
      std::cout << "\nPress Ctrl+C to exit" << std::endl;

      // Wait for interval or signal
      std::unique_lock<std::mutex> lock(mutex);
      auto updateIntervalMs = 10000;
      cv.wait_for(lock, std::chrono::milliseconds(updateIntervalMs), [this] { return !running || received_signal; });

    } catch (const std::exception& e) {
      LOG_ERROR("Error in display loop: " + std::string(e.what()));
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }
}
