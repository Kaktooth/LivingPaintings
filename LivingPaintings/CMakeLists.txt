set(OpenCV_DIR "${VCPKG_INSTALLED_DIR}/x64-windows/share/opencv4")

find_package(Stb REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(CGAL CONFIG REQUIRED)
find_package(FFMPEG REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )

configure_file("src/config.hpp.in" ${CMAKE_BINARY_DIR}/generated/config.hpp )
include_directories( ${CMAKE_BINARY_DIR}/generated/ )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src/include )
include_directories( ${OpenCV_INCLUDE_DIRS} )

file(GLOB SOURCES
     "src/LivingPaintings.cpp"
     "src/segmentation/segmentation_system.cpp"
     "src/utils/*.cpp"
     "src/vulkan/*.cpp"
)

file(GLOB HEADERS
     "src/segmentation/segmentation_system.h"
     "src/utils/*.h"
     "src/vulkan/*.h"
     "src/config.hpp"
)

file(GLOB INCLUDE_HEADERS
     "src/include/sam/sam.h"
     "src/include/inpaint/*.h"
)

add_executable (LivingPaintings ${SOURCES} ${HEADERS} ${INCLUDE_HEADERS} )

target_compile_definitions(LivingPaintings PRIVATE $<$<CONFIG:Debug>:DEBUG>)

target_include_directories(LivingPaintings PRIVATE ${onnxruntime_lib} ${OpenCV_LIBS}
${CMAKE_CURRENT_SOURCE_DIR}/include ${$ENV{VULKAN_SDK}/include} ${Stb_INCLUDE_DIR}
${FFMPEG_INCLUDE_DIRS})

target_link_directories(LivingPaintings PRIVATE ${FFMPEG_LIBRARY_DIRS})

target_link_libraries(LivingPaintings PRIVATE glfw glm::glm Vulkan::Vulkan imgui::imgui
                                              unofficial::shaderc::shaderc ${OpenCV_LIBS} CGAL::CGAL
                                              "${CMAKE_SOURCE_DIR}/include/sam/$<CONFIG>/sam_cpp_lib.lib" 
                                              "${CMAKE_SOURCE_DIR}/include/inpaint/$<CONFIG>/inpaint.lib"
                                              ${FFMPEG_LIBRARIES})

set(TEXTURE_FILE_NAME "Van_Gogh-Starry_Night.png")
set(TEXTURE_WIDTH 1200)
set(TEXTURE_HEIGHT 950)
set(PREPROCESS_SAM_FILE_NAME "edge_sam_3x_encoder.onnx")
set(SAM_TEXTURE_FILE_NAME "edge_sam_3x_decoder.onnx")
set(TEXTURE_FILE_PATH "/resources/textures/${TEXTURE_FILE_NAME}")
set(RESOURCE_SHADER_PATH "/resources/shaders/")
set(PREPROCESS_SAM_PATH "/resources/models/${PREPROCESS_SAM_FILE_NAME}")
set(SAM_PATH "/resources/models/${SAM_TEXTURE_FILE_NAME}")
set(RESOURCE_PREPROCESS_SAM_PATH "/resources/models/${PREPROCESS_SAM_FILE_NAME}")
set(RESOURCE_SAM_PATH "/resources/models/${SAM_TEXTURE_FILE_NAME}")
add_definitions( -DTEXTURE_FILE_PATH=${TEXTURE_FILE_PATH} )
add_definitions( -DTEXTURE_WIDTH=${TEXTURE_WIDTH})
add_definitions( -DTEXTURE_HEIGHT=${TEXTURE_HEIGHT} )
add_definitions( -DRESOURCE_SHADER_PATH=${RESOURCE_SHADER_PATH} )
add_definitions( -DPREPROCESS_SAM_PATH=${PREPROCESS_SAM_PATH} )
add_definitions( -DSAM_PATH=${SAM_PATH} )

add_custom_command(TARGET LivingPaintings POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/include/sam/$<CONFIG>"
    $<TARGET_FILE_DIR:LivingPaintings>)

add_custom_command(TARGET LivingPaintings POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/textures"
    "$<TARGET_FILE_DIR:LivingPaintings>/resources/textures")

add_custom_command(TARGET LivingPaintings POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders"
    "$<TARGET_FILE_DIR:LivingPaintings>/resources/shaders")

add_custom_command(TARGET LivingPaintings POST_BUILD
COMMAND ${CMAKE_COMMAND} -E make_directory
    "$<TARGET_FILE_DIR:LivingPaintings>/resources/models")

add_custom_command(TARGET LivingPaintings POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}${RESOURCE_PREPROCESS_SAM_PATH}" 
    "${CMAKE_CURRENT_SOURCE_DIR}${RESOURCE_SAM_PATH}"
    "$<TARGET_FILE_DIR:LivingPaintings>/resources/models")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LivingPaintings PROPERTY CXX_STANDARD 20)
endif()