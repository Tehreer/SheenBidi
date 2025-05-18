cmake_minimum_required(VERSION 3.16)

project(SheenBidi VERSION 2.8 LANGUAGES C CXX)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------

option(SB_CONFIG_UNITY "Build with a single unity source file" ON)
option(BUILD_GENERATOR "Build the Unicode data generator tool" OFF)
option(ENABLE_COVERAGE "Enable code coverage instrumentation (only enabled with BUILD_TESTING)" OFF)
option(ENABLE_ASAN "Enable address sanitizer" OFF)
option(ENABLE_UBSAN "Enable undefined behavior sanitizer" OFF)

include(CTest)

# C++ is only needed for tools
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clang-tidy

if(WIN32 AND BUILD_SHARED_LIBS)
  set(BUILDING_DLL ON)
else()
  set(BUILDING_DLL OFF)
endif()

if(BUILD_TESTING AND NOT BUILDING_DLL AND NOT SB_CONFIG_UNITY)
  set(BUILD_TESTER ON)
else()
  set(BUILD_TESTER OFF)
endif()

# ------------------------------------------------------------------------------
# MSVC Configuration
# ------------------------------------------------------------------------------

# https://learn.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=msvc-170
# https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-170
add_compile_options("$<$<COMPILE_LANG_AND_ID:C,MSVC>:/utf-8>")
add_compile_options("$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8;/Zc:__cplusplus>")

# ------------------------------------------------------------------------------
# Sanitizer Support
# ------------------------------------------------------------------------------

function(add_sanitizers TARGET)
  if(NOT MSVC)
    if(ENABLE_ASAN)
      target_compile_options(${TARGET} PUBLIC -fsanitize=address -fsanitize-recover=address)
      target_link_libraries(${TARGET} PUBLIC -fsanitize=address -fsanitize-recover=address)
    endif()
    if(ENABLE_UBSAN)
      target_compile_options(${TARGET} PUBLIC -fsanitize=undefined)
      target_link_libraries(${TARGET} PUBLIC -fsanitize=undefined)
    endif()
  endif()
endfunction()

# ------------------------------------------------------------------------------
# SheenBidi Library
# ------------------------------------------------------------------------------

set(SHEENBIDI_HEADERS
  Headers/SBAlgorithm.h
  Headers/SBBase.h
  Headers/SBBidiType.h
  Headers/SBCodepoint.h
  Headers/SBCodepointSequence.h
  Headers/SBGeneralCategory.h
  Headers/SBLine.h
  Headers/SBMirrorLocator.h
  Headers/SBParagraph.h
  Headers/SBRun.h
  Headers/SBScript.h
  Headers/SBScriptLocator.h
  Headers/SheenBidi.h
)
file(GLOB INTERNAL_HEADERS "Source/*.h")

if(SB_CONFIG_UNITY)
  add_definitions(-DSB_CONFIG_UNITY)
  set(SHEENBIDI_SOURCES Source/SheenBidi.c)
else()
  file(GLOB SHEENBIDI_SOURCES Source/*.c)
endif()

add_library(sheenbidi
  ${SHEENBIDI_HEADERS}
  ${INTERNAL_HEADERS}
  ${SHEENBIDI_SOURCES}
)
if(BUILDING_DLL)
  target_compile_definitions(sheenbidi PRIVATE SB_CONFIG_DLL_EXPORT)
  target_compile_definitions(sheenbidi INTERFACE SB_CONFIG_DLL_IMPORT)
endif()
target_include_directories(sheenbidi
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Headers>
    $<INSTALL_INTERFACE:include/SheenBidi>
  PRIVATE
    Source
)
set_target_properties(sheenbidi PROPERTIES
  VERSION "${SheenBidi_VERSION}"
  SOVERSION "${SheenBidi_VERSION_MAJOR}"
  PUBLIC_HEADER "${SHEENBIDI_HEADERS}"
)
add_library(SheenBidi::sheenbidi ALIAS sheenbidi)
add_sanitizers(sheenbidi)

# ------------------------------------------------------------------------------
# Parser Library (Required for generator/tester)
# ------------------------------------------------------------------------------

if(BUILD_GENERATOR OR BUILD_TESTER)
  file(GLOB_RECURSE PARSER_FILES
    Tools/Parser/*.h
    Tools/Parser/*.cpp
  )
  add_library(parser STATIC ${PARSER_FILES})
  add_sanitizers(parser)
endif()

# ------------------------------------------------------------------------------
# Generator Tool (Optional)
# ------------------------------------------------------------------------------

if(BUILD_GENERATOR)
  file(GLOB_RECURSE GENERATOR_FILES
    Tools/Generator/*.h
    Tools/Generator/*.cpp
  )
  add_executable(generator ${GENERATOR_FILES})
  target_include_directories(generator
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/Headers
      ${CMAKE_CURRENT_SOURCE_DIR}/Tools
  )
  target_link_libraries(generator PRIVATE parser)
  add_sanitizers(generator)
endif()

# ------------------------------------------------------------------------------
# Tester Tool (Only non-unity mode)
# ------------------------------------------------------------------------------

if(BUILD_TESTER)
  file(GLOB_RECURSE TESTER_FILES
    Tools/Tester/*.h
    Tools/Tester/*.cpp
  )
  add_executable(tester ${TESTER_FILES})
  target_include_directories(tester
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/Headers
      ${CMAKE_CURRENT_SOURCE_DIR}/Tools
  )
  target_link_libraries(tester PRIVATE parser sheenbidi)
  add_sanitizers(tester)

  add_test(
    NAME tests
    COMMAND tester ${CMAKE_CURRENT_SOURCE_DIR}/Tools/Unicode
  )
  set_tests_properties(tests PROPERTIES
    TIMEOUT 180
    FAIL_REGULAR_EXPRESSION "[1-9][0-9]* error"
  )

  if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      message(WARNING "Code coverage is not supported with MSVC")
    else()
      target_compile_options(sheenbidi PUBLIC --coverage)
      target_link_options(sheenbidi PUBLIC --coverage)
    endif()
  endif()
endif()

# ------------------------------------------------------------------------------
# Install Configuration
# ------------------------------------------------------------------------------

install(
  TARGETS sheenbidi
  EXPORT SheenBidiTargets
  PUBLIC_HEADER
    CONFIGURATIONS Release
    DESTINATION include/SheenBidi
)

if(NOT DEFINED LIB_INSTALL_DIR)
  set(LIB_INSTALL_DIR lib)
endif()
set(ConfigPackageLocation "${LIB_INSTALL_DIR}/cmake/SheenBidi")

include(CMakePackageConfigHelpers)
configure_package_config_file(SheenBidi.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiConfig.cmake"
  INSTALL_DESTINATION ${ConfigPackageLocation}
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiConfigVersion.cmake"
  VERSION ${SheenBidi_VERSION}
  COMPATIBILITY SameMajorVersion
)

export(EXPORT SheenBidiTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiTargets.cmake"
  NAMESPACE SheenBidi::
)

install(EXPORT SheenBidiTargets
  FILE SheenBidiTargets.cmake
  NAMESPACE SheenBidi::
  DESTINATION ${ConfigPackageLocation}
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiConfigVersion.cmake"
  DESTINATION
    ${ConfigPackageLocation}
)
