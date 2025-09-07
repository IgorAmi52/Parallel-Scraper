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

# HTML parser (opciono): gumbo preko vcpkg ili Homebrew (pkg-config).
option(USE_GUMBO "Enable Gumbo HTML parser" ON)

# ====== Sources ======
file(GLOB_RECURSE PS_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/parallel-scraper/*.hpp")
set(PS_SOURCES
  src/main.cpp
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

# ====== Gumbo: vcpkg (CONFIG) -> Homebrew (pkg-config) -> fallback ======
set(HAVE_GUMBO 0)

if (USE_GUMBO)
  # 1) vcpkg način: unofficial-gumbo CONFIG target
  find_package(unofficial-gumbo CONFIG QUIET)
  if (TARGET unofficial::gumbo::gumbo)
    target_link_libraries(scraper PRIVATE unofficial::gumbo::gumbo)
    set(HAVE_GUMBO 1)
    message(STATUS "Gumbo found via vcpkg (unofficial-gumbo).")
  else()
    # 2) Homebrew način: pkg-config gumbo
    find_package(PkgConfig QUIET)
    if (PkgConfig_FOUND)
      pkg_check_modules(GUMBO QUIET gumbo)
      if (GUMBO_FOUND)
        # Napravi importovani target da bi ostatak koda bio isti
        add_library(Gumbo::gumbo UNKNOWN IMPORTED)
        # Neki pkg-config wrapperi vraćaju liste; uzmi prvu lib putanju
        if (GUMBO_LINK_LIBRARIES)
          list(GET GUMBO_LINK_LIBRARIES 0 GUMBO_LIB)
        endif()
        set_target_properties(Gumbo::gumbo PROPERTIES
          IMPORTED_LOCATION            "${GUMBO_LIB}"
          INTERFACE_INCLUDE_DIRECTORIES "${GUMBO_INCLUDE_DIRS}"
        )
        target_link_libraries(scraper PRIVATE Gumbo::gumbo)
        set(HAVE_GUMBO 1)
        message(STATUS "Gumbo found via pkg-config (likely Homebrew).")
      endif()
    endif()
  endif()
endif()

if (HAVE_GUMBO)
  target_compile_definitions(scraper PRIVATE PARALLEL_SCRAPER_WITH_GUMBO=1)
else()
  message(STATUS "Gumbo not found. Build will proceed without HTML parsing backend.")
  message(STATUS "Tips: vcpkg: 'unofficial-gumbo' | Homebrew: 'brew install gumbo-parser' (plus pkg-config).")
endif()

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

# ====== Output dir (čistoća) ======
set_target_properties(scraper PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
