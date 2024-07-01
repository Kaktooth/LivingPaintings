# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#

cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_FLAGS "-g -std=c++20 -Wall -Wextra -v")
set(CMAKE_BUILD_TYPE Debug)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
endif()

project ("LivingPaintings")

# Include sub-projects.
add_subdirectory ("LivingPaintings")
