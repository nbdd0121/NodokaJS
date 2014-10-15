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

int main(int argc, char **argv) {

    bool peehole = true;
    bool conv = true;
    bool fold = true;

    bool dispBytecode = false;


    char *path = NULL;

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
                    peehole = s;
                } else if (strcmp(name, "conversion-optimizer") == 0) {
                    conv = s;
                } else if (strcmp(name, "constant-folding") == 0) {
                    fold = s;
                }  else if (strcmp(name, "optimizer") == 0) {
                    peehole = conv = fold = s;
                } else if (strcmp(name, "output-bytecode") == 0) {
                    dispBytecode = s;
                } else {
                    printf("NodokaJS: Unknown Option %s\n", arg);
                }
            } else {
                switch (arg[1]) {
                    case 'O': {
                        peehole = conv = fold = true;
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

    /* Syntax */
    nodoka_lex *lex = lex_new(buffer);
    free(buffer);
    nodoka_grammar *grammar = grammar_new(lex);
    nodoka_lex_class *ast = grammar_stmt(grammar);

    /* Codegen */
    nodoka_code_emitter *emitter = nodoka_newCodeEmitter();
    nodoka_emitBytecode(emitter, NODOKA_BC_UNDEF);
    nodoka_codegen(emitter, ast);
    nodoka_disposeLexNode(ast);
    nodoka_emitBytecode(emitter, NODOKA_BC_RET);

    /* Optimize */
    for (int i = 0; i < 10; i++) {
        bool mod = false;
        if (conv) {
            mod |= nodoka_intraPcr(emitter, nodoka_convPass);
        }
        if (fold) {
            mod |= nodoka_intraPcr(emitter, nodoka_foldPass);
        }
        if (peehole) {
            mod |= nodoka_intraPcr(emitter, nodoka_peeholePass);
        }
        if (!mod) {
            break;
        }
    }

    /* Execute */
    nodoka_code *code = nodoka_packCode(emitter);

    if (dispBytecode) {
        nodoka_printBytecode(code);
    }

    nodoka_object *global = nodoka_newGlobal();

    nodoka_envRec *env = nodoka_newObjEnvRecord(global, NULL);

    nodoka_context *context = nodoka_newContext(code, env, global);
    nodoka_data *retVal;
    enum nodoka_completion comp = nodoka_exec(context, &retVal);
    nodoka_string *retStr = nodoka_toString(retVal);
    switch (comp) {
        case NODOKA_COMPLETION_RETURN: {
            /* Result */
            char *color = "";
            switch (retVal->type) {
                case NODOKA_UNDEF: color = "\033[2;37m"; break;
                case NODOKA_NULL: color = "\033[1;39m"; break;
                case NODOKA_NUMBER: case NODOKA_BOOL: color = "\033[0;33m"; break;
                case NODOKA_STRING: color = "\033[0;32m"; break;
            }
            printf("> %s", color);
            unicode_putUtf16(retStr->value);
            printf("\033[0m\n");
            break;
        }
        case NODOKA_COMPLETION_THROW: {
            /* Result */
            printf("\033[1;31mUncaught ");
            unicode_putUtf16(retStr->value);
            printf("\033[0m\n");
            break;
        }
        default: assert(0);
    }

    return 0;
}

