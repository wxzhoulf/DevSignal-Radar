cmake_minimum_required(VERSION 3.20)

project(devsignal_radar VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

add_library(techpulse_config
    src/config/radar_config.cpp
    src/dedup/deduplicate.cpp
    src/normalize/normalize.cpp
    src/net/http_client.cpp
    src/scoring/scoring.cpp
    src/publish/report.cpp
    src/sources/hacker_news.cpp
    src/sources/github.cpp
    src/sources/rss.cpp
)
target_include_directories(techpulse_config PUBLIC include)
target_link_libraries(techpulse_config PRIVATE CURL::libcurl)

add_executable(techpulse src/main.cpp)
target_link_libraries(techpulse PRIVATE techpulse_config)

include(CTest)
if(BUILD_TESTING)
    add_executable(config_tests tests/unit/config_tests.cpp)
    target_link_libraries(config_tests PRIVATE techpulse_config)
    add_test(NAME config_tests COMMAND config_tests)
    add_executable(normalize_tests tests/unit/normalize_tests.cpp)
    target_link_libraries(normalize_tests PRIVATE techpulse_config)
    add_test(NAME normalize_tests COMMAND normalize_tests)
    add_executable(deduplicate_tests tests/unit/deduplicate_tests.cpp)
    target_link_libraries(deduplicate_tests PRIVATE techpulse_config)
    add_test(NAME deduplicate_tests COMMAND deduplicate_tests)
    add_executable(scoring_tests tests/unit/scoring_tests.cpp)
    target_link_libraries(scoring_tests PRIVATE techpulse_config)
    add_test(NAME scoring_tests COMMAND scoring_tests)
    add_executable(hacker_news_tests tests/unit/hacker_news_tests.cpp)
    target_link_libraries(hacker_news_tests PRIVATE techpulse_config)
    add_test(NAME hacker_news_tests COMMAND hacker_news_tests)
    add_test(NAME validate_example COMMAND techpulse validate --config config/radar.yaml)
    set_tests_properties(validate_example PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
