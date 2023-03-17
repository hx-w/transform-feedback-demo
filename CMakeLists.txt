cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(demo LANGUAGES CXX)

#-----------------------------------------------------------------------------#
set(ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets)
add_compile_definitions(ASSETS_PATH="${ASSETS_DIR}")

#-----------------------------------------------------------------------------#
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.h")
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS "${ASSETS_DIR}/shaders/*.glsl")

#-----------------------------------------------------------------------------#
include(cmake/Dependency.cmake)

#-----------------------------------------------------------------------------#
add_executable(${PROJECT_NAME} ${SRC_DIR}/main.cpp ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})
GroupSourcesByFolder(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC src)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# specify libraries to link with after compilation
target_link_libraries(${PROJECT_NAME} PRIVATE glad glfw glm)
