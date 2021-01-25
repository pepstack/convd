/***********************************************************************
 * Copyright (c) 2008-2080 pepstack.com, 350137278@qq.com
 *
 * ALL RIGHTS RESERVED.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **********************************************************************/
/**
 * @file:  convd.c
 *  libconvd implementation.
 *
 * @author:
 * @version:
 * @create: $create$
 * @update:
 */
#include "convd_i.h"


static void convd_cleanup_cb (conv_descriptor_t *cvd)
{
    if (cvd->cd != CONVD_ERROR_ICONV) {
        iconv_close(cvd->cd);
    }
}


conv_buf_t * convbufMake(conv_buf_t *cvbuf, char *arraybytes, size_t numbytes)
{
    cvbuf->bufp = arraybytes;
    cvbuf->blen = numbytes;
    return cvbuf;
}


static int ucs_file_read_open(convd_t cvd, const char *pathfile, ub4 linesizemax, convpos_t *_cpos)
{
    filehandle_t hf = file_open_read(pathfile);
    if (hf != filehandle_invalid) {
        convpos_t cpos;

        int bom = UCS_NONE_BOM;
        conv_buf_t headbuf;
        char header[4] = {0x00, 0x00, 0x00, 0x00};

        /* try to read first 4 bytes */
        int offcb = file_readbytes(hf, header, 4);
        bom = UCS_text_detect_bom(convbufMake(&headbuf, header, offcb));
        if (bom == UCS_UTF8_BOM_ASK) {
            file_close(&hf);
            return CONVD_RET_EENCODING;
        }

        offcb = 0;
        if (bom == UCS_2BE_BOM || bom == UCS_2LE_BOM) {
            offcb = 2;
        } else if (bom == UCS_4BE_BOM || bom == UCS_4LE_BOM) {
            offcb = 4;
        } else if (bom == UCS_UTF8_BOM) {
            offcb = 3;
        }
        if (file_seek(hf, offcb, 0) != offcb) {
            file_close(&hf);
            return CONVD_RET_EOPEN;
        }

        if (linesizemax == -1) {
             linesizemax = CONV_LINE_MAXSIZE;
        }

        cpos = (convpos_t)mem_alloc_unset(sizeof(conv_position_t) + (5 * linesizemax) * sizeof(char));

        cpos->cvd = cvd;
        cpos->bom = bom;
        cpos->offset = offcb;
        cpos->textfd = hf;
        cpos->linesize = linesizemax;
        cpos->inputbuf = cpos->linebuf;
        cpos->outputbuf = &cpos->linebuf[cpos->linesize];

        *_cpos= cpos;
        return CONVD_RET_NOERROR;
    }

    return CONVD_RET_EOPEN;
}


static int ucs_file_read_next(convpos_t cpos)
{
    ub1 chbyte;
    int bitflag;
    int offcb = 0;
    int rdlen = file_readbytes(cpos->textfd, cpos->inputbuf, cpos->linesize);
    if (rdlen == -1) {
        return CONV_EBREAK;
    }
    if (rdlen == 0) {
        return CONV_FINISHED;
    }

    switch (cpos->bom) {
    case UCS_2BE_BOM:
        while (offcb <= rdlen - 2) {
            BO_bytes_betoh(cpos->inputbuf + offcb, 2);
            offcb += 2;
        }
        break;

    case UCS_2LE_BOM:
        while (offcb <= rdlen - 2) {
            BO_bytes_letoh(cpos->inputbuf + offcb, 2);
            offcb += 2;
        }
        break;

    case UCS_4BE_BOM:
        while (offcb <= rdlen - 4) {
            BO_bytes_betoh(cpos->inputbuf + offcb, 4);
            offcb += 4;
        }
        break;

    case UCS_4LE_BOM:
        while (offcb <= rdlen - 4) {
            BO_bytes_letoh(cpos->inputbuf + offcb, 4);
            offcb += 4;
        }
        break;

    case UCS_UTF8_BOM:
    case UCS_NONE_BOM:
    default:
        /* 读到换行符(\n), ascii字符和非ascii字符集转换处结束. 包括换行符. 如果遇到(\r\n), 去掉(\r) */
        chbyte = cpos->inputbuf[offcb++];
        bitflag = BO_check_bit(chbyte, 7);
        while (offcb < rdlen) {
            chbyte = cpos->inputbuf[offcb];
            if ((int)BO_check_bit(chbyte, 7) != bitflag) {
                /* 字符集转换处结束 */
                break;
            }
            offcb++;
        }

        if (offcb == cpos->linesize) {
            /* insufficent buf size for line */
            return CONV_EINSUF;
        }
        break;
    }

    cpos->offset += offcb;

    if (file_seek(cpos->textfd, cpos->offset, fseek_pos_set) != cpos->offset) {
        return CONV_EBREAK;
    }

    return offcb;
}


static void ucs_file_read_close(convpos_t cpos)
{
    file_close(&cpos->textfd);
    mem_free(cpos);
}


