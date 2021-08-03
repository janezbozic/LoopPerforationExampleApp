# Example Application for loop perforation on Android

In this example the loop perforation factor is retrieved from the service with AIDL using function
call inserted in compilation step

## Bases for the tests:
[Base project for AIDL service](https://github.com/lakinduboteju/AndroidNdkBinderExamples)
[Parsec Benchmark Suite](https://parsec.cs.princeton.edu/overview.htm)
[Blur filter](https://github.com/kikoso/android-stackblur)
[Brightness Filer](https://github.com/ruckus/android-image-filter-ndk)
[Monte-Carlo](https://cameron-mcelfresh.medium.com/monte-carlo-integration-313b37157852)

## Prerequisites:
For running the perforation part of the code, which is using #pragma clang loop perforate (enable),
we have to use ndk, which uses [LLVM-Project](https://github.com/janezbozic/llvm-project)