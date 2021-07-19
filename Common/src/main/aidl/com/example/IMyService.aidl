// IMyService.aidl
package com.example;

interface IMyService {
    /**
     * Demonstrates some basic types that you can use as parameters
     * and return values in AIDL.
     */
    void basicTypes(int anInt, long aLong, boolean aBoolean, float aFloat,
            double aDouble, String aString);

    int getPerforationFactor (int loopId);
}
