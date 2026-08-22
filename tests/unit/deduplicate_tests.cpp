#include "techpulse/dedup/deduplicate.hpp"

#include <iostream>

namespace {
int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

techpulse::model::RadarItem item(std::string source, std::string id, std::string url) {
    techpulse::model::RadarItem result;
    result.source_type = std::move(source);
    result.source_id = std::move(id);
    result.url = std::move(url);
    result.title = "LLVM release";
    return result;
}
}

int main() {
    auto github = item("github", "llvm/llvm-project", "https://github.com/llvm/llvm-project/releases/tag/20");
    github.summary_raw = "Release notes";
    auto hn = item("hacker_news", "42", "https://github.com/llvm/llvm-project?utm_source=hn");
    hn.discussion_url = "https://news.ycombinator.com/item?id=42";
    hn.metrics.points = 80;

    const auto merged = techpulse::dedup::deduplicate_exact({github, hn});
    expect(merged.items.size() == 1, "canonical URLs should merge across sources");
    expect(merged.url_merges == 1, "URL merge count should be recorded");
    expect(merged.items.front().provenance.size() == 2, "all source evidence should be retained");

    auto repeated = github;
    repeated.summary_raw = "More complete notes";
    const auto same_source = techpulse::dedup::deduplicate_exact({github, repeated});
    expect(same_source.items.size() == 1, "same source key should merge");
    expect(same_source.source_key_merges == 1, "source-key merge count should be recorded");
    expect(same_source.items.front().summary_raw == "More complete notes", "more complete primary item should win");
    return failures == 0 ? 0 : 1;
}

