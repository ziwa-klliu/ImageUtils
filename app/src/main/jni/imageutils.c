#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <math.h>
#include <assert.h>
#include <malloc.h>

#define LOG_TAG "NEL_ImageUtils"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


typedef struct {
    double_t alpha;
    double_t red;
    double_t green;
    double_t blue;
} argb;

static void gaussianBlur(AndroidBitmapInfo* info, void* pixels, uint16_t radius);

static double_t kernel(double_t distance, double_t sigma);

static void get_weights(uint16_t radius, double_t* weights);

static void set_pixels(AndroidBitmapInfo* info, void * pixels, uint32_t x, uint32_t y, int32_t pixel);
static int32_t get_pixels(AndroidBitmapInfo* info, void * pixels, uint32_t x, uint32_t y);

static argb* getColorValue(int32_t color, argb * value)
{
    value->alpha = color >> 24 & 0xFF;
    value->red = color >> 16 & 0xFF;
    value->green = color >> 8 & 0xFF;
    value->blue = color >> 0 & 0xFF;
    return value;
}

static int32_t toColor(argb* color)
{
    return (int32_t)color->alpha << 24 | (int32_t )color->red << 16 | (int32_t )color->green << 8 | (int32_t )color->blue << 0;
}


static void set_pixels(AndroidBitmapInfo* info, void * pixels, uint32_t x, uint32_t y, int32_t pixel)
{
    assert(x < info->width && x >= 0);
    assert(y < info->height && y >= 0);
    int32_t * pixels_p = (int32_t *) pixels;
    pixels_p[y * info->height + x] = pixel;
    pixels_p = NULL;
}

static int32_t get_pixels(AndroidBitmapInfo* info, void * pixels, uint32_t x, uint32_t y)
{
    assert(x < info->width && x >= 0);
    assert(y < info->height && y >= 0);
    int32_t * pixels_p = (int32_t *) pixels;
    int32_t p = pixels_p[y * info->height + x];
    pixels_p = NULL;
    return p;
}

static double_t kernel(double_t distance, double_t sigma)
{
    double_t sigma2 = sigma * sigma;
    double_t result = exp(-0.5 * distance * distance / sigma2) / sqrt(2.0 * M_PI * sigma2);
    return result;
}
#define WEIGHT_FACTOR 1.0

static void get_weights(uint16_t radius, double_t * weights)
{
    assert(radius > 0);
    if (radius == 1)
    {
        weights[0] = 1;
        return;
    }
    double_t sigma = radius / 6.0f;
    double_t  sum_weights = 0.0;
    uint16_t i;
    for (i = 0; i < radius; ++i) {
        weights[i] = kernel(i, sigma);
        sum_weights += weights[i];
    }
    sum_weights = 2 * sum_weights - weights[0];
    sum_weights /= WEIGHT_FACTOR;
    for (i = 0; i < radius; ++i) {
        weights[i] /= sum_weights;
    }
}

static void printWeight(uint16_t len, double_t* weights)
{
    uint16_t i;
    for (i = 0; i < len; ++i)
    {
        LOGD("Weight[%d] = %f", i, weights[i]);
    }
}

static argb* argb_mult(argb* value, double_t mul)
{
    value->alpha *= mul;
    value->red *= mul;
    value->green *= mul;
    value->blue *= mul;
    return value;
}

static argb* argb_add(argb* a, argb* b)
{
    a->alpha += b->alpha;
    a->red += b->red;
    a->green += b->green;
    a->blue += b->blue;
    return a;
}

static void print_argb(argb* value)
{
    LOGD("Print ARGB: %d %d %d %d\n", value->alpha, value->red, value->red, value->blue);
}



static void gaussianBlur(AndroidBitmapInfo* info, void* pixels, uint16_t radius) {
    double_t weights[radius];
    get_weights(radius, weights);

    size_t size = info->height * info->stride;
    uint32_t width = info->width;
    uint32_t height = info->height;
    uint32_t x, y, i;

    void* temp_pixels = malloc(size);

    argb* result = (argb*) malloc(sizeof(argb));
    argb* temp = (argb*) malloc(sizeof(argb));
    LOGD("Start -----------------------------  Ver : %d", sizeof(argb));
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
        {
            result = getColorValue(get_pixels(info, pixels, x, y), result);
            result =  argb_mult(result,weights[0]);
            for (i = 1; i < radius; i++)
            {
                int32_t up = y - i;
                if (up >= 0)
                {
                    temp = getColorValue(get_pixels(info, pixels, x, up), temp);
                    temp = argb_mult(temp, weights[i]);
                    result = argb_add(result, temp);
                }

                int32_t bottom = y + i;
                if (bottom < height)
                {
                    temp = getColorValue(get_pixels(info, pixels, x, bottom), temp);
                    temp = argb_mult(temp, weights[i]);
                    result = argb_add(result, temp);
                }
            }
            set_pixels(info, temp_pixels, x, y, toColor(result));
        }
    }
    LOGD("End -----------------------------  Ver ");

    LOGD("Start -----------------------------  Hor ");
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x)
        {
            result = getColorValue(get_pixels(info, pixels, x, y), result);
            result =  argb_mult(result,weights[0]);
            for (i = 1; i < radius; ++i)
            {
                int32_t left = x - i;
                if (left >= 0)
                {
                    temp = getColorValue(get_pixels(info, temp_pixels, left, y), temp);
                    temp = argb_mult(temp, weights[i]);
                    result = argb_add(result, temp);
                }
                int32_t right = x + i;
                if (right < width)
                {
                    temp = getColorValue(get_pixels(info, temp_pixels, right, y), temp);
                    temp = argb_mult(temp, weights[i]);
                    result = argb_add(result, temp);
                }
            }
            set_pixels(info, pixels, x, y, toColor(result));
        }
    }
    LOGD("End -----------------------------  Hor ");
    LOGD("Free Temp");
    free(temp);
    LOGD("Free result");
    free(result);
    LOGD("Free temp pixels");
    free(temp_pixels);
    LOGD("Free temp pixels end");
    return;
}


JNIEXPORT void JNICALL
Java_com_lewaos_launcher_common_ImageUtils_nGaussianBlur(JNIEnv *env, jobject obj, jobject bitmap, jint radius)
{
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
    gaussianBlur(&info, pixels, radius);
    //printPixels(&info, pixels);
    LOGD("Precess End ................ ");
    if ((ret = AndroidBitmap_unlockPixels(env, bitmap) < 0)){
        LOGE("AndroidBitmap_unlockPixels() failed ! error=%d", ret);
    }
    LOGD("Function End ................ ");
    return;
}


