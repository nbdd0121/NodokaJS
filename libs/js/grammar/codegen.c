#include "c/stdio.h"

#include "js/lex.h"

static void commonCodegen(nodoka_code_emitter *emitter, nodoka_node_list *node) {
    for (int i = 0; i < node->length; i++) {
        nodoka_codegen(emitter, node->_[i]);
        nodoka_emitBytecode(emitter, NODOKA_BC_GET);
    }
}

static void codegenToken(nodoka_code_emitter *emitter, nodoka_token *node) {
    switch (node->type) {
        case NODOKA_TOKEN_ID: {
            nodoka_emitBytecode(emitter, NODOKA_BC_LOAD_STR, nodoka_new_string(node->stringValue));
            nodoka_emitBytecode(emitter, NODOKA_BC_ID);
            node->stringValue.str = NULL;
            break;
        }
        case NODOKA_TOKEN_STR: {
            nodoka_emitBytecode(emitter, NODOKA_BC_LOAD_STR, nodoka_new_string(node->stringValue));
            node->stringValue.str = NULL;
            break;
        }
        case NODOKA_TOKEN_NUM: {
            nodoka_emitBytecode(emitter, NODOKA_BC_LOAD_NUM, node->numberValue);
            break;
        }
        case NODOKA_TOKEN_NULL: nodoka_emitBytecode(emitter, NODOKA_BC_NULL); break;
        case NODOKA_TOKEN_TRUE: nodoka_emitBytecode(emitter, NODOKA_BC_TRUE); break;
        case NODOKA_TOKEN_FALSE: nodoka_emitBytecode(emitter, NODOKA_BC_FALSE); break;
        case NODOKA_TOKEN_THIS: nodoka_emitBytecode(emitter, NODOKA_BC_THIS); break;
        default: assert(0);
    }
}

static void codegenEmpty(nodoka_code_emitter *emitter, nodoka_empty_node *node) {
    switch (node->type) {
        case NODOKA_DEBUGGER_STMT: {
            printf("[Warning] Debugger statement currently not supported, thus ignored.\n");
            break;
        }
        default:
            assert(0);
    }
}

static void commonUnary(nodoka_code_emitter *emitter, nodoka_unary_node *node) {
    nodoka_codegen(emitter, node->_1);
    nodoka_emitBytecode(emitter, NODOKA_BC_GET);
}

