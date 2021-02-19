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
 * @filename   enum_attrs.h
 *    enumerable attributes.
 *
 * @author     Liang Zhang <350137278@qq.com>
 * @version    0.0.10
 * @create     2021-02-17 11:12:10
 * @update     2021-02-17 22:55:37
 */
#ifndef _ATTRS_H_
#define _ATTRS_H_

#if defined(__cplusplus)
extern "C"
{
#endif

#include "memapi.h"
#include "cstrbuf.h"

typedef struct _enum_attrs_t
{
    const char **attrsname;
    int attrsnum;
    cstrbuf attrsval[0];
} enum_attrs_t, *enum_attrs;


NOWARNING_UNUSED(static)
enum_attrs enum_attrs_new(const char **attrsname, int attrsnum)
{
    enum_attrs attrs = (enum_attrs_t *) mem_alloc_zero(1, sizeof(enum_attrs_t) + sizeof(cstrbuf) * attrsnum);
    attrs->attrsname = attrsname;
    attrs->attrsnum = attrsnum;
    return attrs;
}


NOWARNING_UNUSED(static)
void enum_attrs_free(enum_attrs attrs)
{
    while (attrs->attrsnum-- > 0) {
        cstrbuf csb = attrs->attrsval[attrs->attrsnum];
        if (csb) {
            cstrbufFree(&csb);
        }
    }
    mem_free(attrs);
}


NOWARNING_UNUSED(static)
int enum_attrs_set(enum_attrs attrs, int attrid, const char *attrvalue, ub4 valuelen)
{
    if (attrid >= 0 && attrid < attrs->attrsnum) {
        cstrbuf csb = attrs->attrsval[attrid];
        if (csb) {
            cstrbufFree(&csb);
        }
        attrs->attrsval[attrid] = cstrbufNew(0, attrvalue, valuelen);
        return 0;
    }
    return (-1);
}


NOWARNING_UNUSED(static)
int enum_attrs_get(enum_attrs attrs, int attrid, const char **attrnameout, const char **attrvalueout)
{
    if (attrid >= 0 && attrid < attrs->attrsnum) {
        cstrbuf csb = attrs->attrsval[attrid];

        if (attrnameout) {
            *attrnameout = attrs->attrsname[attrid];
        }

        if (attrvalueout) {
            *attrvalueout = cstrbufGetStr(csb);
        }

        /* length of attr value */
        return cstrbufGetLen(csb);
    }

    return (-1);
}


NOWARNING_UNUSED(static)
int enum_attrs_to_xml(enum_attrs attrs, char *outxmlbuf, ub4 xmlbufsize)
{
    int attrid = 0;
    int offlen = 0;

    if (!outxmlbuf || !xmlbufsize) {
        for (; attrid < attrs->attrsnum; attrid++) {
            const char *attrname;
            const char *attrvalue;
            int valuelen = enum_attrs_get(attrs, attrid, &attrname, &attrvalue);
            int namelen = cstr_length(attrname, 255);
            offlen += (namelen * 2 + 8 + valuelen);
        }
        return offlen;
    }

    for (; attrid < attrs->attrsnum; attrid++) {
        const char *attrname;
        const char *attrvalue;
        int valuelen = enum_attrs_get(attrs, attrid, &attrname, &attrvalue);
        int namelen = cstr_length(attrname, 255);

        if (xmlbufsize - offlen > namelen * 2 + 8 + valuelen) {
            offlen += snprintf_chkd_V1(outxmlbuf + offlen, xmlbufsize - offlen, "  <%*s>%*s</%*s>\n", namelen, attrname, valuelen, attrvalue, namelen, attrname);
        } else {
            /* insufficent outxmlbuf */
            return 0;
        }
    }

    return offlen;
}


#ifdef __cplusplus
}
#endif
#endif /* _ATTRS_H_ */
