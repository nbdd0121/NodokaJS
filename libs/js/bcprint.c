#include "c/stdio.h"
#include "c/assert.h"

#include "unicode/convert.h"
#include "util/double.h"

#include "js/js.h"
#include "js/bytecode.h"

#define DEFAULT_BC_LEN 65536
#define BC_INC_SIZE 128

static uint8_t fetchByte(nodoka_code *code, size_t *ptr) {
    return code->bytecode[(*ptr)++];
}

static uint16_t fetch16(nodoka_code *context, size_t *ptr) {
    uint16_t ret = fetchByte(context, ptr) << 8;
    ret |= fetchByte(context, ptr);
    return ret;
}

static uint32_t fetch32(nodoka_code *context, size_t *ptr) {
    uint32_t ret = fetch16(context, ptr) << 16;
    ret |= fetch16(context, ptr);
    return ret;
}

static uint64_t fetch64(nodoka_code *context, size_t *ptr) {
    uint64_t ret = (uint64_t)fetch32(context, ptr) << 32;
    ret |= fetch32(context, ptr);
    return ret;
}

void nodoka_printBytecode(nodoka_code *codeseg, int indent) {
#define DECL_OP(op) case NODOKA_BC_##op: printf(#op); break
    if (codeseg->name && codeseg->name->value.len) {
        printf("%*sName: ", indent, "");
        unicode_putUtf16(codeseg->name->value);
        printf("\n");
    }
    if (codeseg->formalParameters.length) {
        printf("%*sFormalParameters: [", indent, "");
        for (int i = 0; i < codeseg->formalParameters.length; i++) {
            if (i != 0) {
                printf(", ");
            }
            unicode_putUtf16(codeseg->formalParameters.array[i]->value);
        }
        printf("]\n");
    }
    if (codeseg->strPoolLength) {
        printf("%*sString Pool:\n", indent, "");
        for (int i = 0; i < codeseg->strPoolLength; i++) {
            printf("%*s[%d] = \"", indent + 2, "", i);
            unicode_putUtf16(codeseg->stringPool[i]->value);
            printf("\"\n");
        }
    }
    if (codeseg->codePoolLength) {
        printf("%*sCode Pool:\n", indent, "");
        for (int i = 0; i < codeseg->codePoolLength; i++) {
            printf("%*s[%d] = \n", indent + 2, "", i);
            nodoka_printBytecode(codeseg->codePool[i], indent + 4);
        }
    }
    printf("%*sBytecode:\n", indent, "");
    for (size_t i = 0; i < codeseg->bytecodeLength; ) {
        enum nodoka_bytecode bc = fetchByte(codeseg, &i);
        printf("%*s%5d ", indent, "", i - 1);
        switch (bc) {
            case NODOKA_BC_LOAD_STR: {
                uint16_t index = fetch16(codeseg, &i);
                printf("LOAD_STR #%d (\"", index);
                unicode_putUtf16(codeseg->stringPool[index]->value);
                printf("\")");
                break;
            }
            case NODOKA_BC_LOAD_NUM: {
                printf("LOAD_NUM %lf", int2double(fetch64(codeseg, &i)));
                break;
            }
            case NODOKA_BC_FUNC: {
                uint16_t index = fetch16(codeseg, &i);
                printf("FUNC #%d", index);
                break;
            }
            case NODOKA_BC_TRY: {
                uint16_t index = fetch16(codeseg, &i);
                printf("TRY #%d", index);
                break;
            }
            case NODOKA_BC_CALL: {
                printf("CALL %d", fetchByte(codeseg, &i));
                break;
            }
            case NODOKA_BC_NEW: {
                printf("NEW %d", fetchByte(codeseg, &i));
                break;
            }
            case NODOKA_BC_JT: {
                printf("JT %d", fetch16(codeseg, &i));
                break;
            }
            case NODOKA_BC_JMP: {
                printf("JMP %d", fetch16(codeseg, &i));
                break;
            }
            case NODOKA_BC_CATCH: {
                uint16_t index = fetch16(codeseg, &i);
                printf("CATCH %d", index);
                break;
            }
            case NODOKA_BC_DECL: {
                uint16_t index = fetch16(codeseg, &i);
                printf("DECL #%d (\"", index);
                unicode_putUtf16(codeseg->stringPool[index]->value);
                printf("\")");
                break;
            }
            DECL_OP(UNDEF);
            DECL_OP(NULL);
            DECL_OP(TRUE);
            DECL_OP(FALSE);
            DECL_OP(LOAD_OBJ);
            DECL_OP(LOAD_ARR);
            DECL_OP(NOP);
            DECL_OP(DUP);
            DECL_OP(POP);
            DECL_OP(XCHG);
            DECL_OP(RET);
            DECL_OP(BOOL);
            DECL_OP(PRIM);
            DECL_OP(NUM);
            DECL_OP(STR);
            DECL_OP(REF);
            DECL_OP(ID);
            DECL_OP(GET);
            DECL_OP(PUT);
            DECL_OP(DEL);
            DECL_OP(TYPEOF);
            DECL_OP(NEG);
            DECL_OP(NOT);
            DECL_OP(L_NOT);
            DECL_OP(MUL);
            DECL_OP(MOD);
            DECL_OP(DIV);
            DECL_OP(ADD);
            DECL_OP(SUB);
            DECL_OP(SHL);
            DECL_OP(SHR);
            DECL_OP(USHR);
            DECL_OP(LT);
            DECL_OP(LTEQ);
            DECL_OP(EQ);
            DECL_OP(S_EQ);
            DECL_OP(AND);
            DECL_OP(OR);
            DECL_OP(XOR);

            DECL_OP(THIS);

            DECL_OP(XCHG3);

            DECL_OP(THROW);
            DECL_OP(NOCATCH);


            default: assert(0);
        }
        printf("\n");
    }

#undef DECL_OP
}
