cmake_minimum_required(VERSION 3.16)

project(SheenBidi VERSION 2.8 LANGUAGES C CXX)

include(CTest)

option(ENABLE_CODECOVERAGE "Instrument code for code coverage (only enabled with BUILD_TESTING)" OFF)

set(ASAN $<CONFIG:Debug> CACHE STRING "Enable address sanitizer" )
set(UBSAN $<CONFIG:Debug> CACHE STRING "Enable undefined behaviour sanitizer")

# C++ is only used for testing tools.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clang-tidy

function(add_sanitizers TARGET)
  if(NOT MSVC)
    target_compile_options(${TARGET} PUBLIC $<BUILD_INTERFACE:$<$<BOOL:${ASAN}>:-fsanitize=address;-fsanitize-recover=address>>)
    target_link_libraries(${TARGET} PUBLIC $<BUILD_INTERFACE:$<$<BOOL:${ASAN}>:-fsanitize=address;-fsanitize-recover=address>>)
    target_compile_options(${TARGET} PUBLIC $<BUILD_INTERFACE:$<$<BOOL:${UBSAN}>:-fsanitize=undefined>>)
    target_link_libraries(${TARGET} PUBLIC $<BUILD_INTERFACE:$<$<BOOL:${UBSAN}>:-fsanitize=undefined>>)
  endif()
endfunction()

# https://learn.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=msvc-170
# https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-170
add_compile_options("$<$<COMPILE_LANG_AND_ID:C,MSVC>:/utf-8>")
add_compile_options("$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8;/Zc:__cplusplus>")

set(_public_headers
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

add_library(sheenbidi_unity OBJECT
  Source/SheenBidi.c
)
target_compile_definitions(sheenbidi_unity PUBLIC SB_CONFIG_UNITY)

add_library(sheenbidi_non_unity OBJECT
  Source/BidiChain.c
  Source/BidiTypeLookup.c
  Source/BracketQueue.c
  Source/GeneralCategoryLookup.c
  Source/IsolatingRun.c
  Source/LevelRun.c
  Source/PairingLookup.c
  Source/RunQueue.c
  Source/SBAlgorithm.c
  Source/SBBase.c
  Source/SBCodepointSequence.c
  Source/SBLine.c
  Source/SBLog.c
  Source/SBMirrorLocator.c
  Source/SBParagraph.c
  Source/SBScriptLocator.c
  Source/ScriptLookup.c
  Source/ScriptStack.c
  Source/StatusStack.c
)
foreach(_target sheenbidi_unity sheenbidi_non_unity)
  target_include_directories(${_target} PUBLIC Headers)
  target_include_directories(${_target} PRIVATE Source)
  add_sanitizers(${_target})
endforeach()

add_library(sheenbidi)
target_include_directories(sheenbidi PUBLIC
  $<INSTALL_INTERFACE:include/SheenBidi>
)
set(_is_unity $<NOT:$<CONFIG:Debug>>)
target_link_libraries(sheenbidi PUBLIC
  $<BUILD_INTERFACE:$<IF:${_is_unity},sheenbidi_unity,sheenbidi_non_unity>>
)
set_target_properties(sheenbidi PROPERTIES PUBLIC_HEADER "${_public_headers}")
add_library(SheenBidi::sheenbidi ALIAS sheenbidi)
add_sanitizers(sheenbidi)

add_library(
  parser
  Tools/Parser/BidiBrackets.cpp
  Tools/Parser/BidiCharacterTest.cpp
  Tools/Parser/BidiMirroring.cpp
  Tools/Parser/BidiTest.cpp
  Tools/Parser/DerivedBidiClass.cpp
  Tools/Parser/PropertyValueAliases.cpp
  Tools/Parser/Scripts.cpp
  Tools/Parser/UnicodeData.cpp
  Tools/Parser/UnicodeVersion.cpp
)
target_include_directories(parser PUBLIC Tools)
target_link_libraries(parser PUBLIC sheenbidi)
add_sanitizers(parser)

add_executable(
  generator
  Tools/Generator/main.cpp
  Tools/Generator/BidiTypeLookupGenerator.cpp
  Tools/Generator/GeneralCategoryLookupGenerator.cpp
  Tools/Generator/PairingLookupGenerator.cpp
  Tools/Generator/ScriptLookupGenerator.cpp
  Tools/Generator/Utilities/ArrayBuilder.cpp
  Tools/Generator/Utilities/BidiClassDetector.cpp
  Tools/Generator/Utilities/Converter.cpp
  Tools/Generator/Utilities/FileBuilder.cpp
  Tools/Generator/Utilities/GeneralCategoryDetector.cpp
  Tools/Generator/Utilities/Math.cpp
  Tools/Generator/Utilities/ScriptDetector.cpp
  Tools/Generator/Utilities/StreamBuilder.cpp
  Tools/Generator/Utilities/TextBuilder.cpp
)
target_include_directories(generator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(generator PUBLIC parser sheenbidi)
add_sanitizers(generator)

if(BUILD_TESTING)
  add_executable(
    tester
    Tools/Tester/main.cpp
    Tools/Tester/Configuration.cpp
    Tools/Tester/MirrorLookupTester.cpp
    Tools/Tester/ScriptLookupTester.cpp
    Tools/Tester/AlgorithmTester.cpp
    Tools/Tester/ScriptLocatorTester.cpp
    Tools/Tester/BidiTypeLookupTester.cpp
    Tools/Tester/CodepointSequenceTester.cpp
    Tools/Tester/GeneralCategoryLookupTester.cpp
    Tools/Tester/BracketLookupTester.cpp
    Tools/Tester/Utilities/Convert.cpp
    Tools/Tester/Utilities/Unicode.cpp
  )
  target_include_directories(tester PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" Tools)
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
  if(ENABLE_CODECOVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      message(WARNING "Codecoverage not supported with MSVC")
    else()
      target_compile_options(sheenbidi_non_unity PUBLIC --coverage)
      target_link_options(sheenbidi_non_unity PUBLIC --coverage)
    endif()
  endif()
endif()

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
  INSTALL_DESTINATION ${ConfigPackageLocation})
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiConfigVersion.cmake"
  VERSION ${SheenBidi_VERSION}
  COMPATIBILITY AnyNewerVersion
)

export(EXPORT SheenBidiTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiTargets.cmake"
  NAMESPACE SheenBidi::
)

install(EXPORT SheenBidiTargets
  FILE
    SheenBidiTargets.cmake
  NAMESPACE
    SheenBidi::
  DESTINATION
    ${ConfigPackageLocation}
)
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/SheenBidi/SheenBidiConfigVersion.cmake"
  DESTINATION
    ${ConfigPackageLocation}
)
