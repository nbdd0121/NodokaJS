#include "js/js.h"
#include "js/lex.h"

#include "unicode/type.h"
#include "unicode/hash.h"

#include "c/stdlib.h"
#include "c/stdio.h"
#include "c/assert.h"
#include "c/stdbool.h"

enum {
    TAB = 0x9,
    VT = 0xB,
    FF = 0xC,
    SP = 0x20,
    NBSP = 0xA0,
    BOM = 0xFEFF,

    LF = 0xA,
    CR = 0xD,
    LS = 0x2028,
    PS = 0x2029,

    ZWNJ = 0x200C,
    ZWJ = 0x200D
};

static hashmap_t *keywords = NULL;

static void initKeyword(void) {
    keywords = hashmap_new_utf16(97);
    static struct {
        char *name;
        uint16_t value;
    } map[] = {
        {"break", NODOKA_TOKEN_BREAK},
        {"case", NODOKA_TOKEN_CASE},
        {"catch", NODOKA_TOKEN_CATCH},
        {"continue", NODOKA_TOKEN_CONTINUE},
        {"debugger", NODOKA_TOKEN_DEBUGGER},
        {"default", NODOKA_TOKEN_DEFAULT},
        {"delete", NODOKA_TOKEN_DELETE},
        {"do", NODOKA_TOKEN_DO},
        {"else", NODOKA_TOKEN_ELSE},
        {"finally", NODOKA_TOKEN_FINALLY},
        {"for", NODOKA_TOKEN_FOR},
        {"function", NODOKA_TOKEN_FUNCTION},
        {"if", NODOKA_TOKEN_IF},
        {"in", NODOKA_TOKEN_IN},
        {"instanceof", NODOKA_TOKEN_INSTANCEOF},
        {"new", NODOKA_TOKEN_NEW},
        {"return", NODOKA_TOKEN_RETURN},
        {"switch", NODOKA_TOKEN_SWITCH},
        {"this", NODOKA_TOKEN_THIS},
        {"throw", NODOKA_TOKEN_THROW},
        {"try", NODOKA_TOKEN_TRY},
        {"typeof", NODOKA_TOKEN_TYPEOF},
        {"var", NODOKA_TOKEN_VAR},
        {"void", NODOKA_TOKEN_VOID},
        {"while", NODOKA_TOKEN_WHILE},
        {"with", NODOKA_TOKEN_WITH},

        {"class", NODOKA_TOKEN_RESERVED_WORD},
        {"const", NODOKA_TOKEN_RESERVED_WORD},
        {"enum", NODOKA_TOKEN_RESERVED_WORD},
        {"export", NODOKA_TOKEN_RESERVED_WORD},
        {"extends", NODOKA_TOKEN_RESERVED_WORD},
        {"import", NODOKA_TOKEN_RESERVED_WORD},
        {"super", NODOKA_TOKEN_RESERVED_WORD},

        {"implements", NODOKA_TOKEN_RESERVED_STRICT},
        {"interface", NODOKA_TOKEN_RESERVED_STRICT},
        {"let", NODOKA_TOKEN_RESERVED_STRICT},
        {"package", NODOKA_TOKEN_RESERVED_STRICT},
        {"private", NODOKA_TOKEN_RESERVED_STRICT},
        {"protected", NODOKA_TOKEN_RESERVED_STRICT},
        {"public", NODOKA_TOKEN_RESERVED_STRICT},
        {"static", NODOKA_TOKEN_RESERVED_STRICT},
        {"yield", NODOKA_TOKEN_RESERVED_STRICT},

        {"null", NODOKA_TOKEN_NULL},
        {"true", NODOKA_TOKEN_TRUE},
        {"false", NODOKA_TOKEN_FALSE}
    };
    for (int i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
        utf16_string_t *utf16 = malloc(sizeof(utf16_string_t));
        *utf16 = unicode_toUtf16(UTF8_STRING(map[i].name));
        hashmap_put(keywords, utf16, (void *)(size_t)map[i].value);
    }
}

static uint16_t lookupKeyword(utf16_string_t kwd) {
    if (!keywords) {
        initKeyword();
    }
    return (size_t)hashmap_get(keywords, &kwd);
}

static uint16_t lookahead(nodoka_lex *lex) {
    if (lex->ptr == lex->content.len) {
        return 0xFFFF;
    }
    return lex->content.str[lex->ptr];
}

static uint16_t next(nodoka_lex *lex) {
    if (lex->ptr == lex->content.len) {
        return 0xFFFF;
    }
    return lex->content.str[lex->ptr++];
}

