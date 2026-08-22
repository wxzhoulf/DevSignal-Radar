#include "techpulse/config/radar_config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>

namespace techpulse::config {
namespace {

std::string trim(std::string value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) { return std::isspace(c); }).base();
    return first < last ? std::string(first, last) : std::string{};
}

std::string unquote(std::string value) {
    value = trim(std::move(value));
    if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                              (value.front() == '\'' && value.back() == '\''))) {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

bool split_key_value(const std::string& line, std::string& key, std::string& value) {
    const auto colon = line.find(':');
    if (colon == std::string::npos) return false;
    key = trim(line.substr(0, colon));
    value = unquote(line.substr(colon + 1));
    return !key.empty();
}

void validate(RadarConfig& config, std::vector<std::string>& errors) {
    if (config.version != 1) errors.push_back("version must be 1");
    if (config.timezone.empty()) errors.push_back("timezone is required");
    if (config.language.empty()) errors.push_back("language is required");
    if (config.topics.empty()) errors.push_back("at least one topic is required");
    if (config.output.daily_limit <= 0) errors.push_back("output.daily_limit must be positive");
    if (config.output.weekly_limit <= 0) errors.push_back("output.weekly_limit must be positive");
    if (config.output.minimum_score < 0 || config.output.minimum_score > 100) {
        errors.push_back("output.minimum_score must be between 0 and 100");
    }

    std::set<std::string> ids;
    for (const auto& topic : config.topics) {
        if (topic.id.empty()) errors.push_back("topic.id is required");
        else if (!ids.insert(topic.id).second) errors.push_back("topic.id must be unique: " + topic.id);
        if (topic.name.empty()) errors.push_back("topic.name is required for " + topic.id);
        if (topic.weight < 0.0 || topic.weight > 1.0) errors.push_back("topic.weight must be between 0.0 and 1.0 for " + topic.id);
        if (topic.include.empty()) errors.push_back("topic.include must not be empty for " + topic.id);
    }
}

}  // namespace

ValidationResult load_and_validate(const std::filesystem::path& path) {
    ValidationResult result;
    std::ifstream input(path);
    if (!input) {
        result.errors.push_back("cannot read configuration: " + path.string());
        return result;
    }

    enum class Section { root, topics, output, include, exclude } section = Section::root;
    Topic* current_topic = nullptr;
    std::string raw;
    std::size_t line_number = 0;
    while (std::getline(input, raw)) {
        ++line_number;
        const auto comment = raw.find('#');
        if (comment != std::string::npos) raw.erase(comment);
        const auto indentation = raw.find_first_not_of(' ');
        if (indentation == std::string::npos) continue;
        const auto line = trim(raw);

        if (line == "topics:") { section = Section::topics; current_topic = nullptr; continue; }
        if (line == "output:") { section = Section::output; current_topic = nullptr; continue; }
        if (line == "include:") { section = Section::include; continue; }
        if (line == "exclude:") { section = Section::exclude; continue; }

        if (line.rfind("- id:", 0) == 0) {
            if (section != Section::topics && section != Section::include && section != Section::exclude) {
                result.errors.push_back("unexpected topic entry at line " + std::to_string(line_number));
                continue;
            }
            section = Section::topics;
            result.config.topics.push_back({});
            current_topic = &result.config.topics.back();
            current_topic->id = unquote(line.substr(5));
            continue;
        }
        if (line.rfind("- ", 0) == 0) {
            if (current_topic == nullptr || (section != Section::include && section != Section::exclude)) {
                result.errors.push_back("unexpected list entry at line " + std::to_string(line_number));
            } else if (section == Section::include) {
                current_topic->include.push_back(unquote(line.substr(2)));
            } else {
                current_topic->exclude.push_back(unquote(line.substr(2)));
            }
            continue;
        }

        std::string key, value;
        if (!split_key_value(line, key, value)) {
            result.errors.push_back("invalid YAML at line " + std::to_string(line_number));
            continue;
        }
        try {
            if (current_topic != nullptr && indentation >= 4 && section == Section::topics) {
                if (key == "name") current_topic->name = value;
                else if (key == "weight") current_topic->weight = std::stod(value);
                else if ((key == "include" || key == "exclude") && value == "[]") {}
                else result.config.warnings.push_back("unknown topic field ignored at line " + std::to_string(line_number) + ": " + key);
            } else if (section == Section::output && indentation >= 2) {
                if (key == "daily_limit") result.config.output.daily_limit = std::stoi(value);
                else if (key == "weekly_limit") result.config.output.weekly_limit = std::stoi(value);
                else if (key == "minimum_score") result.config.output.minimum_score = std::stoi(value);
                else result.config.warnings.push_back("unknown output field ignored at line " + std::to_string(line_number) + ": " + key);
            } else if (indentation == 0) {
                section = Section::root;
                current_topic = nullptr;
                if (key == "version") result.config.version = std::stoi(value);
                else if (key == "timezone") result.config.timezone = value;
                else if (key == "language") result.config.language = value;
                else result.config.warnings.push_back("unknown root field ignored at line " + std::to_string(line_number) + ": " + key);
            } else {
                result.errors.push_back("field outside a supported section at line " + std::to_string(line_number));
            }
        } catch (const std::exception&) {
            result.errors.push_back("invalid value for " + key + " at line " + std::to_string(line_number));
        }
    }

    validate(result.config, result.errors);
    return result;
}

}  // namespace techpulse::config
