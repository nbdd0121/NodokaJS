#include "c/stdlib.h"

#include "js/js.h"
#include "js/bytecode.h"

enum {
    DEF_BC_CAPACITY = 256,
    DEF_STR_POOL_CAPACITY = 16,
    DEF_CODE_POOL_CAPACITY = 0,
    BC_CAPACITY_INC_STEP = 128,
    STR_POOL_CAPACITY_INC_STEP = 16,
    CODE_POOL_CAPACITY_INC_STEP = 1,
};

nodoka_code_emitter *nodoka_newCodeEmitter(void) {
    nodoka_code_emitter *seg = malloc(sizeof(nodoka_code_emitter));
    seg->stringPool = malloc(DEF_STR_POOL_CAPACITY * sizeof(nodoka_string *));
    seg->codePool = malloc(DEF_CODE_POOL_CAPACITY * sizeof(nodoka_code *));
    seg->bytecode = malloc(DEF_BC_CAPACITY);
    seg->strPoolLength = 0;
    seg->codePoolLength = 0;
    seg->bytecodeLength = 0;
    seg->strPoolCapacity = DEF_STR_POOL_CAPACITY;
    seg->codePoolCapacity = DEF_CODE_POOL_CAPACITY;
    seg->bytecodeCapacity = DEF_BC_CAPACITY;
    return seg;
}

void nodoka_emitBytecode(nodoka_code_emitter *codeseg, uint8_t bc) {
    if (codeseg->bytecodeLength == codeseg->bytecodeCapacity) {
        codeseg->bytecodeCapacity += BC_CAPACITY_INC_STEP;
        codeseg->bytecode = realloc(codeseg->bytecode, codeseg->bytecodeCapacity);
    }
    codeseg->bytecode[codeseg->bytecodeLength++] = bc;
}

nodoka_code *nodoka_packCode(nodoka_code_emitter *emitter) {
    nodoka_code *code = nodoka_new_data(NODOKA_CODE);
    code->stringPool = emitter->stringPool;
    code->codePool = emitter->codePool;
    code->bytecode = emitter->bytecode;
    code->strPoolLength = emitter->strPoolLength;
    code->codePoolLength = emitter->codePoolLength;
    code->bytecodeLength = emitter->bytecodeLength;
    return code;
}
