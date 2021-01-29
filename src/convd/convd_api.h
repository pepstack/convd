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
#define CONVD_ERR_ENCODING       4
#define CONVD_ERR_INSUFF         5
#define CONVD_ERR_ICONV        (-1)
#define CONVD_ERR_FILE         (-2)


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
 * @param[in] fromcode    Convert characters from encoding.
 * @param[in] tocode      Convert characters to encoding.
 * @param[in] suffix      Options controlling conversion problems.   
 * @param[out] outcvd     The conversion descriptor relates the two encoded character sets.
 * @return
 *   On success, it returns CONVD_NOERROR(=0) and the out parameter of outcvd is set by a convd_t object;<br/>
 *   On error, it returns an error code like CONVD_ERR_?, and out parameter of outcvd is undefined.<br/>
 *   The following errors can occur:<br/>
 *      - CONVD_ERR_FROMCODE<br/>
 *          invalid value for fromcode
 *      - CONVD_ERR_TOCODE<br/>
 *          invalid value for tocode
 *      - CONVD_ERR_SUFFIX<br/>
 *          invalid value for suffix
 *      - CONVD_ERR_ICONV<br/>
 *          an error occurred while calling iconv_open().
 *
 * @note
 *   The convd_t object maintains a count of the number of references to itself.<br/>
 *   The initial reference count of a new reference-counted object is 1.<br/>
 *   convd_t object can be safely accessed from multiple threads (MT-safe).
 */
CONVDAPI int convd_create(const char *fromcode, const char *tocode, CONVD_SUFFIX_MODE suffix, convd_t *outcvd);


/**
 * Decrement the reference count of convd_t object.
 * @param[in] cvd    The pointer to a convd_t object to be released.
 * @return
 *   None
 *
 * @note
 *   When release the reference-counted object, its reference count is decreased by 1.<br/>
 *   If the reference count reaches at 0, the reference-counted object is deallocated.
 */
CONVDAPI void convd_release(convd_t *cvd);


/**
 * Increment the reference count of convd_t object.
 * @param[in] cvd    The pointer to a convd_t object to be increment.
 * @return
 *   The convd_t object itself.
 *
 * @note
 *   When retain the reference-counted object, its reference count is increased by 1.<br/>
 *   You must release a reference-counted object once after it has been created or retained.
 */
CONVDAPI convd_t convd_retain(convd_t *cvd);


/**
 * Get characters encoding from which convert.
 * @param[in] cvd    A convd_t object.
 * @return
 *   The read only string of from code.
 * @see convd_create
 */
CONVDAPI const char * convd_fromcode(const convd_t cvd);


/**
 * Get characters encoding to which convert.
 * @param[in] cvd       A convd_t object.
 * @param[out] suffix   Optional output suffix.
 * @return
 *   The read only string of to code.
 * @see convd_create
 */
CONVDAPI const char * convd_tocode(const convd_t cvd, CONVD_SUFFIX_MODE *suffix);


/**
 * Control and query code conversion behavior.
 * @param[in] cvd     A conversion descriptor created using the function convd_create.
 * @param[in] request The following are the supported values for the request parameter:<br/>
 *  - ICONV_TRIVIALP<br/>
 *      argument should be an int * which will receive 1 if the conversion is trivial, or 0 otherwise.<br/>
 *  - ICONV_GET_TRANSLITERATE<br/>
 *      argument should be an int * which will receive 1 if transliteration is enabled in the conversion, or 0 otherwise.<br/>
 *  - ICONV_SET_TRANSLITERATE<br/>
 *      argument should be a const int *, pointing to an int value. A non-zero value is used to enable transliteration in the conversion. A zero value disables it.<br/>
 *  - ICONV_GET_DISCARD_ILSEQ<br/>
 *      argument should be an int * which will receive 1 if "illegal sequence discard and continue" is enabled in the conversion, or 0 otherwise.<br/>
 *  - ICONV_SET_DISCARD_ILSEQ<br/>
 *      argument should be a const int *, pointing to an int value. A non-zero value is used to enable "illegal sequence discard and continue" in the conversion. A zero value disables it.<br/>
 * @param[out] argument see the above request parameter.
 *
 * @return
 *   Upon successful completion, convd_config() returns 0 and, optionally, with a value pointed to by the argument.<br/>
 *   Otherwise, convd_config() returns -1 and sets errno to indicate the error.
 * @note
 *    The convd_config function is a direct wrapper of iconvctl() (see: iconv.h)<br/>
 *    iconvctl() queries or adjusts the behavior of the iconv function, when invoked with the specified conversion descriptor, depending on the request value.<br/>
 *    This function is implemented only in GNU libiconv and not in other iconv implementations.<br/>
 *    It is not backed by a standard. You can test for its presence through (_LIBICONV_VERSION >= 0x0108).<br/>
 *    convd_config() is also MT-safe.
 * @see
 *   https://www.gnu.org/savannah-checkouts/gnu/libiconv/documentation/libiconv-1.15/iconvctl.3.html
 */
