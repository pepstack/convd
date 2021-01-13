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

#include <iconv.h>

#include "convd_api.h"


typedef struct _conv_des_t
{
    iconv_t cd;
} conv_des_t;


#ifdef __cplusplus
}
#endif
#endif /* CONVD_INTER_H_ */
