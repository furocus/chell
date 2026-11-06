cmake_minimum_required(VERSION 3.27)

include("${CMAKE_CURRENT_LIST_DIR}/scripts/check-environment.cmake")
if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
  message(FATAL_ERROR "Use the debug or release preset with Ninja.")
endif()

project(chell LANGUAGES C)
if(NOT CMAKE_C_COMPILER_ID STREQUAL "MSVC" OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "chell requires MSVC targeting x64.")
endif()

include(CTest)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(chell_entry "${PROJECT_SOURCE_DIR}/src/main.c")
if(EXISTS "${chell_entry}")
  add_executable(chell "${chell_entry}")
  set_target_properties(chell PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
  )
  target_compile_options(chell PRIVATE /W4 /utf-8)
  target_compile_definitions(chell PRIVATE UNICODE _UNICODE)
  target_include_directories(chell PRIVATE "${PROJECT_SOURCE_DIR}/include")
else()
  message(STATUS "src/main.c is not present; checking configuration only.")
endif()
