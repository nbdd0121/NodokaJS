#include "c/stdio.h"
#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/assert.h"
#include "c/stdbool.h"

#include "unicode/type.h"
#include "unicode/convert.h"

#include "js/js.h"
#include "js/bytecode.h"
#include "js/lex.h"
#include "js/pass.h"
#include "js/object.h"
#include "js/builtin.h"

struct nodoka_config nodoka_config = {
    .peehole = true,
    .conv = true,
    .fold = true,
};

int main(int argc, char **argv) {

    bool dispBytecode = false;
    bool printResult = false;


    char *path = NULL;
    char *output = NULL;

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (arg[0] == '-') {
            if (arg[1] == '-') {
                char *name = &arg[2];
                bool s = true;
                if (name[0] == 'n' && name[1] == 'o' && name[2] == '-') {
                    s = false;
                    name += 3;
                }
                if (strcmp(name, "peehole") == 0) {
                    nodoka_config.peehole = s;
                } else if (strcmp(name, "conversion-optimizer") == 0) {
                    nodoka_config.conv = s;
                } else if (strcmp(name, "constant-folding") == 0) {
                    nodoka_config.fold = s;
                }  else if (strcmp(name, "optimizer") == 0) {
                    nodoka_config.peehole = nodoka_config.conv = nodoka_config.fold = s;
                } else if (strcmp(name, "print-bytecode") == 0) {
                    dispBytecode = s;
                } else if (strcmp(name, "print-result") == 0) {
                    printResult = s;
                } else {
                    printf("NodokaJS: Unknown Option %s\n", arg);
                }
            } else {
                switch (arg[1]) {
                    case 'O': {
                        nodoka_config.peehole = nodoka_config.conv = nodoka_config.fold = true;
                        break;
                    }
                    case 'o': {
                        output = argv[++i];
                        break;
                    }
                    default:
                        printf("NodokaJS: Unknown Option %s\n", arg);
                }
            }
        } else {
            path = arg;
        }
    }

    if (!path) {
        printf("NodokaJS: No file specified\n");
        return 1;
    }

    nodoka_initConstant();

    size_t size;
    char *buffer = nodoka_readFile(path, &size);
    if (!buffer) {
        printf("NodokaJS: Unable to load file from '%s'\n", path);
        return 1;
    }

    nodoka_global global;
    nodoka_newGlobal(&global);

    {
        size_t size;
        char *buffer = nodoka_readFile("startup.js", &size);
        if (!buffer) {
            printf("NodokaJS: Unable to load startup files\n");
            return 1;
        }

        utf16_string_t str = unicode_toUtf16(UTF8_STRING(buffer));
        nodoka_code *code = nodoka_compile(str);
        free(str.str);

        nodoka_envRec *env = nodoka_newObjEnvRecord(global.global, NULL);
        nodoka_context *context = nodoka_newContext(&global, env, code, global.global);

        nodoka_data *retVal;
        if (nodoka_exec(context, &retVal) == NODOKA_COMPLETION_THROW) {
            nodoka_string *retStr = nodoka_toString(context, retVal);
            /* Result */
            printf("\033[1;31mUncaught ");
            unicode_putUtf16(retStr->value);
            printf("\033[0m\n");
        }
    }

    nodoka_code *code;
    if (buffer[0]) {
        utf16_string_t str = unicode_toUtf16(UTF8_STRING(buffer));
        free(buffer);
        code = nodoka_compile(str);
        free(str.str);
    } else {
        free(buffer);
        code = nodoka_loadBytecode(path);
    }

    if (output)
        nodoka_storeBytecode(output, code);

    if (dispBytecode) {
        nodoka_printBytecode(code, 0);
    }

    nodoka_envRec *env = nodoka_newObjEnvRecord(global.global, NULL);
    nodoka_context *context = nodoka_newContext(&global, env, code, global.global);

    nodoka_data *retVal;
    enum nodoka_completion comp = nodoka_exec(context, &retVal);
    switch (comp) {
        case NODOKA_COMPLETION_RETURN: {
            if (printResult) {
                nodoka_data *ret;
                nodoka_colorDir(context, NULL, NULL, &ret, 1, (nodoka_data *[1]) {
                    retVal
                });
            }
            break;
        }
        case NODOKA_COMPLETION_THROW: {
            nodoka_string *retStr = nodoka_toString(context, retVal);
            /* Result */
            fprintf(stderr, "\033[1;31mUncaught ");
            unicode_fputUtf16(stderr, retStr->value);
            fprintf(stderr, "\033[0m\n");
            return -1;
        }
        default: assert(0);
    }

    return 0;
}

