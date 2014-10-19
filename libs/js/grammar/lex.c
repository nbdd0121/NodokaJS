#include "js/js.h"
#include "js/lex.h"

#include "unicode/type.h"
#include "unicode/hash.h"

#include "c/stdlib.h"
#include "c/stdio.h"
#include "c/assert.h"
#include "c/stdbool.h"
#include "c/math.h"

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

static uint16_t dealUnicodeEscapeSequence(nodoka_lex *lex) {
    uint16_t val;
    uint16_t d1 = lex->next(lex);
    if (d1 >= '0' && d1 <= '9') {
        val = d1 - '0';
    } else if (d1 >= 'A' && d1 <= 'F') {
        val = d1 - 'A' + 10;
    } else if (d1 >= 'a' && d1 <= 'f') {
        val = d1 - 'a' + 10;
    } else {
        assert(!"SyntaxError: Expected hex digits in unicode escape sequence");
    }
    val *= 16;
    uint16_t d2 = lex->next(lex);
    if (d2 >= '0' && d2 <= '9') {
        val += d2 - '0';
    } else if (d2 >= 'A' && d2 <= 'F') {
        val += d2 - 'A' + 10;
    } else if (d2 >= 'a' && d2 <= 'f') {
        val += d2 - 'a' + 10;
    } else {
        assert(!"SyntaxError: Expected hex digits in unicode escape sequence");
    }
    val *= 16;
    uint16_t d3 = lex->next(lex);
    if (d3 >= '0' && d3 <= '9') {
        val += d3 - '0';
    } else if (d3 >= 'A' && d3 <= 'F') {
        val += d3 - 'A' + 10;
    } else if (d3 >= 'a' && d3 <= 'f') {
        val += d3 - 'a' + 10;
    } else {
        assert(!"SyntaxError: Expected hex digits in unicode escape sequence");
    }
    val *= 16;
    uint16_t d4 = lex->next(lex);
    if (d4 >= '0' && d4 <= '9') {
        val += d4 - '0';
    } else if (d4 >= 'A' && d4 <= 'F') {
        val += d4 - 'A' + 10;
    } else if (d4 >= 'a' && d4 <= 'f') {
        val += d4 - 'a' + 10;
    } else {
        assert(!"SyntaxError: Expected hex digits in unicode escape sequence");
    }
    return val;
}

static nodoka_token *stateDefault(nodoka_lex *lex);
static nodoka_token *stateSingleLineComment(nodoka_lex *lex);
static nodoka_token *stateMultiLineComment(nodoka_lex *lex);
static nodoka_token *stateIdentifierPart(nodoka_lex *lex);
static nodoka_token *stateHexIntegerLiteral(nodoka_lex *lex);
static nodoka_token *stateOctIntegerLiteral(nodoka_lex *lex);
static nodoka_token *stateDecLiteral(nodoka_lex *lex);
static nodoka_token *stateDecimal(nodoka_lex *lex);
static nodoka_token *stateExpSign(nodoka_lex *lex);
static nodoka_token *stateExpPart(nodoka_lex *lex);
static nodoka_token *stateDoubleString(nodoka_lex *lex);
static nodoka_token *stateSingleString(nodoka_lex *lex);
static nodoka_token *stateRegexpChar(nodoka_lex *lex);
static nodoka_token *stateRegexpClassChar(nodoka_lex *lex);
static nodoka_token *stateRegexpFlags(nodoka_lex *lex);

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
                return NULL;
            } else if (next == '*') {
                lex->state = stateMultiLineComment;
                return NULL;
            } else if (lex->lookahead(lex) == '=') {
                lex->next(lex);
                return newToken(NODOKA_TOKEN_DIV_ASSIGN);
            } else {
                return newToken(NODOKA_TOKEN_DIV);
            }
        }
        case '$':
        case '_': {
            createBuffer(lex);
            appendToBuffer(lex, next);
            lex->state = stateIdentifierPart;
            return NULL;
        }
        case '\\': {
            if (lex->next(lex) != 'u') {
                assert(!"SyntaxError: Expected unicode escape sequence");
            }
            uint16_t val = dealUnicodeEscapeSequence(lex);
            if (val != '_' && val != '$') {
                switch (unicode_getType(val)) {
                    case UPPERCASE_LETTER:
                    case LOWERCASE_LETTER:
                    case TITLECASE_LETTER:
                    case MODIFIER_LETTER:
                    case OTHER_LETTER:
                    case LETTER_NUMBER: {
                        break;
                    }
                    default: assert(!"SyntaxError: Illegal Identifier Start");
                }
            }
            createBuffer(lex);
            appendToBuffer(lex, val);
            lex->state = stateIdentifierPart;
            return NULL;
        }
        case '.': {
            uint16_t nch = lex->lookahead(lex);
            if (nch >= '0' && nch <= '9') {
                lex->state = stateDecimal;
                lex->data.number = 0;
                lex->data.decimalLen = 0;
                return NULL;
            }
        }
        case '{':
        case '}':
        case '(':
        case ')':
        case '[':
        case ']':
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
                    return newToken(next == '=' ? NODOKA_TOKEN_STRICT_EQ : NODOKA_TOKEN_STRICT_INEQ);
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
            lex->state = stateDecLiteral;
            lex->data.number = next - '0';
            lex->data.decimalLen = 0;
            return NULL;
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
            lex->state = stateIdentifierPart;
            return NULL;
        }
    }
    assert(!"SyntaxError: Unexpected source character");
    return NULL;
}