static void codegenUnary(nodoka_code_emitter *emitter, nodoka_unary_node *node) {
    switch (node->type) {
        case NODOKA_DELETE_NODE: {
            nodoka_codegen(emitter, node->_1);
            nodoka_emitBytecode(emitter, NODOKA_BC_DEL);
            break;
        }
        case NODOKA_TYPEOF_NODE: {
            commonUnary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_TYPEOF);
            break;
        }
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
        case NODOKA_POST_INC_NODE: {
            nodoka_codegen(emitter, node->_1);
            /* a check */
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG3);
            nodoka_emitBytecode(emitter, NODOKA_BC_LOAD_NUM, 1.0);
            nodoka_emitBytecode(emitter, NODOKA_BC_ADD);
            nodoka_emitBytecode(emitter, NODOKA_BC_PUT);
            break;
        }
        case NODOKA_POST_DEC_NODE: {
            nodoka_codegen(emitter, node->_1);
            /* a check */
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG3);
            nodoka_emitBytecode(emitter, NODOKA_BC_LOAD_NUM, 1.0);
            nodoka_emitBytecode(emitter, NODOKA_BC_SUB);
            nodoka_emitBytecode(emitter, NODOKA_BC_PUT);
            break;
        }
        case NODOKA_PRE_INC_NODE: {
            nodoka_codegen(emitter, node->_1);
            /* a check */
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_LOAD_NUM, 1.0);
            nodoka_emitBytecode(emitter, NODOKA_BC_ADD);
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG3);
            nodoka_emitBytecode(emitter, NODOKA_BC_PUT);
            break;
        }
        case NODOKA_PRE_DEC_NODE: {
            nodoka_codegen(emitter, node->_1);
            /* a check */
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_LOAD_NUM, 1.0);
            nodoka_emitBytecode(emitter, NODOKA_BC_SUB);
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG3);
            nodoka_emitBytecode(emitter, NODOKA_BC_PUT);
            break;
        }

        case NODOKA_EXPR_STMT: {
            /* Remove the previous-set completion */
            nodoka_emitBytecode(emitter, NODOKA_BC_POP);
            commonUnary(emitter, node);
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

static void commonBinaryNum(nodoka_code_emitter *emitter, nodoka_binary_node *node) {
    commonBinary(emitter, node);
    nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
    nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
    nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
    nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
}

static void codegenBinary(nodoka_code_emitter *emitter, nodoka_binary_node *node) {
    switch (node->type) {
        case NODOKA_MEMBER_NODE: {
            commonBinary(emitter, node);
            /* Call CheckObjectCoercible(baseValue). */
            nodoka_emitBytecode(emitter, NODOKA_BC_STR);
            nodoka_emitBytecode(emitter, NODOKA_BC_REF);
            break;
        }

        case NODOKA_CALL_NODE: {
            nodoka_codegen(emitter, node->_1);
            nodoka_node_list *list = (nodoka_node_list *)node->_2;
            size_t count;
            if (list) {
                commonCodegen(emitter, list);
                count = list->length;
            } else {
                count = 0;
            }
            nodoka_emitBytecode(emitter, NODOKA_BC_CALL, count);
            break;
        }


        case NODOKA_MUL_NODE: {
            commonBinaryNum(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_MUL);
            break;
        }
        case NODOKA_MOD_NODE: {
            commonBinaryNum(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_MOD);
            break;
        }
        case NODOKA_DIV_NODE: {
            commonBinaryNum(emitter, node);
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
            commonBinaryNum(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_SUB);
            break;
        }
        case NODOKA_SHL_NODE: {
            commonBinaryNum(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_SHL);
            break;
        }
        case NODOKA_SHR_NODE: {
            commonBinaryNum(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_SHR);
            break;
        }
        case NODOKA_USHR_NODE: {
            commonBinaryNum(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_USHR);
            break;
        }
        case NODOKA_LT_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_LT);
            break;
        }
        case NODOKA_GT_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_LT);
            break;
        }
        case NODOKA_LTEQ_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_LTEQ);
            break;
        }
        case NODOKA_GTEQ_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_LTEQ);
            break;
        }
        case NODOKA_EQ_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_EQ);
            break;
        }
        case NODOKA_INEQ_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_EQ);
            nodoka_emitBytecode(emitter, NODOKA_BC_L_NOT);
            break;
        }
        case NODOKA_STRICT_EQ_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_S_EQ);
            break;
        }
        case NODOKA_STRICT_INEQ_NODE: {
            commonBinary(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_S_EQ);
            nodoka_emitBytecode(emitter, NODOKA_BC_L_NOT);
            break;
        }
        case NODOKA_AND_NODE: {
            commonBinaryNum(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_AND);
            break;
        }
        case NODOKA_OR_NODE: {
            commonBinaryNum(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_OR);
            break;
        }
        case NODOKA_XOR_NODE: {
            commonBinaryNum(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_XOR);
            break;
        }
        case NODOKA_L_AND_NODE: {
            nodoka_codegen(emitter, node->_1);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
            nodoka_emitBytecode(emitter, NODOKA_BC_L_NOT);
            nodoka_relocatable rel;
            nodoka_emitBytecode(emitter, NODOKA_BC_JT, &rel);
            nodoka_emitBytecode(emitter, NODOKA_BC_POP);
            nodoka_codegen(emitter, node->_2);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_relocate(emitter, rel, nodoka_putLabel(emitter));
            break;
        }
        case NODOKA_L_OR_NODE: {
            nodoka_codegen(emitter, node->_1);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
            nodoka_relocatable rel;
            nodoka_emitBytecode(emitter, NODOKA_BC_JT, &rel);
            nodoka_emitBytecode(emitter, NODOKA_BC_POP);
            nodoka_codegen(emitter, node->_2);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_relocate(emitter, rel, nodoka_putLabel(emitter));
            break;
        }

        case NODOKA_ASSIGN_NODE: {
            nodoka_codegen(emitter, node->_1);
            nodoka_codegen(emitter, node->_2);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG3);
            /* A check */
            nodoka_emitBytecode(emitter, NODOKA_BC_PUT);
            break;
        }

        case NODOKA_ADD_ASSIGN_NODE: {
            nodoka_codegen(emitter, node->_1);
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_codegen(emitter, node->_2);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_PRIM);
            nodoka_emitBytecode(emitter, NODOKA_BC_ADD);
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG3);
            /* A check */
            nodoka_emitBytecode(emitter, NODOKA_BC_PUT);
            break;
        }
        case NODOKA_MUL_ASSIGN_NODE:
        case NODOKA_MOD_ASSIGN_NODE:
        case NODOKA_DIV_ASSIGN_NODE:
        case NODOKA_SUB_ASSIGN_NODE:
        case NODOKA_SHL_ASSIGN_NODE:
        case NODOKA_SHR_ASSIGN_NODE:
        case NODOKA_USHR_ASSIGN_NODE:
        case NODOKA_AND_ASSIGN_NODE:
        case NODOKA_OR_ASSIGN_NODE:
        case NODOKA_XOR_ASSIGN_NODE: {
            nodoka_codegen(emitter, node->_1);
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_codegen(emitter, node->_2);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG);
            nodoka_emitBytecode(emitter, NODOKA_BC_NUM);
            switch (node->type) {
                case NODOKA_MUL_ASSIGN_NODE: nodoka_emitBytecode(emitter, NODOKA_BC_MUL); break;
                case NODOKA_MOD_ASSIGN_NODE: nodoka_emitBytecode(emitter, NODOKA_BC_MOD); break;
                case NODOKA_DIV_ASSIGN_NODE: nodoka_emitBytecode(emitter, NODOKA_BC_DIV); break;
                case NODOKA_SUB_ASSIGN_NODE: nodoka_emitBytecode(emitter, NODOKA_BC_SUB); break;
                case NODOKA_SHL_ASSIGN_NODE: nodoka_emitBytecode(emitter, NODOKA_BC_SHL); break;
                case NODOKA_SHR_ASSIGN_NODE: nodoka_emitBytecode(emitter, NODOKA_BC_SHR); break;
                case NODOKA_USHR_ASSIGN_NODE: nodoka_emitBytecode(emitter, NODOKA_BC_USHR); break;
                case NODOKA_AND_ASSIGN_NODE: nodoka_emitBytecode(emitter, NODOKA_BC_AND); break;
                case NODOKA_OR_ASSIGN_NODE: nodoka_emitBytecode(emitter, NODOKA_BC_OR); break;
                case NODOKA_XOR_ASSIGN_NODE: nodoka_emitBytecode(emitter, NODOKA_BC_XOR); break;
                default: assert(0);
            }
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_emitBytecode(emitter, NODOKA_BC_XCHG3);
            /* A check */
            nodoka_emitBytecode(emitter, NODOKA_BC_PUT);
            break;
        }


        case NODOKA_COMMA_NODE: {
            nodoka_codegen(emitter, node->_1);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_POP);
            nodoka_codegen(emitter, node->_2);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            break;
        }

        case NODOKA_DO_STMT: {
            nodoka_label bodyLabel = nodoka_putLabel(emitter);
            nodoka_codegen(emitter, node->_1);
            //nodoka_label continuePoint = nodoka_putLabel(emitter);
            nodoka_codegen(emitter, node->_2);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
            nodoka_relocatable rel;
            nodoka_emitBytecode(emitter, NODOKA_BC_JT, &rel);
            //nodoka_label breakPoint = nodoka_putLabel(emitter);
            nodoka_relocate(emitter, rel, bodyLabel);
            break;
        }
        case NODOKA_WHILE_STMT: {
            nodoka_label continuePoint = nodoka_putLabel(emitter);
            nodoka_codegen(emitter, node->_1);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
            nodoka_emitBytecode(emitter, NODOKA_BC_L_NOT);
            nodoka_relocatable rel, rel2;
            nodoka_emitBytecode(emitter, NODOKA_BC_JT, &rel);
            nodoka_codegen(emitter, node->_2);
            nodoka_emitBytecode(emitter, NODOKA_BC_JMP, &rel2);
            nodoka_label breakpoint = nodoka_putLabel(emitter);
            nodoka_relocate(emitter, rel, breakpoint);
            nodoka_relocate(emitter, rel2, continuePoint);
            break;
        }
        default: assert(0);
    }
}

