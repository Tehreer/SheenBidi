cmake_minimum_required(VERSION 3.16)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/Headers/SBVersion.h" VERSION_MACRO_LINE
  REGEX "SHEENBIDI_VERSION_STRING[ \\t]+\"([^\"]+)\""
)
string(REGEX REPLACE ".*\"([^\"]+)\".*" "\\1"
  VERSION_STRING ${VERSION_MACRO_LINE}
)
project(SheenBidi
  VERSION ${VERSION_STRING}
  LANGUAGES C CXX
)

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
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clang-tidy

if(WIN32 AND BUILD_SHARED_LIBS)
  set(BUILDING_DLL ON)
else()
  set(BUILDING_DLL OFF)
endif()

if(BUILD_TESTING AND NOT BUILDING_DLL AND NOT SB_CONFIG_UNITY)
  set(BUILD_TESTS ON)
else()
  set(BUILD_TESTS OFF)
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
  Headers/SBVersion.h
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
    $<INSTALL_INTERFACE:include>
  PRIVATE
    Source
)
set_target_properties(sheenbidi PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  PUBLIC_HEADER "${SHEENBIDI_HEADERS}"
)
add_library(SheenBidi::sheenbidi ALIAS sheenbidi)
add_sanitizers(sheenbidi)

# ------------------------------------------------------------------------------
# Parser Library (Required for generator/tester)
# ------------------------------------------------------------------------------

if(BUILD_GENERATOR OR BUILD_TESTS)
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
# Testing (Only non-unity mode)
# ------------------------------------------------------------------------------

if(BUILD_TESTS)
  file(GLOB TEST_COMMON_FILES
    Tools/Tester/Utilities/*.h
    Tools/Tester/Utilities/*.cpp
  )

  set(TEST_TARGETS
    AlgorithmTests
    BidiTypeLookupTests
    CodepointSequenceTests
    GeneralCategoryLookupTests
    MirrorLookupTests
    ScriptLocatorTests
    ScriptLookupTests
  )
  set(AlgorithmTests
    Tools/Tester/AlgorithmTester.h
    Tools/Tester/AlgorithmTester.cpp
  )
  set(BidiTypeLookupTests
    Tools/Tester/BidiTypeLookupTester.h
    Tools/Tester/BidiTypeLookupTester.cpp
  )
  set(CodepointSequenceTests
    Tools/Tester/CodepointSequenceTester.h
    Tools/Tester/CodepointSequenceTester.cpp
  )
  set(GeneralCategoryLookupTests
    Tools/Tester/GeneralCategoryLookupTester.h
    Tools/Tester/GeneralCategoryLookupTester.cpp
  )
  set(MirrorLookupTests
    Tools/Tester/MirrorLookupTester.h
    Tools/Tester/MirrorLookupTester.cpp
  )
  set(ScriptLocatorTests
    Tools/Tester/ScriptLocatorTester.h
    Tools/Tester/ScriptLocatorTester.cpp
  )
  set(ScriptLookupTests
    Tools/Tester/ScriptLookupTester.h
    Tools/Tester/ScriptLookupTester.cpp
  )

  foreach(TEST_NAME IN LISTS TEST_TARGETS)
    add_executable(${TEST_NAME}
      ${${TEST_NAME}}
      ${TEST_COMMON_FILES}
    )
    target_include_directories(${TEST_NAME}
      PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Headers
        ${CMAKE_CURRENT_SOURCE_DIR}/Tools
    )
    target_compile_definitions(${TEST_NAME} PRIVATE STANDALONE_TESTING)
    target_link_libraries(${TEST_NAME} PRIVATE parser sheenbidi)
    add_sanitizers(${TEST_NAME})

    add_test(
      NAME ${TEST_NAME}
      COMMAND ${TEST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/Tools/Unicode
    )
    set_tests_properties(${TEST_NAME} PROPERTIES
      TIMEOUT 60
      FAIL_REGULAR_EXPRESSION "[1-9][0-9]* error"
    )
  endforeach()

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

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(SheenBidi.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiConfig.cmake"
  INSTALL_DESTINATION ${ConfigPackageLocation}
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
