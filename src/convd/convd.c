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


conv_buf_t * conv_buf_set(conv_buf_t *cvbuf, char *arraybytes, size_t numbytes)
{
    cvbuf->bufp = arraybytes;
    cvbuf->blen = numbytes;
    return cvbuf;
}


int convd_create(const char *fromcode, const char *tocode, CONVD_SUFFIX_MODE suffix, convd_t *outcvd)
{
    convd_t cvd;

    int fromlen, tolen;

    fromlen = cstr_length(fromcode, CVD_ENCODING_LEN_MAX + 1);
    if (fromlen < 1 || fromlen > CVD_ENCODING_LEN_MAX) {
        /* invalid from code */
        return CONVD_RET_EENCODING;
    }

    tolen = cstr_length(tocode, CVD_ENCODING_LEN_MAX + 1);
    if (tolen < 1 || tolen > CVD_ENCODING_LEN_MAX) {
        /* invalid to code */
        return CONVD_RET_EENCODING;
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
            return CONVD_RET_EOPEN;
        }

        /* success */
        *outcvd = cvd;
        return CONVD_RET_NOERROR;
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
        return CONVD_RET_ESUFFIX;
    }

    cvd->cd = iconv_open(&cvd->codebuf[cvd->tocodeat], cvd->codebuf);

    if (cvd->cd == CONVD_ERROR_ICONV) {
        /* Faile to iconv_open. see: errno */
        refc_object_dec((void**)&cvd);
        return CONVD_RET_EOPEN;
    }

    /* success */
    *outcvd = cvd;
    return CONVD_RET_NOERROR;
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
        return CONVD_RET_EICONV;
    }

    outlen = output->blen;

    while ((ret = iconv(cvd->cd, &input->bufp, &input->blen, &output->bufp, &output->blen)) != CONVD_ERROR_SIZE) {
        if (input->blen == 0) {
            refc_object_unlock(cvd);

            // success( >=0 ): all input converted ok
            return (outlen - output->blen);
        }
        // continue to next iconv
    }

    refc_object_unlock(cvd);

    // failed
    return CONVD_RET_EICONV;
}


size_t convd_conv_file(convd_t cvd, const char *textfilein, const char *textfileout, CONVD_UCS_BOM outfilebom)
{
    // TODO:
    UCS_file_detect_bom(textfilein);

    return -1;
}

