project('SheenBidi', ['c', 'cpp'],
  version: '2.8',
  meson_version: '>= 0.55.0',
  default_options: [
    'cpp_std=c++20'
  ]
)

unity_mode = get_option('unity_mode').enabled()
build_generator = get_option('generator').enabled()

is_windows_host = host_machine.system() == 'windows'
static_mode = get_option('default_library') == 'static'
dll_mode = is_windows_host and not is_static_mode
build_tester = not dll_mode and not unity_mode

project_version = meson.project_version()
soversion = project_version.split('.')[0]

# ------------------------------------------------------------------------------
# SheenBidi Library
# ------------------------------------------------------------------------------

sheenbidi_headers = files(
  'Headers/SBAlgorithm.h',
  'Headers/SBBase.h',
  'Headers/SBBidiType.h',
  'Headers/SBCodepoint.h',
  'Headers/SBCodepointSequence.h',
  'Headers/SBGeneralCategory.h',
  'Headers/SBLine.h',
  'Headers/SBMirrorLocator.h',
  'Headers/SBParagraph.h',
  'Headers/SBRun.h',
  'Headers/SBScript.h',
  'Headers/SBScriptLocator.h',
  'Headers/SheenBidi.h',
)
internal_headers = files(
  'Source/BidiChain.h',
  'Source/BidiTypeLookup.h',
  'Source/BracketQueue.h',
  'Source/BracketType.h',
  'Source/GeneralCategoryLookup.h',
  'Source/IsolatingRun.h',
  'Source/LevelRun.h',
  'Source/PairingLookup.h',
  'Source/RunExtrema.h',
  'Source/RunKind.h',
  'Source/RunQueue.h',
  'Source/SBAlgorithm.h',
  'Source/SBAssert.h',
  'Source/SBBase.h',
  'Source/SBCodepoint.h',
  'Source/SBCodepointSequence.h',
  'Source/SBLine.h',
  'Source/SBLog.h',
  'Source/SBMirrorLocator.h',
  'Source/SBParagraph.h',
  'Source/SBScriptLocator.h',
  'Source/ScriptLookup.h',
  'Source/ScriptStack.h',
  'Source/StatusStack.h'
)

if unity_mode
  add_project_arguments('-DSB_CONFIG_UNITY', language: ['c', 'cpp'])
  sheenbidi_sources = files('Source/SheenBidi.c')
else
  sheenbidi_sources = files(
    'Source/BidiChain.c',
    'Source/BidiTypeLookup.c',
    'Source/BracketQueue.c',
    'Source/GeneralCategoryLookup.c',
    'Source/IsolatingRun.c',
    'Source/LevelRun.c',
    'Source/PairingLookup.c',
    'Source/RunQueue.c',
    'Source/SBAlgorithm.c',
    'Source/SBBase.c',
    'Source/SBCodepoint.c',
    'Source/SBCodepointSequence.c',
    'Source/SBLine.c',
    'Source/SBLog.c',
    'Source/SBMirrorLocator.c',
    'Source/SBParagraph.c',
    'Source/SBScriptLocator.c',
    'Source/ScriptLookup.c',
    'Source/ScriptStack.c',
    'Source/StatusStack.c'
  )
endif

sheenbidi = library(
  'sheenbidi',
  sources: [sheenbidi_headers, internal_headers, sheenbidi_sources],
  c_args: dll_mode ? ['-DSB_CONFIG_DLL_EXPORT'] : [],
  include_directories: ['Headers', 'Source'],
  version: project_version,
  soversion: soversion,
  install: true
)

sheenbidi_dep = declare_dependency(
  include_directories: 'Headers',
  link_with: sheenbidi,
  compile_args: dll_mode ? ['-DSB_CONFIG_DLL_IMPORT'] : []
)

# ------------------------------------------------------------------------------
# Parser Library (Required for generator/tester)
# ------------------------------------------------------------------------------

