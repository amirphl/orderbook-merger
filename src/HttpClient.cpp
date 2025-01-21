#include "HttpClient.hpp"

#include <stdexcept>

size_t HttpClient::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
  userp->append((char *)contents, size * nmemb);
  return size * nmemb;
}

std::string HttpClient::get(const std::string &url) {
  CURL *curl = initCurl();
  std::string readBuffer;

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT,
                     "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) "
                     "Chrome/112.0.0.0 Safari/537.36");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
      throw std::runtime_error("Failed to fetch data from " + url);
    }
  }

  return readBuffer;
}

CURL *HttpClient::initCurl() {
  CURL *curl = curl_easy_init();
  if (!curl) {
    throw std::runtime_error("Failed to initialize CURL");
  }
  return curl;
}
