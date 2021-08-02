//
// Created by janez on 18. 07. 21.
//

#ifndef TESTLOOPPERF_PERFORATION_LIB_H
#define TESTLOOPPERF_PERFORATION_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

//Our main perforation function, the call to it is inserted in out new loop perforation pass
int CLANG_LOOP_PERFORATION_FUNCTION(int n);

#ifdef __cplusplus
}
#endif

#endif //TESTLOOPPERF_PERFORATION_LIB_H
