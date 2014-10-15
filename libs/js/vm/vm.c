#include "c/assert.h"
#include "c/stdlib.h"
#include "c/math.h"

#include "util/double.h"

#include "unicode/hash.h"

#include "js/js.h"
#include "js/bytecode.h"
#include "js/object.h"

nodoka_context *nodoka_newContext(nodoka_code *code, nodoka_envRec *env, nodoka_object *this) {
    nodoka_context *context = malloc(sizeof(nodoka_context));
    context->env = env;
    context->code = code;
    context->stack = malloc(sizeof(nodoka_data *) * 128);
    context->stackTop = context->stack;
    context->stackLimit = context->stack + 128;
    context->this = this;
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

int8_t nodoka_absRelComp(nodoka_data *sp1, nodoka_data *sp0) {
    assertPrimitive(sp1);
    assertPrimitive(sp0);
    if (sp1->type != NODOKA_STRING || sp0->type != NODOKA_STRING) {
        nodoka_number *nx = (nodoka_number *)nodoka_toNumber(sp1);
        nodoka_number *ny = (nodoka_number *)nodoka_toNumber(sp0);
        if (isnan(nx->value) || isnan(ny->value)) {
            return -1;
        }
        if (nx->value < ny->value) {
            return 1;
        } else {
            return 0;
        }
    } else {
        nodoka_string *lstr = nodoka_toString(sp1);
        nodoka_string *rstr = nodoka_toString(sp0);
        int result = unicode_utf16Cmp(&lstr->value, &rstr->value);
        if (result < 0) {
            return 1;
        } else {
            return 0;
        }
    }
}

bool nodoka_sameValue(nodoka_data *x, nodoka_data *y) {
    if (x->type != y->type) {
        return false;
    }
    switch (x->type) {
        case NODOKA_UNDEF:
        case NODOKA_NULL:
            return true;
        case NODOKA_NUMBER: {
            nodoka_number *v0 = (nodoka_number *)x;
            nodoka_number *v1 = (nodoka_number *)y;
            if (isnan(v0->value) && isnan(v1->value)) {
                return true;
            }
            return double2int(v0->value) == double2int(v1->value);
        }
        case NODOKA_STRING:
            if (unicode_utf16Cmp(&((nodoka_string *)x)->value, &((nodoka_string *)y)->value) == 0) {
                return true;
            } else {
                return false;
            }
        case NODOKA_BOOL:
            if (x == y) {
                return true;
            } else {
                return false;
            }
        default:
            assert(0);
    }
}

bool nodoka_strictEqComp(nodoka_data *x, nodoka_data *y) {
    if (x->type != y->type) {
        return false;
    }
    switch (x->type) {
        case NODOKA_UNDEF:
        case NODOKA_NULL:
            return true;
        case NODOKA_NUMBER:
            if (((nodoka_number *)x)->value == ((nodoka_number *)y)->value) {
                return true;
            } else {
                return false;
            }
        case NODOKA_STRING:
            if (unicode_utf16Cmp(&((nodoka_string *)x)->value, &((nodoka_string *)y)->value) == 0) {
                return true;
            } else {
                return false;
            }
        case NODOKA_BOOL:
            if (x == y) {
                return true;
            } else {
                return false;
            }
        default:
            assert(0);
    }
}

bool nodoka_absEqComp(nodoka_data *x, nodoka_data *y) {
    if (x->type == y->type) {
        return nodoka_strictEqComp(x, y);
    } else if (x->type == NODOKA_NULL && y->type == NODOKA_UNDEF) {
        return true;
    } else if (x->type == NODOKA_UNDEF && y->type == NODOKA_NULL) {
        return true;
    } else if (x->type == NODOKA_NUMBER && y->type == NODOKA_STRING) {
        return nodoka_absEqComp(x, (nodoka_data *)nodoka_toNumber(y));
    } else if (x->type == NODOKA_STRING && y->type == NODOKA_NUMBER) {
        return nodoka_absEqComp((nodoka_data *)nodoka_toNumber(x), y);
    } else if (x->type == NODOKA_BOOL) {
        return nodoka_absEqComp((nodoka_data *)nodoka_toNumber(x), y);
    } else if (y->type == NODOKA_BOOL) {
        return nodoka_absEqComp(x, (nodoka_data *)nodoka_toNumber(y));
    } else if ((x->type == NODOKA_STRING || x->type == NODOKA_NUMBER) && y->type == NODOKA_OBJECT) {
        return nodoka_absEqComp(x, nodoka_toPrimitive(y));
    } else if ((y->type == NODOKA_STRING || y->type == NODOKA_NUMBER) && x->type == NODOKA_OBJECT) {
        return nodoka_absEqComp(nodoka_toPrimitive(x), y);
    } else {
        return false;
    }
}

static nodoka_data *getValue(nodoka_data *val) {
    if (val->type == NODOKA_REFERENCE) {
        nodoka_reference *ref = (nodoka_reference *)val;
        nodoka_data *result;
        /* Notice that ref->base = NULL indicates a environment record */
        if (ref->base && ref->base == nodoka_undefined) {
            assert(!"ReferenceError");
        }
        if (ref->base) {
            if (ref->base->type == NODOKA_OBJECT) {
                result = nodoka_get((nodoka_object *)ref->base, ref->name);
            } else {
                assert(0);
                //Special get, see ECMA-262
            }
        } else {
            //base must be an environment record.
            //a. Return the result of calling the GetBindingValue (see 10.2.1) concrete method of base passing
            //GetReferencedName(V) and IsStrictReference(V) as arguments.
            assert(0);
        }
        return result;
    } else {
        return val;
    }
}

static void throwError(nodoka_context *context, nodoka_data *data) {
    context->stackTop = context->stack;
    nodoka_push(context, data);
}

static void throwTypeError(nodoka_context *context) {
    throwError(context, (nodoka_data *)nodoka_newStringFromUtf8("TypeError"));
}

static enum nodoka_completion nodoka_stepExec(nodoka_context *context) {
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
        case NODOKA_BC_XCHG3: {
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_data *sp1 = nodoka_pop(context);
            nodoka_data *sp2 = nodoka_pop(context);
            nodoka_push(context, sp0);
            nodoka_push(context, sp2);
            nodoka_push(context, sp1);
            break;
        }
        case NODOKA_BC_RET: {
            return NODOKA_COMPLETION_RETURN;
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
        case NODOKA_BC_REF: {
            nodoka_string *sp0 = (nodoka_string *)nodoka_pop(context);
            nodoka_data *sp1 = nodoka_pop(context);
            assertString(sp0);
            nodoka_push(context, (nodoka_data *)nodoka_newReference(sp1, sp0));
            break;
        }
        case NODOKA_BC_GET: {
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_push(context, getValue(sp0));
            break;
        }
        case NODOKA_BC_PUT: {
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_reference *sp1 = (nodoka_reference *)nodoka_pop(context);
            assertType(sp1, NODOKA_REFERENCE);
            // If IsUnresolvableReference(V)
            //   If IsStrictReference(V) is true, then Throw ReferenceError exception.
            //   Call the [[Put]] internal method of the global object, passing GetReferencedName( V) for the property name, W for the value, and false for the Throw flag.
            if (true/*IsPropertyReference(V)*/) {
                if (true/*!HasPrimitiveBase(V)*/) {
                    //Strict?
                    nodoka_put((nodoka_object *)sp1->base, sp1->name, sp0, false/*Strict*/);
                } else {
                    assert(0);
                }
            } else {
                assert(0);
            }
            break;
        }
        case NODOKA_BC_DEL: {
            nodoka_data *sp0 = nodoka_pop(context);
            if (sp0->type != NODOKA_REFERENCE) {
                nodoka_push(context, nodoka_true);
                break;
            }
            /* A lot of check currently ignored */
            nodoka_reference *ref = (nodoka_reference *)sp0;
            bool result = nodoka_delete(nodoka_toObject(ref->base), ref->name, false);
            nodoka_push(context, result ? nodoka_true : nodoka_false);
            break;
        }
        case NODOKA_BC_CALL: {
            uint8_t count = fetchByte(context);
            nodoka_data **args = malloc(sizeof(nodoka_data *)*count);
            for (int i = count - 1; i >= 0; i--) {
                args[i] = nodoka_pop(context);
            }
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_object *func = (nodoka_object *)getValue(sp0);
            if (func->base.type != NODOKA_OBJECT || !func->call) {
                throwTypeError(context);
                return NODOKA_COMPLETION_THROW;
            }
            nodoka_data *this;
            if (sp0->type == NODOKA_REFERENCE) {
                nodoka_reference *ref = (nodoka_reference *)sp0;
                if (true) {
                    this = ref->base;
                } else {
                    //Env record
                    assert(0);
                }
            } else {
                this = nodoka_undefined;
            }
            nodoka_data *ret;
            enum nodoka_completion comp = nodoka_call(func, this, &ret, count, args);
            free(args);
            switch (comp) {
                case NODOKA_COMPLETION_THROW: {
                    throwError(context, ret);
                    return NODOKA_COMPLETION_THROW;
                }
                case NODOKA_COMPLETION_RETURN: {
                    nodoka_push(context, ret);
                    break;
                }
                default: assert(0);
            }
            break;
        }
        case NODOKA_BC_NEG: {
            nodoka_data *sp0 = nodoka_pop(context);
            assertNumber(sp0);
            nodoka_number *sp0num = (nodoka_number *)sp0;
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(-sp0num->value));
            break;
        }
        case NODOKA_BC_NOT: {
            nodoka_data *sp0 = nodoka_pop(context);
            assertNumber(sp0);
            nodoka_number *sp0num = (nodoka_number *)sp0;
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(~nodoka_toInt32(sp0num)));
            break;
        }
        case NODOKA_BC_L_NOT: {
            nodoka_data *sp0 = nodoka_pop(context);
            assertBoolean(sp0);
            nodoka_push(context, sp0 == nodoka_true ? nodoka_false : nodoka_true);
            break;
        }
        case NODOKA_BC_MUL: {
            nodoka_number *sp0 = (nodoka_number *)nodoka_pop(context);
            nodoka_number *sp1 = (nodoka_number *)nodoka_pop(context);
            assertNumber(sp1);
            assertNumber(sp0);
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(sp1->value * sp0->value));
            break;
        }
        case NODOKA_BC_MOD: {
            nodoka_number *sp0 = (nodoka_number *)nodoka_pop(context);
            nodoka_number *sp1 = (nodoka_number *)nodoka_pop(context);
            assertNumber(sp1);
            assertNumber(sp0);
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(fmod(sp1->value, sp0->value)));
            break;
        }
        case NODOKA_BC_DIV: {
            nodoka_number *sp0 = (nodoka_number *)nodoka_pop(context);
            nodoka_number *sp1 = (nodoka_number *)nodoka_pop(context);
            assertNumber(sp1);
            assertNumber(sp0);
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(sp1->value / sp0->value));
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
        case NODOKA_BC_SUB: {
            nodoka_number *sp0 = (nodoka_number *)nodoka_pop(context);
            nodoka_number *sp1 = (nodoka_number *)nodoka_pop(context);
            assertNumber(sp1);
            assertNumber(sp0);
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(sp1->value - sp0->value));
            break;
        }
        case NODOKA_BC_SHL: {
            nodoka_number *sp0 = (nodoka_number *)nodoka_pop(context);
            nodoka_number *sp1 = (nodoka_number *)nodoka_pop(context);
            assertNumber(sp1);
            assertNumber(sp0);
            int32_t lnum = nodoka_toInt32(sp1);
            uint32_t rnum = nodoka_toUint32(sp0) & 0x1F;
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(lnum << rnum));
            break;
        }
        case NODOKA_BC_SHR: {
            nodoka_number *sp0 = (nodoka_number *)nodoka_pop(context);
            nodoka_number *sp1 = (nodoka_number *)nodoka_pop(context);
            assertNumber(sp1);
            assertNumber(sp0);
            int32_t lnum = nodoka_toInt32(sp1);
            uint32_t rnum = nodoka_toUint32(sp0) & 0x1F;
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(lnum >> rnum));
            break;
        }
        case NODOKA_BC_USHR: {
            nodoka_number *sp0 = (nodoka_number *)nodoka_pop(context);
            nodoka_number *sp1 = (nodoka_number *)nodoka_pop(context);
            assertNumber(sp1);
            assertNumber(sp0);
            uint32_t lnum = nodoka_toUint32(sp1);
            uint32_t rnum = nodoka_toUint32(sp0) & 0x1F;
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(lnum >> rnum));
            break;
        }
        case NODOKA_BC_LT: {
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_data *sp1 = nodoka_pop(context);
            int8_t ret = nodoka_absRelComp(sp1, sp0);
            nodoka_push(context, (ret == 1) ? nodoka_true : nodoka_false);
            break;
        }
        case NODOKA_BC_LTEQ: {
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_data *sp1 = nodoka_pop(context);
            int8_t ret = nodoka_absRelComp(sp0, sp1);
            nodoka_push(context, (ret == 0) ? nodoka_true : nodoka_false);
            break;
        }
        case NODOKA_BC_EQ: {
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_data *sp1 = nodoka_pop(context);
            bool ret = nodoka_absEqComp(sp1, sp0);
            nodoka_push(context, ret ? nodoka_true : nodoka_false);
            break;
        }
        case NODOKA_BC_S_EQ: {
            nodoka_data *sp0 = nodoka_pop(context);
            nodoka_data *sp1 = nodoka_pop(context);
            bool ret = nodoka_strictEqComp(sp1, sp0);
            nodoka_push(context, ret ? nodoka_true : nodoka_false);
            break;
        }
        case NODOKA_BC_AND: {
            nodoka_number *sp0 = (nodoka_number *)nodoka_pop(context);
            nodoka_number *sp1 = (nodoka_number *)nodoka_pop(context);
            assertNumber(sp1);
            assertNumber(sp0);
            int32_t lnum = nodoka_toInt32(sp1);
            int32_t rnum = nodoka_toInt32(sp0);
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(lnum & rnum));
            break;
        }
        case NODOKA_BC_OR: {
            nodoka_number *sp0 = (nodoka_number *)nodoka_pop(context);
            nodoka_number *sp1 = (nodoka_number *)nodoka_pop(context);
            assertNumber(sp1);
            assertNumber(sp0);
            int32_t lnum = nodoka_toInt32(sp1);
            int32_t rnum = nodoka_toInt32(sp0);
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(lnum | rnum));
            break;
        }
        case NODOKA_BC_XOR: {
            nodoka_number *sp0 = (nodoka_number *)nodoka_pop(context);
            nodoka_number *sp1 = (nodoka_number *)nodoka_pop(context);
            assertNumber(sp1);
            assertNumber(sp0);
            int32_t lnum = nodoka_toInt32(sp1);
            int32_t rnum = nodoka_toInt32(sp0);
            nodoka_push(context, (nodoka_data *)nodoka_newNumber(lnum ^ rnum));
            break;
        }

        case NODOKA_BC_JMP: {
            uint16_t offset = fetch16(context);
            context->insPtr = offset;
            break;
        }
        case NODOKA_BC_JT: {
            nodoka_data *sp0 = nodoka_pop(context);
            assertBoolean(sp0);
            if (sp0 == nodoka_true) {
                uint16_t offset = fetch16(context);
                context->insPtr = offset;
            } else {
                context->insPtr += 2;
            }
            break;
        }
        case NODOKA_BC_THIS: {
            nodoka_push(context, (nodoka_data *)context->this);
            break;

        }
        case NODOKA_BC_THROW: {
            return NODOKA_COMPLETION_THROW;
        }
        default: assert(0);
    }
    return NODOKA_COMPLETION_NORMAL;
}

enum nodoka_completion nodoka_exec(nodoka_context *context, nodoka_data **retPtr) {
    enum nodoka_completion comp;
    while ((comp = nodoka_stepExec(context)) == NODOKA_COMPLETION_NORMAL);
    nodoka_data *ret = nodoka_pop(context);
    assert(context->stack == context->stackTop);
    if (retPtr)
        *retPtr = ret;
    return comp;
}
