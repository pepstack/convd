/**
 * @filename   testapp.c
 *   sample application shows how to use libconvd.
 *   字符集转换例子
 *
 * @author     Liang Zhang <350137278@qq.com>
 * @version    0.0.1
 * @create     2020-01-13 10:19:05
 * @update     2020-01-13 19:19:05
 */
#include "app_incl.h"

#include <convd/convd_api.h>

/* should changed as your want ! */
#define  NUMTHREADS   1
#define  MAXMSGCOUNT  (1000000 * 1)

void test_textfile();

static void do_convert(convd_t cvd, int num)
{
    char intext[] = "china中国";

    char outgbk[256];
    size_t outlen;

    int i = 0;
    conv_buf_t input, output;

    for (; i < num; i++) {
        outlen = convd_conv_text(cvd, convbufMake(&input, intext, strlen(intext)), convbufMake(&output, outgbk, sizeof(outgbk)));
        if (outlen == CONVD_RET_EICONV) {
            printf("convd_conv_text error(CONVD_RET_EICONV): %s\n", strerror(errno));
            break;
        }

        if (i % 100000 == 0) {
            printf("[%d] output={%.*s}\n", i, (int)outlen, outgbk);
        }
    }
}


static void * convd_thread (void *arg)
{
    convd_t cvd = (convd_t)arg;

    do_convert(cvd, MAXMSGCOUNT);

    convd_release(&cvd);
    return (void*) 0;
}




