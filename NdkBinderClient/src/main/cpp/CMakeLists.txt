cmake_minimum_required(VERSION 3.10.2)

project("ndkbinderclient")

add_library (
        native-lib
        SHARED
        LoopPerf.c
        perforation/LoopInfo.cpp
        perforation/perforation_lib.cpp
        tests/adjustbrightness/adjustBrightness.cpp
        tests/montecarlo/montecarlo.cpp
        tests/blackscholes/blackScholes.cpp
        native-lib.cpp
        ${aidl_src_dir}/com/example/IMyService.cpp
)

target_include_directories (
        native-lib
        PRIVATE
        ${common_inc_dir}
        includes
)

find_library (
        log-lib
        log
)

find_library (
        binder_ndk-lib
        binder_ndk
)

target_link_libraries (
        native-lib
        ${log-lib}
        ${binder_ndk-lib}
)
