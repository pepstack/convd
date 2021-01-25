/**
 * @file convd_api.h
 * @brief libconvd public api.
 *
 * @author cheungmine@qq.com
 * @version
 * @date
 * @note
 * @since
 */
#ifndef CONVD_PUBLIC_API_H_
#define CONVD_PUBLIC_API_H_

// install: doxygen graphviz
// cd project dir
// doxygen -g -s
// https://www.doxygen.nl/manual/docblocks.html#docexamples
// https://blog.csdn.net/wangxvfeng101/article/details/7301115
// https://blog.csdn.net/weixin_43636423/article/details/103783811

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
 * success result code
 */
#define CONVD_NOERROR            0


/**
 * failure result codes 
 */
#define CONVD_ERR_FROMCODE       1
#define CONVD_ERR_TOCODE         2
#define CONVD_ERR_SUFFIX         3

#define CONVD_ERR_ICONV_OPEN   (-1)




#define CONVD_RET_NOERROR    0
#define CONVD_RET_ERROR    (-1)
#define CONVD_RET_EICONV   (-1)
#define CONVD_RET_EOPEN    (-1)
#define CONVD_RET_EENCODING  1
#define CONVD_RET_ESUFFIX    2
#define CONVD_RET_EOVERFLOW  3



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
    size_t blen;
    char *bufp;
} conv_buf_t;


typedef struct
{
    CONVD_UCS_BOM bom;
    char version[16];
    char encoding[CVD_ENCODING_LEN_MAX];
} conv_xmlhead_t;


/**
 * format given xml head to output of ansi string.
 * @param[in] xmlhead
 * @param[in] output
 * @return
 *   On success, it returns length of chars in output buf not including ending null char('\0');
 *   On error, it returns:
 *     0 - insufficent output buf;
 *    -1 - unknown bom.
 */
CONVDAPI int conv_xmlhead_format(const conv_xmlhead_t *xmlhead, conv_buf_t *output);


/**
 * Create a new convd_t object.
 * @param[in] fromcode    Convert characters from encoding
 * @param[in] tocode      Convert characters to encoding
 * @param[in] suffix      Options controlling conversion problems    
 * @param[out] outcvd     A convd_t object created
 * @return
 *   On success, it returns CONVD_NOERROR(=0) and the out parameter of outcvd is set by a convd_t object;
 *   On error, it returns an error code like CONVD_ERR_?, and out parameter of outcvd is undefined.
 *
 * @retal CONVD_NOERROR         success
 * @retal CONVD_ERR_FROMCODE    invalid value for fromcode
 * @retal CONVD_ERR_TOCODE      invalid value for tocode
 * @retal CONVD_ERR_SUFFIX      invalid value for suffix
 * @retal CONVD_ERR_ICONV_OPEN  an error occurred while calling iconv_open().
 *
 * @note  convd_t object can be safely accessed from multiple threads (MT-safe).
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
CONVDAPI int XML_text_parse_head(const conv_buf_t *xmltext, conv_xmlhead_t *xmlhead);

CONVDAPI int XML_file_parse_head(const char *xmlfile, conv_xmlhead_t *xmlhead);


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

CONVDAPI int convd_conv_file(convd_t cvd, const char *textfilein, size_t inoffset, const char *textfileout, const char *outhead, size_t outheadlen, ub4 linesizemax, ub8 *outfilesize, int addbom);

CONVDAPI size_t convd_conv_xmltext(convd_t cvd, conv_buf_t *input, conv_buf_t *output);

CONVDAPI int convd_conv_xmlfile(convd_t cvd, const char *xmlfilein, const char *xmlfileout, ub4 linesizemax, ub8 *outfilesize, int addbom);

#ifdef __cplusplus
}
#endif
#endif /* CONVD_PUBLIC_API_H_ */
