#include "c/stdio.h"
#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "unicode/type.h"
#include "unicode/convert.h"

#include "js/js.h"
#include "js/bytecode.h"

int main() {
    /*long size;
    char *buffer = nodoka_readFile("./test.js", &size);*/

    nodoka_initConstant();

    nodoka_code *code = nodoka_loadBytecode("./bytecode.nbc");
    if (!code) {
        nodoka_code_emitter *seg = nodoka_newCodeEmitter();
        nodoka_emitBytecode(seg, NODOKA_BC_LOAD_NUM, 1.0);
        nodoka_emitBytecode(seg, NODOKA_BC_LOAD_NUM, 2.0);
        nodoka_emitBytecode(seg, NODOKA_BC_STR);
        nodoka_emitBytecode(seg, NODOKA_BC_ADD);
        nodoka_emitBytecode(seg, NODOKA_BC_RET);
        /*for (int i = 0; i < 10; i++) {
            bool mod = false;
            mod |= nodoka_peeholePass(seg);
            mod |= nodoka_nopPass(seg);
            if (!mod) {
                break;
            }
        }*/
        code = nodoka_packCode(seg);
    }

    //nodoka_storeBytecode("./bytecode.nbc", code);

    nodoka_printBytecode(code);

    nodoka_context *context = nodoka_newContext(code);
    nodoka_string *ret = nodoka_toString(nodoka_exec(context));

    printf("return ");
    unicode_putUtf16(ret->value);
    printf("\n");




    return 0;
}

