#include "c/stdlib.h"
#include "c/stdio.h"
#include "unicode/hash.h"
#include "c/assert.h"

#include "js/js.h"
#include "js/lex.h"


#define BINARY_HEAD(_name, _previous) nodoka_lex_class *grammar_##_name(nodoka_grammar *gmr) {\
        nodoka_lex_class *cur = grammar_##_previous(gmr);\
        while (true) {\
            uint16_t type;\
            switch (lookahead(gmr)->type) {

#define BINARY_CASE(_value) case NODOKA_TOKEN_##_value: {\
    type = NODOKA_##_value##_NODE;\
    break;\
}\
 
#define BINARY_FOOT(_previous) default:\
    return cur;\
    }\
    disposeNext(gmr);\
    nodoka_binary_node *node = newBinaryNode(type);\
    node->_1 = cur;\
    node->_2 = grammar_##_previous(gmr);\
    cur = (nodoka_lex_class *)node;\
    }\
    }

#define BINARY_1(_name, _previous, _first) \
    BINARY_HEAD(_name, _previous)\
    BINARY_CASE(_first)\
    BINARY_FOOT(_previous)

#define BINARY_2(_name, _previous, _first, _second) \
    BINARY_HEAD(_name, _previous)\
    BINARY_CASE(_first)\
    BINARY_CASE(_second)\
    BINARY_FOOT(_previous)

#define BINARY_3(_name, _previous, _first, _second, _third) \
    BINARY_HEAD(_name, _previous)\
    BINARY_CASE(_first)\
    BINARY_CASE(_second)\
    BINARY_CASE(_third)\
    BINARY_FOOT(_previous)

typedef struct struct_grammar nodoka_grammar;

struct struct_grammar {
    nodoka_lex *lex;
    nodoka_token *next;
    size_t listLen;
    bool noIn;
};

