#include "techpulse/config/radar_config.hpp"
#include "techpulse/net/http_client.hpp"
#include "techpulse/sources/hacker_news.hpp"
#include "techpulse/sources/github.hpp"
#include "techpulse/sources/rss.hpp"
#include "techpulse/dedup/deduplicate.hpp"
#include "techpulse/scoring/scoring.hpp"
#include "techpulse/publish/report.hpp"
#include <cstdlib>

#include <iostream>
#include <string_view>

namespace {
void print_usage() {
    std::cerr << "Usage: techpulse validate [--config <path>]\n"
                 "       techpulse fetch-hn [--limit <count>]\n"
                 "       techpulse fetch-github <query>\n"
                 "       techpulse fetch-rss <url>\n";
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
    if (std::string_view(argv[1]) == "fetch-rss") {
        if (argc != 3) { print_usage(); return 10; }
        techpulse::net::HttpClient client;
        const auto result = techpulse::sources::fetch_rss(argv[2], [&client](const std::string& url) { return client.get(url); });
        for (const auto& item : result.items) std::cout << item.title << '\t' << item.url << '\n';
        return result.items.empty() ? 20 : 0;
    }
    if (std::string_view(argv[1]) == "run") {
        const auto config = techpulse::config::load_and_validate("config/radar.yaml");
        if (!config.ok()) return 10;
        techpulse::net::HttpClient client;
        if (const char* token = std::getenv("GITHUB_TOKEN")) client.set_bearer_token(token);
        auto hn = techpulse::sources::HackerNewsSource{3}.fetch([&](const std::string& url){ return client.get(url); });
        auto gh = techpulse::sources::fetch_github_repositories("language:C++ stars:>20", [&](const std::string& url){ return client.get(url); });
        hn.items.insert(hn.items.end(), std::make_move_iterator(gh.items.begin()), std::make_move_iterator(gh.items.end()));
        hn.errors.insert(hn.errors.end(), gh.errors.begin(), gh.errors.end());
        auto unique = techpulse::dedup::deduplicate_exact(std::move(hn.items)); std::vector<techpulse::model::ScoredItem> scored;
        const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()); for (const auto& item : unique.items) scored.push_back(techpulse::scoring::score_item(item,config.config,now));
        scored=techpulse::scoring::rank_items(std::move(scored)); char date[11]{}; const auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); std::strftime(date,sizeof(date),"%F",std::gmtime(&t));
        return techpulse::publish::write_daily_report(".",date,scored,hn.errors)?(hn.errors.empty()?0:2):30;
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