int convd_create(const char *fromcode, const char *tocode, CONVD_SUFFIX_MODE suffix, convd_t *outcvd)
{
    convd_t cvd;

    int fromlen, tolen;

    fromlen = cstr_length(fromcode, CVD_ENCODING_LEN_MAX + 1);
    if (fromlen < 1 || fromlen > CVD_ENCODING_LEN_MAX) {
        /* invalid from code */
        return CONVD_ERR_FROMCODE;
    }

    tolen = cstr_length(tocode, CVD_ENCODING_LEN_MAX + 1);
    if (tolen < 1 || tolen > CVD_ENCODING_LEN_MAX) {
        /* invalid to code */
        return CONVD_ERR_TOCODE;
    }

    cvd = (conv_descriptor_t *)refc_object_new(0, sizeof(conv_descriptor_t) + fromlen + tolen + sizeof(char) * 12, convd_cleanup_cb);

    cvd->cd = CONVD_ERROR_ICONV;

    memcpy(cvd->codebuf, fromcode, fromlen);
    cstr_toupper(cvd->codebuf, fromlen);

    cvd->tocodeat = fromlen + 1;

    if (suffix == CVD_SUFFIX_NONE) {
        memcpy(&cvd->codebuf[cvd->tocodeat], tocode, tolen);
        cstr_toupper(&cvd->codebuf[cvd->tocodeat], tolen);

        cvd->cd = iconv_open(&cvd->codebuf[cvd->tocodeat], cvd->codebuf);
        if (cvd->cd == CONVD_ERROR_ICONV) {
            /* Faile to iconv_open. see: errno */
            refc_object_dec((void**)&cvd);
            return CONVD_ERR_ICONV_OPEN;
        }

        /* success */
        *outcvd = cvd;
        return CONVD_NOERROR;
    }

    memcpy(&cvd->codebuf[cvd->tocodeat], tocode, tolen);
    cstr_toupper(&cvd->codebuf[cvd->tocodeat], tolen);

    if (suffix == CVD_SUFFIX_IGNORE) {
        memcpy(&cvd->codebuf[cvd->tocodeat + tolen], "//IGNORE", 8);
    } else if (suffix == CVD_SUFFIX_TRANSLIT) {
        memcpy(&cvd->codebuf[cvd->tocodeat + tolen], "//TRANSLIT", 10);
    } else {
        /* Invalid suffix argument: errno = 0 */
        refc_object_dec((void**)&cvd);
        return CONVD_ERR_SUFFIX;
    }

    cvd->cd = iconv_open(&cvd->codebuf[cvd->tocodeat], cvd->codebuf);
    if (cvd->cd == CONVD_ERROR_ICONV) {
        /* Faile to iconv_open. see: errno */
        refc_object_dec((void**)&cvd);
        return CONVD_ERR_ICONV_OPEN;
    }

    /* success */
    *outcvd = cvd;
    return CONVD_NOERROR;
}


void convd_release(convd_t *pcvd)
{
    refc_object_dec((void**)pcvd);
}


convd_t convd_retain(convd_t *cvd)
{
    return (convd_t)refc_object_inc((void**)cvd);
}


const char * convd_fromcode(const convd_t cvd)
{
    return cvd->codebuf;
}


const char * convd_tocode(const convd_t cvd)
{
    return &cvd->codebuf[cvd->tocodeat];
}


int convd_config(const convd_t cvd, int request, void *argument)
{
    int ret;

    refc_object_lock(cvd, 0);

    ret = iconvctl(cvd->cd, request, argument);

    refc_object_unlock(cvd);
    return ret;
}


size_t convd_conv_text(convd_t cvd, conv_buf_t *input, conv_buf_t *output)
{
    size_t ret, outlen;

    refc_object_lock(cvd, 0);

    ret = iconv(cvd->cd, NULL, NULL, NULL, NULL);
    if (ret == CONVD_ERROR_SIZE) {
        refc_object_unlock(cvd);
        return (-1);
    }

    outlen = output->blen;
    for (;;) {
        ret = iconv(cvd->cd, &input->bufp, &input->blen, &output->bufp, &output->blen);
        if (ret == CONVD_ERROR_SIZE) {
            refc_object_unlock(cvd);
            return (-1);
        }

        if (input->blen == 0) {
            refc_object_unlock(cvd);

            /* success( >=0 ): all input converted ok */
            return (outlen - output->blen);
        }
    }

    refc_object_unlock(cvd);
    return (-1);
}


/**
 *   https://worthsen.blog.csdn.net/article/details/86585271
 */
