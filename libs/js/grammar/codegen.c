#include "js/lex.h"

static void codegenToken(nodoka_code_emitter *emitter, nodoka_token *node) {
    switch (node->type) {
        case NODOKA_TOKEN_STR: {
            nodoka_emitBytecode(emitter, NODOKA_BC_LOAD_STR, nodoka_new_string(node->stringValue));
            break;
        }
        case NODOKA_TOKEN_NUM: {
            nodoka_emitBytecode(emitter, NODOKA_BC_LOAD_NUM, node->numberValue);
            break;
        }
        case NODOKA_TOKEN_NULL: nodoka_emitBytecode(emitter, NODOKA_BC_NULL); break;
        case NODOKA_TOKEN_TRUE: nodoka_emitBytecode(emitter, NODOKA_BC_TRUE); break;
        case NODOKA_TOKEN_FALSE: nodoka_emitBytecode(emitter, NODOKA_BC_FALSE); break;
        default: assert(0);
    }
}

static void commonUnary(nodoka_code_emitter *emitter, nodoka_unary_node *node) {
    nodoka_codegen(emitter, node->_1);
    nodoka_emitBytecode(emitter, NODOKA_BC_GET);
}

static void codegenUnary(nodoka_code_emitter *emitter, nodoka_unary_node *node) {
    switch (node->type) {
        case NODOKA_VOID_NODE: {
            commonUnary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_POP);
            nodoka_emitBytecode(emitter, NODOKA_BC_UNDEF);
            break;
        }
        case NODOKA_POS_NODE: {
            commonUnary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            break;
        }
        case NODOKA_NEG_NODE: {
            commonUnary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_NEG);
            break;
        }
        case NODOKA_NOT_NODE: {
            commonUnary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_NOT);
            break;
        }
        case NODOKA_LNOT_NODE: {
            commonUnary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
            nodoka_emitBytecode(emitter, NODOKA_BC_L_NOT);
            break;
        }
        default: assert(0);
    }
}

static void commonBinary(nodoka_code_emitter *emitter, nodoka_binary_node *node) {
    nodoka_codegen(emitter, node->_1);
    nodoka_emitBytecode(emitter, NODOKA_BC_GET);
    nodoka_codegen(emitter, node->_2);
    nodoka_emitBytecode(emitter, NODOKA_BC_GET);
}

static void codegenBinary(nodoka_code_emitter *emitter, nodoka_binary_node *node) {
    switch (node->type) {
        case NODOKA_MUL_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_MUL);
            break;
        }
        case NODOKA_MOD_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_MOD);
            break;
        }
        case NODOKA_DIV_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_DIV);
            break;
        }
        case NODOKA_ADD_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_ADD);
            break;
        }
        case NODOKA_SUB_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_SUB);
            break;
        }
        default: assert(0);
    }
}

void nodoka_codegen(nodoka_code_emitter *emitter, nodoka_lex_class *node) {
    switch (node->clazz) {
        case NODOKA_LEX_TOKEN: {
            codegenToken(emitter, (nodoka_token *)node);
            break;
        }
        case NODOKA_LEX_UNARY_NODE: {
            codegenUnary(emitter, (nodoka_unary_node *)node);
            break;
        }
        case NODOKA_LEX_BINARY_NODE: {
            codegenBinary(emitter, (nodoka_binary_node *)node);
            break;
        }
        default: assert(0);
    }
}
