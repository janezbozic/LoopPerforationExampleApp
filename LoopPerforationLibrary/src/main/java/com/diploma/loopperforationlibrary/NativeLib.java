package com.diploma.loopperforationlibrary;

public class NativeLib {

    // Used to load the 'loopperforationlibrary' library on application startup.
    static {
        System.loadLibrary("loopperforationlibrary");
    }

    /**
     * A native method that is implemented by the 'loopperforationlibrary' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}