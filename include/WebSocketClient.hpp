#pragma once
#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

struct WebSocketConfig {
  int maxReconnectAttempts = 5;
  int reconnectIntervalMs = 5000;
  int connectionTimeoutMs = 10000;
  bool autoReconnect = true;
};

class WebSocketClient {
 public:
  explicit WebSocketClient(const WebSocketConfig& config = WebSocketConfig{});
  ~WebSocketClient();

  void connect(const std::string& uri);
  void send(const std::string& message);
  void close();
  void setMessageHandler(std::function<void(const std::string&)> handler);
  void setErrorHandler(std::function<void(const std::string&)> handler);
  void setConnectHandler(std::function<void()> handler);
  void setDisconnectHandler(std::function<void()> handler);
  void run();
  void stop();
  bool isConnected() const;

 private:
  void doConnect(const std::string& uri);
  void doSend(const std::string& message);
  void doClose();
  void readMessage();
  void attemptReconnect();
  void scheduleReconnect();
  bool shouldReconnect() const;
  void resetReconnectAttempts();

  boost::asio::io_context io_context_;
  boost::asio::ssl::context ssl_context_;
  std::unique_ptr<boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>> ws_;
  std::thread io_thread_;
  std::atomic<bool> is_running_;
  std::atomic<bool> is_connected_;
  std::string current_uri_;
  WebSocketConfig config_;
  int reconnect_attempts_;
  std::mutex reconnect_mutex_;
  std::condition_variable reconnect_cv_;
  std::atomic<bool> reconnect_scheduled_;

  std::function<void(const std::string&)> message_handler_;
  std::function<void(const std::string&)> error_handler_;
  std::function<void()> connect_handler_;
  std::function<void()> disconnect_handler_;
};
