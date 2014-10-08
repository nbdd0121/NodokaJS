#include "c/stdio.h"
#include "c/stdint.h"
#include "c/stdlib.h"
#include "c/assert.h"

#include "unicode/type.h"
#include "unicode/convert.h"

#include "js/js.h"
#include "js/bytecode.h"

int main() {
    /* Read js from disk */
    FILE *f = fopen("./test.js", "r+");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    char *buffer = malloc(size + 1);
    rewind(f);
    size_t numread = fread(buffer, sizeof(char), size, f);
    assert(numread == size);
    buffer[size] = 0;

    nodoka_code_emitter *seg = nodoka_newCodeEmitter();
    nodoka_emitBytecode(seg, NODOKA_BC_XCHG);
    nodoka_emitBytecode(seg, NODOKA_BC_NOP);
    nodoka_emitBytecode(seg, NODOKA_BC_XCHG);

    for (int i = 0; i < 10; i++) {
        bool mod = false;
        mod |= nodoka_peeholePass(seg);
        mod |= nodoka_nopPass(seg);
        if (!mod) {
            break;
        }
    }

    nodoka_code *code = nodoka_packCode(seg);

    nodoka_printBytecode(code);



    return 0;
}

