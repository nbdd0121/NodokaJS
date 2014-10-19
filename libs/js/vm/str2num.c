#include "c/math.h"

#include "js/js.h"

#include "unicode/type.h"

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

nodoka_number *nodoka_str2num(nodoka_string *str) {
    if (str->numberCache) {
        return str->numberCache;
    }
    size_t ptr = 0;
    for (; ptr < str->value.len; ptr++) {
        uint16_t cha = str->value.str[ptr];
        switch (cha) {
            case TAB:
            case VT:
            case FF:
            case SP:
            case NBSP:
            case BOM:
            case CR:
            case LF:
            case LS:
            case PS: continue;
        }
        if (unicode_getType(cha) != SPACE_SEPARATOR) {
            break;
        }
    }
    if (ptr >= str->value.len) {
        str->numberCache = nodoka_nan;
        return nodoka_nan;
    }
    bool sign = true;
    switch (str->value.str[ptr]) {
        case '0': {
            if (ptr + 1 < str->value.len) {
                if (str->value.str[ptr + 1] == 'x' || str->value.str[ptr + 1] == 'X') {
                    bool invalid = true;
                    double base = 0;
                    for (ptr += 2; ptr < str->value.len; ptr++) {
                        uint16_t cha = str->value.str[ptr];
                        if (cha >= '0' && cha <= '9') {
                            invalid = false;
                            base = base * 16 + (cha - '0');
                        } else if (cha >= 'A' && cha <= 'F') {
                            invalid = false;
                            base = base * 16 + (cha - 'A' + 10);
                        } else if (cha >= 'a' && cha <= 'f') {
                            invalid = false;
                            base = base * 16 + (cha - 'a' + 10);
                        } else {
                            break;
                        }
                    }
                    for (ptr += 8; ptr < str->value.len; ptr++) {
                        uint16_t cha = str->value.str[ptr];
                        switch (cha) {
                            case TAB:
                            case VT:
                            case FF:
                            case SP:
                            case NBSP:
                            case BOM:
                            case CR:
                            case LF:
                            case LS:
                            case PS: continue;
                        }
                        if (unicode_getType(cha) != SPACE_SEPARATOR) {
                            str->numberCache = nodoka_nan;
                            return nodoka_nan;
                        }
                    }
                    if (invalid) {
                        str->numberCache = nodoka_nan;
                        return nodoka_nan;
                    }
                    str->numberCache = nodoka_newNumber(base);
                    return str->numberCache;
                } else {
                    break;
                }
            } else {
                str->numberCache = nodoka_zero;
                return nodoka_zero;
            }
        }
        case '+': ptr++; break;
        case '-': ptr++; sign = false; break;
    }
    if (ptr + 7 < str->value.len &&
            str->value.str[ptr] == 'I' &&
            str->value.str[ptr + 1] == 'n' &&
            str->value.str[ptr + 2] == 'f' &&
            str->value.str[ptr + 3] == 'i' &&
            str->value.str[ptr + 4] == 'n' &&
            str->value.str[ptr + 5] == 'i' &&
            str->value.str[ptr + 6] == 't' &&
            str->value.str[ptr + 7] == 'y') {
        for (ptr += 8; ptr < str->value.len; ptr++) {
            uint16_t cha = str->value.str[ptr];
            switch (cha) {
                case TAB:
                case VT:
                case FF:
                case SP:
                case NBSP:
                case BOM:
                case CR:
                case LF:
                case LS:
                case PS: continue;
            }
            if (unicode_getType(cha) != SPACE_SEPARATOR) {
                str->numberCache = nodoka_nan;
                return nodoka_nan;
            }
        }
        str->numberCache = nodoka_newNumber((sign ? 1 : -1) / 0.0);
        return str->numberCache;
    } else {
        double base = 0;
        uint16_t power = 0;
        bool litPowerSign = true;
        uint16_t litPower = 0;
        bool invalid = true;
        for (; ptr < str->value.len; ptr++) {
            uint16_t cha = str->value.str[ptr];
            if (cha >= '0' && cha <= '9') {
                invalid = false;
                base = base * 10 + (cha - '0');
            } else if (cha == '.') {
                ptr++;
                goto decimalPoint;
            } else if (cha == 'e' || cha == 'E') {
                ptr++;
                goto exponential;
            } else {
                goto finish;
            }
        }
        goto finish;
decimalPoint:
        for (; ptr < str->value.len; ptr++) {
            uint16_t cha = str->value.str[ptr];
            if (cha >= '0' && cha <= '9') {
                invalid = false;
                base = base * 10 + (cha - '0');
                power++;
            } else if (cha == 'e' || cha == 'E') {
                ptr++;
                goto exponential;
            } else {
                goto finish;
            }
        }
        goto finish;
exponential:
        if (ptr < str->value.len) {
            switch (str->value.str[ptr]) {
                case '+': ptr++; break;
                case '-': litPowerSign = false; ptr++; break;
            }
        }
        for (; ptr < str->value.len; ptr++) {
            uint16_t cha = str->value.str[ptr];
            if (cha >= '0' && cha <= '9') {
                base = base * 10 + cha - '0';
                power++;
            } else {
                goto finish;
            }
        }
finish:
        if (invalid) {
            str->numberCache = nodoka_nan;
            return nodoka_nan;
        }
        for (; ptr < str->value.len; ptr++) {
            uint16_t cha = str->value.str[ptr];
            switch (cha) {
                case TAB:
                case VT:
                case FF:
                case SP:
                case NBSP:
                case BOM:
                case CR:
                case LF:
                case LS:
                case PS: continue;
            }
            if (unicode_getType(cha) != SPACE_SEPARATOR) {
                str->numberCache = nodoka_nan;
                return nodoka_nan;
            }
        }
        str->numberCache = nodoka_newNumber((sign ? 1 : -1) * base * pow(10, -power + (litPowerSign ? litPower : -litPower)));
        return str->numberCache;
    }
}
