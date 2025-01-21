#include "WebSocketClient.hpp"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <functional>
#include <string>
#include <thread>

WebSocketClient::WebSocketClient(const WebSocketConfig& config)
    : ssl_context_(boost::asio::ssl::context::tlsv12_client),
      is_running_(false),
      is_connected_(false),
      config_(config),
      reconnect_attempts_(0),
      reconnect_scheduled_(false) {}

WebSocketClient::~WebSocketClient() { stop(); }

void WebSocketClient::connect(const std::string& uri) {
  current_uri_ = uri;
  io_thread_ = std::thread([this]() { io_context_.run(); });
  doConnect(uri);
}

void WebSocketClient::doConnect(const std::string& uri) {
  try {
    auto resolver = boost::asio::ip::tcp::resolver(io_context_);
    auto endpoints = resolver.resolve(uri, "443");
    ws_ = std::make_unique<boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>>>(
        io_context_, ssl_context_);

    boost::asio::connect(ws_->next_layer().next_layer(), endpoints);
    ws_->next_layer().handshake(boost::asio::ssl::stream_base::client);
    ws_->handshake(uri, "/");

    is_connected_ = true;
    if (connect_handler_) connect_handler_();
    readMessage();
  } catch (const std::exception& e) {
    is_connected_ = false;
    if (error_handler_) error_handler_(e.what());
    if (config_.autoReconnect) scheduleReconnect();
  }
}

void WebSocketClient::send(const std::string& message) {
  if (is_connected_) {
    boost::asio::post(io_context_, [this, message]() { doSend(message); });
  }
}

void WebSocketClient::doSend(const std::string& message) {
  try {
    ws_->write(boost::asio::buffer(message));
  } catch (const std::exception& e) {
    if (error_handler_) error_handler_(e.what());
  }
}

void WebSocketClient::close() {
  boost::asio::post(io_context_, [this]() { doClose(); });
}

void WebSocketClient::doClose() {
  try {
    ws_->close(boost::beast::websocket::close_code::normal);
    is_connected_ = false;
    if (disconnect_handler_) disconnect_handler_();
  } catch (const std::exception& e) {
    if (error_handler_) error_handler_(e.what());
  }
}

void WebSocketClient::readMessage() {
  // auto buffer = boost::asio::dynamic_buffer(current_uri_);
  auto buffer = std::make_shared<boost::beast::flat_buffer>();  // Use a flat_buffer for WebSocket reads

  ws_->async_read(*buffer, [this, buffer](boost::system::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
      is_connected_ = false;
      if (error_handler_) error_handler_(ec.message());
      if (config_.autoReconnect) scheduleReconnect();
      return;
    }

    // Convert the received data to a string
    if (message_handler_) {
      message_handler_(boost::beast::buffers_to_string(buffer->data()));
    }

    // Clear the buffer and continue reading
    buffer->clear();
    readMessage();
  });
}

void WebSocketClient::setMessageHandler(std::function<void(const std::string&)> handler) {
  message_handler_ = std::move(handler);
}

void WebSocketClient::setErrorHandler(std::function<void(const std::string&)> handler) {
  error_handler_ = std::move(handler);
}

void WebSocketClient::setConnectHandler(std::function<void()> handler) { connect_handler_ = std::move(handler); }

void WebSocketClient::setDisconnectHandler(std::function<void()> handler) { disconnect_handler_ = std::move(handler); }

void WebSocketClient::run() {
  is_running_ = true;
  if (io_thread_.joinable()) io_thread_.join();
}

void WebSocketClient::stop() {
  is_running_ = false;
  io_context_.stop();
  if (io_thread_.joinable()) io_thread_.join();
}

bool WebSocketClient::isConnected() const { return is_connected_; }

void WebSocketClient::scheduleReconnect() {
  if (!reconnect_scheduled_.exchange(true)) {
    reconnect_attempts_++;
    if (reconnect_attempts_ <= config_.maxReconnectAttempts) {
      std::this_thread::sleep_for(std::chrono::milliseconds(config_.reconnectIntervalMs));
      doConnect(current_uri_);
    }
    reconnect_scheduled_ = false;
  }
}

void WebSocketClient::resetReconnectAttempts() { reconnect_attempts_ = 0; }

// #include "WebSocketClient.hpp"

// #include <websocketpp/common/connection_hdl.hpp>

// #include "Logger.hpp"

// WebSocketClient::WebSocketClient(const WebSocketConfig& cfg)
//     : is_running(false), is_connected(false), config(cfg), reconnect_attempts(0), reconnect_scheduled(false) {
//   endpoint.clear_access_channels(websocketpp::log::alevel::all);
//   endpoint.set_error_channels(websocketpp::log::elevel::all);
//   endpoint.init_asio();

//   setupHandlers();
// }

// void WebSocketClient::setupHandlers() {
//   // Message handler
//   endpoint.set_message_handler([this](ConnectionHdl hdl, WebSocketClientTls::message_ptr msg) {
//     if (message_handler) {
//       try {
//         message_handler(msg->get_payload());
//       } catch (const std::exception& e) {
//         LOG_ERROR("Error in message handler: " + std::string(e.what()));
//         if (error_handler) {
//           error_handler("Message handler error: " + std::string(e.what()));
//         }
//       }
//     }
//   });

