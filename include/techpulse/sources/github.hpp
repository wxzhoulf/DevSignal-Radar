#pragma once
#include "techpulse/sources/source.hpp"
namespace techpulse::sources { FetchResult fetch_github_repositories(const std::string& query, const HttpGet& get); }
