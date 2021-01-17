/**
 * @file:
 *   libconvd internal header.
 *   http://www.voidcn.com/article/p-pxsttpnn-tt.html
 *
 * @author:
 * @version:
 * @create: $create$
 * @update:
 */
#ifndef CONVD_INTER_H_
#define CONVD_INTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <common/mscrtdbg.h>
#include <common/unitypes.h>
#include <common/memapi.h>
#include <common/emerglog.h>
#include <common/refcobject.h>
#include <common/bo.h>

#include <iconv.h>

#include "convd_api.h"

#define CONVD_ERROR_ICONV    ((iconv_t)(-1))

#define CONVD_ERROR_SIZE     ((size_t)(-1))

#define CONV_CONTINUE       1
#define CONV_FINISHED       0
#define CONV_EBREAK       (-1)


typedef struct _conv_descriptor_t
{
    iconv_t cd;

    int tocodeat;
    char codebuf[0];
} conv_descriptor_t;


typedef struct _conv_position_t
{
    filehandle_t textfd;
    CONVD_UCS_BOM bom;
    sb8 offset;
} conv_position_t, *convpos_t;


#ifdef __cplusplus
}
#endif
#endif /* CONVD_INTER_H_ */
