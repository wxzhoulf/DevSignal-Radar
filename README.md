# DevSignal Radar

An explainable technical-signal radar for systems software engineers. It will aggregate public GitHub, Hacker News, and RSS signals, then produce deterministic daily reports and a static dashboard.

## Current status

The foundation now includes the C++20 `techpulse validate` command, normalized `RadarItem` records, exact provenance-preserving deduplication, explainable rule scoring, an example configuration, and cross-platform CI.

## Quick start

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
./build/techpulse validate --config config/radar.yaml
ctest --test-dir build --output-on-failure
```

On multi-config generators, append `--config Debug` to build and test commands.

## Configuration

`config/radar.yaml` defines the user's topics, weights, include/exclude terms, and report thresholds. `techpulse validate` exits with code `10` for malformed or invalid configuration and reports ignored unknown fields as warnings.

## Roadmap

1. Source adapters and normalized records
2. Deterministic URL deduplication and explainable scoring
3. Daily reports and static dashboard
4. Scheduled automation, weekly reports, and optional AI summaries

