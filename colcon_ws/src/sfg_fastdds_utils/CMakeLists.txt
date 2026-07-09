cmake_minimum_required(VERSION 3.8)
project(sfg_fastdds_utils)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ########################################################################################################
# Find dependencies.                                                                                     #
# ########################################################################################################
find_package(ament_cmake REQUIRED)
find_package(rmw REQUIRED)
find_package(rmw_fastrtps_cpp REQUIRED)
find_package(rcpputils REQUIRED)
find_package(sfg_utils REQUIRED)

# ########################################################################################################
# Add nodes and libraries.                                                                               #
# ########################################################################################################
sfg_utils__add_library(${PROJECT_NAME}
  SOURCES
  "src/message_type_support.cpp"
  "src/request_type_support.cpp"
  "src/response_type_support.cpp"
  "src/service_type_support.cpp"
  "src/type_support.cpp"
  DEPENDENCIES
  PUBLIC
  rmw::rmw
  rmw_fastrtps_cpp::rmw_fastrtps_cpp
  rcpputils::rcpputils
  EXPORT_DEPENDENCIES
  rmw
  rmw_fastrtps_cpp
  rcpputils
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)

  # The following line skips the linter which checks for copyrights.
  # Comment the line when a copyright and license is added to all source files.
  set(ament_cmake_copyright_FOUND TRUE)

  # The following line skips cpplint (only works in a git repo).
  # Comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files.
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()