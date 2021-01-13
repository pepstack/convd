/**
 * @file: convd_api.h
 *   libconvd public api.
 *
 * @author:
 * @version:
 * @create: $create$
 * @update:
 */
#ifndef CONVD_PUBLIC_API_H_
#define CONVD_PUBLIC_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef LIBCDPOOL_DLL
/* win32 dynamic dll */
# ifdef LIBCDPOOL_EXPORTS
#   define CDPOOLAPI __declspec(dllexport)
# else
#   define CDPOOLAPI __declspec(dllimport)
# endif
#else
/* static lib or linux so */
# define CDPOOLAPI  extern
#endif


typedef struct _conv_des_t    *convd_t;


typedef int CDP_SUFFIX;

/**
 * No suffix string appended to tocode.
 */
#define CDP_SUFFIX_NONE      0

/**
 * The string "//IGNORE" will be appended to tocode,
 * This means that characters that cannot be represented in the target character
 * set will be silently discarded.
 */
#define CDP_SUFFIX_IGNORE    1

/**
 * The string "//TRANSLIT" will be appended to tocode:
 * This means that when a character cannot be represented in the target character
 * set, it can be approximated through one or several similarly looking characters.
 */
#define CDP_SUFFIX_TRANSLIT  2          


typedef struct
{
    size_t blen;
    char *blob;
} conv_buf_t;

/**
 * Usage: UTF-8 to GBK
 *
    static void test_convd(int num)
    {
        // 临时变量
        conv_buf_t input, output;

        // 输入 UTF-8 字符串
        char intext[] = "china中国";

        // 输出缓冲区的尺寸至少是输入长度的 4 倍
        char outgbk[256];

        // 创建转换器. 这个对象是线程安全的
        convd_t cvd = convd_create("UTF-8", "GB2312", 1);

        for (int i = 0; i < num; i++) {
            int outlen = (int) convd_conv(cvd, conv_buf_set(&input, intext, strlen(intext)), conv_buf_set(&output, outgbk, sizeof(outgbk)));

            if (i % 20000 == 0) {
                printf("[%d] outgbk={%.*s}\n", i, outlen, outgbk);
            }
        }

        // 使用完毕一定要释放
        convd_release(&cvd);
    }
*/


/**
 * NOTES: convd_t is MT-safety
 */

CDPOOLAPI convd_t convd_create(const char *fromcode, const char *tocode, CDP_SUFFIX suffix);
CDPOOLAPI convd_t convd_retain(convd_t *cvd);
CDPOOLAPI void convd_release(convd_t *cvd);

CDPOOLAPI conv_buf_t * conv_buf_set(conv_buf_t *blob, char *arraybytes, size_t numbytes);
CDPOOLAPI size_t convd_conv(convd_t cvd, conv_buf_t *input, conv_buf_t *output);

#ifdef __cplusplus
}
#endif
#endif /* CONVD_PUBLIC_API_H_ */
