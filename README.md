SheenBidi
=========

SheenBidi implements Unicode Bidirectional Algorithm available at http://www.unicode.org/reports/tr9. It is a sophisticated implementaion which provides the developers an easy way to use UBA in their applications.

Here are some of the advantages of SheenBidi.

* Object based.
* Optimized to the core.
* Designed to be thread safe.
* Lightweight API for interaction.
* Clear differentiation of public and private API.

## Dependency
SheenBidi does not depend on any external library. It only uses standard C library headers ```stddef.h```, ```stdint.h``` and ```stdlib.h```.

## Configuration
The configuration options are available in `Headers/SBConifg.h`.

* ```SB_CONFIG_LOG``` logs every activity performed in order to apply bidirectional algorithm.
* ```SB_CONFIG_UNITY``` builds the library as a single module and lets the compiler make decisions to inline functions.

## Compiling
SheenBidi can be compiled with any C compiler. The best way for compiling is to add all the files in an IDE and hit build. The only thing to consider however is that if ```SB_CONFIG_UNITY``` is enabled then only ```Source/SheenBidi.c``` should be compiled.

## Example
Here is a simple example written in C11.
```
#include <stdio.h>
#include <string.h>

#include <SheenBidi.h>

int main(int argc, const char * argv[]) {
    const char *bidiText = u8"یہ ایک )car( ہے۔";

    SBCodepointSequence sequence = { SBStringEncodingUTF8, (void *)bidiText, strlen(bidiText) };
    SBAlgorithmRef algorithm = SBAlgorithmCreate(&sequence);
    SBParagraphRef paragraph = SBAlgorithmCreateParagraph(algorithm, 0, sequence.stringLength, SBLevelDefaultLTR);
    SBLineRef line = SBParagraphCreateLine(paragraph, 0, sequence.stringLength);

    SBUInteger runCount = SBLineGetRunCount(line);
    const SBRun *runArray = SBLineGetRunsPtr(line);
    for (SBUInteger i = 0; i < runCount; i++) {
        printf("Run Level: %ld\n", (long)runArray[i].level);
        printf("Run Offset: %ld\n", (long)runArray[i].offset);
        printf("Run Length: %ld\n\n", (long)runArray[i].length);
    }

    SBMirrorLocatorRef locator = SBMirrorLocatorCreate();
    SBMirrorLocatorLoadLine(locator, line, sequence.stringBuffer);
    const SBMirrorAgentRef agent = SBMirrorLocatorGetAgent(locator);
    while (SBMirrorLocatorMoveNext(locator)) {
        printf("Mirror Location: %ld\n", (long)agent->index);
        printf("Mirror Code Point: %ld\n\n", (long)agent->mirror);
    }
    SBMirrorLocatorRelease(locator);

    SBLineRelease(line);
    SBParagraphRelease(paragraph);
    SBAlgorithmRelease(algorithm);
    
    return 0;
}
```
