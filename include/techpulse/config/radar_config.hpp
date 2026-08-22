#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace techpulse::config {

struct Topic {
    std::string id;
    std::string name;
    double weight{};
    std::vector<std::string> include;
    std::vector<std::string> exclude;
};

struct Output {
    int daily_limit{};
    int weekly_limit{};
    int minimum_score{};
};

struct RadarConfig {
    int version{};
    std::string timezone;
    std::string language;
    std::vector<Topic> topics;
    Output output;
    std::vector<std::string> warnings;
};

struct ValidationResult {
    RadarConfig config;
    std::vector<std::string> errors;

    [[nodiscard]] bool ok() const { return errors.empty(); }
};

ValidationResult load_and_validate(const std::filesystem::path& path);

}  // namespace techpulse::config

