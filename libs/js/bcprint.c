#include "c/stdio.h"
#include "c/assert.h"

#include "js/js.h"
#include "js/bytecode.h"

#define DEFAULT_BC_LEN 65536
#define BC_INC_SIZE 128

void nodoka_printBytecode(nodoka_code *codeseg) {
    for (int i = 0; i < codeseg->bytecodeLength; i++) {
        enum nodoka_bytecode bc = codeseg->bytecode[i];
        switch (bc) {
            case NODOKA_BC_NOP: printf("NOP\n"); break;
            case NODOKA_BC_XCHG: printf("XCHG\n"); break;
            default: assert(0);
        }
    }
}