void codegenTernary(nodoka_code_emitter *emitter, nodoka_ternary_node *node) {
    switch (node->type) {
        case NODOKA_COND_NODE: {
            nodoka_codegen(emitter, node->_1);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
            nodoka_relocatable t, c;
            nodoka_emitBytecode(emitter, NODOKA_BC_JT, &t);
            nodoka_codegen(emitter, node->_3);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_JMP, &c);
            nodoka_relocate(emitter, t, nodoka_putLabel(emitter));
            nodoka_codegen(emitter, node->_2);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_relocate(emitter, c, nodoka_putLabel(emitter));
            break;
        }
        case NODOKA_IF_STMT: {
            /* Maybe reduced if interprocedural optimization is done */
            if (node->_2 && node->_3) {
                nodoka_codegen(emitter, node->_1);
                nodoka_emitBytecode(emitter, NODOKA_BC_GET);
                nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
                nodoka_relocatable t, c;
                nodoka_emitBytecode(emitter, NODOKA_BC_JT, &t);
                nodoka_codegen(emitter, node->_3);
                nodoka_emitBytecode(emitter, NODOKA_BC_JMP, &c);
                nodoka_relocate(emitter, t, nodoka_putLabel(emitter));
                nodoka_codegen(emitter, node->_2);
                nodoka_relocate(emitter, c, nodoka_putLabel(emitter));
            } else if (node->_2) {
                nodoka_codegen(emitter, node->_1);
                nodoka_emitBytecode(emitter, NODOKA_BC_GET);
                nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
                nodoka_emitBytecode(emitter, NODOKA_BC_L_NOT);
                nodoka_relocatable c;
                nodoka_emitBytecode(emitter, NODOKA_BC_JT, &c);
                nodoka_codegen(emitter, node->_2);
                nodoka_relocate(emitter, c, nodoka_putLabel(emitter));
            } else if (node->_3) {
                nodoka_codegen(emitter, node->_1);
                nodoka_emitBytecode(emitter, NODOKA_BC_GET);
                nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
                nodoka_relocatable c;
                nodoka_emitBytecode(emitter, NODOKA_BC_JT, &c);
                nodoka_codegen(emitter, node->_3);
                nodoka_relocate(emitter, c, nodoka_putLabel(emitter));
            } else {
                nodoka_codegen(emitter, node->_1);
                nodoka_emitBytecode(emitter, NODOKA_BC_GET);
                nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
                nodoka_emitBytecode(emitter, NODOKA_BC_POP);
            }
            break;
        }
        default: assert(0);
    }
}

