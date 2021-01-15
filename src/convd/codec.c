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
 * @file:  codec.c
 *  UCS and XML codec utility.
 *
 * @author:
 * @version:
 * @create: $create$
 * @update:
 */
#include "convd_i.h"


CONVD_UCS_BOM UCS_text_detect_bom(const conv_buf_t *header)
{
    if (header->blen > 1) {
        if ((unsigned char)header->bufp[0] == 0xFE &&
            (unsigned char)header->bufp[1] == 0xFF) {
            return UCS_UTF_16BE;
        }

        if ((unsigned char)header->bufp[0] == 0xFF &&
            (unsigned char)header->bufp[1] == 0xFE) {
            return UCS_UTF_16LE;
        }

        if ((unsigned char)header->bufp[0] == 0xEF &&
            (unsigned char)header->bufp[1] == 0xBB) {
            if (header->blen == 2) {
                /* need one more byte */
                return UCS_UTF_8BOM_ASK;
            }

            if ((unsigned char)header->bufp[2] == 0xBF) {
                return UCS_UTF_8BOM;
            }
        }
    }

// TODO: UCS_UTF_32BE UCS_UTF_32LE

    return UCS_BOM_NONE;
}


CONVDAPI CONVD_UCS_BOM UCS_file_detect_bom(const char *textfile)
{
    int bom = UCS_BOM_NONE;

    filehandle_t hf = file_open_read(textfile);

    if (hf != filehandle_invalid) {
        conv_buf_t headbuf;
        char header[4] = {0x00, 0x00, 0x00, 0x00};

        /* try to read first 2 bytes */
        if (file_readbytes(hf, header, 2) == 2) {
            bom = UCS_text_detect_bom(conv_buf_set(&headbuf, header, 2));

            if (bom == UCS_BOM_NONE) {
                goto finish_done;
            }
            if (bom != UCS_UTF_8BOM_ASK) {
                goto finish_done;
            }
            if (file_readbytes(hf, &header[2], 1) == 1) {
                bom = UCS_text_detect_bom(conv_buf_set(&headbuf, header, 3));
            }
        }
    }

finish_done:
    file_close(&hf);
    return bom;
}


int XML_text_parse_head(const conv_buf_t *xmltext, conv_xmlhead_t *xmlhead, CONVD_UCS_BOM *bomtag)
{
    return 0;
}


int XML_file_parse_head(const char *xmlfile, conv_xmlhead_t *xmlhead, CONVD_UCS_BOM *bomtag)
{
    return 0;
}
