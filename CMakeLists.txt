# Setting up CMake Project
cmake_minimum_required(VERSION 3.1)
project(autograph LANGUAGES CXX)

# Setting compile options.
set(CMAKE_CXX_STANDARD 20)

# Setting CMake build options, adding path for binary.
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Lots of warnings.
find_package(PkgConfig REQUIRED)
add_compile_options(-Wall -Wextra -pedantic)

# Finding appropriate packages.
pkg_search_module(graphviz REQUIRED libgvc IMPORTED_TARGET)
include_directories(PkgConfig::graphviz)
link_libraries(PkgConfig::graphviz)

# Adding C++ files needed for compilation.
add_executable(
  autograph
  src/main.cpp
)

# Selecting compiler
if(APPLE)

  # Using LLVM clang, installed via homebrew, instaed of Apple clang.
  if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")

    # Homebrew install path on arm64 (apple silicon, M1 etc.) machines.
    set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")
    set(CMAKE_C_COMPILER "/opt/homebrew/opt/llvm/bin/clang")
  else()

    # Homebrew install path on x86 machines (Intel).
    set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
  endif()
elseif(UNIX)

  # Using GNU gcc-11.
  set(CMAKE_CXX_COMPILER "g++-11")
  set(CMAKE_C_COMPILER "gcc-11")
endif()

target_include_directories(autograph PUBLIC ${PROJECT_SOURCE_DIR}/include)
# Linking appropriate libraries to autograph target.
target_link_libraries(autograph PUBLIC PkgConfig::graphviz)

# Providing make with install target.
install(TARGETS autograph DESTINATION bin)

# Providing make with uninstall target.
# TODO: Polish
# From https://stackoverflow.com/questions/41471620/cmake-support-make-uninstall
add_custom_target("uninstall")
add_custom_command(
    TARGET "uninstall"
    POST_BUILD
    COMMENT "Uninstalling autograph..."
    COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in
            install_manifest.txt to be uninstalled!
)
