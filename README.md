SheenBidi
=========
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Linux CI](https://github.com/Tehreer/SheenBidi/actions/workflows/linux.yml/badge.svg)](https://github.com/Tehreer/SheenBidi/actions/workflows/linux.yml)
[![macOS CI](https://github.com/Tehreer/SheenBidi/actions/workflows/macos.yml/badge.svg)](https://github.com/Tehreer/SheenBidi/actions/workflows/macos.yml)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/k2vvegcdqsb9ld5a?svg=true)](https://ci.appveyor.com/project/mta452/sheenbidi)
[![Coverage Status](https://coveralls.io/repos/github/Tehreer/SheenBidi/badge.svg?branch=master)](https://coveralls.io/github/Tehreer/SheenBidi)

SheenBidi implements Unicode Bidirectional Algorithm available at http://www.unicode.org/reports/tr9. It is a sophisticated implementation which provides the developers an easy way to use UBA in their applications.

Here are some of the advantages of SheenBidi.

* Object based.
* Optimized to the core.
* Designed to be thread safe.
* Lightweight API for interaction.
* Supports UTF-8, UTF-16 and UTF-32 encodings.

## API
<img src="https://user-images.githubusercontent.com/2664112/39663208-716af1c4-5088-11e8-855c-ababe3e58c58.png" width="350">
The above screenshot depicts a visual representation of the API on a sample text.

### SBCodepointSequence
It works as a code point decoder by accepting a string buffer in specified encoding.

### SBAlgorithm
It provides bidirectional type of each code unit in source string. Paragraph boundaries can be queried from it as determined by rule [P1](https://www.unicode.org/reports/tr9/#P1). Individual paragraph objects can be created from it by explicitly specifying the base level or deriving it from rules [P2](https://www.unicode.org/reports/tr9/#P2)-[P3](https://www.unicode.org/reports/tr9/#P3).

### SBParagraph
It represents a single paragraph of text processed with rules [X1](https://www.unicode.org/reports/tr9/#X1)-[I2](https://www.unicode.org/reports/tr9/#I2). It provides resolved embedding levels of all the code units of a paragraph.

### SBLine
It represents a single line of text processed with rules [L1](https://www.unicode.org/reports/tr9/#L1)-[L2](https://www.unicode.org/reports/tr9/#L2). However, it provides reordered level runs instead of reordered characters.

### SBRun
It represents a sequence of characters which have the same embedding level. The direction of a run would be right-to-left, if its embedding level is odd.

### SBMirrorLocator
It provides the facility to find out the mirrored characters in a line as determined by rule [L4](https://www.unicode.org/reports/tr9/#L4).

### SBScriptLocator
Not directly related to UBA but can be useful for text shaping. It provides the facility to find out the script runs as specified in [UAX #24](https://www.unicode.org/reports/tr24/).

## Dependency
SheenBidi does not depend on any external library. It only uses standard C library headers ```stddef.h```, ```stdint.h``` and ```stdlib.h```.

## Configuration
The configuration options are available in `Headers/SBConfig.h`.

* ```SB_CONFIG_LOG``` logs every activity performed in order to apply bidirectional algorithm.
* ```SB_CONFIG_UNITY``` builds the library as a single module and lets the compiler make decisions to inline functions.

## Compiling
SheenBidi can be compiled with any C compiler. The best way for compiling is to add all the files in an IDE and hit build. The only thing to consider however is that if ```SB_CONFIG_UNITY``` is enabled then only ```Source/SheenBidi.c``` should be compiled.

### CMake

SheenBidi can also be compiled and installed with CMake:

```bash
cmake -S. -Bbuild-rel -DCMAKE_BUILD_TYPE=Release
cmake --build build-rel -j $(getconf _NPROCESSORS_ONLN)
sudo cmake --install build-rel
```

The installed package can be found in other CMake projects via
`find_package(SheenBidi)`.

SheenBidi provides a single target, `SheenBidi::sheenbidi`.
SheenBidi can also be used via `FetchContent`.

### Testing with CMake

If you're working on SheenBidi itself, you can build and run the tests with:

```bash
cmake -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(getconf _NPROCESSORS_ONLN)
ctest --test-dir build --output-on-failure -j $(getconf _NPROCESSORS_ONLN)
```

## Example
Here is a simple example written in C11.

```c
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <SheenBidi.h>

int main(int argc, const char * argv[]) {
    /* Create code point sequence for a sample bidirectional text. */
    const char *bidiText = "یہ ایک )car( ہے۔";
    SBCodepointSequence codepointSequence = { SBStringEncodingUTF8, (void *)bidiText, strlen(bidiText) };

    /* Extract the first bidirectional paragraph. */
    SBAlgorithmRef bidiAlgorithm = SBAlgorithmCreate(&codepointSequence);
    SBParagraphRef firstParagraph = SBAlgorithmCreateParagraph(bidiAlgorithm, 0, INT32_MAX, SBLevelDefaultLTR);
    SBUInteger paragraphLength = SBParagraphGetLength(firstParagraph);

    /* Create a line consisting of whole paragraph and get its runs. */
    SBLineRef paragraphLine = SBParagraphCreateLine(firstParagraph, 0, paragraphLength);
    SBUInteger runCount = SBLineGetRunCount(paragraphLine);
    const SBRun *runArray = SBLineGetRunsPtr(paragraphLine);

    /* Log the details of each run in the line. */
    for (SBUInteger i = 0; i < runCount; i++) {
        printf("Run Offset: %ld\n", (long)runArray[i].offset);
        printf("Run Length: %ld\n", (long)runArray[i].length);
        printf("Run Level: %ld\n\n", (long)runArray[i].level);
    }

    /* Create a mirror locator and load the line in it. */
    SBMirrorLocatorRef mirrorLocator = SBMirrorLocatorCreate();
    SBMirrorLocatorLoadLine(mirrorLocator, paragraphLine, (void *)bidiText);
    const SBMirrorAgent *mirrorAgent = SBMirrorLocatorGetAgent(mirrorLocator);

    /* Log the details of each mirror in the line. */
    while (SBMirrorLocatorMoveNext(mirrorLocator)) {
        printf("Mirror Index: %ld\n", (long)mirrorAgent->index);
        printf("Actual Code Point: %ld\n", (long)mirrorAgent->codepoint);
        printf("Mirrored Code Point: %ld\n\n", (long)mirrorAgent->mirror);
    }

    /* Release all objects. */
    SBMirrorLocatorRelease(mirrorLocator);
    SBLineRelease(paragraphLine);
    SBParagraphRelease(firstParagraph);
    SBAlgorithmRelease(bidiAlgorithm);

    return 0;
}
```

## License
```
Copyright (C) 2014-2025 Muhammad Tayyab Akram

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
