#pragma once
#include <stdexcept>
#include <string>

#include "OrderBookEntry.hpp"

class InsufficientLiquidityException : public std::runtime_error {
 public:
  InsufficientLiquidityException(const std::string& message, cpp_dec_float_50& requested, cpp_dec_float_50& available)
      : std::runtime_error(message), requestedAmount(requested), availableAmount(available) {}

  // Constructor using move semantics for efficiency
  InsufficientLiquidityException(std::string&& message, cpp_dec_float_50&& requested, cpp_dec_float_50&& available)
      : std::runtime_error(std::move(message)),
        requestedAmount(std::move(requested)),
        availableAmount(std::move(available)) {}

  const cpp_dec_float_50& getRequestedAmount() const { return requestedAmount; }
  const cpp_dec_float_50& getAvailableAmount() const { return availableAmount; }

  // Defaulted copy and move operations
  InsufficientLiquidityException(const InsufficientLiquidityException&) = default;
  InsufficientLiquidityException& operator=(const InsufficientLiquidityException&) = default;
  InsufficientLiquidityException(InsufficientLiquidityException&&) noexcept = default;
  InsufficientLiquidityException& operator=(InsufficientLiquidityException&&) noexcept = default;

 private:
  cpp_dec_float_50 requestedAmount;
  cpp_dec_float_50 availableAmount;
};
