#include "c/stdio.h"
#include "c/stdlib.h"

#include "js/lex.h"
#include "js/pass.h"

static void declgenEmpty(nodoka_code_emitter *emitter, nodoka_empty_node *node) {
    switch (node->type) {
        case NODOKA_DEBUGGER_STMT: {
            break;
        }
        default:
            assert(0);
    }
}

static void declgenUnary(nodoka_code_emitter *emitter, nodoka_unary_node *node) {
    switch (node->type) {
        case NODOKA_EXPR_STMT: {
            break;
        }
        default: assert(0);
    }
}

static void declgenBinary(nodoka_code_emitter *emitter, nodoka_binary_node *node) {
    switch (node->type) {
        case NODOKA_DO_STMT: {
            nodoka_declgen(emitter, node->_1);
            break;
        }
        case NODOKA_WHILE_STMT: {
            nodoka_declgen(emitter, node->_2);
            break;
        }
        default: assert(0);
    }
}

static void declgenTernary(nodoka_code_emitter *emitter, nodoka_ternary_node *node) {
    switch (node->type) {
        case NODOKA_IF_STMT: {
            nodoka_declgen(emitter, node->_2);
            nodoka_declgen(emitter, node->_3);
            break;
        }
        default: assert(0);
    }
}

static void declgen(nodoka_code_emitter *emitter, nodoka_node_list *node) {
    switch (node->type) {
        case NODOKA_RETURN_STMT: {
            /* must be function */
            break;
        }
        case NODOKA_THROW_STMT: {
            break;
        }
        case NODOKA_VAR_STMT: {
            nodoka_token *id = (nodoka_token *)node->_[0];
            nodoka_emitBytecode(emitter, NODOKA_BC_DECL, nodoka_newStringDup(id->stringValue));
            break;
        }
        case NODOKA_FUNC_DECL: {
            nodoka_token *id = (nodoka_token *)node->_[0];
            nodoka_emitBytecode(emitter, NODOKA_BC_DECL, nodoka_newStringDup(id->stringValue));
            nodoka_codegen(emitter, node->_[0]);
            nodoka_codegen(emitter, node->_[1]);
            nodoka_emitBytecode(emitter, NODOKA_BC_PUT);
            break;
        }
        case NODOKA_STMT_LIST: {
            for (int i = 0; i < node->length; i++) {
                nodoka_declgen(emitter, node->_[i]);
            }
            break;
        }
        case NODOKA_FOR_STMT: {
            nodoka_declgen(emitter, node->_[3]);
            break;
        }
        case NODOKA_FOR_VAR_STMT: {
            // TODO multiple kind of for
            nodoka_declgen(emitter, node->_[0]);
            nodoka_declgen(emitter, node->_[3]);
            break;
        }
        case NODOKA_TRY_STMT: {
            nodoka_declgen(emitter, node->_[0]);
            nodoka_declgen(emitter, node->_[2]);
            nodoka_declgen(emitter, node->_[3]);
            break;
        }
        default: assert(0);
    }
}


void nodoka_declgen(nodoka_code_emitter *emitter, nodoka_lex_class *node) {
    if (!node) {
        return;
    }
    switch (node->clazz) {
        case NODOKA_LEX_TOKEN: {
            break;
        }
        case NODOKA_LEX_EMPTY_NODE: {
            declgenEmpty(emitter, (nodoka_empty_node *)node);
            break;
        }
        case NODOKA_LEX_UNARY_NODE: {
            declgenUnary(emitter, (nodoka_unary_node *)node);
            break;
        }
        case NODOKA_LEX_BINARY_NODE: {
            declgenBinary(emitter, (nodoka_binary_node *)node);
            break;
        }
        case NODOKA_LEX_TERNARY_NODE: {
            declgenTernary(emitter, (nodoka_ternary_node *)node);
            break;
        }
        case NODOKA_LEX_NODE_LIST: {
            declgen(emitter, (nodoka_node_list *)node);
            break;
        }
        default: assert(0);
    }
}

