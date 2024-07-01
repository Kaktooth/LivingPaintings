# CMakeList.txt : CMake project for DeepPicture, include source and define
# project specific logic here.
#

find_package(Stb REQUIRED)
find_package(gflags CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(
    PVS_CMakeModule
    GIT_REPOSITORY "https://github.com/viva64/pvs-studio-cmake-module.git"
    GIT_TAG        "master" 
)
FetchContent_MakeAvailable(PVS_CMakeModule)
include("${pvs_cmakemodule_SOURCE_DIR}/PVS-Studio.cmake")

file(GLOB vulkan_engine_SRC
     "src/vulkan/*.h"
     "src/vulkan/*.cpp"
)

set(TEXTURE_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/painting-342116_1920.jpg")
set(RESOURCE_SHADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/")
add_definitions( -DTEXTURE_FILE_PATH=${TEXTURE_FILE_PATH} )
add_definitions( -DRESOURCE_SHADER_PATH=${RESOURCE_SHADER_PATH} )
configure_file("src/config.hpp.in" ${CMAKE_BINARY_DIR}/generated/config.hpp )
include_directories( ${CMAKE_BINARY_DIR}/generated/ )

add_executable (LivingPaintings "src/LivingPaintings.cpp" "src/LivingPaintings.h" ${vulkan_engine_SRC} "src/config.hpp")
target_link_libraries(LivingPaintings PRIVATE gflags::gflags glfw glm::glm Vulkan::Vulkan imgui::imgui unofficial::shaderc::shaderc)
target_include_directories(LivingPaintings PRIVATE ${$ENV{VULKAN_SDK}/include} ${Stb_INCLUDE_DIR})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LivingPaintings PROPERTY CXX_STANDARD 20)
endif()

pvs_studio_add_target(TARGET LivingPaintings.analyze ALL
                      OUTPUT FORMAT html
                      ANALYZE LivingPaintings
                      SOURCES ${CMAKE_CURRENT_SOURCE_DIR}
                      MODE GA:1,2
                      PREPROCESSOR clang
                      LOG LivingPaintings.err)