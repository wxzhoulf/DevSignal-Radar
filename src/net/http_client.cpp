#include "techpulse/net/http_client.hpp"

#include <curl/curl.h>

#include <mutex>
#include <stdexcept>
#include <algorithm>

namespace techpulse::net {
namespace {

std::once_flag curl_initialized;

size_t append_body(char* bytes, size_t size, size_t count, void* target) {
    const auto length = size * count;
    static_cast<std::string*>(target)->append(bytes, length);
    return length;
}
size_t append_header(char* bytes, size_t size, size_t count, void* target) {
    const std::string line(bytes, size * count); const auto colon = line.find(':');
    if (colon != std::string::npos) { auto key = line.substr(0, colon); auto value = line.substr(colon + 1);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        value.erase(0, value.find_first_not_of(" \t")); value.erase(value.find_last_not_of("\r\n") + 1);
        static_cast<sources::HttpResponse*>(target)->headers[key] = value; }
    return size * count;
}

}  // namespace

HttpClient::HttpClient(std::chrono::seconds connect_timeout, std::chrono::seconds request_timeout)
    : connect_timeout_(connect_timeout), request_timeout_(request_timeout) {
    std::call_once(curl_initialized, [] {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) throw std::runtime_error("failed to initialize libcurl");
    });
}
void HttpClient::set_bearer_token(std::string token) { bearer_token_ = std::move(token); }

sources::HttpResponse HttpClient::get(const std::string& url) const {
    sources::HttpResponse response;
    CURL* handle = curl_easy_init();
    if (handle == nullptr) {
        response.error = "failed to create HTTP request";
        return response;
    }

    char error[CURL_ERROR_SIZE]{};
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, static_cast<long>(connect_timeout_.count()));
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, static_cast<long>(request_timeout_.count()));
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "DevSignal-Radar/0.1");
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, append_body);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, append_header);
    curl_easy_setopt(handle, CURLOPT_HEADERDATA, &response);
    curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error);

    curl_slist* headers = nullptr;
    if (!bearer_token_.empty()) { headers = curl_slist_append(headers, ("Authorization: Bearer " + bearer_token_).c_str()); curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers); }
    const auto result = curl_easy_perform(handle);
    if (result == CURLE_OK) curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response.status);
    else response.error = error[0] == '\0' ? curl_easy_strerror(result) : error;
    curl_easy_cleanup(handle);
    curl_slist_free_all(headers);
    return response;
}

}  // namespace techpulse::net