void codegen(nodoka_code_emitter *emitter, nodoka_node_list *node) {
    switch (node->type) {
        case NODOKA_THROW_STMT: {
            nodoka_emitBytecode(emitter, NODOKA_BC_POP);
            commonCodegen(emitter, node);
            nodoka_emitBytecode(emitter, NODOKA_BC_THROW);
            break;
        }
        case NODOKA_OBJ_LIT_VAL: {
            nodoka_emitBytecode(emitter, NODOKA_BC_DUP);
            nodoka_codegen(emitter, node->_[0]);
            nodoka_emitBytecode(emitter, NODOKA_BC_REF);
            nodoka_codegen(emitter, node->_[1]);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_emitBytecode(emitter, NODOKA_BC_PUT);
            break;
        }
        case NODOKA_NEW_NODE: {
            nodoka_codegen(emitter, node->_[0]);
            nodoka_emitBytecode(emitter, NODOKA_BC_GET);
            nodoka_node_list *list = (nodoka_node_list *)node->_[1];
            size_t count;
            if (list) {
                commonCodegen(emitter, list);
                count = list->length;
            } else {
                count = 0;
            }
            nodoka_emitBytecode(emitter, NODOKA_BC_NEW, count);
            break;
        }
        case NODOKA_STMT_LIST: {
            for (int i = 0; i < node->length; i++) {
                nodoka_codegen(emitter, node->_[i]);
            }
            break;
        }
        case NODOKA_FOR_STMT: {
            nodoka_relocatable rel, rel2;
            if (node->_[0]) {
                nodoka_codegen(emitter, node->_[0]);
                nodoka_emitBytecode(emitter, NODOKA_BC_GET);
                nodoka_emitBytecode(emitter, NODOKA_BC_POP);
            }
            nodoka_label checkpoint = nodoka_putLabel(emitter);
            if (node->_[1]) {
                nodoka_codegen(emitter, node->_[1]);
                nodoka_emitBytecode(emitter, NODOKA_BC_GET);
                nodoka_emitBytecode(emitter, NODOKA_BC_BOOL);
                nodoka_emitBytecode(emitter, NODOKA_BC_L_NOT);
                nodoka_emitBytecode(emitter, NODOKA_BC_JT, &rel);
            }
            nodoka_codegen(emitter, node->_[3]);
            if (node->_[2]) {
                nodoka_codegen(emitter, node->_[2]);
                nodoka_emitBytecode(emitter, NODOKA_BC_GET);
                nodoka_emitBytecode(emitter, NODOKA_BC_POP);
            }
            nodoka_emitBytecode(emitter, NODOKA_BC_JMP, &rel2);
            nodoka_label breakpoint = nodoka_putLabel(emitter);
            if (node->_[1]) {
                nodoka_relocate(emitter, rel, breakpoint);
            }
            nodoka_relocate(emitter, rel2, checkpoint);
            break;
        }
        case NODOKA_OBJ_LIT: {
            nodoka_emitBytecode(emitter, NODOKA_BC_LOAD_OBJ);
            nodoka_emitBytecode(emitter, NODOKA_BC_NEW, 0);
            for (int i = 0; i < node->length; i++) {
                nodoka_codegen(emitter, node->_[i]);
            }
            break;
        }
        default: assert(0);
    }
}


void nodoka_codegen(nodoka_code_emitter *emitter, nodoka_lex_class *node) {
    if (!node) {
        return;
    }
    switch (node->clazz) {
        case NODOKA_LEX_TOKEN: {
            codegenToken(emitter, (nodoka_token *)node);
            break;
        }
        case NODOKA_LEX_EMPTY_NODE: {
            codegenEmpty(emitter, (nodoka_empty_node *)node);
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
        case NODOKA_LEX_TERNARY_NODE: {
            codegenTernary(emitter, (nodoka_ternary_node *)node);
            break;
        }
        case NODOKA_LEX_NODE_LIST: {
            codegen(emitter, (nodoka_node_list *)node);
            break;
        }
        default: assert(0);
    }
}

