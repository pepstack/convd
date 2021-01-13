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

static void conv_des_cleanup (conv_des_t *cvd)
{
    iconv_close(cvd->cd);
}


conv_buf_t * conv_buf_set(conv_buf_t *cvbuf, char *arraybytes, size_t numbytes)
{
    cvbuf->blob = arraybytes;
    cvbuf->blen = numbytes;
    return cvbuf;
}


convd_t convd_create(const char *fromcode, const char *tocode, CDP_SUFFIX suffix)
{
    convd_t cvd;
    iconv_t cd;

    if (suffix == CDP_SUFFIX_NONE) {
        cd = iconv_open(tocode, fromcode);
    } else {
        char tobuf[128];
        int len = cstr_length(tocode, 110);

        if (suffix == CDP_SUFFIX_IGNORE) {
            snprintf_chkd_V1(tobuf, sizeof(tobuf), "%.*s//IGNORE", len, tocode, tocode);
        } else if (suffix == CDP_SUFFIX_TRANSLIT) {
            snprintf_chkd_V1(tobuf, sizeof(tobuf), "%.*s//TRANSLIT", len, tocode, tocode);
        } else {
            /* Invalid argument: errno = 0 */
            return NULL;
        }

        cd = iconv_open(tobuf, fromcode);
    }

    if (cd == (iconv_t)(-1)) {
        /* Faile to open. see: errno */
        return NULL;
    }

    cvd = (conv_des_t *)refc_object_new(0, sizeof(conv_des_t), conv_des_cleanup);
    cvd->cd = cd;

    return cvd;
}


void convd_release(convd_t *pcvd)
{
    refc_object_dec((void**)pcvd);
}


convd_t convd_retain(convd_t *cvd)
{
    return (convd_t)refc_object_inc((void**)cvd);
}


size_t convd_conv(convd_t cvd, conv_buf_t *input, conv_buf_t *output)
{
    size_t ret, outlen;

    refc_object_lock(cvd, 0);

    ret = iconv(cvd->cd, NULL, NULL, NULL, NULL);
    if (ret == (size_t)(-1)) {
        /* SHOULD NEVER RUN TO THIS! */
        emerglog_exit("libconvd", "iconv error(%d): %s", errno, strerror(errno));
    }

    outlen = output->blen;

    while ((ret = iconv(cvd->cd, &input->blob, &input->blen, &output->blob, &output->blen)) != (size_t)(-1)) {
        if (input->blen == 0) {
            refc_object_unlock(cvd);

            // success( >=0 ): all input converted ok
            return (outlen - output->blen);
        }
        // continue to next iconv
    }

    refc_object_unlock(cvd);

    // error( -1 )
    return (size_t)(-1);
}