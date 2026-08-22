#pragma once

#include "techpulse/sources/source.hpp"

#include <cstddef>

namespace techpulse::sources {

class HackerNewsSource {
public:
    explicit HackerNewsSource(std::size_t candidates_per_feed = 50) : candidates_per_feed_(candidates_per_feed) {}

    FetchResult fetch(const HttpGet& get) const;

private:
    std::size_t candidates_per_feed_;
};

}  // namespace techpulse::sources

