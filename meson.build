project('sheenbidi', 'c', version: '2.6',
  meson_version : '>= 0.55.0')

sheenbidi_includes = include_directories('Headers')

sheenbidi_headers = files([
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
])
install_headers(sheenbidi_headers, subdir: 'SheenBidi')

sheenbidi_sources = [
  'Source/SheenBidi.c',
]

sheenbidi_library = library('sheenbidi',
  sources: sheenbidi_sources,
  include_directories: sheenbidi_includes,
  c_args: ['-DSB_CONFIG_UNITY'],
  version: meson.project_version(),
  install: true)

sheenbidi_dep = declare_dependency(
  include_directories : sheenbidi_includes,
  link_with : sheenbidi_library)
