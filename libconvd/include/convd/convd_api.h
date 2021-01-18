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

#ifdef LIBCONVD_DLL
/* win32 dynamic dll */
# ifdef LIBCONVD_EXPORTS
#   define CONVDAPI __declspec(dllexport)
# else
#   define CONVDAPI __declspec(dllimport)
# endif
#else
/* static lib or linux so */
# define CONVDAPI  extern
#endif


/**
 * error codes
 */
#define CONVD_RET_NOERROR    0
#define CONVD_RET_ERROR    (-1)
#define CONVD_RET_EICONV   (-1)
#define CONVD_RET_EOPEN    (-1)
#define CONVD_RET_EENCODING  1
#define CONVD_RET_ESUFFIX    2

typedef struct _conv_descriptor_t    *convd_t;


/**
 * max length for charset encoding including end null char.
 * Lists the supported encodings:
 *   $ iconv --list
 */
#define CVD_ENCODING_LEN_MAX  64


typedef enum {
    /**
     * No suffix string appended to tocode.
     */
    CVD_SUFFIX_NONE =     0,

    /**
     * The string "//IGNORE" will be appended to tocode,
     * This means that characters that cannot be represented in the target character
     * set will be silently discarded.
     */
    CVD_SUFFIX_IGNORE =   1,

    /**
     * The string "//TRANSLIT" will be appended to tocode:
     * This means that when a character cannot be represented in the target character
     * set, it can be approximated through one or several similarly looking characters.
     */
    CVD_SUFFIX_TRANSLIT  = 2
} CONVD_SUFFIX_MODE;


/**
 * Unicode Character Set
 */
typedef enum {
    /**
     * Byte Order Mark not found
     */
    UCS_NONE_BOM     = 0,

    /**
     * UTF-8: 'EF BB BF'
     *   8-bit UCS Transformation Format with BOM header
     */
    UCS_UTF8_BOM     = 1,

    /**
     * UTF-16BE: 'FE FF'
     *   16-bit UCS Transformation Format, big-endian byte order
     */
    UCS_2BE_BOM     = 2,

    /**
     * UTF-16LE: 'FF FE'
     *   16-bit UCS Transformation Format, little-endian byte order
     */
    UCS_2LE_BOM     = 3,

    /**
     * UTF-32BE: '00 00 FE FF'
     *   32-bit UCS Transformation Format, big-endian byte order
     */
    UCS_4BE_BOM     = 4,

    /**
     * UTF-32LE: 'FF FE 00 00'
     *   32-bit UCS Transformation Format, little-endian byte order
     */
    UCS_4LE_BOM     = 5,

    /**
     * UTF-8?: 'EF BB ?'
     *   Ask next one byte to determine if it is UTF_UTF_8BOM
     */
    UCS_UTF8_BOM_ASK = 6
} CONVD_UCS_BOM;


typedef struct
{
    int converr;    // DEL
    size_t blen;
    char *bufp;
} conv_buf_t;


typedef struct
{
    char version[16];
    char encoding[CVD_ENCODING_LEN_MAX];
} conv_xmlhead_t;


/**
 * Name
 *   convd_create - create a new convd_t object.
 *
 * Return Value
 *   On success, it returns CONVD_RET_NOERROR(0) and *outcvd is a convd_t object;
 *   On error, it returns an error code like CONVD_RET_E???, and the content of *outcvd is undefined.
 *
 * Errors
 *
 *   CONVD_RET_EENCODING - Invalid fromcode or tocode.
 *   CONVD_RET_ESUFFIX - Invalid suffix argument.
 *   CONVD_RET_EOPEN - Faile to call iconv_open. uses strerror(errno) to get more error details.
 *
 * Notes
 *   convd_t object is MT-safety.
 *
 * Example
 *
 *   The program below demonstrates the use of convd_create(), as well as other
 *    functions in the libconvd API.
 *
 *   The test_convd program shows how to do charset encodings conversion from UTF-8 to GBK.
 *
        void test_convd(int num)
        {
            // 创建转换器. 这个对象是线程安全的
            convd_t cvd;

            int err = convd_create("UTF-8", "GB2312", CVD_SUFFIX_IGNORE, &cvd);

            if (err == CONVD_RET_NOERROR) {
                // 临时变量
                conv_buf_t input, output;

                // 输入 UTF-8 字符串
                char intext[] = "china中国";

                // 输出缓冲区的尺寸至少是输入长度的 4 倍
                char outgbk[256];

                for (int i = 0; i < num; i++) {
                    int outlen = (int) convd_conv_text(cvd, convbufMake(&input, intext, strlen(intext)), convbufMake(&output, outgbk, sizeof(outgbk)));

                    if (i % 20000 == 0) {
                        printf("[%d] outgbk={%.*s}\n", i, outlen, outgbk);
                    }
                }

                // 使用完毕一定要释放
                convd_release(&cvd);
            }
        }
 */
