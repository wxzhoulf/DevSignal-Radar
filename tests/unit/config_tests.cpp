#include "techpulse/config/radar_config.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

std::filesystem::path write_fixture(const char* name, const char* content) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream(path) << content;
    return path;
}
}

int main() {
    const auto valid = write_fixture("techpulse-valid.yaml", R"(version: 1
timezone: Asia/Shanghai
language: zh-CN
topics:
  - id: cpp
    name: C++
    weight: 1.0
    include:
      - C++
    exclude: []
output:
  daily_limit: 10
  weekly_limit: 30
  minimum_score: 45
)");
    const auto valid_result = techpulse::config::load_and_validate(valid);
    expect(valid_result.ok(), "valid configuration should pass");
    expect(valid_result.config.topics.size() == 1, "one topic should be parsed");

    const auto invalid = write_fixture("techpulse-invalid.yaml", R"(version: 1
timezone: Asia/Shanghai
language: zh-CN
topics:
  - id: cpp
    name: C++
    weight: 1.2
    include:
      - C++
output:
  daily_limit: 0
  weekly_limit: 30
  minimum_score: 101
)");
    const auto invalid_result = techpulse::config::load_and_validate(invalid);
    expect(!invalid_result.ok(), "invalid ranges should fail");

    std::filesystem::remove(valid);
    std::filesystem::remove(invalid);
    return failures == 0 ? 0 : 1;
}

