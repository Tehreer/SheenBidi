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
Here is a simple example written in C++.
```
#include <stdio.h>
#include <SheenBidi.h>

int main(int argc, const char * argv[]) {
    SBCodepoint text[] = { 0x06CC,0x06C1,0x0627,0x06CC,0x06A9,' ',')','c','a','r','(',' ',0x06C1,0x06D2,0x06D4 };
    SBUInteger length = sizeof(text) / sizeof(SBCodepoint);
    SBRunAdapterRef runAdapter = SBRunAdapterCreate();
    SBRunAgentRef runAgent = SBRunAdapterGetAgent(runAdapter);
    SBMirrorLocatorRef mirrorLocator = SBMirrorLocatorCreate();
    SBMirrorAgentRef mirrorAgent = SBMirrorLocatorGetAgent(mirrorLocator);
    SBParagraphRef bidiParagraph;
    SBLineRef bidiLine;

    bidiParagraph = SBParagraphCreateWithCodepoints(text, length, SBBaseDirectionAutoLTR, SBParagraphOptionsDefault);
    bidiLine = SBLineCreateWithParagraph(bidiParagraph, 0, length, SBLineOptionsDefault);

    SBRunAdapterLoadLine(runAdapter, bidiLine);
    while (SBRunAdapterMoveNext(runAdapter)) {
        printf("Run Level: %d\n", (int)runAgent->level);
        printf("Run Offset: %ld\n", (long)runAgent->offset);
        printf("Run Length: %ld\n\n", (long)runAgent->length);
    }

    SBMirrorLocatorLoadLine(mirrorLocator, bidiLine, text);
    while (SBMirrorLocatorMoveNext(mirrorLocator)) {
        printf("Mirror Location: %ld\n", (long)mirrorAgent->index);
        printf("Mirror Codepoint: %ld\n\n", (long)mirrorAgent->mirror);
    }

    SBLineRelease(bidiLine);
    SBParagraphRelease(bidiParagraph);
    SBRunAdapterRelease(runAdapter);
    SBMirrorLocatorRelease(mirrorLocator);

    return 0;
}
```