CONVDAPI int convd_create(const char *fromcode, const char *tocode, CONVD_SUFFIX_MODE suffix, convd_t *outcvd);

CONVDAPI convd_t convd_retain(convd_t *cvd);
CONVDAPI void convd_release(convd_t *cvd);

CONVDAPI const char * convd_fromcode(const convd_t cvd);
CONVDAPI const char * convd_tocode(const convd_t cvd);


/**
 * Name
 *   convd_config - control and query code conversion behavior.
 *
 * Synopsis
 *   #include <iconv.h>
 *   #include <convd_api.h>
 *
 *   int convd_config(const convd_t cvd, int request, void *arg);
 *
 * Description
 *   The convd_config function is a direct wrapper of iconvctl() (see: iconv.h)
 *    and can be used to control code conversion behavior by setting or getting
 *    the current code conversion behavior settings from the current code
 *    conversion pointed to by the conversion descriptor that was returned from
 *    a successful convd_create call.
 *
 *   The following are the supported values for the request argument:
 *    (See also: https://docs.oracle.com/cd/E36784_01/html/E36874/iconvctl-3c.html)
 *
 *      ICONV_GET_CONVERSION_BEHAVIOR
 *      ICONV_SET_CONVERSION_BEHAVIOR
 *      ICONV_GET_DISCARD_ILSEQ
 *      ICONV_SET_DISCARD_ILSEQ
 *      ICONV_GET_TRANSLITERATE
 *      ICONV_SET_TRANSLITERATE
 *      ICONV_TRIVIALP
 *
 * Return Value
 *   Upon successful completion, convd_config() returns 0 and, optionally, with a
 *    value pointed to by the arg argument. Otherwise, convd_config() returns -1
 *    and sets errno to indicate the error.
 *
 * Notes
 *   convd_config is MT-safety.
 *
 */
CONVDAPI int convd_config(const convd_t cvd, int request, void *argument);


CONVDAPI conv_buf_t * convbufMake(conv_buf_t *cvbuf, char *arraybytes, size_t numbytes);

/**
 * Name
 *   convd_conv_text - character set conversion.
 *
 * Return Value
 *   On success, it returns length of bytes (not less than 0) successfully converted;
 *   On error, it returns CONVD_RET_ECONV(-1) and caller can use strerror(errno) to
 *    get more error details.
 *
 * Errors
 *
 *   CONVD_RET_EICONV - Faile to call iconv_open. uses strerror(errno) to get more error details.
 *
 * Notes
 *   input and output should be changed after calling.
 *
 */
CONVDAPI size_t convd_conv_text(convd_t cvd, conv_buf_t *input, conv_buf_t *output);

/**
 * TODO:
 *   https://worthsen.blog.csdn.net/article/details/86585271
 */
CONVDAPI ub8 convd_conv_file(convd_t cvd, const char *textfilein, const char *textfileout, CONVD_UCS_BOM outfilebom);


/**
 * Name
 *   UCS_text_detect_bom - detect text buffer for bom tag.
 *   UCS_file_detect_bom - detect file for bom tag.
 *
 * Return Value
 *   Always returns one of CONVD_UCS_BOM.
 *
 * Errors
 *
 *   UCS_NONE_BOM - Faile to detect header.
 */
CONVDAPI CONVD_UCS_BOM UCS_text_detect_bom(const conv_buf_t *textbuf);

CONVDAPI int UCS_file_detect_bom(const char *pathfile, CONVD_UCS_BOM *outbom);


/**
 * Name
 *   UCS_text_detect_bom - detect text buffer for bom tag.
 *   UCS_file_detect_bom - detect file for bom tag.
 *
 * Return Value
 *   Always returns one of CONVD_UCS_BOM.
 *
 * Errors
 *
 *   UCS_NONE_BOM - Faile to detect header.
 */
CONVDAPI int XML_text_parse_head(const conv_buf_t *xmltext, conv_xmlhead_t *xmlhead, CONVD_UCS_BOM *bomtag);

CONVDAPI int XML_file_parse_head(const char *xmlfile, conv_xmlhead_t *xmlhead, CONVD_UCS_BOM *bomtag);

CONVDAPI int XML_text_encode(conv_buf_t *xmltextin, conv_buf_t *xmltextout, const char *encoding, CONVD_SUFFIX_MODE suffix);

CONVDAPI int XML_file_encode(const char *xmlfile, const char *toxmlfile, const char *encoding, CONVD_SUFFIX_MODE suffix);

#ifdef __cplusplus
}
#endif
#endif /* CONVD_PUBLIC_API_H_ */
