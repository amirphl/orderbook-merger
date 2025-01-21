#pragma once
#include <curl/curl.h>

#include <string>

class HttpClient {
 public:
  static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);
  static std::string get(const std::string &url);

 private:
  static CURL *initCurl();
};
