#pragma once

#include <string>

namespace techpulse::normalize {

std::string canonical_url(const std::string& url);
std::string clean_title(const std::string& title);

}  // namespace techpulse::normalize

