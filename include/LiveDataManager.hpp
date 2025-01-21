#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "AggregatedOrderBook.hpp"

class LiveDataManager {
 public:
  LiveDataManager(AggregatedOrderBook& orderBook);
  void start(int updateIntervalMs);
  void stop();
  void handleSignal(int signal);
  static std::atomic<bool> received_signal;

 private:
  void displayLoop();
  void setupSignalHandlers();

  AggregatedOrderBook& orderBook;
  std::atomic<bool> running;
  std::mutex mutex;
  std::condition_variable cv;
  std::thread display_thread;
};
