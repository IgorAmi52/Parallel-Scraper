cmake_minimum_required(VERSION 3.16)

project(parallel_scraper
  VERSION 0.1.0
  LANGUAGES CXX
)

# ===================== C++ standard =====================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(TBB CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# ===================== Headers & Sources =====================
file(GLOB_RECURSE PS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/parallel-scraper/*.hpp")

# Sav "core" ide u biblioteku (bez main.cpp)
set(PS_LIB_SOURCES
  src/net/HttpClient.cpp
  src/utils/Config.cpp
  src/utils/FileIO.cpp
  src/parse/Parser.cpp
  src/store/ResultStore.cpp
  src/orchestrator/Orchestrator.cpp
  src/stats/Stats.cpp
)

add_library(parallel_scraper_lib ${PS_LIB_SOURCES} ${PS_HEADERS})
target_include_directories(parallel_scraper_lib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(parallel_scraper_lib
  PUBLIC
    TBB::tbb
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

# Warnings
if (MSVC)
  target_compile_options(parallel_scraper_lib PRIVATE /W4 /permissive-)
else()
  target_compile_options(parallel_scraper_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

# (opciono) AddressSanitizer u Debug modu za Clang/GCC
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU"))
  target_compile_options(parallel_scraper_lib PRIVATE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(parallel_scraper_lib PRIVATE -fsanitize=address)
endif()

# ===================== Executable (scraper) =====================
add_executable(scraper src/main.cpp)
target_link_libraries(scraper PRIVATE parallel_scraper_lib)

# Warnings i ASan i za exe
if (MSVC)
  target_compile_options(scraper PRIVATE /W4 /permissive-)
else()
  target_compile_options(scraper PRIVATE -Wall -Wextra -Wpedantic)
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU"))
  target_compile_options(scraper PRIVATE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(scraper PRIVATE -fsanitize=address)
endif()

# ===================== Build type default =====================
if (NOT CMAKE_CONFIGURATION_TYPES)
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
  endif()
endif()

# ===================== Output dir =====================
set_target_properties(scraper PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# ===================== Tests =====================
option(PS_BUILD_TESTS "Build unit tests" ON)
if (PS_BUILD_TESTS)
  include(CTest)
  enable_testing()

  find_package(Catch2 3 CONFIG QUIET)
  if (Catch2_FOUND)
    add_executable(ps_tests
      tests/test_parser.cpp
      tests/test_stats.cpp
      tests/test_resultstore.cpp
    )
    target_link_libraries(ps_tests
      PRIVATE
        parallel_scraper_lib     
        Catch2::Catch2WithMain
    )
    if (MSVC)
      target_compile_options(ps_tests PRIVATE /W4 /permissive-)
    else()
      target_compile_options(ps_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU"))
      target_compile_options(ps_tests PRIVATE -fsanitize=address -fno-omit-frame-pointer)
      target_link_options(ps_tests PRIVATE -fsanitize=address)
    endif()

    # Registracija testa sa punom putanjom do binara:
    add_test(NAME unit_tests COMMAND $<TARGET_FILE:ps_tests>)


  # Ensure test data is available in the binary directory
  file(COPY "${CMAKE_SOURCE_DIR}/tests/data"
     DESTINATION "${CMAKE_BINARY_DIR}/tests")

  else()
    message(STATUS "Catch2 not found; tests target will not be built.")
  endif()
endif()
