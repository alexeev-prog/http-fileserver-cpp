cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    httpfileserver
    VERSION 0.1.0
    DESCRIPTION "Simple HTTP Server in C++"
    HOMEPAGE_URL "https://github.com/alexeev-prog/http-fileserver-cpp"
    LANGUAGES CXX
)

find_package(Boost REQUIRED COMPONENTS filesystem system)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})

# ---- Declare library ----

add_library(
    httpfileserver_lib OBJECT
    source/server.cpp
    source/server.hpp
    source/logger.hpp
)

include_directories(${Boost_INCLUDE_DIRS})

target_include_directories(
    httpfileserver_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(httpfileserver_lib PUBLIC cxx_std_20)

# ---- Declare executable ----

add_executable(httpfileserver_exe source/main.cpp)
add_executable(httpfileserver::exe ALIAS httpfileserver_exe)

set_property(TARGET httpfileserver_exe PROPERTY OUTPUT_NAME httpfileserver)

target_compile_features(httpfileserver_exe PRIVATE cxx_std_20)

target_link_libraries(httpfileserver_exe PRIVATE httpfileserver_lib)

target_link_libraries(httpfileserver_exe
        PRIVATE
        Boost::filesystem
        Boost::system
)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT httpfileserver_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of httpfileserver"
  )
endif()

include(cmake/dev-mode.cmake)