CONVDAPI int convd_config(const convd_t cvd, int request, void *argument);


/**
 * Populate a temporary conversion buffer with array of chars.
 * @param[in] cvbuf      Pointer to a temporary conversion buffer.
 * @param[in] arraybytes Pointer to the actual chars array.
 * @param[in] numbytes   Allowed size in bytes of arraybytes.
 * @return
 *   The pointer to the input temporary conversion buffer.
 */
CONVDAPI conv_buf_t * convbuf_mk(conv_buf_t *cvbuf, char *arraybytes, size_t numbytes);


/**
 * Detect the BOM (Byte Order Mark) of an UCS (Unicode Character Set) text.
 * @param[in] textbuf    buffer for UCS text to be BOM detected.
 * @return               one value of CONVD_UCS_BOM.
 */
CONVDAPI CONVD_UCS_BOM UCS_text_detect_bom(const conv_buf_t *textbuf);


/**
 * Detect the BOM (Byte Order Mark) of an UCS (Unicode Character Set) text file.
 * @param[in] pathfile    Full path to a text file to be BOM detected.
 * @param[out] outbom     Upon successful completion it is set by one value of CONVD_UCS_BOM.
 * @return
 *   Upon successful completion, it returns 0.<br/>
 *   Otherwise, returns CONVD_ERR_FILE(-1) and sets errno to indicate the error.
 */
CONVDAPI int UCS_file_detect_bom(const char *pathfile, CONVD_UCS_BOM *outbom);


/**
 * Parse xml text to get xml head.
 * @param[in] xmltext    Xml test to be parsed.
 * @param[out] xmlhead   Upon successful completion its members are set.
 * @return
 *   Upon successful completion, it returns bytes length(>0) of xml head for xmltext.<br/>
 *   Otherwise, returns 0 to indicate the xml head not found.
 */
CONVDAPI int XML_text_parse_head(const conv_buf_t *xmltext, conv_xmlhead_t *xmlhead);


/**
 * Parse xml file to get xml head.
 * @param[in] xmlfile    Full path to an xml file to be xmlhead parsed.
 * @param[out] xmlhead   Upon successful completion its members are set.
 * @return
 *   Upon successful completion, it returns bytes length(>0) of xml head in the xmlfile.<br/>
 *   Otherwise, returns 0 to indicate the xml head not found, or returns CONVD_ERR_FILE(-1)<br/>
 *   and sets errno to indicate the error.
 */
CONVDAPI int XML_file_parse_head(const char *xmlfile, conv_xmlhead_t *xmlhead);


/**
 * Convert characters from input to output.
 * @param[in] cvd      The conversion descriptor.
 * @param[in] input    The input argument points to the characters of the input sequence.
 * @param[out] output  The output argument points to the characters of the output sequence.
 * @return
 *   Upon successful completion, it returns number of bytes saved in output buffer.<br/>
 *   Otherwise, returns CONVD_ERR_ICONV(-1) and might set errno to indicate the error.
 * @note
 *   The convd_conv_text() function converts a sequence of characters in one<br/>
 *   charset encoding to a sequence of characters in another charset encoding.<br/>
 * @see
 *   https://man7.org/linux/man-pages/man3/iconv.3.html
 */
CONVDAPI size_t convd_conv_text(convd_t cvd, conv_buf_t *input, conv_buf_t *output);


