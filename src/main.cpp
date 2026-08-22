#include "techpulse/config/radar_config.hpp"
#include "techpulse/net/http_client.hpp"
#include "techpulse/sources/hacker_news.hpp"
#include "techpulse/sources/github.hpp"
#include <cstdlib>

#include <iostream>
#include <string_view>

namespace {
void print_usage() {
    std::cerr << "Usage: techpulse validate [--config <path>]\n"
                 "       techpulse fetch-hn [--limit <count>]\n"
                 "       techpulse fetch-github <query>\n";
}
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 10;
    }

    if (std::string_view(argv[1]) == "fetch-hn") {
        std::size_t limit = 50;
        if (argc == 4 && std::string_view(argv[2]) == "--limit") {
            try { limit = std::stoul(argv[3]); }
            catch (...) { print_usage(); return 10; }
        } else if (argc != 2) {
            print_usage();
            return 10;
        }
        techpulse::net::HttpClient client;
        const auto result = techpulse::sources::HackerNewsSource{limit}.fetch([&client](const std::string& url) {
            return client.get(url);
        });
        for (const auto& error : result.errors) std::cerr << "warning: " << error << '\n';
        for (const auto& item : result.items) std::cout << item.source_type << '\t' << item.title << '\t' << item.url << '\n';
        return result.items.empty() ? 20 : (result.errors.empty() ? 0 : 2);
    }
    if (std::string_view(argv[1]) == "fetch-github") {
        if (argc != 3) { print_usage(); return 10; }
        techpulse::net::HttpClient client;
        if (const char* token = std::getenv("GITHUB_TOKEN")) client.set_bearer_token(token);
        const auto result = techpulse::sources::fetch_github_repositories(argv[2], [&client](const std::string& url) { return client.get(url); });
        for (const auto& error : result.errors) std::cerr << "warning: " << error << '\n';
        for (const auto& item : result.items) std::cout << item.title << '\t' << item.metrics.stars.value_or(0) << '\t' << item.url << '\n';
        return result.items.empty() ? 20 : (result.errors.empty() ? 0 : 2);
    }

    if (std::string_view(argv[1]) != "validate") {
        print_usage();
        return 10;
    }

    std::string config_path = "config/radar.yaml";
    if (argc == 4 && std::string_view(argv[2]) == "--config") config_path = argv[3];
    else if (argc != 2) {
        print_usage();
        return 10;
    }

    const auto result = techpulse::config::load_and_validate(config_path);
    for (const auto& warning : result.config.warnings) std::cerr << "warning: " << warning << '\n';
    if (!result.ok()) {
        for (const auto& error : result.errors) std::cerr << "error: " << error << '\n';
        return 10;
    }

    std::cout << "Configuration is valid: " << result.config.topics.size() << " topic(s)\n";
    return 0;
}