static void createBuffer(nodoka_lex *lex) {
    lex->data.length = 0;
    lex->data.size = 10;
    lex->data.buffer = malloc(10 * sizeof(uint16_t));
}

static void appendToBuffer(nodoka_lex *lex, uint16_t ch) {
    if (lex->data.length == lex->data.size) {
        lex->data.size *= (lex->data.size * 3) / 2 + 1;
        lex->data.buffer = realloc(lex->data.buffer, lex->data.size * sizeof(uint16_t));
    }
    lex->data.buffer[lex->data.length++] = ch;
}

static utf16_string_t cleanBuffer(nodoka_lex *lex) {
    utf16_string_t ret = {
        .str = lex->data.buffer,
        .len = lex->data.length
    };
    return ret;
}

static nodoka_token *newToken(enum nodoka_token_type type) {
    nodoka_token *token = malloc(sizeof(nodoka_token));
    token->base.clazz = NODOKA_LEX_TOKEN;
    token->type = type;
    token->next = NULL;
    token->lineBefore = false;
    return token;
}

static nodoka_token *stateDefault(nodoka_lex *lex);
static nodoka_token *stateSingleLineComment(nodoka_lex *lex);
static nodoka_token *stateMultiLineComment(nodoka_lex *lex);
static nodoka_token *stateIdentiferPart(nodoka_lex *lex);
static nodoka_token *stateHexIntegerLiteral(nodoka_lex *lex);
static nodoka_token *stateOctIntegerLiteral(nodoka_lex *lex);
static nodoka_token *stateDoubleString(nodoka_lex *lex);
static nodoka_token *stateSingleString(nodoka_lex *lex);

static nodoka_token *stateDefault(nodoka_lex *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case TAB:
        case VT:
        case FF:
        case SP:
        case NBSP:
        case BOM: {
            return NULL;
        }
        case CR: {
            if (lex->lookahead(lex) == LF) {
                lex->next(lex);
            }
        }
        case LF:
        case LS:
        case PS: {
            lex->lineBefore = true;
            return NULL;
        }
        case '/': {
            uint16_t next = lex->lookahead(lex);
            if (next == '/') {
                lex->state = stateSingleLineComment;
            } else if (next == '*') {
                lex->state = stateMultiLineComment;
            } else {
                if (lex->regexp) {
                    //TODO
                    assert(!"Regexp is not currently supported");
                } else {
                    if (lex->lookahead(lex) == '=') {
                        lex->next(lex);
                        return newToken(NODOKA_TOKEN_DIV_ASSIGN);
                    } else {
                        return newToken(NODOKA_TOKEN_DIV);
                    }
                }
            }
            return NULL;
        }
        case '$':
        case '_': {
            createBuffer(lex);
            appendToBuffer(lex, next);
            lex->state = stateIdentiferPart;
            return NULL;
        }
        case '\\':
            //TODO Unicode Escape Sequence
            assert(0);
        case '{':
        case '}':
        case '(':
        case ')':
        case '[':
        case ']':
        case '.':
        case ';':
        case ',':
        case '~':
        case '?':
        case ':': {
            return newToken(next);
        }
        case '<': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == '=') {
                lex->next(lex);
                return newToken(NODOKA_TOKEN_LTEQ);
            } else if (nch == '<') {
                lex->next(lex);
                if (lex->lookahead(lex) == '=') {
                    lex->next(lex);
                    return newToken(NODOKA_TOKEN_SHL_ASSIGN);
                } else {
                    return newToken(NODOKA_TOKEN_SHL);
                }
            } else {
                return newToken(NODOKA_TOKEN_LT);
            }
        }
        case '>': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == '=') {
                lex->next(lex);
                return newToken(NODOKA_TOKEN_GTEQ);
            } else if (nch == '>') {
                lex->next(lex);
                uint16_t n2ch = lex->lookahead(lex);
                if (n2ch == '=') {
                    lex->next(lex);
                    return newToken(NODOKA_TOKEN_SHR_ASSIGN);
                } else if (n2ch == '>') {
                    lex->next(lex);
                    if (lex->lookahead(lex) == '=') {
                        lex->next(lex);
                        return newToken(NODOKA_TOKEN_USHR_ASSIGN);
                    } else {
                        return newToken(NODOKA_TOKEN_USHR);
                    }
                } else {
                    return newToken(NODOKA_TOKEN_SHR);
                }
            } else {
                return newToken(NODOKA_TOKEN_GT);
            }
        }
        case '=':
        case '!': {
            if (lex->lookahead(lex) == '=') {
                lex->next(lex);
                if (lex->lookahead(lex) == '=') {
                    lex->next(lex);
                    return newToken(next == '=' ? NODOKA_TOKEN_FULL_EQ : NODOKA_TOKEN_FULL_INEQ);
                } else {
                    return newToken(next | ASSIGN_FLAG);
                }
            } else {
                return newToken(next);
            }
        }
        case '+':
        case '-':
        case '&':
        case '|': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == '=') {
                lex->next(lex);
                return newToken(next | ASSIGN_FLAG);
            } else if (nch == next) {
                lex->next(lex);
                return newToken(next | DOUBLE_FLAG);
            } else {
                return newToken(next);
            }
        }
        case '*':
        case '%':
        case '^': {
            if (lex->lookahead(lex) == '=') {
                lex->next(lex);
                return newToken(next | ASSIGN_FLAG);
            } else {
                return newToken(next);
            }
        }
        case '0': {
            uint16_t nch = lex->lookahead(lex);
            if (nch == 'x' || nch == 'X') {
                lex->next(lex);
                lex->state = stateHexIntegerLiteral;
                lex->data.number = 0;
                return NULL;
            } else if (nch >= '0' && nch <= '9') {
                if (lex->strictMode) {
                    assert(!"Syntax Error: Octal literals are not allowed in strict mode.");
                } else {
                    lex->state = stateOctIntegerLiteral;
                    lex->data.number = 0;
                }
                return NULL;
            }
        }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            assert(!"DEC not supported yet");
        }
        case '"': {
            lex->state = stateDoubleString;
            createBuffer(lex);
            return NULL;
        }
        case '\'': {
            lex->state = stateSingleString;
            createBuffer(lex);
            return NULL;
        }
        case 0xFFFF: {
            lex->lineBefore = true;
            return newToken(NODOKA_TOKEN_EOF);
        }
    }

    switch (unicode_getType(next)) {
        case SPACE_SEPARATOR: {
            return NULL;
        }
        case UPPERCASE_LETTER:
        case LOWERCASE_LETTER:
        case TITLECASE_LETTER:
        case MODIFIER_LETTER:
        case OTHER_LETTER:
        case LETTER_NUMBER: {
            createBuffer(lex);
            appendToBuffer(lex, next);
            lex->state = stateIdentiferPart;
            return NULL;
        }
    }
    assert(0);
    return NULL;
}