/**
 * Convert characters from input file to output file.
 * @param[in] cvd           The conversion descriptor.
 * @param[in] textfilein    Full path to the input text file to be converted.
 * @param[in] inoffset      The size in bytes should be ignord in textfilein.<br/>
 *   set it to 0 that asks to convert from the beginning of the input file.
 * @param[in] textfileout   Full path to the output file as result of conversion.<br/>
 *   The textfileout must not exist before calling.
 * @param[in] outhead       Optional content to be written into the head of output file.
 * @param[in] outheadlen    Size in bytes of outhead.
 * @param[in] linesizemax   Max line size in bytes for textfilein.<br/>
 *   This important argument must be big enough to read entire line of input text.
 * @param[out] outfilesize  Optional parameter to save bytes size of textfileout.<br/>
 * @return
 *   Upon successful completion, it returns CONVD_NOERROR(0).<br/>
 *   Otherwise, returns error codes list in the following:<br/>
 *      - CONVD_ERR_ENCODING<br/>
 *          Not a regular text file.
 *      - CONVD_ERR_INSUFF<br/>
 *          Insufficent line size for reading. You should increase linesizemax and try again.
 *      - CONVD_ERR_ICONV<br/>
 *          Error occurred while calling iconv().
 *      - CONVD_ERR_FILE<br/>
 *          Error occurred while reading or writing file.
 * @note
 *   The convd_conv_file() function converts characters file in one<br/>
 *   charset encoding to characters file in another charset encoding.<br/>
 * @see
 *   https://man7.org/linux/man-pages/man3/iconv.3.html
 */
CONVDAPI int convd_conv_file(convd_t cvd, const char *textfilein, size_t inoffset,
                const char *textfileout, const char *outhead, size_t outheadlen,
                ub4 linesizemax, ub8 *outfilesize);


/**
 * Convert xml characters from input to output.
 * @param[in] cvd      The conversion descriptor.
 * @param[in] input    The input xml characters sequence to be converted from.
 * @param[out] output  The output xml characters sequence to be converted to.
 * @return
 *   Upon successful completion, it returns number of bytes converted to in output buffer.<br/>
 *   Otherwise, returns CONVD_ERR_ICONV(-1) and might set errno to indicate the error.
 * @note
 *   The convd_conv_xmltext() function converts a sequence of xml characters in one<br/>
 *   charset encoding to a sequence of xml characters in another charset encoding.<br/>
 *   The encoding attribute in the first line of output xml will be assigned with tocode.<br/>
 * @see convd_conv_text()
 */
CONVDAPI size_t convd_conv_xmltext(convd_t cvd, conv_buf_t *input, conv_buf_t *output);


/**
 * Convert characters from input xml file to output xml file.
 * @param[in] cvd          The conversion descriptor.
 * @param[in] xmlfilein    Full path to the input xml file to be converted.
 * @param[in] xmlfileout   Full path to the output xml file as result of conversion.<br/>
 *   The output file must not exist before calling.
 * @param[in] addbom       Optional BOM to be written into the head of output file.
 * @param[in] linesizemax   Max size in bytes of buffer for reading input file.<br/>
 *   This important argument must be big enough to read entire line of input file.
 * @param[out] outfilesize  Optional parameter to save bytes size of output xml file.<br/>
 * @return
 *   Upon successful completion, it returns CONVD_NOERROR(0).<br/>
 *   Otherwise, returns error codes list in the following:<br/>
 *      - CONVD_ERR_ENCODING<br/>
 *          Not a regular text file.
 *      - CONVD_ERR_INSUFF<br/>
 *          Insufficent line size for reading. You should increase linesizemax and try again.
 *      - CONVD_ERR_ICONV<br/>
 *          Error occurred while calling iconv().
 *      - CONVD_ERR_FILE<br/>
 *          Error occurred while reading or writing file.
 * @note
 *   The convd_conv_xmlfile() function converts xml characters file in one<br/>
 *   charset encoding to xml characters file in another charset encoding.<br/>
 * @see
 *   https://man7.org/linux/man-pages/man3/iconv.3.html
 */
CONVDAPI int convd_conv_xmlfile(convd_t cvd, const char *xmlfilein, const char *xmlfileout, ub4 linesizemax, int addbom, ub8 *outfilesize);

#ifdef __cplusplus
}
#endif
#endif /* CONVD_PUBLIC_API_H_ */
