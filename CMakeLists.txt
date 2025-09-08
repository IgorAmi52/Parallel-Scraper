cmake_minimum_required(VERSION 3.16)

project(parallel_scraper
  VERSION 0.1.0
  LANGUAGES CXX
)

# ====== C++ standard ======
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(TBB CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# ====== Sources ======
file(GLOB_RECURSE PS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/parallel-scraper/*.hpp")
set(PS_SOURCES
  src/main.cpp
  src/net/HttpClient.cpp
  src/utils/Config.cpp
  src/utils/FileIO.cpp
  src/parse/Parser.cpp
  src/store/ResultStore.cpp
  src/orchestrator/Orchestrator.cpp
  src/stats/Stats.cpp
)

add_executable(scraper ${PS_SOURCES} ${PS_HEADERS})

target_include_directories(scraper
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(scraper
  PRIVATE
    TBB::tbb
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

# ====== Warnings ======
if (MSVC)
  target_compile_options(scraper PRIVATE /W4 /permissive-)
else()
  target_compile_options(scraper PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ====== Build type default for single-config generators ======
if (NOT CMAKE_CONFIGURATION_TYPES)
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  endif()
endif()

set_target_properties(scraper PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