static nodoka_token *stateSingleLineComment(nodoka_lex *lex) {
    uint16_t next = lex->lookahead(lex);
    switch (next) {
        case CR:
        case LF:
        case LS:
        case PS: {
            lex->state = stateDefault;
            return NULL;
        }
    }
    lex->next(lex);
    return NULL;
}

static nodoka_token *stateMultiLineComment(nodoka_lex *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case '*': {
            if (lex->lookahead(lex) == '/') {
                lex->next(lex);
                lex->state = stateDefault;
            }
            return NULL;
        }
        case CR:
        case LF:
        case LS:
        case PS: {
            lex->lineBefore = true;
        }
    }
    return NULL;
}

static nodoka_token *stateIdentiferPart(nodoka_lex *lex) {
    uint16_t next = lex->lookahead(lex);
    switch (next) {
        case '$':
        case '_':
        case ZWNJ:
        case ZWJ: {
            lex->next(lex);
            appendToBuffer(lex, next);
            return NULL;
        }
        case '\\': {
            //TODO Unicode Escape Sequence
            assert(0);
        }
    }
    switch (unicode_getType(next)) {
        case UPPERCASE_LETTER:
        case LOWERCASE_LETTER:
        case TITLECASE_LETTER:
        case MODIFIER_LETTER:
        case OTHER_LETTER:
        case LETTER_NUMBER:
        case CONNECTOR_PUNCTUATION:
        case DECIMAL_DIGIT_NUMBER:
        case NON_SPACING_MARK:
        case COMBINING_SPACING_MARK: {
            lex->next(lex);
            appendToBuffer(lex, next);
            return NULL;
        }
    }
    lex->state = stateDefault;
    utf16_string_t str = cleanBuffer(lex);

    if (!lex->parseId) {
        nodoka_token *token = newToken(NODOKA_TOKEN_ID);
        token->stringValue = str;
        return token;
    }

    uint16_t type = lookupKeyword(str);

    if (type == NODOKA_TOKEN_RESERVED_STRICT) {
        if (lex->strictMode) {
            type = NODOKA_TOKEN_RESERVED_WORD;
        } else {
            type = 0;
        }
    }
    if (!type) {
        nodoka_token *token = newToken(NODOKA_TOKEN_ID);
        token->stringValue = str;
        return token;
    } else if (type == NODOKA_TOKEN_RESERVED_WORD) {
        assert(!"SyntaxError: Unexpected reserved word.");
        return NULL;
    } else {
        free(str.str);
        return newToken(type);
    }
}

