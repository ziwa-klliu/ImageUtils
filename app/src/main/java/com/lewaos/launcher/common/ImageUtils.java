package com.lewaos.launcher.common;

import android.graphics.Bitmap;

/**
 * Provider image process method
 */
public class ImageUtils {
    private static final String LIBRARY_NAME = "ImageUtils";

    static {
        System.loadLibrary(LIBRARY_NAME);
    }

    public static Bitmap gaussianBlur(Bitmap bitmap) {
        nGaussianBlur(bitmap);
        return bitmap;
    }

    private static native void nGaussianBlur(Bitmap bitmap);
}
