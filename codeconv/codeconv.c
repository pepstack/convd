/**
 * @filename   codeconv.c
 *   command tool for charset code conversion
 *   字符集转换命令行工具
 *
 * @author     Liang Zhang <350137278@qq.com>
 * @version    0.0.1
 * @create     2020-01-23 10:19:05
 * @update     2020-01-24 19:19:05
 */
#include "app_incl.h"

#include <convd/convd_api.h>


int main (int argc, char *argv[])
{
    WINDOWS_CRTDBG_ON

    int ret;

    convd_t cvd;

    parse_arguments(argc, argv);

    if (! from_code) {
        printf("[ERROR] Parameter Not Specified: --from-code=?\n");
        exit(-1);
    }

    if (! to_code) {
        printf("[ERROR] Parameter Not Specified: --to-code=?\n");
        exit(-1);
    }

    if (! input_file) {
        printf("[ERROR] Parameter Not Specified: --input-file=?\n");
        exit(-1);
    }

    if (! output_file) {
        printf("[ERROR] Parameter Not Specified: --output-file=?\n");
        exit(-1);
    }

    cstr_endwith(to_code->str, to_code->len, "//IGNORE", 8);

    cstr_endwith(to_code->str, to_code->len, "//TRANSLIT", 10);

    if (cstr_endwith(to_code->str, to_code->len, "//IGNORE", 8)) {
        to_code = cstrbufTrunc(to_code, to_code->len - 8);
        ret = convd_create(from_code->str, to_code->str, CVD_SUFFIX_IGNORE, &cvd);
    } else if (cstr_endwith(to_code->str, to_code->len, "//TRANSLIT", 10)) {
        to_code = cstrbufTrunc(to_code, to_code->len - 10);
        ret = convd_create(from_code->str, to_code->str, CVD_SUFFIX_TRANSLIT, &cvd);
    } else {
        ret = convd_create(from_code->str, to_code->str, CVD_SUFFIX_IGNORE, &cvd);
    }

    if (ret != CONVD_RET_NOERROR) {
        printf("[ERROR] convd_create error(%d)\n", ret);
        exit(-1);
    }

    if (cstr_endwith(input_file->str, input_file->len, ".xml", 4)) {
        ret = convd_conv_xmlfile(cvd, input_file->str, output_file->str, 1024, NULL, add_bom);
        if (ret != 0) {
            printf("[ERROR] convd_conv_xmlfile error(%d).\n", ret);
            exit(-1);
        }
    } else {
        ret = convd_conv_file(cvd, input_file->str, 0, output_file->str, 0, 0, 1024, NULL, add_bom);
        if (ret != 0) {
            printf("[ERROR] convd_conv_file error(%d).\n", ret);
            exit(-1);
        }
    }

    convd_release(&cvd);
    cstrbufFree(&from_code);
    cstrbufFree(&to_code);
    cstrbufFree(&input_file);
    cstrbufFree(&output_file);

    printf("[SUCCESS] file converted successfully.\n");
    return 0;
}
