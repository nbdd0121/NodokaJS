#include "js/js.h"
#include "js/bytecode.h"

#include "c/string.h"
#include "c/assert.h"
#include "c/stdlib.h"
#include "c/stdio.h"

char *nodoka_readFile(char *path, size_t *sizePtr) {
    FILE *f = fopen(path, "r+");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    char *buffer = malloc(size + 1);
    rewind(f);
    size_t numread = fread(buffer, sizeof(char), size, f);
    assert(numread == size);
    buffer[size] = 0;
    if (sizePtr) {
        *sizePtr = size;
    }
    return path;
}

static nodoka_string *readConstString(char *buffer, size_t *ptr) {
    size_t length = (buffer[*ptr] << 8) | buffer[*ptr + 1];
    *ptr += 2;
    uint16_t *str = malloc(length);
    memcpy(str, &buffer[*ptr], length);
    *ptr += length;
    return nodoka_new_string((utf16_string_t) {
        .len = length,
         .str = str
    });
}

static nodoka_code *readConstCodeSegment(char *buffer, size_t *ptr) {
    nodoka_code *code = (nodoka_code *)nodoka_new_data(NODOKA_CODE);
    code->strPoolLength = (buffer[*ptr] << 8) | buffer[*ptr + 1];
    *ptr += 2;
    code->codePoolLength = (buffer[*ptr] << 8) | buffer[*ptr + 1];
    *ptr += 2;
    code->bytecodeLength = (buffer[*ptr] << 8) | buffer[*ptr + 1];
    *ptr += 2;
    code->stringPool = malloc(code->strPoolLength * sizeof(nodoka_string *));
    code->codePool = malloc(code->codePoolLength * sizeof(nodoka_code *));
    code->bytecode = malloc(code->bytecodeLength);
    for (int i = 0; i < code->strPoolLength; i++) {
        code->stringPool[i] = readConstString(buffer, ptr);
    }
    for (int i = 0; i < code->codePoolLength; i++) {
        code->codePool[i] = readConstCodeSegment(buffer, ptr);
    }
    memcpy(code->bytecode, &buffer[*ptr], code->bytecodeLength);
    return code;
}

nodoka_code *nodoka_loadBytecode(char *path) {
    char *buffer = nodoka_readFile(path, NULL);
    size_t ptr = 0;
    nodoka_code *code = readConstCodeSegment(buffer, &ptr);
    free(buffer);
    return code;
}