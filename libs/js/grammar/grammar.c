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
static nodoka_lex_class *grammar_memberExpr(nodoka_grammar *gmr);
//static nodoka_lex_class *grammar_arguments(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_argumentList(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_leftHandSideExpr(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_postfixExpr(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_assignExpr(nodoka_grammar *gmr);
nodoka_lex_class *grammar_expr(nodoka_grammar *gmr);

nodoka_lex_class *grammar_stmt(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_block(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_varStmt(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_exprStmt(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_ifStmt(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_doWhile(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_while(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_for(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_throw(nodoka_grammar *gmr);

/*static nodoka_lex_class *grammar_funcDecl(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_funcExpr(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_formalParamList(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_funcBody(nodoka_grammar *gmr);

nodoka_lex_class *grammar_program(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_sourceElements(nodoka_grammar *gmr);
static nodoka_lex_class *grammar_sourceElement(nodoka_grammar *gmr);*/

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

static nodoka_token *lookaheadRegexp(nodoka_grammar *gmr) {
    gmr->lex->regexp = true;
    nodoka_token *ret = next(gmr);
    pushback(gmr, ret);
    gmr->lex->regexp = false;
    return ret;
}

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
    gmr->lex->regexp = true;
    nodoka_token *nxt = lookahead(gmr);
    gmr->lex->regexp = false;
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

nodoka_empty_node *newEmptyNode(enum nodoka_empty_node_type type) {
    nodoka_empty_node *node = malloc(sizeof(nodoka_empty_node));
    node->base.clazz = NODOKA_LEX_EMPTY_NODE;
    node->type = type;
    return node;
}

nodoka_unary_node *newUnaryNode(enum nodoka_unary_node_type type) {
    nodoka_unary_node *node = malloc(sizeof(nodoka_binary_node));
    node->base.clazz = NODOKA_LEX_UNARY_NODE;
    node->type = type;
    return node;
}

nodoka_binary_node *newBinaryNode(enum nodoka_empty_node_type type) {
    nodoka_binary_node *node = malloc(sizeof(nodoka_binary_node));
    node->base.clazz = NODOKA_LEX_BINARY_NODE;
    node->type = type;
    return node;
}

nodoka_ternary_node *newTernaryNode(enum nodoka_ternary_node_type type) {
    nodoka_ternary_node *node = malloc(sizeof(nodoka_ternary_node));
    node->base.clazz = NODOKA_LEX_TERNARY_NODE;
    node->type = type;
    return node;
}

nodoka_node_list *newNodeList(enum nodoka_node_list_type type, size_t length) {
    nodoka_node_list *node = malloc(sizeof(nodoka_node_list) + sizeof(nodoka_lex_class *)*length);
    node->base.clazz = NODOKA_LEX_NODE_LIST;
    node->type = type;
    node->length = length;
    return node;
}

nodoka_grammar *grammar_new(nodoka_lex *lex) {
    nodoka_grammar *gmr = malloc(sizeof(struct struct_grammar));
    gmr->lex = lex;
    gmr->next = NULL;
    gmr->listLen = 0;
    gmr->noIn = false;
    return gmr;
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
    gmr->lex->regexp = true;
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
        case NODOKA_TOKEN_REGEXP: {
            assert(0);
        }

        case NODOKA_TOKEN_LPAREN: {
            next(gmr);
            nodoka_lex_class *ret = grammar_expr(gmr);
            expect(gmr, NODOKA_TOKEN_RPAREN);
            return ret;
        }
        /* ArrayLiteral */
        /* ObjectLiteral */
        default:
            assert(0);
            return NULL;
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
    gmr->lex->regexp = true;
    nodoka_lex_class *cur;
    switch (lookahead(gmr)->type) {
        case NODOKA_TOKEN_FUNCTION: {
            assert(!"FunctionLiteral not supported yet");
            break;
        }
        case NODOKA_TOKEN_NEW: {
            /*
            discard(gmr);
            node_t *node = grammar_memberExpr(gmr);
            node_t *args;
            if (lookahead(gmr)->type == L_PAREN) {
                args = grammar_arguments(gmr);
            } else {
                args = NULL;
            }
            member_expr_new_node_t *newNode =
                (member_expr_new_node_t *)createNode(MEMBER_EXPR_NEW_NODE, sizeof(member_expr_new_node_t));
            newNode->expr = node;
            newNode->args = args;
            cur = (node_t *)newNode;
            break;*/
            assert(0);
        }
        default:
            cur = grammar_primaryExpr(gmr);
            break;
    }
    gmr->lex->regexp = false;
    while (1) {
        switch (lookahead(gmr)->type) {
            case NODOKA_TOKEN_POINT: {
                next(gmr);
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
                next(gmr);
                nodoka_lex_class *expr = grammar_expr(gmr);
                expect(gmr, NODOKA_TOKEN_RBRACKET);
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
    expect(gmr, NODOKA_TOKEN_LPAREN);
    if (lookahead(gmr)->type != NODOKA_TOKEN_RPAREN) {
        nodoka_binary_node *ret = (nodoka_binary_node *)grammar_argumentList(gmr);
        nodoka_binary_node *cur;
        size_t count;
        for (count = 1, cur = ret;
                cur->base.clazz == NODOKA_LEX_BINARY_NODE && cur->type == NODOKA_ARG_LIST_INTERNAL;
                count++, cur = (nodoka_binary_node *)cur->_1);
        nodoka_node_list *list = newNodeList(NODOKA_ARG_LIST, count);

        cur = ret;
        for (size_t i = count - 1; i > 0; i--, cur = (nodoka_binary_node *)cur->_1) {
            list->_[i] = cur->_2;
        }
        list->_[0] = (nodoka_lex_class *)cur;

        expect(gmr, NODOKA_TOKEN_RPAREN);
        return (nodoka_lex_class *)list;
    }
    next(gmr);
    return NULL;
}

BINARY_HEAD(argumentList, assignExpr)
case NODOKA_TOKEN_COMMA: {
    type = NODOKA_ARG_LIST_INTERNAL;
    break;
}
BINARY_FOOT(assignExpr);

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
                next(gmr);
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
                next(gmr);
                nodoka_lex_class *expr = grammar_expr(gmr);
                expect(gmr, NODOKA_TOKEN_RBRACKET);
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
        next(gmr);
        nodoka_unary_node *node = newUnaryNode(NODOKA_POST_INC_NODE);
        node->_1 = expr;
        return (nodoka_lex_class *)node;
    } else if (nxt->type == NODOKA_TOKEN_DEC) {
        next(gmr);
        nodoka_unary_node *node = newUnaryNode(NODOKA_POST_DEC_NODE);
        node->_1 = expr;
        return (nodoka_lex_class *)node;
    }

    return expr;
}

static nodoka_lex_class *grammar_unaryExpr(nodoka_grammar *gmr) {
    gmr->lex->regexp = true;
    uint16_t nodeClass;
    nodoka_token *lh = lookahead(gmr);
    gmr->lex->regexp = false;
    switch (lh->type) {
        case NODOKA_TOKEN_DELETE:
            nodeClass = NODOKA_DELETE_NODE;
            goto produceExpr;
        case NODOKA_TOKEN_VOID:
            nodeClass = NODOKA_VOID_NODE;
            goto produceExpr;
        case NODOKA_TOKEN_TYPEOF:
            nodeClass = TYPEOF_NODE;
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
            next(gmr);
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
        next(gmr);
        nodoka_lex_class *t_exp = grammar_assignExpr(gmr);
        expect(gmr, NODOKA_TOKEN_COLON);
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
    next(gmr);
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
 *           | TODO ContinueStatement        LOOKAHEAD(1)=CONTINUE
 *           | TODO BreakStatement           LOOKAHEAD(1)=BREAK
 *           | TODO ReturnStatement          LOOKAHEAD(1)=RETURN
 *           | TODO WithStatement            LOOKAHEAD(1)=WITH
 *           | TODO LabelledStatement        LOOKAHEAD(1)=ID         LOOKAHEAD(2)=COLON
 *           | TODO SwitchStatement          LOOKAHEAD(1)=SWITCH
 *           | ThrowStatement           LOOKAHEAD(1)=THROW
 *           | TODO TryStatement             LOOKAHEAD(1)=TRY
 *           | DebuggerStatement        LOOKAHEAD(1)=DEBUGGER
*/
nodoka_lex_class *grammar_stmt(nodoka_grammar *gmr) {
    gmr->lex->regexp = true;
    nodoka_token *nxt = lookahead(gmr);
    gmr->lex->regexp = false;
    switch (nxt->type) {
        case NODOKA_TOKEN_LBRACE:
            return grammar_block(gmr);
        case NODOKA_TOKEN_VAR:
            return grammar_varStmt(gmr);
        case NODOKA_TOKEN_SEMICOLON:
            next(gmr);
            return NULL;
        case NODOKA_TOKEN_IF:
            return grammar_ifStmt(gmr);
        case NODOKA_TOKEN_DO:
            return grammar_doWhile(gmr);
        case NODOKA_TOKEN_WHILE:
            return grammar_while(gmr);
        case NODOKA_TOKEN_FOR:
            return grammar_for(gmr);
        case NODOKA_TOKEN_THROW:
            return grammar_throw(gmr);
        case NODOKA_TOKEN_DEBUGGER:
            next(gmr);
            expectSemicolon(gmr);
            return (nodoka_lex_class *)newEmptyNode(NODOKA_DEBUGGER_STMT);
        default: {
            return grammar_exprStmt(gmr);
        }
    }
}

static nodoka_lex_class *grammar_block(nodoka_grammar *gmr) {
    expect(gmr, NODOKA_TOKEN_LBRACE);
    gmr->lex->regexp = true;
    nodoka_token *nxt = lookahead(gmr);
    gmr->lex->regexp = false;
    if (nxt->type == NODOKA_TOKEN_RBRACE) {
        next(gmr);
        return NULL;
    }
    /* Accroding to ECMA, we should check exception
     * but I am lazy */
    nodoka_lex_class *block = grammar_stmt(gmr);
    while (true) {
        gmr->lex->regexp = true;
        nodoka_token *nxt = lookahead(gmr);
        gmr->lex->regexp = false;
        if (nxt->type == NODOKA_TOKEN_RBRACE) {
            next(gmr);
            return block;
        }
        nodoka_binary_node *sb = newBinaryNode(NODOKA_STMT_LIST);
        sb->_1 = block;
        sb->_2 = grammar_stmt(gmr);
        block = (nodoka_lex_class *)sb;
    }
}

static nodoka_lex_class *grammar_varStmt(nodoka_grammar *gmr) {
    expect(gmr, NODOKA_TOKEN_VAR);
    assert(0);
}

static nodoka_lex_class *grammar_exprStmt(nodoka_grammar *gmr) {
    nodoka_lex_class *expr = grammar_expr(gmr);

    nodoka_unary_node *node = newUnaryNode(NODOKA_EXPR_STMT);
    node->_1 = expr;

    expectSemicolon(gmr);
    return (nodoka_lex_class *)node;
}

static nodoka_lex_class *grammar_ifStmt(nodoka_grammar *gmr) {
    expect(gmr, NODOKA_TOKEN_IF);
    expect(gmr, NODOKA_TOKEN_LPAREN);
    nodoka_lex_class *expr = grammar_expr(gmr);
    expect(gmr, NODOKA_TOKEN_RPAREN);
    nodoka_lex_class *first = grammar_stmt(gmr);
    nodoka_ternary_node *node = newTernaryNode(NODOKA_IF_STMT);
    node->_1 = expr;
    node->_2 = first;

    gmr->lex->regexp = true;
    nodoka_token *nxt = lookahead(gmr);
    gmr->lex->regexp = false;
    if (nxt->type == NODOKA_TOKEN_ELSE) {
        next(gmr);
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

    gmr->lex->regexp = true;
    nodoka_token *t = lookahead(gmr);
    gmr->lex->regexp = false;

    if (t->type == NODOKA_TOKEN_VAR) {
        assert(0);
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
        if (lookaheadRegexp(gmr)->type != NODOKA_TOKEN_SEMICOLON) {
            testExpr = grammar_expr(gmr);
        }
        expectAndDispose(gmr, NODOKA_TOKEN_SEMICOLON);
        nodoka_lex_class *incExpr = NULL;
        if (lookaheadRegexp(gmr)->type != NODOKA_TOKEN_RPAREN) {
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

static nodoka_lex_class *grammar_throw(nodoka_grammar *gmr) {
    expectAndDispose(gmr, NODOKA_TOKEN_THROW);

    nodoka_token *nxt = lookahead(gmr);
    if (nxt->lineBefore) {
        assert(!"Expected Expression");
    }

    nodoka_lex_class *expr = grammar_expr(gmr);
    nodoka_node_list *node = newNodeList(NODOKA_THROW_STMT, 1);
    node->_[0] = expr;
    return (nodoka_lex_class *)node;
}

/*
static node_t *grammar_funcDecl(nodoka_grammar *gmr) {
    function_node_t *func = (function_node_t *)grammar_funcExpr(gmr);
    assert(func->name.len);
    return (node_t *)func;
}

static node_t *grammar_funcExpr(nodoka_grammar *gmr) {
    expectAndDispose(gmr, FUNCTION);
    function_node_t *func = (function_node_t *)createNode(FUNCTION_NODE, sizeof(function_node_t));

    if (lookahead(gmr)->type == ID) {
        nodoka_token *id = next(gmr);
        moveString(&func->name, &id->stringValue);
        lex_disposeToken(id);
    } else {
        func->name.str = NULL;
        func->name.len = 0;
    }

    expectAndDispose(gmr, L_PAREN);

    if (lookahead(gmr)->type == ID) {
        func->args = grammar_formalParamList(gmr);
    } else {
        func->args = NULL;
    }

    expectAndDispose(gmr, R_PAREN);
    expectAndDispose(gmr, NODOKA_TOKEN_LBRACE);
    func->body = grammar_funcBody(gmr);
    expectAndDispose(gmr, NODOKA_TOKEN_RBRACE);

    return (node_t *)func;
}

/ **
 * LOOKAHEAD(1) = Identifier
 *
 * FormalParameterList := Identifier {, Identifier}
 * /
static node_t *grammar_formalParamList(nodoka_grammar *gmr) {
    nodoka_token *id = expect(gmr, ID);

    var_node_t *var = (var_node_t *)createNode(VAR_NODE, sizeof(var_node_t));
    moveString(&var->name, &id->stringValue);
    lex_disposeToken(id);
    var_node_t *last = var;

    while (lookahead(gmr)->type == COMMA) {
        nodoka_token *nextId = expect(gmr, ID);
        var_node_t *cur = (var_node_t *)createNode(VAR_NODE, sizeof(var_node_t));
        moveString(&cur->name, &nextId->stringValue);
        lex_disposeToken(nextId);
        last->header.next = (node_t *)cur;
        last = cur;
    }

    return (node_t *)var;
}

static node_t *grammar_funcBody(nodoka_grammar *gmr) {
    gmr->lex->regexp = true;
    nodoka_token *next = lookahead(gmr);
    gmr->lex->regexp = false;
    if (next->type == END_OF_FILE || next->type == NODOKA_TOKEN_RBRACE) {
        return NULL;
    }
    return grammar_sourceElements(gmr);
}

node_t *grammar_program(nodoka_grammar *gmr) {
    return grammar_funcBody(gmr);
}

static node_t *grammar_sourceElements(nodoka_grammar *gmr) {
    node_t *first = grammar_sourceElement(gmr);
    node_t *cur = first;
    while (true) {
        gmr->lex->regexp = true;
        nodoka_token *next = lookahead(gmr);
        gmr->lex->regexp = false;
        if (next->type == END_OF_FILE || next->type == NODOKA_TOKEN_RBRACE) {
            return first;
        }
        node_t *n = grammar_sourceElement(gmr);
        cur->next = n;
        cur = n;
    }
}

static node_t *grammar_sourceElement(nodoka_grammar *gmr) {
    gmr->lex->regexp = true;
    nodoka_token *next = lookahead(gmr);
    gmr->lex->regexp = false;
    if (next->type == FUNCTION) {
        return grammar_funcDecl(gmr);
    } else {
        return grammar_stmt(gmr);
    }
}*/
