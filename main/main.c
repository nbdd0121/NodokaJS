#include "c/stdio.h"
#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "unicode/type.h"
#include "unicode/convert.h"

#include "js/js.h"
#include "js/bytecode.h"
#include "js/lex.h"

int main(int argc, char **argv) {
    size_t size;
    char *buffer = nodoka_readFile("./test.js", &size);

    nodoka_initConstant();

    nodoka_lex *lex = lex_new(buffer);
    nodoka_grammar *grammar = grammar_new(lex);
    nodoka_lex_class *ast = grammar_expr(grammar);

    nodoka_code_emitter *emitter = nodoka_newCodeEmitter();
    nodoka_codegen(emitter, ast);

    nodoka_emitBytecode(emitter, NODOKA_BC_RET);


    if (true) {
        /* Optimizer */
        nodoka_convPass(emitter);
        for (int i = 0; i < 10; i++) {
            bool mod = false;
            mod |= nodoka_peeholePass(emitter);
            mod |= nodoka_nopPass(emitter);
            if (!mod) {
                break;
            }
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