static nodoka_token *stateSingleLineComment(nodoka_lex *lex) {
    uint16_t next = lex->lookahead(lex);
    switch (next) {
        case CR:
        case LF:
        case LS:
        case PS:
        case 0xFFFF: {
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
            break;
        }
        case 0xFFFF: {
            assert(!"SyntaxError: MultiLineComment not enclosed");
        }
    }
    return NULL;
}

static nodoka_token *stateIdentifierPart(nodoka_lex *lex) {
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
            lex->next(lex);
            if (lex->next(lex) != 'u') {
                assert(!"SyntaxError: Expected unicode escape sequence");
            }
            uint16_t val = dealUnicodeEscapeSequence(lex);
            if (val != '_' && val != '$' && val != ZWNJ && val != ZWJ) {
                switch (unicode_getType(val)) {
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
                        break;
                    }
                    default: assert(!"SyntaxError: Illegal Identifier Part");
                }
            }
            appendToBuffer(lex, val);
            lex->state = stateIdentifierPart;
            return NULL;
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

static nodoka_token *stateDecLiteral(nodoka_lex *lex) {
    uint16_t next = lex->lookahead(lex);
    if (next >= '0' && next <= '9') {
        lex->next(lex);
        lex->data.number = lex->data.number * 10. + (next - '0');
    } else if (next == '.') {
        lex->next(lex);
        lex->state = stateDecimal;
        return NULL;
    } else if (next == 'e' || next == 'E') {
        lex->next(lex);
        lex->state = stateExpSign;
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

static nodoka_token *stateDecimal(nodoka_lex *lex) {
    uint16_t next = lex->lookahead(lex);
    if (next >= '0' && next <= '9') {
        lex->next(lex);
        lex->data.number = lex->data.number * 10. + (next - '0');
        lex->data.decimalLen++;
    } else if (next == 'e' || next == 'E') {
        lex->next(lex);
        lex->state = stateExpSign;
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
        token->numberValue = lex->data.number * pow(10., -1.*lex->data.decimalLen);

        return token;
    }
    return NULL;
}

static nodoka_token *stateExpSign(nodoka_lex *lex) {
    uint16_t next = lex->lookahead(lex);
    if (next >= '0' && next <= '9') {
        lex->data.sign = true;
    } else if (next == '+') {
        lex->next(lex);
        lex->data.sign = true;
    } else if (next == '-') {
        lex->next(lex);
        lex->data.sign = false;
    } else {
        assert(!"SyntaxError: Expected +, - or digits after the exponential mark.");
    }
    lex->data.expPart = 0;
    lex->state = stateExpPart;
    return NULL;
}

static nodoka_token *stateExpPart(nodoka_lex *lex) {
    uint16_t next = lex->lookahead(lex);
    if (next >= '0' && next <= '9') {
        lex->next(lex);
        lex->data.expPart = lex->data.expPart * 10 + (next - '0');
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
        int expPart = (lex->data.sign ? 1 : -1) * (int)lex->data.expPart - (int)lex->data.decimalLen;
        token->numberValue = lex->data.number * pow(10., 1.*expPart);

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
        case PS: return;
        case '\'': appendToBuffer(lex, '\''); return;
        case '"': appendToBuffer(lex, '"'); return;
        case '\\': appendToBuffer(lex, '\\'); return;
        case 'b': appendToBuffer(lex, '\b'); return;
        case 'f': appendToBuffer(lex, '\f'); return;
        case 'n': appendToBuffer(lex, '\n'); return;
        case 'r': appendToBuffer(lex, '\r'); return;
        case 't': appendToBuffer(lex, '\t'); return;
        case 'v': appendToBuffer(lex, '\v'); return;
        case '0': {
            uint16_t next = lex->lookahead(lex);
            if (next >= '0' && next <= '9') {
                assert(!"UnsupprtedError: Oct Escape Sequence");
            }
            return;
        }
        case 'x': {
            uint16_t val;
            uint16_t d1 = lex->next(lex);
            if (d1 >= '0' && d1 <= '9') {
                val = d1 - '0';
            } else if (d1 >= 'A' && d1 <= 'F') {
                val = d1 - 'A' + 10;
            } else if (d1 >= 'a' && d1 <= 'f') {
                val = d1 - 'a' + 10;
            } else {
                assert(!"SyntaxError: Expected hex digits in hex escape sequence");
            }
            uint16_t d2 = lex->next(lex);
            val *= 16;
            if (d2 >= '0' && d2 <= '9') {
                val += d2 - '0';
            } else if (d2 >= 'A' && d2 <= 'F') {
                val += d2 - 'A' + 10;
            } else if (d2 >= 'a' && d2 <= 'f') {
                val += d2 - 'a' + 10;
            } else {
                assert(!"SyntaxError: Expected hex digits in hex escape sequence");
            }
            appendToBuffer(lex, val);
            return;
        }
        case 'u': {
            uint16_t val;
            uint16_t d1 = lex->next(lex);
            if (d1 >= '0' && d1 <= '9') {
                val = d1 - '0';
            } else if (d1 >= 'A' && d1 <= 'F') {
                val = d1 - 'A' + 10;
            } else if (d1 >= 'a' && d1 <= 'f') {
                val = d1 - 'a' + 10;
            } else {
                assert(!"SyntaxError: Expected hex digits in unicode escape sequence");
            }
            val *= 16;
            uint16_t d2 = lex->next(lex);
            if (d2 >= '0' && d2 <= '9') {
                val += d2 - '0';
            } else if (d2 >= 'A' && d2 <= 'F') {
                val += d2 - 'A' + 10;
            } else if (d2 >= 'a' && d2 <= 'f') {
                val += d2 - 'a' + 10;
            } else {
                assert(!"SyntaxError: Expected hex digits in unicode escape sequence");
            }
            val *= 16;
            uint16_t d3 = lex->next(lex);
            if (d3 >= '0' && d3 <= '9') {
                val += d3 - '0';
            } else if (d3 >= 'A' && d3 <= 'F') {
                val += d3 - 'A' + 10;
            } else if (d3 >= 'a' && d3 <= 'f') {
                val += d3 - 'a' + 10;
            } else {
                assert(!"SyntaxError: Expected hex digits in unicode escape sequence");
            }
            val *= 16;
            uint16_t d4 = lex->next(lex);
            if (d4 >= '0' && d4 <= '9') {
                val += d4 - '0';
            } else if (d4 >= 'A' && d4 <= 'F') {
                val += d4 - 'A' + 10;
            } else if (d4 >= 'a' && d4 <= 'f') {
                val += d4 - 'a' + 10;
            } else {
                assert(!"SyntaxError: Expected hex digits in unicode escape sequence");
            }
            appendToBuffer(lex, val);
            return;
        }
        default: appendToBuffer(lex, next); return;
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

static nodoka_token *stateRegexpChar(nodoka_lex *lex) {
    uint16_t nxt = lex->next(lex);
    switch (nxt) {
        case '/':
            lex->state = stateRegexpFlags;
            lex->data.regexpContent = cleanBuffer(lex);
            return NULL;
        case '\'':
            nxt = lex->next(lex);
            switch (nxt) {
                case CR:
                case LF:
                case LS:
                case PS:
                case 0xFFFF:
                    assert(!"SyntaxError: Regexp literal is not enclosed");
            }
            appendToBuffer(lex, '\'');
            appendToBuffer(lex, nxt);
            return NULL;
        case '[':
            appendToBuffer(lex, '[');
            lex->state = stateRegexpClassChar;
            return NULL;
        case CR:
        case LF:
        case LS:
        case PS:
        case 0xFFFF:
            assert(!"SyntaxError: Regexp literal is not enclosed");
        default:
            appendToBuffer(lex, nxt);
            return NULL;
    }
}

static nodoka_token *stateRegexpClassChar(nodoka_lex *lex) {
    uint16_t nxt = lex->next(lex);
    switch (nxt) {
        case ']':
            appendToBuffer(lex, ']');
            lex->state = stateRegexpChar;
            return NULL;
        case '\'':
            nxt = lex->next(lex);
            switch (nxt) {
                case CR:
                case LF:
                case LS:
                case PS:
                case 0xFFFF:
                    assert(!"SyntaxError: Regexp literal is not enclosed");
            }
            appendToBuffer(lex, '\'');
            appendToBuffer(lex, nxt);
            return NULL;
        case CR:
        case LF:
        case LS:
        case PS:
        case 0xFFFF:
            assert(!"SyntaxError: Regexp literal is not enclosed");
        default:
            appendToBuffer(lex, nxt);
            return NULL;
    }
}

static nodoka_token *stateRegexpFlags(nodoka_lex *lex) {
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
            lex->next(lex);
            if (lex->next(lex) != 'u') {
                assert(!"SyntaxError: Expected unicode escape sequence");
            }
            uint16_t val = dealUnicodeEscapeSequence(lex);
            if (val != '_' && val != '$' && val != ZWNJ && val != ZWJ) {
                switch (unicode_getType(val)) {
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
                        break;
                    }
                    default: assert(!"SyntaxError: Illegal Identifier Part");
                }
            }
            appendToBuffer(lex, val);
            lex->state = stateIdentifierPart;
            return NULL;
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

    nodoka_token *token = newToken(NODOKA_TOKEN_REGEXP);
    token->regexp = lex->data.regexpContent;
    token->flags = cleanBuffer(lex);
    return token;
}

nodoka_token *lex_regexp(nodoka_lex *lex, bool assign) {
    createBuffer(lex);
    if (assign) {
        appendToBuffer(lex, '=');
    }
    lex->state = stateRegexpChar;
    return lex_next(lex);
}

nodoka_lex *lex_new(utf16_string_t utf16) {
    nodoka_lex *l = malloc(sizeof(struct struct_lex));
    l->next = next;
    l->lookahead = lookahead;
    l->state = stateDefault;
    l->content = utf16;
    l->ptr = 0;
    l->strictMode = true;
    l->lineBefore = false;
    l->parseId = true;
    return l;
}

void lex_dispose(nodoka_lex *lex) {
    free(lex);
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

