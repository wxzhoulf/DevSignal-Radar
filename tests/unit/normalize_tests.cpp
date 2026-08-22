#include "techpulse/normalize/normalize.hpp"

#include <iostream>

namespace {
int failures = 0;

void expect_equal(const std::string& actual, const std::string& expected, const char* message) {
    if (actual != expected) {
        std::cerr << "FAILED: " << message << "\n  expected: " << expected << "\n  actual:   " << actual << '\n';
        ++failures;
    }
}
}

int main() {
    using techpulse::normalize::canonical_url;
    expect_equal(canonical_url("HTTPS://Example.COM/news?utm_source=rss&id=7#comments"),
                 "https://example.com/news?id=7", "host, scheme, tracking parameters, and fragments are normalized");
    expect_equal(canonical_url("https://example.com/a?b=2&a=1"),
                 "https://example.com/a?a=1&b=2", "retained query parameters have a stable order");
    expect_equal(canonical_url("https://github.com/llvm/llvm-project/releases/tag/20.1?utm_campaign=x"),
                 "https://github.com/llvm/llvm-project", "GitHub repository links collapse to a repository URL");
    expect_equal(techpulse::normalize::clean_title("  C++\t toolchain\n update  "),
                 "C++ toolchain update", "title whitespace is collapsed");
    return failures == 0 ? 0 : 1;
}

