#include "c/stdlib.h"

#include "js/lex.h"

void nodoka_disposeLexNode(nodoka_lex_class *node) {
    switch (node->clazz) {
        case NODOKA_LEX_TOKEN: {
            nodoka_token *n = (nodoka_token *)node;
            if (n->type == NODOKA_TOKEN_STR || n->type == NODOKA_TOKEN_ID) {
                uint16_t *str = n->stringValue.str;
                if (str) {
                    free(str);
                }
            }
            free(n);
            break;
        }
        case NODOKA_LEX_UNARY_NODE: {
            nodoka_unary_node *n = (nodoka_unary_node *)node;
            nodoka_disposeLexNode(n->_1);
            free(n);
            break;
        }
        case NODOKA_LEX_BINARY_NODE: {
            nodoka_binary_node *n = (nodoka_binary_node *)node;
            nodoka_disposeLexNode(n->_1);
            nodoka_disposeLexNode(n->_2);
            free(n);
            break;
        }
        case NODOKA_LEX_TERNARY_NODE: {
            nodoka_ternary_node *n = (nodoka_ternary_node *)node;
            nodoka_disposeLexNode(n->_1);
            nodoka_disposeLexNode(n->_2);
            nodoka_disposeLexNode(n->_3);
            free(n);
            break;
        }
        default: assert(0);
    }
}