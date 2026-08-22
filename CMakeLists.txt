cmake_minimum_required(VERSION 3.20)

project(devsignal_radar VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(techpulse_config
    src/config/radar_config.cpp
)
target_include_directories(techpulse_config PUBLIC include)

add_executable(techpulse src/main.cpp)
target_link_libraries(techpulse PRIVATE techpulse_config)

include(CTest)
if(BUILD_TESTING)
    add_executable(config_tests tests/unit/config_tests.cpp)
    target_link_libraries(config_tests PRIVATE techpulse_config)
    add_test(NAME config_tests COMMAND config_tests)
    add_test(NAME validate_example COMMAND techpulse validate --config config/radar.yaml)
    set_tests_properties(validate_example PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
