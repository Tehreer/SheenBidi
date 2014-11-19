SheenBidi
=========

SheenBidi implements Unicode Bidirectional Algorithm available at http://www.unicode.org/reports/tr9. It is the world's most sophisticated implementaion which provides the developers an easy way to use UBA in their applications.

Here are some of the advantages of SheenBidi.
- Object based.
- Optimized to the core.
- Designed to be thread safe.
- Lightweight API for interaction.
- Clear differentiation of public and private API.

## Dependency
SheenBidi does not depend on any external library. It only uses standard C library headers ```stddef.h```, ```stdint.h``` and ```stdlib.h```.

## Configuration
The configuration options are available in `Headers/SBConifg.h`.
- ```SB_CONFIG_LOG``` logs every activity performed in order to apply bidirectional algorithm.
- ```SB_CONFIG_UNITY``` builds the library as a single module and lets the compiler take decisions to inline functions.

## Compiling
SheenBidi can be compiled with any C compiler. The best way for compiling is to add all the files in an IDE and hit build. The only thing to consider however is that if ```SB_CONFIG_UNITY``` is enabled then only ```Source/SheenBidi.c``` should be compiled.

## Example
Here is a simple example written in C++.
```
#include <iostream>

extern "C" {
#include <SheenBidi.h>
}

using namespace std;

int main(int argc, const char * argv[]) {
  SBUnichar text[15] = { 0x06CC,0x06C1,0x0627,0x06CC,0x06A9,' ',')','c','a','r','(',' ',0x06C1,0x06D2,0x06D4 };
  SBUInteger length = 15;

  SBParagraphRef paragraph = SBParagraphCreateWithUnicodeCharacters(text, length, SBBaseDirectionAutoLTR, SBParagraphOptionsNone);
  SBLineRef line = SBLineCreateWithParagraph(paragraph, 0, length, SBLineOptionsNone);

  SBRunAdapterRef adapter = SBRunAdapterCreate();
  SBRunAdapterLoadLine(adapter, line);
  const SBRunAgentRef run = SBRunAdapterGetAgent(adapter);
  while (SBRunAdapterMoveNext(adapter)) {
    cout << "Run Level: " << (int)run->level << endl;
    cout << "Run Offset: " << run->offset << endl;
    cout << "Run Length: " << run->length << endl << endl;
  }
  SBRunAdapterRelease(adapter);

  SBMirrorLocatorRef locator = SBMirrorLocatorCreate();
  SBMirrorLocatorLoadLine(locator, line, text);
  const SBMirrorAgentRef mirror = SBMirrorLocatorGetAgent(locator);
  while (SBMirrorLocatorMoveNext(locator)) {
    cout << "Mirror Location: " << mirror->index << endl;
    cout << "Mirror Unicode: " << mirror->mirror << endl << endl;
  }
  SBMirrorLocatorRelease(locator);

  SBLineRelease(line);
  SBParagraphRelease(paragraph);
    
  return 0;
}
```
