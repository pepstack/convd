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
         if ((ub1)header->bufp[0] == 0xFE && (ub1)header->bufp[1] == 0xFF) {
            return UCS_2BE_BOM;
        }

        if ((ub1)header->bufp[0] == 0x00 && (ub1)header->bufp[1] == 0x00) {
            if (header->blen > 3) {
                if ((ub1)header->bufp[2] == 0xFE && (ub1)header->bufp[3] == 0xFF) {
                    return UCS_4BE_BOM;
                }
            }
        }

        if ((ub1)header->bufp[0] == 0xFF && (ub1)header->bufp[1] == 0xFE) {
            if (header->blen > 3) {
                if ((ub1)header->bufp[2] == 0x00 && (ub1)header->bufp[3] == 0x00) {
                    return UCS_4LE_BOM;
                }
            }
            return UCS_2LE_BOM;
        }

        if ((ub1)header->bufp[0] == 0xEF && (ub1)header->bufp[1] == 0xBB) {
            if (header->blen == 2) {
                /* need one more byte */
                return UCS_UTF8_BOM_ASK;
            }

            if ((ub1)header->bufp[2] == 0xBF) {
                return UCS_UTF8_BOM;
            }
        }
    }

    return UCS_NONE_BOM;
}


CONVDAPI int UCS_file_detect_bom(const char *textfile, CONVD_UCS_BOM *outbom)
{
    int bom = UCS_NONE_BOM;

    filehandle_t hf = file_open_read(textfile);

    if (hf != filehandle_invalid) {
        conv_buf_t headbuf;
        char header[4] = {0x00, 0x00, 0x00, 0x00};

        /* try to read first 4 bytes */
        int cb = file_readbytes(hf, header, 4);

        bom = UCS_text_detect_bom(conv_buf_set(&headbuf, header, cb));

        file_close(&hf);

        if (bom == UCS_UTF8_BOM_ASK) {
            *outbom = UCS_NONE_BOM;
        } else {
            *outbom = bom;
        }

        return CONVD_RET_NOERROR;
    }

    return CONVD_RET_EOPEN;
}


int XML_text_parse_head(const conv_buf_t *xmltext, conv_xmlhead_t *xmlhead, CONVD_UCS_BOM *bomtag)
{
    CONVD_UCS_BOM bom = UCS_text_detect_bom(xmltext);

    switch (bom) {
    case UCS_UTF8_BOM:
        break;

    case UCS_2BE_BOM:
        break;

    case UCS_2LE_BOM:
        break;

    case UCS_4BE_BOM:
        break;

    case UCS_4LE_BOM:
        break;

    case UCS_NONE_BOM:
    default:
        break;
    }

    if (bomtag) {
        *bomtag = bom;
    }

    return 0;
}


int XML_file_parse_head(const char *xmlfile, conv_xmlhead_t *xmlhead, CONVD_UCS_BOM *bomtag)
{
    filehandle_t hf = file_open_read(xmlfile);

    if (hf != filehandle_invalid) {
        conv_buf_t headbuf;
        char header[32 + CVD_ENCODING_LEN_MAX];

        /* try to read first 96 bytes */
        int cbhead = file_readbytes(hf, header, sizeof(header));

        file_close(&hf);

        return XML_text_parse_head(conv_buf_set(&headbuf, header, cbhead), xmlhead, bomtag);
    }

    return CONVD_RET_EOPEN;
}
