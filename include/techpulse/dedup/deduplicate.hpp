#pragma once

#include "techpulse/model/radar_item.hpp"

#include <cstddef>
#include <vector>

namespace techpulse::dedup {

struct DeduplicationResult {
    std::vector<model::RadarItem> items;
    std::size_t source_key_merges{};
    std::size_t url_merges{};
};

DeduplicationResult deduplicate_exact(std::vector<model::RadarItem> items);

}  // namespace techpulse::dedup