static nodoka_token *stateOctIntegerLiteral(nodoka_lex *lex) {
    uint16_t next = lex->lookahead(lex);
    if (next >= '0' && next <= '7') {
        lex->next(lex);
        lex->data.number = lex->data.number * 8 + (next - '0');
    }  else {
        if (next == '8' || next == '9' || next == '$' || next == '_' || next == '\\') {
            assert(!"SyntaxError: Unexpected character after number literal.");
        }
        switch (unicode_getType(next)) {
            case UPPERCASE_LETTER:
            case LOWERCASE_LETTER:
            case TITLECASE_LETTER:
            case MODIFIER_LETTER:
            case OTHER_LETTER:
            case LETTER_NUMBER:
                assert(!"SyntaxError: Unexpected character after number literal.");
        }

        lex->state = stateDefault;

        nodoka_token *token = newToken(NODOKA_TOKEN_NUM);
        token->numberValue = lex->data.number;

        return token;
    }
    return NULL;
}

static nodoka_token *stateHexIntegerLiteral(nodoka_lex *lex) {
    uint16_t next = lex->lookahead(lex);
    if (next >= '0' && next <= '9') {
        lex->next(lex);
        lex->data.number = lex->data.number * 16. + (next - '0');
    } else if (next >= 'a' && next <= 'f') {
        lex->next(lex);
        lex->data.number = lex->data.number * 16. + (10 + (next - 'a'));
    } else if (next >= 'A' && next <= 'F') {
        lex->next(lex);
        lex->data.number = lex->data.number * 16. + (10 + (next - 'A'));
    } else {
        if (next == '$' || next == '_' || next == '\\') {
            assert(!"SyntaxError: Unexpected character after number literal.");
        }
        switch (unicode_getType(next)) {
            case UPPERCASE_LETTER:
            case LOWERCASE_LETTER:
            case TITLECASE_LETTER:
            case MODIFIER_LETTER:
            case OTHER_LETTER:
            case LETTER_NUMBER:
                assert(!"SyntaxError: Unexpected character after number literal.");
        }

        lex->state = stateDefault;

        nodoka_token *token = newToken(NODOKA_TOKEN_NUM);
        token->numberValue = lex->data.number;

        return token;
    }
    return NULL;
}

static void dealEscapeSequence(nodoka_lex *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case CR: {
            if (lex->lookahead(lex) == LF) {
                lex->next(lex);
                return;
            }
        }
        case LF:
        case LS:
        case PS:
            return;
        default:
            assert(0);
    }
}

static nodoka_token *stateDoubleString(nodoka_lex *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case '"': {
            lex->state = stateDefault;

            nodoka_token *token = newToken(NODOKA_TOKEN_STR);
            token->stringValue = cleanBuffer(lex);

            return token;
        }
        case '\\': {
            dealEscapeSequence(lex);
            return NULL;
        }
        case CR:
        case LF:
        case LS:
        case PS:
        case 0xFFFF:
            assert(!"SyntaxError: String literal is not enclosed.");
        default: {
            appendToBuffer(lex, next);
            return NULL;
        }
    }
}

static nodoka_token *stateSingleString(nodoka_lex *lex) {
    uint16_t next = lex->next(lex);
    switch (next) {
        case '\'': {
            lex->state = stateDefault;

            nodoka_token *token = newToken(NODOKA_TOKEN_STR);
            token->stringValue = cleanBuffer(lex);

            return token;
        }
        case '\\': {
            dealEscapeSequence(lex);
            return NULL;
        }
        case CR:
        case LF:
        case LS:
        case PS:
        case 0xFFFF:
            assert(!"SyntaxError: String literal is not enclosed.");
        default: {
            appendToBuffer(lex, next);
            return NULL;
        }
    }
}

nodoka_lex *lex_new(char *chr) {
    nodoka_lex *l = malloc(sizeof(struct struct_lex));
    l->next = next;
    l->lookahead = lookahead;
    l->state = stateDefault;
    l->content = unicode_toUtf16(UTF8_STRING(chr));
    l->ptr = 0;
    l->regexp = false;
    l->strictMode = true;
    l->lineBefore = false;
    l->parseId = true;
    return l;
}

nodoka_token *lex_next(nodoka_lex *lex) {
    while (1) {
        nodoka_token *ret = lex->state(lex);
        if (ret) {
            ret->lineBefore = lex->lineBefore;
            lex->lineBefore = false;
            return ret;
        }
    }
}

