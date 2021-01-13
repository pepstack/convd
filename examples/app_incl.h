/**
 * @filename   app_incl.h
 *
 * @author     Liang Zhang <350137278@qq.com>
 * @version    0.0.1
 * @create     2020-01-13 10:19:05
 * @update     2020-01-13 19:19:05
 */
#include <common/mscrtdbg.h>
#include <common/unitypes.h>
#include <common/emerglog.h>

/* using pthread or pthread-w32 */
#include <sched.h>
#include <pthread.h>

#ifdef __WINDOWS__
    # include "win32/getoptw.h"

    # if !defined(__MINGW__)
        // link to pthread-w32 lib for MS Windows
        #pragma comment(lib, "pthreadVC2.lib")

        // link to libconvd.lib for MS Windows
        #pragma comment(lib, "libconvd.lib")
    # endif
#else
    // Linux: see Makefile
    # include <getopt.h>
#endif

#define  APPNAME     "testapp"
#define  APPVER      "1.0.0"
