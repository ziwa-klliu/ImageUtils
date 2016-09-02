#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>


#define LOG_TAG "NEL_ImageUtils"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static void printPixels(AndroidBitmapInfo* info, void* pixels);

static void gaussianBlur(AndroidBitmapInfo* info, void* pixels) {
    //size_t size = info->width * info->height * info->stride;
    //void * result = malloc(size);

    uint32_t width = info->width;
    uint32_t height = info->height;
    /*int y;
    for (y = 0; y < height; ++y) {
        uint16_t* line = (uint16_t*) pixels;
        int x;
        for (x = 0; x < width; ++x) {
            line[x] = 255;
        }
        pixels = (char*) pixels + info->stride; // next line
        LOGD("GaussianBlur: Height： %d", y);
    }*/
    printPixels(info, pixels);

}

static void printPixels(AndroidBitmapInfo* info, void* pixels)
{
    uint32_t width = info->width;
    uint32_t height = info->height;
    int x, y;
    LOGD("ImagePixels: ---------------Start---------------");
    for (y = 0; y < height; y++)
    {
        uint16_t* line = (uint16_t*) pixels;
        char str[(width << 1) + 1];
        for (x = 0; x < width; x++)
        {
            str[x<<1] = line[x];
            str[(x<<1) + 1] = ' ';
        }
        str[width<<1] =  "\n";
        LOGD("ImagePixels: %s", str);
    }
    LOGD("ImagePixels: ----------------End----------------");
}


JNIEXPORT void JNICALL
Java_com_lewaos_launcher_common_ImageUtils_nGaussianBlur(JNIEnv *env, jclass type, jobject bitmap) {

    AndroidBitmapInfo info;
    void* pixels;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error = %d", ret);
        return;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGBA_8888!");
        return;
    }

    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return;
    }
    LOGI("AndroidBitmapInfo: Size:%d x %d, Stride:%d, Format:%d, Flag: %d", info.width, info.height, info.stride, info.format, info.flags);

    //Do real thing
    gaussianBlur(&info, &pixels);

    if ((ret = AndroidBitmap_unlockPixels(env, bitmap) < 0)){
        LOGE("AndroidBitmap_unlockPixels() failed ! error=%d", ret);
    }

}

