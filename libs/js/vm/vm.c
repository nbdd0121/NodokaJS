#include "c/assert.h"
#include "c/stdlib.h"

#include "util/double.h"

#include "js/js.h"
#include "js/bytecode.h"

nodoka_context *nodoka_newContext(nodoka_code *code) {
    nodoka_context *context = malloc(sizeof(nodoka_context));
    context->code = code;
    context->stack = malloc(sizeof(nodoka_data *) * 128);
    context->stackTop = context->stack;
    context->stackLimit = context->stack + 128;
    context->insPtr = 0;
    return context;
}

static nodoka_data *nodoka_pop(nodoka_context *context) {
    assert(context->stackTop > context->stack);
    nodoka_data *ret = *(--context->stackTop);
    return ret;
}

static void nodoka_push(nodoka_context *context, nodoka_data *data) {
    assert(context->stackTop < context->stackLimit);
    *(context->stackTop++) = data;
}

static uint8_t fetchByte(nodoka_context *context) {
    assert(context->insPtr < context->code->bytecodeLength);
    return context->code->bytecode[context->insPtr++];
}

static uint16_t fetch16(nodoka_context *context) {
    uint16_t ret = fetchByte(context) << 8;
    ret |= fetchByte(context);
    return ret;
}

static uint32_t fetch32(nodoka_context *context) {
    uint32_t ret = fetch16(context) << 16;
    ret |= fetch16(context);
    return ret;
}

static uint64_t fetch64(nodoka_context *context) {
    uint64_t ret = (uint64_t)fetch32(context) << 32;
    ret |= fetch32(context);
    return ret;
}

void *nodoka_stepExec(nodoka_context *context) {
    assert(context->insPtr < context->code->bytecodeLength);
    uint8_t bc = fetchByte(context);
    switch (bc) {
        case NODOKA_BC_UNDEF: {
            nodoka_push(context, nodoka_undefined);
            break;
        }
        case NODOKA_BC_NULL: {
            nodoka_push(context, nodoka_null);
            break;
        }
        case NODOKA_BC_TRUE: {
            nodoka_push(context, nodoka_true);
            break;
        }
        case NODOKA_BC_FALSE: {
            nodoka_push(context, nodoka_false);
            break;
        }
        case NODOKA_BC_LOAD_STR: {
            uint16_t imm16 = fetch16(context);
            nodoka_push(context, (nodoka_data *)context->code->stringPool[imm16]);
            break;
        }
        case NODOKA_BC_LOAD_NUM: {
            double val = int2double(fetch64(context));
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(val));
            break;
        }
        case NODOKA_BC_NOP: {
            break;
        }
        case NODOKA_BC_DUP: {
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_push(context, sp0);
            nodoka_push(context, sp0);
            break;
        }
        case NODOKA_BC_POP: {
            nodoka_pop(context);
            break;
        }
        case NODOKA_BC_XCHG: {
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_data *sp1 = nodoka_pop(context);
            nodoka_push(context, sp0);
            nodoka_push(context, sp1);
            break;
        }
        case NODOKA_BC_RET: {
            nodoka_data *sp0 = nodoka_pop(context);
            return sp0;
        }
        case NODOKA_BC_PRIM: {
            nodoka_push(context, nodoka_toPrimitive(nodoka_pop(context)));
            break;
        }
        case NODOKA_BC_BOOL: {
            nodoka_push(context, nodoka_toBoolean(nodoka_pop(context)));
            break;
        }
        case NODOKA_BC_NUM: {
            nodoka_push(context, (nodoka_data *)nodoka_toNumber(nodoka_pop(context)));
            break;
        }
        case NODOKA_BC_STR: {
            nodoka_push(context, (nodoka_data *)nodoka_toString(nodoka_pop(context)));
            break;
        }
        case NODOKA_BC_ADD: {
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_data *sp1 = nodoka_pop(context);
            assertPrimitive(sp1);
            assertPrimitive(sp0);
            if (sp1->type == NODOKA_STRING || sp0->type == NODOKA_STRING) {
                nodoka_string *lstr = nodoka_toString(sp1);
                nodoka_string *rstr = nodoka_toString(sp0);
                nodoka_push(context, (nodoka_data *)nodoka_concatString(lstr, rstr));
            } else {
                nodoka_number *lnum = nodoka_toNumber(sp1);
                nodoka_number *rnum = nodoka_toNumber(sp0);
                nodoka_push(context, (nodoka_data *)nodoka_newNumber(lnum->value + rnum->value));
            }
            break;
        }
        default: assert(0);
    }
    return NULL;
}

void *nodoka_exec(nodoka_context *context) {
    void *ret;
    while (!(ret = nodoka_stepExec(context))) {
    }
    return ret;
}