static nodoka_lex_class *grammar_primaryExpr(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_arrayLiteral(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_arrayElemList(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_arrayElem(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_objectLiteral(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_objectLiteralPropList(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_objectLiteralProp(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_memberExpr(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_arguments(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_argumentList(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_leftHandSideExpr(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_postfixExpr(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_assignExpr(nodoka_grammar *gmr);
nodoka_lex_class *grammar_expr(nodoka_grammar *gmr);

nodoka_lex_class *grammar_stmt(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_block(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_varStmt(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_varDeclList(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_varDecl(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_exprStmt(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_ifStmt(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_doWhile(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_while(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_for(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_return(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_throw(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_try(nodoka_grammar *gmr);

static nodoka_lex_class *grammar_funcDecl(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_funcExpr(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_formalParamList(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_funcBody(nodoka_grammar *gmr);

nodoka_lex_class *grammar_program(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_sourceElements(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_sourceElement(nodoka_grammar *gmr);


__attribute__((warn_unused_result))
static nodoka_token *next(nodoka_grammar *gmr) {
    if (gmr->listLen) {
        gmr->listLen--;
        nodoka_token *removed = gmr->next;
        gmr->next = removed->next;
        return removed;
    } else {
        nodoka_token *ret = lex_next(gmr->lex);
        return ret;
    }
}

static void disposeNext(nodoka_grammar *gmr) {
    nodoka_disposeLexNode((nodoka_lex_class *)next(gmr));
}

static void pushback(nodoka_grammar *gmr, nodoka_token *token) {
    gmr->listLen++;
    token->next = gmr->next;
    gmr->next = token;
}

static nodoka_token *lookahead(nodoka_grammar *gmr) {
    nodoka_token *ret = next(gmr);
    pushback(gmr, ret);
    return ret;
}

static nodoka_token *lookahead2(nodoka_grammar *gmr) {
    nodoka_token *ret = next(gmr);
    nodoka_token *ret2 = next(gmr);
    pushback(gmr, ret2);
    pushback(gmr, ret);
    return ret2;
}

__attribute__((warn_unused_result))
static nodoka_token *expect(nodoka_grammar *gmr, uint16_t type) {
    nodoka_token *ret = next(gmr);
    if (ret->type != type) {
        printf("SyntaxError: Encountered %d, but %d expected\n", ret->type, type);
        assert(0);
    }
    return ret;
}

static void expectAndDispose(nodoka_grammar *gmr, uint16_t type) {
    nodoka_disposeLexNode((nodoka_lex_class *)expect(gmr, type));
}

static bool expectSemicolon(nodoka_grammar *gmr) {
    nodoka_token *nxt = lookahead(gmr);
    if (nxt->type == NODOKA_TOKEN_SEMICOLON) {
        disposeNext(gmr);
        return true;
    }
    if (nxt->type == NODOKA_TOKEN_RBRACE || nxt->lineBefore) {
        return true;
    } else {
        assert(!"SyntaxError: Expected semicolon after statement.");
        return false;
    }
}

static nodoka_empty_node *newEmptyNode(enum nodoka_empty_node_type type) {
    nodoka_empty_node *node = malloc(sizeof(nodoka_empty_node));
    node->base.clazz = NODOKA_LEX_EMPTY_NODE;
    node->type = type;
    return node;
}

static nodoka_unary_node *newUnaryNode(enum nodoka_unary_node_type type) {
    nodoka_unary_node *node = malloc(sizeof(nodoka_unary_node));
    node->base.clazz = NODOKA_LEX_UNARY_NODE;
    node->type = type;
    return node;
}

static nodoka_binary_node *newBinaryNode(enum nodoka_binary_node_type type) {
    nodoka_binary_node *node = malloc(sizeof(nodoka_binary_node));
    node->base.clazz = NODOKA_LEX_BINARY_NODE;
    node->type = type;
    return node;
}

static nodoka_ternary_node *newTernaryNode(enum nodoka_ternary_node_type type) {
    nodoka_ternary_node *node = malloc(sizeof(nodoka_ternary_node));
    node->base.clazz = NODOKA_LEX_TERNARY_NODE;
    node->type = type;
    return node;
}

static nodoka_node_list *newNodeList(enum nodoka_node_list_type type, size_t length) {
    nodoka_node_list *node = malloc(sizeof(nodoka_node_list) + sizeof(nodoka_lex_class *)*length);
    node->base.clazz = NODOKA_LEX_NODE_LIST;
    node->type = type;
    node->length = length;
    return node;
}

static nodoka_lex_class *binList2List(enum nodoka_node_list_type type, nodoka_lex_class *binList) {
    nodoka_node_list *ret = (nodoka_node_list *)binList;
    nodoka_node_list *cur;
    size_t count;
    for (count = 1, cur = ret;
            cur->base.clazz == NODOKA_LEX_NODE_LIST && cur->type == NODOKA_INTERNAL_LIST;
            count++, cur = (nodoka_node_list *)cur->_[0]);
    nodoka_node_list *list = newNodeList(type, count);

    cur = ret;

    size_t i = count - 1;
    while (i > 0) {
        list->_[i] = (nodoka_lex_class *)cur->_[1];

        i--;
        nodoka_node_list *next = (nodoka_node_list *)cur->_[0];
        cur->_[0] = NULL;
        cur->_[1] = NULL;
        nodoka_disposeLexNode((nodoka_lex_class *)cur);
        cur = next;
    }
    list->_[0] = (nodoka_lex_class *)cur;

    return (nodoka_lex_class *)list;
}

nodoka_grammar *grammar_new(nodoka_lex *lex) {
    nodoka_grammar *gmr = malloc(sizeof(struct struct_grammar));
    gmr->lex = lex;
    gmr->next = NULL;
    gmr->listLen = 0;
    gmr->noIn = false;
    return gmr;
}

void grammar_dispose(nodoka_grammar *gmr) {
    free(gmr);
}

/**
 * PrimaryExpression := this                LOOKAHEAD(1)=THIS
 *                    | Identifier          LOOKAHEAD(1)=ID
 *                    | Literal             LOOKAHEAD(1)=NULL, TRUE, FALSE, NUM, STR, REGEXP
 *                    | ArrayLiteral        LOOKAHEAD(1)=LBRACKET
 *                    | ObjectLiteral       LOOKAHEAD(1)=LBRACE
 *                    | (Expression)        LOOKAHEAD(1)=LPAREN
 */
static nodoka_lex_class *grammar_primaryExpr(nodoka_grammar *gmr) {
    switch (lookahead(gmr)->type) {
        case NODOKA_TOKEN_THIS:
        case NODOKA_TOKEN_ID:
        case NODOKA_TOKEN_NULL:
        case NODOKA_TOKEN_TRUE:
        case NODOKA_TOKEN_FALSE:
        case NODOKA_TOKEN_NUM:
        case NODOKA_TOKEN_STR: {
            nodoka_token *nxt = next(gmr);
            return (nodoka_lex_class *)nxt;
        }
        case NODOKA_TOKEN_DIV:
        case NODOKA_TOKEN_DIV_ASSIGN: {
            nodoka_token *tk = next(gmr);
            bool assign = tk->type == NODOKA_TOKEN_DIV_ASSIGN;
            nodoka_disposeLexNode((nodoka_lex_class *)tk);
            return (nodoka_lex_class *)lex_regexp(gmr->lex, assign);
        }
        case NODOKA_TOKEN_LPAREN: {
            disposeNext(gmr);
            nodoka_lex_class *ret = grammar_expr(gmr);
            expectAndDispose(gmr, NODOKA_TOKEN_RPAREN);
            return ret;
        }
        case NODOKA_TOKEN_LBRACKET:
            return grammar_arrayLiteral(gmr);
        case NODOKA_TOKEN_LBRACE: {
            return grammar_objectLiteral(gmr);
        }
        default:
            assert(!"SyntaxError: Expected Expression");
            return NULL;
    }
}

static nodoka_lex_class *grammar_arrayLiteral(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_LBRACKET);
    if (lookahead(gmr)->type == NODOKA_TOKEN_RBRACKET) {
        disposeNext(gmr);
        return (nodoka_lex_class *)newNodeList(NODOKA_ARR_LIT, 0);
    } else {
        nodoka_lex_class *ret = binList2List(NODOKA_ARR_LIT, grammar_arrayElemList(gmr));
        expectAndDispose(gmr, NODOKA_TOKEN_RBRACKET);
        return ret;
    }
}

static nodoka_lex_class *grammar_arrayElemList(nodoka_grammar *gmr) {
    nodoka_lex_class *cur = grammar_arrayElem(gmr);
    while (true) {
        nodoka_token *token = lookahead(gmr);
        if (token->type == NODOKA_TOKEN_RBRACKET) {
            return cur;
        } else {
            expectAndDispose(gmr, NODOKA_TOKEN_COMMA);
            nodoka_token *n = lookahead(gmr);
            if (n->type == NODOKA_TOKEN_RBRACKET) {
                return cur;
            }
            nodoka_lex_class *ptr = grammar_arrayElem(gmr);
            nodoka_node_list *list = newNodeList(NODOKA_INTERNAL_LIST, 2);
            list->_[0] = cur;
            list->_[1] = ptr;
            cur = (nodoka_lex_class *)list;
        }
    }
}

static nodoka_lex_class *grammar_arrayElem(nodoka_grammar *gmr) {
    nodoka_token *t = lookahead(gmr);
    if (t == NODOKA_TOKEN_COMMA) {
        return NULL;
    }
    return grammar_assignExpr(gmr);
}

static nodoka_lex_class *grammar_objectLiteral(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_LBRACE);
    if (lookahead(gmr)->type == NODOKA_TOKEN_RBRACE) {
        disposeNext(gmr);
        return (nodoka_lex_class *)newNodeList(NODOKA_OBJ_LIT, 0);
    } else {
        nodoka_lex_class *ret = binList2List(NODOKA_OBJ_LIT, grammar_objectLiteralPropList(gmr));
        expectAndDispose(gmr, NODOKA_TOKEN_RBRACE);
        return ret;
    }
}

static nodoka_lex_class *grammar_objectLiteralPropList(nodoka_grammar *gmr) {
    nodoka_lex_class *cur = grammar_objectLiteralProp(gmr);
    while (true) {
        nodoka_token *token = lookahead(gmr);
        if (token->type == NODOKA_TOKEN_RBRACE) {
            return cur;
        } else {
            assert(token->type == NODOKA_TOKEN_COMMA);
            disposeNext(gmr);
            nodoka_token *n = lookahead(gmr);
            if (n->type == NODOKA_TOKEN_RBRACE) {
                return cur;
            }
            nodoka_lex_class *ptr = grammar_objectLiteralProp(gmr);
            nodoka_node_list *list = newNodeList(NODOKA_INTERNAL_LIST, 2);
            list->_[0] = cur;
            list->_[1] = ptr;
            cur = (nodoka_lex_class *)list;
        }
    }
}

static nodoka_lex_class *grammar_objectLiteralProp(nodoka_grammar *gmr) {
    nodoka_token *name = expect(gmr, NODOKA_TOKEN_ID);
    nodoka_token *colon = next(gmr);
    if (colon->type == NODOKA_TOKEN_COLON) {
        name->type = NODOKA_TOKEN_STR;
        nodoka_disposeLexNode((nodoka_lex_class *)colon);
        nodoka_lex_class *expr = grammar_assignExpr(gmr);
        nodoka_node_list *list = newNodeList(NODOKA_OBJ_LIT_VAL, 2);
        list->_[0] = (nodoka_lex_class *)name;
        list->_[1] = (nodoka_lex_class *)expr;
        return (nodoka_lex_class *)list;
    } else {
        assert(!"UnsupportedFeature: Get and set not supported");
    }
}

/**
 * MemberExpression :=(
 *      PrimaryExpression
            LOOKAHEAD(1)=THIS, ID, NULL, TRUE, FALSE, NUM, STR, REGEXP, LBRACKET, LBRACE, LPAREN
 *      | FunctionExpression        LOOKAHEAD(1)=FUNCTION
 *      | new MemberExpression Arguments_opt    LOOKAHEAD(1)=NEW
 *  ) (
 *      [ Expression ]
 *      | . IdentifierName
 *  )*
 */
static nodoka_lex_class *grammar_memberExpr(nodoka_grammar *gmr) {
    nodoka_lex_class *cur;
    switch (lookahead(gmr)->type) {
        case NODOKA_TOKEN_FUNCTION: {
            return grammar_funcExpr(gmr);
        }
        case NODOKA_TOKEN_NEW: {
            disposeNext(gmr);
            nodoka_lex_class *expr = grammar_memberExpr(gmr);
            nodoka_lex_class *args = NULL;
            if (lookahead(gmr)->type == NODOKA_TOKEN_LPAREN) {
                args = grammar_arguments(gmr);
            }
            nodoka_node_list *node = newNodeList(NODOKA_NEW_NODE, 2);
            node->_[0] = expr;
            node->_[1] = args;
            cur = (nodoka_lex_class *)node;
            break;
        }
        default:
            cur = grammar_primaryExpr(gmr);
            break;
    }
    while (1) {
        switch (lookahead(gmr)->type) {
            case NODOKA_TOKEN_POINT: {
                disposeNext(gmr);
                gmr->lex->parseId = false;
                nodoka_token *id = expect(gmr, NODOKA_TOKEN_ID);
                gmr->lex->parseId = true;
                nodoka_binary_node *node = (nodoka_binary_node *)newBinaryNode(NODOKA_MEMBER_NODE);
                id->type = NODOKA_TOKEN_STR;
                node->_1 = cur;
                node->_2 = (nodoka_lex_class *)id;
                cur = (nodoka_lex_class *)node;
                break;
            }
            case NODOKA_TOKEN_LBRACKET: {
                disposeNext(gmr);
                nodoka_lex_class *expr = grammar_expr(gmr);
                expectAndDispose(gmr, NODOKA_TOKEN_RBRACKET);
                nodoka_binary_node *node = (nodoka_binary_node *)newBinaryNode(NODOKA_MEMBER_NODE);
                node->_1 = cur;
                node->_2 = expr;
                cur = (nodoka_lex_class *)node;
                break;
            }
            default:
                return cur;
        }
    }
}

/*
 * Arguments := ( ArgumentList_opt )
 */
static nodoka_lex_class *grammar_arguments(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_LPAREN);
    if (lookahead(gmr)->type != NODOKA_TOKEN_RPAREN) {
        nodoka_lex_class *list = binList2List(NODOKA_ARG_LIST, grammar_argumentList(gmr));
        expectAndDispose(gmr, NODOKA_TOKEN_RPAREN);
        return list;
    }
    disposeNext(gmr);
    return NULL;
}

static nodoka_lex_class *grammar_argumentList(nodoka_grammar *gmr) {
    nodoka_lex_class *cur = grammar_assignExpr(gmr);
    while (lookahead(gmr)->type == NODOKA_TOKEN_COMMA) {
        disposeNext(gmr);
        nodoka_lex_class *arg = grammar_assignExpr(gmr);
        nodoka_node_list *list = newNodeList(NODOKA_INTERNAL_LIST, 2);
        list->_[0] = cur;
        list->_[1] = arg;
        cur = (nodoka_lex_class *)list;
    }
    return cur;
}


/*
 * LeftHandSideExpression := MemberExpression
 *      (
            Arguments
            | . IdentifierName
            | [ Expression ]
 *      )*
 */
static nodoka_lex_class *grammar_leftHandSideExpr(nodoka_grammar *gmr) {
    nodoka_lex_class *cur = grammar_memberExpr(gmr);
    while (1) {
        switch (lookahead(gmr)->type) {
            case NODOKA_TOKEN_POINT: {
                disposeNext(gmr);
                gmr->lex->parseId = false;
                nodoka_token *id = expect(gmr, NODOKA_TOKEN_ID);
                gmr->lex->parseId = true;
                nodoka_binary_node *node = (nodoka_binary_node *)newBinaryNode(NODOKA_MEMBER_NODE);
                id->type = NODOKA_TOKEN_STR;
                node->_1 = cur;
                node->_2 = (nodoka_lex_class *)id;
                cur = (nodoka_lex_class *)node;
                break;
            }
            case NODOKA_TOKEN_LBRACKET: {
                disposeNext(gmr);
                nodoka_lex_class *expr = grammar_expr(gmr);
                expectAndDispose(gmr, NODOKA_TOKEN_RBRACKET);
                nodoka_binary_node *node = (nodoka_binary_node *)newBinaryNode(NODOKA_MEMBER_NODE);
                node->_1 = cur;
                node->_2 = expr;
                cur = (nodoka_lex_class *)node;
                break;
            }
            case NODOKA_TOKEN_LPAREN: {
                nodoka_lex_class *args = grammar_arguments(gmr);
                nodoka_binary_node *node = (nodoka_binary_node *)newBinaryNode(NODOKA_CALL_NODE);
                node->_1 = cur;
                node->_2 = args;
                cur = (nodoka_lex_class *)node;
                break;
            }
            default:
                return cur;
        }
    }
}

static nodoka_lex_class *grammar_postfixExpr(nodoka_grammar *gmr) {
    nodoka_lex_class *expr = grammar_leftHandSideExpr(gmr);

    nodoka_token *nxt = lookahead(gmr);
    if (nxt->lineBefore) {
        return expr;
    }

    if (nxt->type == NODOKA_TOKEN_INC) {
        disposeNext(gmr);
        nodoka_unary_node *node = newUnaryNode(NODOKA_POST_INC_NODE);
        node->_1 = expr;
        return (nodoka_lex_class *)node;
    } else if (nxt->type == NODOKA_TOKEN_DEC) {
        disposeNext(gmr);
        nodoka_unary_node *node = newUnaryNode(NODOKA_POST_DEC_NODE);
        node->_1 = expr;
        return (nodoka_lex_class *)node;
    }

    return expr;
}

static nodoka_lex_class *grammar_unaryExpr(nodoka_grammar *gmr) {
    uint16_t nodeClass;
    switch (lookahead(gmr)->type) {
        case NODOKA_TOKEN_DELETE:
            nodeClass = NODOKA_DELETE_NODE;
            goto produceExpr;
        case NODOKA_TOKEN_VOID:
            nodeClass = NODOKA_VOID_NODE;
            goto produceExpr;
        case NODOKA_TOKEN_TYPEOF:
            nodeClass = NODOKA_TYPEOF_NODE;
            goto produceExpr;
        case NODOKA_TOKEN_INC:
            nodeClass = NODOKA_PRE_INC_NODE;
            goto produceExpr;
        case NODOKA_TOKEN_DEC:
            nodeClass = NODOKA_PRE_DEC_NODE;
            goto produceExpr;
        case NODOKA_TOKEN_ADD:
            nodeClass = NODOKA_POS_NODE;
            goto produceExpr;
        case NODOKA_TOKEN_SUB:
            nodeClass = NODOKA_NEG_NODE;
            goto produceExpr;
        case NODOKA_TOKEN_NOT:
            nodeClass = NODOKA_NOT_NODE;
            goto produceExpr;
        case NODOKA_TOKEN_L_NOT:
            nodeClass = NODOKA_LNOT_NODE;
produceExpr:
            disposeNext(gmr);
            nodoka_unary_node *node = newUnaryNode(nodeClass);
            node->_1 = grammar_unaryExpr(gmr);
            return (nodoka_lex_class *)node;
        default:
            return grammar_postfixExpr(gmr);
    }
}

static BINARY_3(mulExpr, unaryExpr, MUL, MOD, DIV)
static BINARY_2(addExpr, mulExpr, ADD, SUB)
static BINARY_3(shiftExpr, addExpr, SHL, SHR, USHR)

static BINARY_HEAD(relExpr, shiftExpr)
BINARY_CASE(LT)
BINARY_CASE(GT)
BINARY_CASE(LTEQ)
BINARY_CASE(GTEQ)
BINARY_CASE(INSTANCEOF)
case NODOKA_TOKEN_IN: {
    if (gmr->noIn) {
        return cur;
    }
    type = NODOKA_IN_NODE;
    break;
}
BINARY_FOOT(shiftExpr)

static BINARY_HEAD(eqExpr, relExpr)
BINARY_CASE(EQ)
BINARY_CASE(INEQ)
BINARY_CASE(STRICT_EQ)
BINARY_CASE(STRICT_INEQ)
BINARY_FOOT(relExpr)

static BINARY_1(andExpr, eqExpr, AND)
static BINARY_1(xorExpr, andExpr, XOR)
static BINARY_1(orExpr, xorExpr, OR)
static BINARY_1(lAndExpr, orExpr, L_AND)
static BINARY_1(lOrExpr, lAndExpr, L_OR)

static nodoka_lex_class *grammar_condExpr(nodoka_grammar *gmr) {
    nodoka_lex_class *node = grammar_lOrExpr(gmr);
    if (lookahead(gmr)->type == NODOKA_TOKEN_QUESTION) {
        disposeNext(gmr);
        nodoka_lex_class *t_exp = grammar_assignExpr(gmr);
        expectAndDispose(gmr, NODOKA_TOKEN_COLON);
        nodoka_lex_class *f_exp = grammar_assignExpr(gmr);
        nodoka_ternary_node *ret = newTernaryNode(NODOKA_COND_NODE);
        ret->_1 = node;
        ret->_2 = t_exp;
        ret->_3 = f_exp;
        return (nodoka_lex_class *)ret;
    } else {
        return node;
    }
}

static nodoka_lex_class *grammar_assignExpr(nodoka_grammar *gmr) {
    nodoka_lex_class *node = grammar_condExpr(gmr);
    /* We do not care wether it is LeftHandSide. We can check it later */
    uint16_t type;
    switch (lookahead(gmr)->type) {
        case NODOKA_TOKEN_ASSIGN: type = NODOKA_ASSIGN_NODE; break;
        case NODOKA_TOKEN_MUL_ASSIGN: type = NODOKA_MUL_ASSIGN_NODE; break;
        case NODOKA_TOKEN_DIV_ASSIGN: type = NODOKA_DIV_ASSIGN_NODE; break;
        case NODOKA_TOKEN_MOD_ASSIGN: type = NODOKA_MOD_ASSIGN_NODE; break;
        case NODOKA_TOKEN_ADD_ASSIGN: type = NODOKA_ADD_ASSIGN_NODE; break;
        case NODOKA_TOKEN_SUB_ASSIGN: type = NODOKA_SUB_ASSIGN_NODE; break;
        case NODOKA_TOKEN_SHL_ASSIGN: type = NODOKA_SHL_ASSIGN_NODE; break;
        case NODOKA_TOKEN_SHR_ASSIGN: type = NODOKA_SHR_ASSIGN_NODE; break;
        case NODOKA_TOKEN_USHR_ASSIGN: type = NODOKA_USHR_ASSIGN_NODE; break;
        case NODOKA_TOKEN_AND_ASSIGN: type = NODOKA_AND_ASSIGN_NODE; break;
        case NODOKA_TOKEN_XOR_ASSIGN: type = NODOKA_XOR_ASSIGN_NODE; break;
        case NODOKA_TOKEN_OR_ASSIGN: type = NODOKA_OR_ASSIGN_NODE; break;
        default: return node;
    }
    disposeNext(gmr);
    nodoka_binary_node *ass = newBinaryNode(type);
    ass->_1 = node;
    ass->_2 = grammar_assignExpr(gmr);
    return (nodoka_lex_class *)ass;
}

BINARY_1(expr, assignExpr, COMMA);

/*
 * Statement:= Block                    LOOKAHEAD(1)=LBRACE
 *           | VariableStatement        LOOKAHEAD(1)=VAR
 *           | EmptyStatement           LOOKAHEAD(1)=SEMICOLON
 *           | ExpressionStatement      LOOKAHEAD(1)=Blah
 *           | IfStatement              LOOKAHEAD(1)=IF
 *           | IterationStatement       LOOKAHEAD(1)=DO WHILE FOR
 *           | ContinueStatement        LOOKAHEAD(1)=CONTINUE
 *           | BreakStatement           LOOKAHEAD(1)=BREAK
 *           | ReturnStatement          LOOKAHEAD(1)=RETURN
 *           | WithStatement            LOOKAHEAD(1)=WITH
 *           | LabelledStatement        LOOKAHEAD(1)=ID         LOOKAHEAD(2)=COLON
 *           | SwitchStatement          LOOKAHEAD(1)=SWITCH
 *           | ThrowStatement           LOOKAHEAD(1)=THROW
 *           | TryStatement             LOOKAHEAD(1)=TRY
 *           | DebuggerStatement        LOOKAHEAD(1)=DEBUGGER
*/
nodoka_lex_class *grammar_stmt(nodoka_grammar *gmr) {
    switch (lookahead(gmr)->type) {
        case NODOKA_TOKEN_LBRACE:
            return grammar_block(gmr);
        case NODOKA_TOKEN_VAR:
            return grammar_varStmt(gmr);
        case NODOKA_TOKEN_SEMICOLON:
            disposeNext(gmr);
            return NULL;
        case NODOKA_TOKEN_IF:
            return grammar_ifStmt(gmr);
        case NODOKA_TOKEN_DO:
            return grammar_doWhile(gmr);
        case NODOKA_TOKEN_WHILE:
            return grammar_while(gmr);
        case NODOKA_TOKEN_FOR:
            return grammar_for(gmr);
        case NODOKA_TOKEN_CONTINUE:
            assert(0);
        case NODOKA_TOKEN_BREAK:
            assert(0);
        case NODOKA_TOKEN_RETURN:
            return grammar_return(gmr);
        case NODOKA_TOKEN_WITH:
            assert(0);
        case NODOKA_TOKEN_SWITCH:
            assert(0);
        case NODOKA_TOKEN_THROW:
            return grammar_throw(gmr);
        case NODOKA_TOKEN_TRY:
            return grammar_try(gmr);
        case NODOKA_TOKEN_DEBUGGER:
            disposeNext(gmr);
            expectSemicolon(gmr);
            return (nodoka_lex_class *)newEmptyNode(NODOKA_DEBUGGER_STMT);
        case NODOKA_TOKEN_ID: {
            if (lookahead2(gmr)->type == NODOKA_TOKEN_COLON) {
                assert(0);
            }
        }
        default: {
            return grammar_exprStmt(gmr);
        }
    }
}

static nodoka_lex_class *grammar_block(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_LBRACE);
    if (lookahead(gmr)->type == NODOKA_TOKEN_RBRACE) {
        disposeNext(gmr);
        return NULL;
    }
    /* Accroding to ECMA, we should check exception
     * but I am lazy */
    nodoka_lex_class *block = grammar_stmt(gmr);
    while (true) {
        if (lookahead(gmr)->type == NODOKA_TOKEN_RBRACE) {
            disposeNext(gmr);
            return binList2List(NODOKA_STMT_LIST, block);
        }
        nodoka_node_list *sb = newNodeList(NODOKA_INTERNAL_LIST, 2);
        sb->_[0] = block;
        sb->_[1] = grammar_stmt(gmr);
        block = (nodoka_lex_class *)sb;
    }
}

static nodoka_lex_class *grammar_varStmt(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_VAR);
    nodoka_lex_class *ret = binList2List(NODOKA_STMT_LIST, grammar_varDeclList(gmr));
    expectSemicolon(gmr);
    return ret;
}

static nodoka_lex_class *grammar_varDeclList(nodoka_grammar *gmr) {
    nodoka_lex_class *cur = grammar_varDecl(gmr);
    while (lookahead(gmr)->type == NODOKA_TOKEN_COMMA) {
        disposeNext(gmr);
        nodoka_node_list *list = newNodeList(NODOKA_INTERNAL_LIST, 2);
        list->_[0] = cur;
        list->_[1] = grammar_varDecl(gmr);
        cur = (nodoka_lex_class *)list;
    }
    return cur;
}

static nodoka_lex_class *grammar_varDecl(nodoka_grammar *gmr) {
    nodoka_node_list *list = newNodeList(NODOKA_VAR_STMT, 2);
    nodoka_token *id = expect(gmr, NODOKA_TOKEN_ID);
    list->_[0] = (nodoka_lex_class *)id;
    if (lookahead(gmr)->type == NODOKA_TOKEN_ASSIGN) {
        disposeNext(gmr);
        list->_[1] = grammar_assignExpr(gmr);
    } else {
        list->_[1] = NULL;
    }
    return (nodoka_lex_class *)list;
}

static nodoka_lex_class *grammar_exprStmt(nodoka_grammar *gmr) {
    nodoka_lex_class *expr = grammar_expr(gmr);

    nodoka_unary_node *node = newUnaryNode(NODOKA_EXPR_STMT);
    node->_1 = expr;

    expectSemicolon(gmr);
    return (nodoka_lex_class *)node;
}

static nodoka_lex_class *grammar_ifStmt(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_IF);
    expectAndDispose(gmr, NODOKA_TOKEN_LPAREN);
    nodoka_lex_class *expr = grammar_expr(gmr);
    expectAndDispose(gmr, NODOKA_TOKEN_RPAREN);
    nodoka_lex_class *first = grammar_stmt(gmr);
    nodoka_ternary_node *node = newTernaryNode(NODOKA_IF_STMT);
    node->_1 = expr;
    node->_2 = first;

    if (lookahead(gmr)->type == NODOKA_TOKEN_ELSE) {
        disposeNext(gmr);
        nodoka_lex_class *second = grammar_stmt(gmr);
        node->_3 = second;
    } else {
        node->_3 = NULL;
    }
    return (nodoka_lex_class *)node;
}

static nodoka_lex_class *grammar_doWhile(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_DO);
    nodoka_lex_class *stmt = grammar_stmt(gmr);
    expectAndDispose(gmr, NODOKA_TOKEN_WHILE);
    expectAndDispose(gmr, NODOKA_TOKEN_LPAREN);
    nodoka_lex_class *expr = grammar_expr(gmr);
    expectAndDispose(gmr, NODOKA_TOKEN_RPAREN);
    expectSemicolon(gmr);

    nodoka_binary_node *node = newBinaryNode(NODOKA_DO_STMT);
    node->_1 = stmt;
    node->_2 = expr;
    return (nodoka_lex_class *)node;
}

static nodoka_lex_class *grammar_while(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_WHILE);
    expectAndDispose(gmr, NODOKA_TOKEN_LPAREN);
    nodoka_lex_class *expr = grammar_expr(gmr);
    expectAndDispose(gmr, NODOKA_TOKEN_RPAREN);
    nodoka_lex_class *stmt = grammar_stmt(gmr);
    expectSemicolon(gmr);
    nodoka_binary_node *node = newBinaryNode(NODOKA_WHILE_STMT);
    node->_1 = expr;
    node->_2 = stmt;
    return (nodoka_lex_class *)node;
}

static nodoka_lex_class *grammar_for(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_FOR);
    expectAndDispose(gmr, NODOKA_TOKEN_LPAREN);

    nodoka_token *t = lookahead(gmr);
    if (t->type == NODOKA_TOKEN_VAR) {
        disposeNext(gmr);
        nodoka_lex_class *vars = grammar_varDeclList(gmr);
        if (lookahead(gmr)->type == NODOKA_TOKEN_IN) {
            assert(!"for(var expr in expr)");
        }
        expectAndDispose(gmr, NODOKA_TOKEN_SEMICOLON);
        nodoka_lex_class *testExpr = NULL;
        if (lookahead(gmr)->type != NODOKA_TOKEN_SEMICOLON) {
            testExpr = grammar_expr(gmr);
        }
        expectAndDispose(gmr, NODOKA_TOKEN_SEMICOLON);
        nodoka_lex_class *incExpr = NULL;
        if (lookahead(gmr)->type != NODOKA_TOKEN_RPAREN) {
            incExpr = grammar_expr(gmr);
        }
        expectAndDispose(gmr, NODOKA_TOKEN_RPAREN);
        nodoka_lex_class *stmt = grammar_stmt(gmr);
        nodoka_node_list *list = newNodeList(NODOKA_FOR_VAR_STMT, 4);
        list->_[0] = vars;
        list->_[1] = testExpr;
        list->_[2] = incExpr;
        list->_[3] = stmt;
        return (nodoka_lex_class *)list;
    } else {
        nodoka_lex_class *expr = NULL;
        if (t->type != NODOKA_TOKEN_SEMICOLON) {
            gmr->noIn = true;
            expr = grammar_expr(gmr);
            gmr->noIn = false;

            if (lookahead(gmr)->type == NODOKA_TOKEN_IN) {
                assert(!"for(expr in expr)");
            }
        }
        expectAndDispose(gmr, NODOKA_TOKEN_SEMICOLON);
        nodoka_lex_class *testExpr = NULL;
        if (lookahead(gmr)->type != NODOKA_TOKEN_SEMICOLON) {
            testExpr = grammar_expr(gmr);
        }
        expectAndDispose(gmr, NODOKA_TOKEN_SEMICOLON);
        nodoka_lex_class *incExpr = NULL;
        if (lookahead(gmr)->type != NODOKA_TOKEN_RPAREN) {
            incExpr = grammar_expr(gmr);
        }
        expectAndDispose(gmr, NODOKA_TOKEN_RPAREN);
        nodoka_lex_class *stmt = grammar_stmt(gmr);

        nodoka_node_list *list = newNodeList(NODOKA_FOR_STMT, 4);
        list->_[0] = expr;
        list->_[1] = testExpr;
        list->_[2] = incExpr;
        list->_[3] = stmt;

        return (nodoka_lex_class *)list;
    }
}

static nodoka_lex_class *grammar_return(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_RETURN);

    nodoka_token *nxt = lookahead(gmr);
    if (nxt->type == NODOKA_TOKEN_SEMICOLON) {
        disposeNext(gmr);
        nodoka_node_list *node = newNodeList(NODOKA_RETURN_STMT, 1);
        node->_[0] = NULL;
        return (nodoka_lex_class *)node;
    } else if (nxt->lineBefore) {
        nodoka_node_list *node = newNodeList(NODOKA_RETURN_STMT, 1);
        node->_[0] = NULL;
        return (nodoka_lex_class *)node;
    }
    nodoka_lex_class *expr = grammar_expr(gmr);
    nodoka_node_list *node = newNodeList(NODOKA_RETURN_STMT, 1);
    node->_[0] = expr;

    expectSemicolon(gmr);

    return (nodoka_lex_class *)node;
}

static nodoka_lex_class *grammar_throw(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_THROW);

    nodoka_token *nxt = lookahead(gmr);
    if (nxt->lineBefore) {
        assert(!"Expected Expression");
    }

    nodoka_lex_class *expr = grammar_expr(gmr);
    nodoka_node_list *node = newNodeList(NODOKA_THROW_STMT, 1);
    node->_[0] = expr;

    expectSemicolon(gmr);

    return (nodoka_lex_class *)node;
}

static nodoka_lex_class *grammar_try(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_TRY);
    nodoka_node_list *list = newNodeList(NODOKA_TRY_STMT, 4);
    list->_[0] = grammar_block(gmr);
    if (lookahead(gmr)->type == NODOKA_TOKEN_FINALLY) {
        disposeNext(gmr);
        list->_[1] = NULL;
        list->_[2] = NULL;
        list->_[3] = grammar_block(gmr);
    } else {
        expectAndDispose(gmr, NODOKA_TOKEN_CATCH);
        expectAndDispose(gmr, NODOKA_TOKEN_LPAREN);
        list->_[1] = (nodoka_lex_class *)expect(gmr, NODOKA_TOKEN_ID);
        expectAndDispose(gmr, NODOKA_TOKEN_RPAREN);
        list->_[2] = grammar_block(gmr);
        if (lookahead(gmr)->type == NODOKA_TOKEN_FINALLY) {
            disposeNext(gmr);
            list->_[3] = grammar_block(gmr);
        } else {
            list->_[3] = NULL;
        }
    }
    return (nodoka_lex_class *)list;
}


static nodoka_lex_class *grammar_funcDecl(nodoka_grammar *gmr) {
    nodoka_node_list *func = (nodoka_node_list *)grammar_funcExpr(gmr);
    assert(func->_[0]); //Decl must have name
    nodoka_node_list *ret = newNodeList(NODOKA_FUNC_DECL, 2);
    ret->_[0] = func->_[0];
    func->_[0] = NULL;
    ret->_[1] = (nodoka_lex_class *)func;
    return (nodoka_lex_class *)ret;
}

static nodoka_lex_class *grammar_funcExpr(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_FUNCTION);
    nodoka_node_list *func = newNodeList(NODOKA_FUNCTION_NODE, 3);

    if (lookahead(gmr)->type == NODOKA_TOKEN_ID) {
        func->_[0] = (nodoka_lex_class *)next(gmr);
    } else {
        func->_[0] = NULL;
    }

    expectAndDispose(gmr, NODOKA_TOKEN_LPAREN);

    if (lookahead(gmr)->type == NODOKA_TOKEN_ID) {
        func->_[1] = binList2List(NODOKA_ARG_LIST, grammar_formalParamList(gmr));
    } else {
        func->_[1] = NULL;
    }

    expectAndDispose(gmr, NODOKA_TOKEN_RPAREN);
    expectAndDispose(gmr, NODOKA_TOKEN_LBRACE);
    func->_[2] = grammar_funcBody(gmr);
    expectAndDispose(gmr, NODOKA_TOKEN_RBRACE);

    return (nodoka_lex_class *)func;
}

/**
 * LOOKAHEAD(1) = Identifier
 *
 * FormalParameterList : = Identifier {, Identifier}
 */
static nodoka_lex_class *grammar_formalParamList(nodoka_grammar *gmr) {
    nodoka_lex_class *cur = (nodoka_lex_class *)expect(gmr, NODOKA_TOKEN_ID);
    while (lookahead(gmr)->type == NODOKA_TOKEN_COMMA) {
        disposeNext(gmr);
        nodoka_node_list *list = newNodeList(NODOKA_INTERNAL_LIST, 2);
        list->_[0] = cur;
        list->_[1] = (nodoka_lex_class *)expect(gmr, NODOKA_TOKEN_ID);
        cur = (nodoka_lex_class *)list;
    }
    return cur;
}

static nodoka_lex_class *grammar_funcBody(nodoka_grammar *gmr) {
    nodoka_token *next = lookahead(gmr);
    if (next->type == NODOKA_TOKEN_EOF || next->type == NODOKA_TOKEN_RBRACE) {
        return NULL;
    }
    return grammar_sourceElements(gmr);
}

nodoka_lex_class *grammar_program(nodoka_grammar *gmr) {
    return grammar_funcBody(gmr);
}

static nodoka_lex_class *grammar_sourceElements(nodoka_grammar *gmr) {
    nodoka_lex_class *cur = grammar_sourceElement(gmr);
    while (true) {
        nodoka_token *next = lookahead(gmr);
        if (next->type == NODOKA_TOKEN_EOF || next->type == NODOKA_TOKEN_RBRACE) {
            return binList2List(NODOKA_STMT_LIST, cur);
        }
        nodoka_node_list *list = newNodeList(NODOKA_INTERNAL_LIST, 2);
        list->_[0] = cur;
        list->_[1] = grammar_sourceElement(gmr);
        cur = (nodoka_lex_class *)list;
    }
}

static nodoka_lex_class *grammar_sourceElement(nodoka_grammar *gmr) {
    if (lookahead(gmr)->type == NODOKA_TOKEN_FUNCTION) {
        return grammar_funcDecl(gmr);
    } else {
        return grammar_stmt(gmr);
    }
}
