// IMyService.aidl
package com.example;

import com.example.LoopPerfFactor;

interface IMyService {
    /**
     * Demonstrates some basic types that you can use as parameters
     * and return values in AIDL.
     */
    void basicTypes(int anInt, long aLong, boolean aBoolean, float aFloat,
            double aDouble, String aString);

    //Declaration of a method for returning perforation factor
    //  AIDL will build the source
    LoopPerfFactor getPerforationFactor (int loopId);
}
