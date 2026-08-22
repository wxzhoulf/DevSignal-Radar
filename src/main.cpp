#include "techpulse/config/radar_config.hpp"

#include <iostream>
#include <string_view>

namespace {
void print_usage() {
    std::cerr << "Usage: techpulse validate [--config <path>]\n";
}
}

int main(int argc, char* argv[]) {
    if (argc < 2 || std::string_view(argv[1]) != "validate") {
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

