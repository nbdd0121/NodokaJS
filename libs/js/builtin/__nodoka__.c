#include "c/stdio.h"
#include "c/math.h"
#include "c/stdlib.h"

#include "js/builtin.h"
#include "js/object.h"

static enum nodoka_completion print_native(nodoka_global *global, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    for (int i = 0; i < argc; i++) {
        nodoka_string *str = nodoka_toString(global, argv[i]);
        unicode_putUtf16(str->value);
    }
    *ret = nodoka_undefined;
    return NODOKA_COMPLETION_RETURN;
}

enum nodoka_completion nodoka_colorDir(nodoka_global *global, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    for (int i = 0; i < argc; i++) {
        nodoka_data *data = argv[i];
        switch (data->type) {
            case NODOKA_UNDEF: printf("\033[2;37mundefined"); break;
            case NODOKA_NULL: printf("\033[1;39mnull"); break;
            case NODOKA_NUMBER:
            case NODOKA_BOOL: printf("\033[0;33m"); unicode_putUtf16(nodoka_toString(global, data)->value); break;
            case NODOKA_STRING: printf("\033[0;32m'"); unicode_putUtf16(((nodoka_string *)data)->value); printf("'"); break;
            case NODOKA_OBJECT: unicode_putUtf16(nodoka_toString(global, data)->value); break;
            default: assert(0);
        }
        printf("\033[0m ");
    }
    printf("\n");
    *ret = nodoka_undefined;
    return NODOKA_COMPLETION_RETURN;
}

static uint32_t get6Bits(uint16_t cha) {
    if (cha >= 'A' && cha <= 'Z') {
        return cha - 'A';
    } else if (cha >= 'a' && cha <= 'z') {
        return cha - 'a' + 26;
    } else if (cha >= '0' && cha <= '9') {
        return cha - '0' + 52;
    } else if (cha == '+') {
        return 62;
    } else {
        assert(cha == '/');
        return 63;
    }
}

static enum nodoka_completion base64_decodeNative(nodoka_global *global, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc != 1 || argv[0]->type != NODOKA_STRING) {
        *ret = (nodoka_data *)nodoka_newStringFromUtf8("TypeError: Illegal Signature");
        return NODOKA_COMPLETION_THROW;
    }
    nodoka_string *str = (nodoka_string *)argv[0];
    if (str->value.len % 4 != 0) {
        *ret = (nodoka_data *)nodoka_newStringFromUtf8("Error: Invalid Base64 String");
        return NODOKA_COMPLETION_THROW;
    }

    size_t triples = str->value.len / 4;
    size_t decodedLen = triples * 3;
    if (str->value.str[str->value.len - 1] == '=') {
        decodedLen--;
        if (str->value.str[str->value.len - 2] == '=') {
            decodedLen--;
        }
    }
    char *decodedStr = malloc(decodedLen + 1);
    for (int i = 0; i < triples - 1; i++) {
        uint32_t buffer = (get6Bits(str->value.str[4 * i]) << 18) |
                          (get6Bits(str->value.str[4 * i + 1]) << 12) |
                          (get6Bits(str->value.str[4 * i + 2]) << 6) |
                          get6Bits(str->value.str[4 * i + 3]);
        decodedStr[3 * i] = (buffer >> 16) & 0xFF;
        decodedStr[3 * i + 1] = (buffer >> 8) & 0xFF;
        decodedStr[3 * i + 2] = buffer & 0xFF;
    }
    if (str->value.str[str->value.len - 1] == '=') {
        if (str->value.str[str->value.len - 2] == '=') {
            uint32_t buffer = (get6Bits(str->value.str[str->value.len - 4]) << 2) |
                              (get6Bits(str->value.str[str->value.len - 3]) >> 4);
            decodedStr[decodedLen - 1] = buffer & 0xFF;
        } else {
            uint32_t buffer = (get6Bits(str->value.str[str->value.len - 4]) << 10) |
                              (get6Bits(str->value.str[str->value.len - 3]) << 4) |
                              (get6Bits(str->value.str[str->value.len - 2]) >> 2);
            decodedStr[decodedLen - 1] = (buffer >> 8) & 0xFF;
            decodedStr[decodedLen - 2] = buffer & 0xFF;
        }
    }
    decodedStr[decodedLen] = 0;

    *ret = (nodoka_data *)nodoka_newStringFromUtf8(decodedStr);
    return NODOKA_COMPLETION_RETURN;
}

enum nodoka_completion memoryAddress(nodoka_global *global, nodoka_object *func, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    if (argc == 0) {
        *ret = (nodoka_data *)nodoka_newStringFromUtf8("TypeError: Expected argument for __nodoka__.memoryAddress");
        return NODOKA_COMPLETION_THROW;
    }
    *ret = (nodoka_data *)nodoka_newNumber((size_t)argv[0]);
    return NODOKA_COMPLETION_RETURN;
}

nodoka_object *nodoka_newGlobal_nodoka(nodoka_global *global) {
    nodoka_object *nodoka = nodoka_newObject(global);
    nodoka_global_defineFunc(global, nodoka, "print", print_native, 1, false, false, false);
    nodoka_global_defineFunc(global, nodoka, "colorDir", nodoka_colorDir, 1, false, false, false);
    nodoka_global_defineFunc(global, nodoka, "decodeBase64", base64_decodeNative, 1, false, false, false);
    nodoka_global_defineFunc(global, nodoka, "memoryAddress", memoryAddress, 1, false, false, false);
    return nodoka;
}