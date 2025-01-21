cmake_minimum_required(VERSION 3.30)
project(Merger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++")
endif()

# Find required packages
find_package(CURL REQUIRED)
find_package(GTest REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread program_options)
find_package(OpenSSL REQUIRED)

# Handle JsonCpp manually
if(APPLE)
  # macOS-specific setup
  find_package(JsonCpp REQUIRED)
else()
  # Non-macOS (e.g., Alpine Linux)
  find_path(JsonCpp_INCLUDE_DIR json/json.h PATH_SUFFIXES jsoncpp)
  find_library(JsonCpp_LIBRARIES jsoncpp)

  if(NOT JsonCpp_INCLUDE_DIR OR NOT JsonCpp_LIBRARIES)
    message(FATAL_ERROR "JsonCpp library not found")
  endif()
endif()

# Main executable
add_executable(${PROJECT_NAME}
    main.cpp
    src/HttpClient.cpp
    src/AggregatedOrderBook.cpp
    src/Logger.cpp
    src/CoinbasePro.cpp
    src/Gemini.cpp
    src/Kraken.cpp
		src/WebSocketClient.cpp
		src/LiveDataManager.cpp
)

# Include directories
if(APPLE)
  target_include_directories(${PROJECT_NAME} PRIVATE include)
else()
  target_include_directories(${PROJECT_NAME} PRIVATE 
      ${JsonCpp_INCLUDE_DIR} 
      include
  )
endif()

# Link libraries
if(APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE 
      CURL::libcurl 
      JsonCpp::JsonCpp
      Boost::system
      Boost::filesystem
      Boost::thread
      Boost::program_options
      OpenSSL::SSL
      OpenSSL::Crypto
			# c++
  )
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE 
      CURL::libcurl 
      ${JsonCpp_LIBRARIES}
      Boost::system
      Boost::filesystem
      Boost::thread
      Boost::program_options
      OpenSSL::SSL
      OpenSSL::Crypto
			# -lc++
			-L/usr/lib -lstdc++
  )
endif()

# Tests executable
add_executable(merger_tests
    tests/OrderBookTests.cpp
    src/AggregatedOrderBook.cpp
    src/Logger.cpp
)

if(APPLE)
  target_include_directories(merger_tests PRIVATE include)
  target_link_libraries(merger_tests PRIVATE 
      GTest::GTest
      GTest::Main
      JsonCpp::JsonCpp
      Boost::system
      Boost::filesystem
      Boost::thread
      Boost::program_options
      OpenSSL::SSL
      OpenSSL::Crypto
			# c++
  )
else()
  target_include_directories(merger_tests PRIVATE 
      ${JsonCpp_INCLUDE_DIR} 
      include
  )
  target_link_libraries(merger_tests PRIVATE 
      GTest::GTest
      GTest::Main
      ${JsonCpp_LIBRARIES}
      Boost::system
      Boost::filesystem
      Boost::thread
      Boost::program_options
      OpenSSL::SSL
      OpenSSL::Crypto
			# -lc++
			-L/usr/lib -lstdc++
  )
endif()

enable_testing()
add_test(NAME MergerTests COMMAND merger_tests)
