# add dependencies via CPM
include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

# GLAD
CPMAddPackage(
  NAME glad
  GITHUB_REPOSITORY Dav1dde/glad
  VERSION 0.1.35
)

# GLM
CPMAddPackage(
  NAME glm
  GITHUB_REPOSITORY g-truc/glm
  GIT_TAG 0.9.9.8
)

# GLFW
CPMAddPackage(
  NAME glfw
  GITHUB_REPOSITORY glfw/glfw
  GIT_TAG 3.3.8
  OPTIONS
    "GLFW_BUILD_TESTS Off"
    "GLFW_BUILD_EXAMPLES Off"
    "GLFW_BUILD_DOCS Off"
    "GLFW_INSTALL Off"
    "GLFW_USE_HYBRID_HPG On"
)

CPMAddPackage(
  NAME GroupSourcesByFolder.cmake
  GITHUB_REPOSITORY TheLartians/GroupSourcesByFolder.cmake
  VERSION 1.0
)

# put all external targets into a seperate folder to not pollute the project folder
set_target_properties(glad glad-generate-files glfw PROPERTIES FOLDER ExternalTargets)
