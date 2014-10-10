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

int main(int argc, char **argv) {

    bool peehole = true;
    bool conv = true;

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (arg[0] == '-') {
            char *name = &arg[1];
            bool s = true;
            if (arg[1] == 'n' && arg[2] == 'o' && arg[3] == '-') {
                s = false;
                name += 3;
            }
            if (strcmp(name, "peehole-optimizer") == 0) {
                peehole = s;
            } else if (strcmp(name, "conversion-optimizer") == 0) {
                conv = s;
            } else if (strcmp(name, "optimizer") == 0) {
                peehole = conv = s;
            } else {
                printf("NodokaJS: Unknown Option %s\n", arg);
            }
        }
    }


    nodoka_initConstant();

    size_t size;
    char *buffer = nodoka_readFile("./test.js", &size);

    nodoka_lex *lex = lex_new(buffer);
    free(buffer);

    nodoka_grammar *grammar = grammar_new(lex);
    nodoka_lex_class *ast = grammar_expr(grammar);

    nodoka_code_emitter *emitter = nodoka_newCodeEmitter();
    nodoka_codegen(emitter, ast);
    nodoka_disposeLexNode(ast);

    nodoka_emitBytecode(emitter, NODOKA_BC_RET);

    for (int i = 0; i < 10; i++) {
        bool mod = false;
        if (conv) {
            mod |= nodoka_intraPcr(emitter, nodoka_convPass);
        }
        if (peehole) {
            mod |= nodoka_intraPcr(emitter, nodoka_peeholePass);
        }
        if (!mod) {
            break;
        }
    }


    nodoka_code *code = nodoka_packCode(emitter);

    //nodoka_storeBytecode("./bytecode.nbc", code);

    nodoka_printBytecode(code);

    nodoka_context *context = nodoka_newContext(code);
    nodoka_string *ret = nodoka_toString(nodoka_exec(context));

    printf("return ");
    unicode_putUtf16(ret->value);
    printf("\n");




    return 0;
}

