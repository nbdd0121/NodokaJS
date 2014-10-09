#include "c/stdlib.h"
#include "c/stdarg.h"

#include "util/double.h"

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

static void nodoka_emit8(nodoka_code_emitter *codeseg, uint8_t val) {
    if (codeseg->bytecodeLength == codeseg->bytecodeCapacity) {
        codeseg->bytecodeCapacity += BC_CAPACITY_INC_STEP;
        codeseg->bytecode = realloc(codeseg->bytecode, codeseg->bytecodeCapacity);
    }
    codeseg->bytecode[codeseg->bytecodeLength++] = val;
}

static void nodoka_emit16(nodoka_code_emitter *emitter, uint16_t val) {
    nodoka_emit8(emitter, (val >> 8) & 0xFF);
    nodoka_emit8(emitter, val & 0xFF);
}

static void nodoka_emit32(nodoka_code_emitter *emitter, uint32_t val) {
    nodoka_emit16(emitter, (val >> 16) & 0xFFFF);
    nodoka_emit16(emitter, val & 0xFFFF);
}

static void nodoka_emit64(nodoka_code_emitter *emitter, uint64_t val) {
    nodoka_emit32(emitter, (val >> 32) & 0xFFFFFFFF);
    nodoka_emit32(emitter, val & 0xFFFFFFFF);
}

static uint16_t nodoka_emitString(nodoka_code_emitter *emitter, nodoka_string *str) {
    if (emitter->strPoolLength == emitter->strPoolCapacity) {
        emitter->strPoolCapacity += STR_POOL_CAPACITY_INC_STEP;
        emitter->stringPool = realloc(emitter->stringPool, emitter->strPoolCapacity);
    }
    uint16_t id = emitter->strPoolLength++;
    emitter->stringPool[id] = str;
    return id;
}

void nodoka_emitBytecode(nodoka_code_emitter *emitter, uint8_t bc, ...) {
    nodoka_emit8(emitter, bc);
    va_list ap;
    va_start(ap, bc);
    switch (bc) {
        case NODOKA_BC_LOAD_STR: {
            nodoka_string *str = va_arg(ap, nodoka_string *);
            uint16_t imm16 = nodoka_emitString(emitter, str);
            nodoka_emit16(emitter, imm16);
            break;
        }
        case NODOKA_BC_LOAD_NUM: {
            double val = va_arg(ap, double);
            uint64_t imm64 = double2int(val);
            nodoka_emit64(emitter, imm64);
            break;
        }
    }
    va_end(ap);
}

nodoka_code *nodoka_packCode(nodoka_code_emitter *emitter) {
    nodoka_code *code = (nodoka_code *)nodoka_new_data(NODOKA_CODE);
    code->stringPool = emitter->stringPool;
    code->codePool = emitter->codePool;
    code->bytecode = emitter->bytecode;
    code->strPoolLength = emitter->strPoolLength;
    code->codePoolLength = emitter->codePoolLength;
    code->bytecodeLength = emitter->bytecodeLength;
    return code;
}

nodoka_code_emitter *nodoka_unpackCode(nodoka_code *code) {
    nodoka_code_emitter *emitter = malloc(sizeof(nodoka_code_emitter));
    emitter->stringPool = code->stringPool;
    emitter->codePool = code->codePool;
    emitter->bytecode = code->bytecode;
    emitter->strPoolCapacity = emitter->strPoolLength = code->strPoolLength;
    emitter->codePoolCapacity = emitter->codePoolLength = code->codePoolLength;
    emitter->bytecodeCapacity = emitter->bytecodeLength = code->bytecodeLength;
    return emitter;
}