int main (int argc, const char *argv[])
{
    WINDOWS_CRTDBG_ON

    int err, i;
    size_t count;

    convd_t cvd;

    time_t t0, t1;

    pthread_t tids[NUMTHREADS] = {0};

    err = convd_create("UTF-8", "GB2312", 1, &cvd);
    if (err) {
        if (err == CONVD_RET_EOPEN) {
            printf("convd_create error(CONVD_RET_EOPEN): %s\n", strerror(errno));
        } else {
            printf("convd_create error(%d).\n", err);
        }

        exit(EXIT_FAILURE);
    }

    t0 = time(0);

    for (i = 0; i < NUMTHREADS; i++) {
        if (pthread_create(&tids[i], NULL, convd_thread, (void*)convd_retain(&cvd)) == -1) {
            printf("pthread_create failed.\n");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < NUMTHREADS; i++) {
        int err = pthread_join(tids[i], NULL);
        if (err) {
            printf("pthread_join error: %s.\n", strerror(err));
            exit(EXIT_FAILURE);
        }
    }

    t1 = time(0);
    count = (size_t)(NUMTHREADS * MAXMSGCOUNT);

    printf("Conversion from '%s' to '%s' end.\n", convd_fromcode(cvd), convd_tocode(cvd));
    printf("Elapsed: %d seconds. Speed: %d/Sec.\n\n", (int)(t1-t0), (int) (count/(t1-t0 + 0.01)));

    convd_release(&cvd);

    test_textfile();

    system("pause");
    return 0;
}


void test_textfile()
{
    CONVD_UCS_BOM bom;
    char *endp;

    int err, backslash = 0;
    cstrbuf apppath = get_proc_abspath();

    printf("%s path: %.*s\n\n", APPNAME, cstrbufGetLen(apppath), cstrbufGetStr(apppath));

    endp = strstr(apppath->str, "/test/");
    if (! endp) {
        backslash = -1;
        endp = strstr(apppath->str, "\\test\\");
        if (endp) {
            backslash = 1;
        }
    }

    if (backslash != -1) {
        cstrbuf textfile;

        endp += 6;
        *endp = 0;

        textfile = cstrbufCat(NULL, "%sucs-2be.txt", apppath->str);
        UCS_file_detect_bom(textfile->str, &bom);
        printf("bom=%d file=%.*s\n", bom, cstrbufGetLen(textfile), cstrbufGetStr(textfile));
        cstrbufFree(&textfile);
        assert(bom == UCS_2BE_BOM);

        textfile = cstrbufCat(NULL, "%sucs-2le.txt", apppath->str);
        UCS_file_detect_bom(textfile->str, &bom);
        printf("bom=%d file=%.*s\n", bom, cstrbufGetLen(textfile), cstrbufGetStr(textfile));
        cstrbufFree(&textfile);
        assert(bom == UCS_2LE_BOM);

        textfile = cstrbufCat(NULL, "%sutf-8bom.txt", apppath->str);
        UCS_file_detect_bom(textfile->str, &bom);
        printf("bom=%d file=%.*s\n", bom, cstrbufGetLen(textfile), cstrbufGetStr(textfile));
        cstrbufFree(&textfile);
        assert(bom == UCS_UTF8_BOM);

        textfile = cstrbufCat(NULL, "%sutf-8.txt", apppath->str);
        UCS_file_detect_bom(textfile->str, &bom);
        printf("bom=%d file=%.*s\n", bom, cstrbufGetLen(textfile), cstrbufGetStr(textfile));
        cstrbufFree(&textfile);
        assert(bom == UCS_NONE_BOM);

        textfile = cstrbufCat(NULL, "%sgb2312.txt", apppath->str);
        UCS_file_detect_bom(textfile->str, &bom);
        printf("bom=%d file=%.*s\n", bom, cstrbufGetLen(textfile), cstrbufGetStr(textfile));
        cstrbufFree(&textfile);
        assert(bom == UCS_NONE_BOM);

       // textfile = cstrbufCat(NULL, "%sutf8bom.xml", apppath->str);
       // rc = encoding_detect_xmlfile(textfile->str, encoding, &bom);
       // if (rc == CONVD_RET_NOERROR) {
       //     printf("bom=%d encoding=%s file=%.*s\n", bom, encoding, cstrbufGetLen(textfile), cstrbufGetStr(textfile));
       // }
       // cstrbufFree(&textfile);

        conv_xmlhead_t xmlhead;
        textfile = cstrbufCat(NULL, "%sutf8.xml", apppath->str);
        if (XML_file_parse_head(textfile->str, &xmlhead, NULL) > 0) {
            printf("<?xml version=\"%s\" encoding=\"%s\"?>\n", xmlhead.version, xmlhead.encoding);
        }
        cstrbufFree(&textfile);

        textfile = cstrbufCat(NULL, "%sgbk.xml", apppath->str);
        if (XML_file_parse_head(textfile->str, &xmlhead, NULL) > 0) {
            printf("<?xml version=\"%s\" encoding=\"%s\"?>\n", xmlhead.version, xmlhead.encoding);
        }
        cstrbufFree(&textfile);

        textfile = cstrbufCat(NULL, "%sgb2312.xml", apppath->str);
        if (XML_file_parse_head(textfile->str, &xmlhead, NULL) > 0) {
            printf("<?xml version=\"%s\" encoding=\"%s\"?>\n", xmlhead.version, xmlhead.encoding);
        }

        XML_file_encode(textfile->str, "C:\\TEMP\\DEBUG\\utf8out.xml", "UTF-8", 0);

        XML_file_encode("C:\\TEMP\\DEBUG\\utf8out.xml", "C:\\TEMP\\DEBUG\\gbkout.xml", "gb2312", 0);

        XML_file_encode("C:\\TEMP\\DEBUG\\gbkout.xml", "C:\\TEMP\\DEBUG\\utf8out2.xml", "UTF-8", 0);

        cstrbufFree(&textfile);

        convd_t cvd;
        err = convd_create("GB18030", "UTF-8", 1, &cvd);
        textfile = cstrbufCat(NULL, "%sgb2312.xml", apppath->str);
        convd_conv_file(cvd, textfile->str, "C:\\TEMP\\DEBUG\\utf8.xml", 0);
        cstrbufFree(&textfile);
        convd_release(&cvd);
    }
    cstrbufFree(&apppath);
}