include(CMakeFindDependencyMacro)

find_dependency(nlohmann_json REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/datacoeTargets.cmake")