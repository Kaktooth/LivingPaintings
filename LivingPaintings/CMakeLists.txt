# CMakeList.txt : CMake project for LivingPaintings, include source and define
# project specific logic here.
#

find_package(Stb REQUIRED)
find_package(gflags CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(CGAL CONFIG REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )

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
set(PREPROCESS_SAM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/models/edge_sam_3x_encoder.onnx")
set(SAM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources/models/edge_sam_3x_decoder.onnx")
add_definitions( -DTEXTURE_FILE_PATH=${TEXTURE_FILE_PATH} )
add_definitions( -DRESOURCE_SHADER_PATH=${RESOURCE_SHADER_PATH} )
add_definitions( -DPREPROCESS_SAM_PATH=${PREPROCESS_SAM_PATH} )
add_definitions( -DSAM_PATH=${SAM_PATH} )


configure_file("src/config.hpp.in" ${CMAKE_BINARY_DIR}/generated/config.hpp )
include_directories( ${CMAKE_BINARY_DIR}/generated/ )


include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )

add_executable (LivingPaintings "src/LivingPaintings.cpp" "src/LivingPaintings.h" ${vulkan_engine_SRC} "src/config.hpp"   "src/segmentation/segmentation_system.h" "src/segmentation/segmentation_system.cpp")
target_link_libraries(LivingPaintings PRIVATE  gflags::gflags glfw glm::glm Vulkan::Vulkan imgui::imgui unofficial::shaderc::shaderc ${OpenCV_LIBS} CGAL::CGAL
"${CMAKE_SOURCE_DIR}/include/sam/$<CONFIG>/sam_cpp_lib.lib")
target_include_directories(LivingPaintings PRIVATE ${onnxruntime_lib}
 ${OpenCV_LIBS} ${CMAKE_CURRENT_SOURCE_DIR}/include ${$ENV{VULKAN_SDK}/include} ${Stb_INCLUDE_DIR})
add_custom_command(TARGET LivingPaintings POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/include/sam/$<CONFIG>"
    $<TARGET_FILE_DIR:LivingPaintings>)

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