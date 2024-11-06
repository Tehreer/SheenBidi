cmake_minimum_required(VERSION 3.16)

project(sheenbidi VERSION 2.8 LANGUAGES C CXX)

option(ENABLE_INSTALL "Enable install targets" ON)

if(CMAKE_BUILD_TYPE MATCHES "Release|MinSizeRel")
  set(_testing_default OFF)
else()
  set(_testing_default ON)
endif()
option(BUILD_TESTING "Enable test targets" ${_testing_default})
include(CTest)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(_sanitizers_default ON)
else()
  set(_sanitizers_default OFF)
endif()
option(ASAN "Enable address sanitizer" ${_sanitizers_default})
option(UBSAN "Enable undefined behaviour sanitizer" ${_sanitizers_default})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clang-tidy
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

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
add_library(
  sheenbidi
  Source/SheenBidi.c
)
target_include_directories(sheenbidi PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Headers>
  $<INSTALL_INTERFACE:include/SheenBidi>
)
target_include_directories(sheenbidi PRIVATE Source)
set_target_properties(sheenbidi PROPERTIES PUBLIC_HEADER "${_public_headers}")
target_compile_definitions(sheenbidi PRIVATE SB_CONFIG_UNITY)
add_library(SheenBidi::sheenbidi ALIAS sheenbidi)
add_sanitizers(sheenbidi)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_link_libraries(sheenbidi PUBLIC "-static-libgcc -static-libstdc++")
endif()

if(BUILD_TESTING)
  add_library(
    sheenbidi_non_unity
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
    Source/SheenBidi.c
    Source/StatusStack.c
  )
  target_include_directories(sheenbidi_non_unity PUBLIC Headers)
  target_include_directories(sheenbidi_non_unity PRIVATE Source)
  add_sanitizers(sheenbidi_non_unity)

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
  target_link_libraries(parser PUBLIC sheenbidi_non_unity)
  add_sanitizers(parser)

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
  target_link_libraries(tester PRIVATE parser sheenbidi_non_unity)
  add_sanitizers(tester)

  add_test(
    NAME tests
    COMMAND tester ${CMAKE_CURRENT_SOURCE_DIR}/Tools/Unicode
  )
  set_tests_properties(tests PROPERTIES TIMEOUT 180)
endif()

if(ENABLE_INSTALL)
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
  configure_package_config_file(cmake/SheenBidi.cmake.in
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
endif()
