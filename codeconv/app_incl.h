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

#define  APPNAME     "codeconv"
#define  APPVERS     "1.0.0"

cstrbuf from_code = NULL;
cstrbuf to_code = NULL;
cstrbuf input_file = NULL;
cstrbuf output_file = NULL;
int add_bom = 0;

static void print_version()
{
    #ifdef NDEBUG
        #ifdef DEBUG
        # undef DEBUG
        #endif
        
        #ifdef _DEBUG
        # undef _DEBUG
        #endif

        printf("\033[47;35m%s, NDEBUG Version: %s, Build: %s %s\033[0m\n",
            APPNAME, APPVERS, __DATE__, __TIME__);
    #else
        #ifndef DEBUG
        # define DEBUG
        #endif
        
        #ifndef _DEBUG
        # define _DEBUG
        #endif

        printf("\033[47;35m%s, DEBUG Version: %s, Build: %s %s\033[0m\n",
            APPNAME, APPVERS, __DATE__, __TIME__);
    #endif

    exit(0);
}


static void print_usage()
{
    printf("\033[35mUsage:\033[0m %s [Options]\n", APPNAME);

    printf("\033[35m  command tool for text file encoding conversion.\033[0m\n");

    printf("\033[35mOptions:\033[0m\n");
    printf("  %-30s  \033[35m show app help \033[0m\n", "-h, --help");
    printf("  %-30s  \033[35m show app version \033[0m\n", "-V, --version");

    printf("  %-30s  \033[35m from charset encoding\033[0m\n", "--from-code=ENCODING");
    printf("  %-30s  \033[35m convert to charset encoding\033[0m\n", "--to-code=ENCODING");
    printf("  %-30s  \033[35m input path file\033[0m\n", "--input-file=FILE");
    printf("  %-30s  \033[35m output path file\033[0m\n", "--output-file=FILE");
    printf("  %-30s  \033[35m add bom head to output file. (not supported)\033[0m\n", "--add-bom");

    printf("Example: codeconv.exe --from-code=gbk --to-code=utf-8 --input-file=/path/to/textfile.in --output-file=/path/to/textfile.out\n");

    exit(0);
}


static void parse_arguments (int argc, char *argv[])
{
    int opt, optindex, flag;
    
    const struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {"from-code", required_argument, &flag, 3 },
        {"to-code", required_argument, &flag, 4 },
        {"input-file", required_argument, &flag, 5 },
        {"output-file", required_argument, &flag, 6 },
        {"add-bom", no_argument, &flag, 7 },
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long_only(argc, argv, "hV", longopts, &optindex)) != -1) {
        switch (opt) {
        case '?':
            fprintf(stderr, "\033[1;31m[error]\033[0m option not defined.\n");
            exit(EXIT_FAILURE);
            break;
        
        case 'h':
            print_usage();
            break;
        
        case 'V':
            print_version();
            break;
        
        case 0:
            switch(flag) {
            case 3:
                from_code = cstrbufNew(0, optarg, -1);
                break;
            case 4:
                to_code = cstrbufNew(0, optarg, -1);
                break;
            case 5:
                input_file = cstrbufNew(0, optarg, -1);
                break;
            case 6:
                output_file = cstrbufNew(0, optarg, -1);
                break;
            case 7:
                add_bom = 1;
                break;
            }
            break;
        }
    }
}
