cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_FLAGS "-std=c++20 -Wall -v ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O1")
set(CMAKE_MSVC_PARALLEL_ENABLE ON)
set(CMAKE_CXX_CLANG_TIDY 
clang-tidy; 
-header-filter=.;
-checks=-*,google-*,performance-*,portability-*;)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
endif()

project (LivingPaintings CXX)

add_subdirectory (LivingPaintings)