if build_generator or build_tester
  parser_files = files(
    'Tools/Parser/BidiBrackets.cpp',
    'Tools/Parser/BidiBrackets.h',
    'Tools/Parser/BidiCharacterTest.cpp',
    'Tools/Parser/BidiCharacterTest.h',
    'Tools/Parser/BidiMirroring.cpp',
    'Tools/Parser/BidiMirroring.h',
    'Tools/Parser/BidiTest.cpp',
    'Tools/Parser/BidiTest.h',
    'Tools/Parser/DataFile.cpp',
    'Tools/Parser/DataFile.h',
    'Tools/Parser/DerivedBidiClass.cpp',
    'Tools/Parser/DerivedBidiClass.h',
    'Tools/Parser/DerivedCoreProperties.cpp',
    'Tools/Parser/DerivedCoreProperties.h',
    'Tools/Parser/DerivedGeneralCategory.cpp',
    'Tools/Parser/DerivedGeneralCategory.h',
    'Tools/Parser/PropertyValueAliases.cpp',
    'Tools/Parser/PropertyValueAliases.h',
    'Tools/Parser/PropList.cpp',
    'Tools/Parser/PropList.h',
    'Tools/Parser/Scripts.cpp',
    'Tools/Parser/Scripts.h',
    'Tools/Parser/UnicodeData.cpp',
    'Tools/Parser/UnicodeData.h',
    'Tools/Parser/UnicodeVersion.cpp',
    'Tools/Parser/UnicodeVersion.h'
  )
  
  parser = static_library(
    'parser',
    sources: parser_files,
    include_directories: ['Tools'],
  )
  
  parser_dep = declare_dependency(
    include_directories: 'Tools',
    link_with: parser,
  )
endif

# ------------------------------------------------------------------------------
# Generator Tool (Optional)
# ------------------------------------------------------------------------------

if build_generator
  generator_files = files(
    'Tools/Generator/Utilities/ArrayBuilder.cpp',
    'Tools/Generator/Utilities/ArrayBuilder.h',
    'Tools/Generator/Utilities/Converter.cpp',
    'Tools/Generator/Utilities/Converter.h',
    'Tools/Generator/Utilities/FileBuilder.cpp',
    'Tools/Generator/Utilities/FileBuilder.h',
    'Tools/Generator/Utilities/Math.cpp',
    'Tools/Generator/Utilities/Math.h',
    'Tools/Generator/Utilities/StreamBuilder.cpp',
    'Tools/Generator/Utilities/StreamBuilder.h',
    'Tools/Generator/Utilities/TextBuilder.cpp',
    'Tools/Generator/Utilities/TextBuilder.h',
    'Tools/Generator/BidiTypeLookupGenerator.cpp',
    'Tools/Generator/BidiTypeLookupGenerator.h',
    'Tools/Generator/GeneralCategoryLookupGenerator.cpp',
    'Tools/Generator/GeneralCategoryLookupGenerator.h',
    'Tools/Generator/main.cpp',
    'Tools/Generator/PairingLookupGenerator.cpp',
    'Tools/Generator/PairingLookupGenerator.h',
    'Tools/Generator/ScriptLookupGenerator.cpp',
    'Tools/Generator/ScriptLookupGenerator.h'
  )
  
  executable(
    'generator',
    sources: generator_files,
    include_directories: ['Headers', 'Tools'],
    dependencies: [parser_dep],
    install: false
  )
endif

# ------------------------------------------------------------------------------
# Tester Tool (Only non-unity mode)
# ------------------------------------------------------------------------------

if build_tester
  tester_files = files(
    'Tools/Tester/Utilities/Convert.cpp',
    'Tools/Tester/Utilities/Convert.h',
    'Tools/Tester/Utilities/Unicode.h',
    'Tools/Tester/AlgorithmTester.cpp',
    'Tools/Tester/AlgorithmTester.h',
    'Tools/Tester/BidiTypeLookupTester.cpp',
    'Tools/Tester/BidiTypeLookupTester.h',
    'Tools/Tester/BracketLookupTester.cpp',
    'Tools/Tester/BracketLookupTester.h',
    'Tools/Tester/CodepointSequenceTester.cpp',
    'Tools/Tester/CodepointSequenceTester.h',
    'Tools/Tester/Configuration.cpp',
    'Tools/Tester/Configuration.h',
    'Tools/Tester/GeneralCategoryLookupTester.cpp',
    'Tools/Tester/GeneralCategoryLookupTester.h',
    'Tools/Tester/main.cpp',
    'Tools/Tester/MirrorLookupTester.cpp',
    'Tools/Tester/MirrorLookupTester.h',
    'Tools/Tester/ScriptLocatorTester.cpp',
    'Tools/Tester/ScriptLocatorTester.h',
    'Tools/Tester/ScriptLookupTester.cpp',
    'Tools/Tester/ScriptLookupTester.h'
  )

  tester = executable(
    'tester',
    sources: tester_files,
    include_directories: ['Headers', 'Tools'],
    dependencies: [parser_dep, sheenbidi_dep],
    install: false
  )
  
  unicode_data_dir = meson.current_source_dir() / 'Tools/Unicode'
  
  test(
    'tests',
    tester,
    args: [unicode_data_dir],
    timeout: 180
  )
endif

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------

install_headers(
  sheenbidi_headers,
  subdir: 'SheenBidi',
)