//   // Open handler
//   endpoint.set_open_handler([this](ConnectionHdl hdl) {
//     {
//       std::lock_guard<std::mutex> lock(reconnect_mutex);
//       is_connected = true;
//       resetReconnectAttempts();
//     }
//     LOG_INFO("WebSocket connection established");
//     if (connect_handler) {
//       connect_handler();
//     }
//   });

//   // Close handler
//   endpoint.set_close_handler([this](ConnectionHdl hdl) { handleDisconnect(hdl); });

//   // Fail handler
//   endpoint.set_fail_handler([this](ConnectionHdl hdl) {
//     auto con = endpoint.get_con_from_hdl(hdl);
//     std::string error_msg = "Connection failed: " + con->get_ec().message();
//     LOG_ERROR(error_msg);

//     handleDisconnect(hdl);

//     if (error_handler) {
//       error_handler(error_msg);
//     }
//   });

//   // TLS handler
//   endpoint.set_tls_init_handler([](ConnectionHdl hdl) {
//     return websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12_client);
//   });
// }

// void WebSocketClient::handleDisconnect(websocketpp::connection_hdl hdl) {
//   {
//     std::lock_guard<std::mutex> lock(reconnect_mutex);
//     is_connected = false;
//   }

//   LOG_WARNING("WebSocket connection lost");

//   if (disconnect_handler) {
//     disconnect_handler();
//   }

//   if (shouldReconnect()) {
//     scheduleReconnect();
//   }
// }

// void WebSocketClient::scheduleReconnect() {
//   if (!reconnect_scheduled.exchange(true)) {
//     std::thread([this]() { attemptReconnect(); }).detach();
//   }
// }

// void WebSocketClient::attemptReconnect() {
//   while (shouldReconnect() && !is_connected) {
//     {
//       std::lock_guard<std::mutex> lock(reconnect_mutex);
//       reconnect_attempts++;

//       LOG_INFO("Attempting reconnection (" + std::to_string(reconnect_attempts) + "/" +
//                std::to_string(config.maxReconnectAttempts) + ")");
//     }

//     try {
//       connect(current_uri);

//       // Wait for connection timeout
//       std::unique_lock<std::mutex> lock(reconnect_mutex);
//       reconnect_cv.wait_for(lock, std::chrono::milliseconds(config.connectionTimeoutMs),
//                             [this] { return is_connected || !is_running; });

//       if (is_connected) {
//         reconnect_scheduled = false;
//         return;
//       }
//     } catch (const std::exception& e) {
//       LOG_ERROR("Reconnection attempt failed: " + std::string(e.what()));
//     }

//     // Wait before next attempt
//     std::this_thread::sleep_for(std::chrono::milliseconds(config.reconnectIntervalMs));
//   }

//   if (!is_connected) {
//     LOG_ERROR("Max reconnection attempts reached");
//     if (error_handler) {
//       error_handler("Failed to reconnect after " + std::to_string(config.maxReconnectAttempts) + " attempts");
//     }
//   }

//   reconnect_scheduled = false;
// }

// bool WebSocketClient::shouldReconnect() const {
//   return is_running && config.autoReconnect && reconnect_attempts < config.maxReconnectAttempts;
// }

// void WebSocketClient::resetReconnectAttempts() { reconnect_attempts = 0; }

// void WebSocketClient::connect(const std::string& uri) {
//   if (is_connected) {
//     return;
//   }

//   current_uri = uri;
//   websocketpp::lib::error_code ec;

//   auto con = endpoint.get_connection(uri, ec);
//   if (ec) {
//     LOG_ERROR("WebSocket connection error: " + ec.message());
//     if (error_handler) {
//       error_handler("Connection error: " + ec.message());
//     }
//     return;
//   }

//   // Set connection timeout
//   con->set_open_handshake_timeout(config.connectionTimeoutMs);
//   con->set_close_handshake_timeout(config.connectionTimeoutMs);

//   connection = con->get_handle();
//   endpoint.connect(con);
// }

// // Update main.cpp to include WebSocket configuration
// int main(int argc, char* argv[]) {
//   try {
//     po::options_description desc("Allowed options");
//     desc.add_options()("help", "produce help message")("live", "enable live data mode")(
//         "interval", po::value<int>()->default_value(1000), "update interval in milliseconds")(
//         "quantity", po::value<double>()->default_value(10.0), "quantity of BTC to calculate")(
//         "max-reconnect", po::value<int>()->default_value(5), "maximum WebSocket reconnection attempts")(
//         "reconnect-interval", po::value<int>()->default_value(5000), "WebSocket reconnection interval in
//         milliseconds")( "connection-timeout", po::value<int>()->default_value(10000), "WebSocket connection timeout
//         in milliseconds");

//     po::variables_map vm;
//     po::store(po::parse_command_line(argc, argv, desc), vm);
//     po::notify(vm);

//     // Configure WebSocket
//     WebSocketConfig wsConfig;
//     wsConfig.maxReconnectAttempts = vm["max-reconnect"].as<int>();
//     wsConfig.reconnectIntervalMs = vm["reconnect-interval"].as<int>();
//     wsConfig.connectionTimeoutMs = vm["connection-timeout"].as<int>();

//     // ... rest of the main function
//   }
// }
