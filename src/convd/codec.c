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


int XML_text_parse_head(const conv_buf_t *xmltext, conv_xmlhead_t *xmlhead, CONVD_UCS_BOM *bomtag)
{
    int flag = 0;
    size_t offset = 0;

    int headlen = 0;
    char headbuf[64 + CVD_ENCODING_LEN_MAX];
 
    CONVD_UCS_BOM bom = UCS_text_detect_bom(xmltext);

    if (bomtag) {
        *bomtag = bom;
    }

    memset(xmlhead, 0, sizeof(*xmlhead));

    switch (bom) {
    case UCS_2BE_BOM:
        offset = 2;
        while(offset + 1 < xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_betoh_i16(&xmltext->bufp[offset]);
                offset += 2;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    flag = 1;
                    break;
                }
            }
        }
        break;

    case UCS_2LE_BOM:
        offset = 2;
        while(offset + 1 < xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_letoh_i16(&xmltext->bufp[offset]);
                offset += 2;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    flag = 1;
                    break;
                }
            }
        }
        break;

    case UCS_4BE_BOM:
        offset = 4;
        while(offset + 3 < xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_betoh_i32(&xmltext->bufp[offset]);
                offset += 4;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    flag = 1;
                    break;
                }
            }
        }
        break;

    case UCS_4LE_BOM:
        offset = 4;
        while(offset + 3 < xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_letoh_i32(&xmltext->bufp[offset]);
                offset += 4;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    flag = 1;
                    break;
                }
            }
        } 
        break;

    case UCS_UTF8_BOM:
        offset = 3;
    case UCS_NONE_BOM:
    default:
        while(offset < xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = xmltext->bufp[offset++];
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    flag = 1;
                    break;
                }
            }
        }
        break;
    }

    if (flag && headlen > 20 && cstr_startwith(headbuf, headlen, "<?xml ", 6) &&
        headbuf[headlen-2] == '?' && headbuf[headlen-1] == '>') {
        char *s, *end;

        s = strstr(headbuf, "version=\"");
        if (s) {
            end = strstr(s + 9, "\"");
            if (end && (int)(end - s) < 4) {
                strncpy(xmlhead->version, s + 9, (end - s));
                xmlhead->encoding[sizeof(xmlhead->version) - 1] = 0;
            }

            s = strstr(headbuf, "encoding=\"");
            if (s) {
                end = strstr(s + 10, "\"");
                if (end && (int)(end - s) > 0 && (end - s) < sizeof(xmlhead->encoding)) {
                    strncpy(xmlhead->encoding, s + 10, (end - s));
                    xmlhead->encoding[sizeof(xmlhead->encoding) - 1] = 0;
                }
            }

            return offset;
        }
    }

    /* xml head not found */
    return (-1);
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

        return XML_text_parse_head(convbufMake(&headbuf, header, cbhead), xmlhead, bomtag);
    }

    return CONVD_RET_EOPEN;
}

// TODO:
int XML_text_encode(conv_buf_t *xmltextin, conv_buf_t *xmltextout, const char *encoding, CONVD_SUFFIX_MODE suffix)
{
    conv_xmlhead_t xmlhead;
    int bodyat = XML_text_parse_head(xmltextin, &xmlhead, NULL);
    if (bodyat > 0) {
        convd_t cvd;
        int ret = convd_create(xmlhead.encoding, "UTF-8", suffix, &cvd);

        cstrbuf utf8buf = cstrbufNew(xmltextin->blen * 4, NULL, 0);
        conv_buf_t cvbin, cvbout;

        utf8buf->len = (int) convd_conv_text(cvd, xmltextin, convbufMake(&cvbout, utf8buf->str, utf8buf->maxsz));

        bodyat = XML_text_parse_head(convbufMake(&cvbout, utf8buf->str, utf8buf->len), &xmlhead, NULL);

        if (stricmp(encoding, "UTF-8")) {
            convd_t cvd2;
            convd_create("UTF-8", encoding, suffix, &cvd2);

            cstrbuf outbuf = cstrbufNew(utf8buf->len * 4, NULL, 0);

            outbuf->len = convd_conv_text(cvd2,
                convbufMake(&cvbin, &utf8buf->str[bodyat], utf8buf->len - bodyat),
                convbufMake(&cvbout, outbuf->str, outbuf->maxsz));

            
        }
    }
/*
        convd_t cvd;
        conv_buf_t cvbin, cvbout;

        if (convd_create("UTF-8", xmlhead.encoding, 0, &cvd) == 0) {
            char utf8headbuf[32 + CVD_ENCODING_LEN_MAX];
            int headbuflen = snprintf_chkd_V1(utf8headbuf, sizeof(utf8headbuf), "<?xml version=\"%s\" encoding=\"%s\"?>", xmlhead.version, encoding);

            headbuflen = (int)convd_conv_text(cvd, convbufMake(&cvbin, utf8headbuf, headbuflen), convbufMake(&cvbout, xmltextout->bufp, xmltextout->blen));

            convd_release(&cvd);

            if (headbuflen > 32) {
                if (convd_create(xmlhead.encoding, encoding, suffix, &cvd) == 0) {
                    len = (int) convd_conv_text(cvd,
                                    convbufMake(&cvbin, &xmltextin->bufp[len], xmltextin->blen - len),
                                    convbufMake(&cvbout, &xmltextout->bufp[headbuflen], xmltextout->blen - headbuflen));

                    convd_release(&cvd);

                    if (len != -1) {
                        xmltextout->blen = headbuflen + len;
                        return (int) xmltextout->blen;
                    }
                }
            }
        }
    }

    
*/return (-1);
}


int XML_file_encode(const char *xmlfile, const char *toxmlfile, const char *encoding, CONVD_SUFFIX_MODE suffix)
{
    filehandle_t hf = file_open_read(xmlfile);

    if (hf != filehandle_invalid) {
        sb8 sz = file_size(hf);
        if (sz > cstr_length_maximum / 4) {
            file_close(&hf);
            return (-1);
        }

        cstrbuf inbuf = cstrbufNew((ub4)sz, 0, 0);
        cstrbuf outbuf = cstrbufNew((ub4)sz*4, 0, 0);

        inbuf->len = (ub4) file_readbytes(hf, inbuf->str, inbuf->maxsz);

        file_close(&hf);

        if (inbuf->len == sz) {
            conv_buf_t cvbin, cvbout;

            outbuf->len = (ub4) XML_text_encode(convbufMake(&cvbin, inbuf->str, inbuf->len), convbufMake(&cvbout, outbuf->str, outbuf->maxsz), encoding, suffix);

            cstrbufFree(&inbuf);

            if (outbuf->len > 0) {
                filehandle_t outhf = file_write_new(toxmlfile);
            
                if (outhf != filehandle_invalid) {
                    if (file_writebytes(outhf, outbuf->str, outbuf->len) == 0) {
                        /* all success */
                        file_close(&outhf);
                        sz = outbuf->len;
                        cstrbufFree(&outbuf);
                        return (int) sz;
                    }
                }

                file_close(&outhf);
            }
        }

        cstrbufFree(&inbuf);
        cstrbufFree(&outbuf);
    }

    return (-1);
}