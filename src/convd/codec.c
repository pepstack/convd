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


int conv_xmlhead_format(const conv_xmlhead_t *xmlhead, conv_buf_t *output)
{
    char headbuf[128];
    int headlen = snprintf_chkd_V1(headbuf, sizeof(headbuf), "<?xml version=\"%s\" encoding=\"%s\"?>", xmlhead->version, xmlhead->encoding);

    switch (xmlhead->bom) {
    case UCS_NONE_BOM:
        if (cstr_safecopy(output->bufp, output->blen, 0, headbuf, headlen)) {
            return headlen;
        }
        return 0;

    case UCS_UTF8_BOM:
        output->bufp[0] = (char)0xEF;
        output->bufp[1] = (char)0xBB;
        output->bufp[2] = (char)0xBF;
        if (cstr_safecopy(output->bufp, output->blen, 3, headbuf, headlen)) {
            return headlen + 3;
        }
        return 0;

    case UCS_2BE_BOM:
        if ((int)output->blen > headlen * 2 + 2) {
            ub2 be2;
            int i = 0, offcb = 2;
            output->bufp[0] = (char)0xFE;
            output->bufp[1] = (char)0xFF;
            for (; i < headlen; i++) {
                be2 = (ub2) BO_i16_htobe(headbuf[i]);
                memcpy(&output->bufp[i*2 + 2], &be2, sizeof(be2));
                offcb += sizeof(be2);
            }
            return offcb;
        }
        return 0;

    case UCS_2LE_BOM:
        if ((int)output->blen > headlen * 2 + 2) {
            ub2 le2;
            int i = 0, offcb = 2;
            output->bufp[0] = (char)0xFF;
            output->bufp[1] = (char)0xFE;
            for (; i < headlen; i++) {
                le2 = (ub2) BO_i16_htole(headbuf[i]);
                memcpy(&output->bufp[i*2 + 2], &le2, sizeof(le2));
                offcb += sizeof(le2);
            }
            return offcb;
        }
        return 0;

    case UCS_4BE_BOM:
         if ((int)output->blen > headlen * 4 + 4) {
            ub4 be4;
            int i = 0, offcb = 4;
            output->bufp[0] = (char)0x00;
            output->bufp[1] = (char)0x00;
            output->bufp[2] = (char)0xFE;
            output->bufp[3] = (char)0xFF;
            for (; i < headlen; i++) {
                be4 = (ub4) BO_i32_htobe(headbuf[i]);
                memcpy(&output->bufp[i*4 + 4], &be4, sizeof(be4));
                offcb += sizeof(be4);
            }
            return offcb;
        }
        return 0;

    case UCS_4LE_BOM:
        if ((int)output->blen > headlen * 4 + 4) {
            ub4 le4;
            int i = 0, offcb = 4;
            output->bufp[0] = (char)0xFF;
            output->bufp[1] = (char)0xFE;
            output->bufp[2] = (char)0x00;
            output->bufp[3] = (char)0x00;
            for (; i < headlen; i++) {
                le4 = (ub4) BO_i32_htole(headbuf[i]);
                memcpy(&output->bufp[i*4 + 4], &le4, sizeof(le4));
                offcb += sizeof(le4);
            }
            return offcb;
        }
        return 0;
    }

    /* unknown bom */
    return -1;    
}


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

        bom = UCS_text_detect_bom(convbufMake(&headbuf, header, cb));

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


int XML_text_parse_head(const conv_buf_t *xmltext, conv_xmlhead_t *xmlhead)
{
    int found = 0;
    int offset = 0;

    int headlen = 0;
    char headbuf[CVD_ENCODING_LEN_MAX + 16 + 32];

    memset(xmlhead, 0, sizeof(*xmlhead));
    xmlhead->bom = UCS_text_detect_bom(xmltext);

    switch (xmlhead->bom) {
    case UCS_2BE_BOM:
        offset = 2;
        while(offset + 1 < (int)xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_betoh_i16(&xmltext->bufp[offset]);
                offset += 2;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    found = 1;
                    break;
                }
            }
        }
        break;

    case UCS_2LE_BOM:
        offset = 2;
        while(offset + 1 < (int)xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_letoh_i16(&xmltext->bufp[offset]);
                offset += 2;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    found = 1;
                    break;
                }
            }
        }
        break;

    case UCS_4BE_BOM:
        offset = 4;
        while(offset + 3 < (int)xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_betoh_i32(&xmltext->bufp[offset]);
                offset += 4;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    found = 1;
                    break;
                }
            }
        }
        break;

    case UCS_4LE_BOM:
        offset = 4;
        while(offset + 3 < (int)xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_letoh_i32(&xmltext->bufp[offset]);
                offset += 4;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    found = 1;
                    break;
                }
            }
        } 
        break;

    case UCS_UTF8_BOM:
        offset = 3;
    case UCS_NONE_BOM:
    default:
        while(offset < (int)xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = xmltext->bufp[offset++];
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    found = 1;
                    break;
                }
            }
        }
        break;
    }

    if (found && headlen > 20 && cstr_startwith(headbuf, headlen, "<?xml ", 6) &&
        headbuf[headlen-2] == '?' && headbuf[headlen-1] == '>') {
        char *start, *end, *s;

        start = strstr(headbuf, "version=\"");
        if (start) {
            s = &start[9];
            end = strstr(s, "\"");
            if (end && (int)(end - s) > 1) {
                memcpy(xmlhead->version, s, (end - s));
                xmlhead->version[(end - s)] = 0;
            }

            start = strstr(headbuf, "encoding=\"");
            if (start) {
                s = &start[10];
                end = strstr(s, "\"");
                if (end && (int)(end - s) > 0 && (end - s) < sizeof(xmlhead->encoding)) {
                    memcpy(xmlhead->encoding, s, (end - s));
                    xmlhead->encoding[(end - s)] = 0;
                    cstr_toupper(xmlhead->encoding, (int)(end - s));
                }
            }

            /* 成功: 返回xml头的字节长度 */
            return offset;
        }
    }

    /* xml head not found */
    return 0;
}


int XML_file_parse_head(const char *xmlfile, conv_xmlhead_t *xmlhead)
{
    filehandle_t hf = file_open_read(xmlfile);

    if (hf != filehandle_invalid) {
        conv_buf_t input;
        char headbuf[CVD_ENCODING_LEN_MAX + 16 + 32];

        /* try to read head bytes */
        int headlen = file_readbytes(hf, headbuf, sizeof(headbuf));

        file_close(&hf);

        return XML_text_parse_head(convbufMake(&input, headbuf, headlen), xmlhead);
    }

    return (-1);
}