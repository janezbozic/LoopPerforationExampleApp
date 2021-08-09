# Example Application for loop perforation on Android

In this example the loop perforation factor is retrieved from the service with AIDL using function
call inserted in compilation step

## Bases for the tests:
- [Base project for AIDL service](https://github.com/lakinduboteju/AndroidNdkBinderExamples)
- [Parsec Benchmark Suite](https://parsec.cs.princeton.edu/overview.htm)
- [Blur filter](https://github.com/kikoso/android-stackblur)
- [Brightness Filer](https://github.com/ruckus/android-image-filter-ndk)
- [Monte-Carlo](https://cameron-mcelfresh.medium.com/monte-carlo-integration-313b37157852)

## Prerequisites:
For running the perforation part of the code, which is using #pragma clang loop perforate (enable),
we have to use ndk, which uses [LLVM-Project](https://github.com/janezbozic/llvm-project)

## Video demo of picture brightness:

On the video dynamic loop perforation is demonstrated. The perforation factor is being changed dynamically, when method from the service is called with specific loopId number, which is unique for every translation of loop's source file (if we make changes to a source file, we have to change loopId, for which perforation factor changes). One is image, which is perforated, and the other is unperforated, but with lesser brightness factor (see table for the relation between perforation factor of perforated loop and brightness factor of unperforated loop)

| Perforation factor for perforated image  | Smaller brightness factor for unperforated image |
| ------------- | ------------- |
| 2  | 40%  |
| 3  | 75%  |
| 4  | 85%  |

For code switch to brightnessChanges branch

https://user-images.githubusercontent.com/33355095/128679963-01009d55-23bd-4280-b73a-cd0b735d82a0.mp4