int convd_conv_file(convd_t cvd, const char *textfilein, size_t inoffset, const char *textfileout, const char *outhead, size_t outheadlen, ub4 linesizemax, ub8 *outfilesize, int addbom)
{
    convpos_t cpos;
    ub8 outfdsize = 0;

    int ret = ucs_file_read_open(cvd, textfilein, linesizemax, &cpos);
    if (ret == CONVD_RET_NOERROR) {
        filehandle_t outfd = file_write_new(textfileout);
        if (outfd != filehandle_invalid) {
            conv_buf_t input, output;

            if (outhead && outheadlen) {
                if (file_writebytes(outfd, outhead, (ub4)outheadlen) == -1) {
                    /* error */
                    file_close(&outfd);
                    ucs_file_read_close(cpos);
                    return (-1);
                }
            }            

            cpos->offset += inoffset;
            file_seek(cpos->textfd, inoffset, fseek_pos_cur);

            while ((ret = ucs_file_read_next(cpos)) > 0) {
                size_t convcb = convd_conv_text(cvd, convbufMake(&input, cpos->inputbuf, ret), convbufMake(&output, cpos->outputbuf, cpos->linesize * 4));
                if (convcb == CONVD_ERROR_SIZE) {
                    /* error */
                    file_close(&outfd);
                    ucs_file_read_close(cpos);
                    return (-1);
                }

                if (file_writebytes(outfd, cpos->outputbuf, (ub4)convcb) == -1) {
                    /* error */
                    file_close(&outfd);
                    ucs_file_read_close(cpos);
                    return (-1);
                }

                outfdsize += convcb;
            }

            file_close(&outfd);
            ucs_file_read_close(cpos);

            if (ret == CONV_FINISHED) {
                /* success returns 0 */
                if (outfilesize) {
                    *outfilesize = outfdsize;
                }
            }
            return ret;
        }
        ucs_file_read_close(cpos);
    }
    return ret;
}


size_t convd_conv_xmltext(convd_t cvd, conv_buf_t *input, conv_buf_t *output)
{
    conv_xmlhead_t xmlhead;
    conv_buf_t inbuf, outbuf;

    size_t convsize = 0;
    int offslen = 0;

    int xmlheadlen = XML_text_parse_head(convbufMake(&inbuf, input->bufp, input->blen), &xmlhead);

    if (stricmp(convd_fromcode(cvd), xmlhead.encoding)) {
        return -1;
    }

    if (! memcmp(convd_tocode(cvd), "UTF-8", 5) ||
        ! memcmp(convd_tocode(cvd), "GB2312", 6) ||
        ! memcmp(convd_tocode(cvd), "BIG5", 4) ||
        ! memcmp(convd_tocode(cvd), "GBK", 3) ||
        ! memcmp(convd_tocode(cvd), "GB18030", 7)) {
        // ansi == utf8
        xmlhead.bom = 0;
        cstr_safecopy(xmlhead.encoding, sizeof(xmlhead.encoding), 0, convd_tocode(cvd), cstr_length(convd_tocode(cvd), sizeof(xmlhead.encoding) - 1));

        offslen = conv_xmlhead_format(&xmlhead, convbufMake(&outbuf, output->bufp, output->blen));
    } else {

        // TODO:
        // utf8 -> tocode
    }

    convsize = convd_conv_text(cvd,
                    convbufMake(&inbuf, &input->bufp[xmlheadlen], input->blen - xmlheadlen),
                    convbufMake(&outbuf, &output->bufp[offslen], output->blen - offslen));
    if (convsize == -1) {
        return -1;
    }

    return convsize + (size_t) offslen;
}


int convd_conv_xmlfile(convd_t cvd, const char *xmlfilein, const char *xmlfileout, ub4 linesizemax, ub8 *outfilesize, int addbom)
{
    char xmlheadbuf[CVD_ENCODING_LEN_MAX + 16 + 32];
    int xmlheadlen = 0;

    conv_xmlhead_t xmlhead;
    int headofflen = XML_file_parse_head(xmlfilein, &xmlhead);
    if (headofflen > 0) {
        if (! memcmp(convd_tocode(cvd), "UTF-8", 5) ||
            ! memcmp(convd_tocode(cvd), "GB2312", 6) ||
            ! memcmp(convd_tocode(cvd), "BIG5", 4) ||
            ! memcmp(convd_tocode(cvd), "GBK", 3) ||
            ! memcmp(convd_tocode(cvd), "GB18030", 7)) {
            conv_buf_t output;

            const char *tocode = convd_tocode(cvd);
            int tocodelen = cstr_length(tocode, -1);
            char *end = strstr(tocode, "//");
            if (end) {
                tocodelen = (int)(end - tocode);
            }

            // ansi == utf8
            xmlhead.bom = 0;
            if (! cstr_safecopy(xmlhead.encoding, sizeof(xmlhead.encoding), 0, tocode, tocodelen)) {
                return -1;
            }
            xmlheadlen = conv_xmlhead_format(&xmlhead, convbufMake(&output, xmlheadbuf, sizeof(xmlheadbuf)));
        } else {
            // TODO:
            // utf8 -> tocode
        }

        return convd_conv_file(cvd, xmlfilein, headofflen, xmlfileout, xmlheadbuf, xmlheadlen, linesizemax, outfilesize, addbom);
    }

    return -1;
}
