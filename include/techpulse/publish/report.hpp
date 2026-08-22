#pragma once
#include "techpulse/model/scored_item.hpp"
#include <filesystem>
#include <string>
#include <vector>
namespace techpulse::publish { bool write_daily_report(const std::filesystem::path& root,const std::string& date,const std::vector<model::ScoredItem>& items,const std::vector<std::string>& errors); }
