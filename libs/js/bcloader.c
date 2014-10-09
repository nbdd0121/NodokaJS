#include "js/js.h"
#include "js/bytecode.h"

#include "c/string.h"
#include "c/assert.h"
#include "c/stdlib.h"
#include "c/stdio.h"

static uint16_t read16(char *buffer, size_t *ptr) {
    uint16_t val = (buffer[*ptr] << 8) | buffer[*ptr + 1];
    *ptr += 2;
    return val;
}

static void write16(char *buffer, size_t *ptr, uint16_t val) {
    buffer[*ptr] = (val >> 8) & 0xFF;
    buffer[*ptr + 1] = val & 0xFF;
    *ptr += 2;
}

char *nodoka_readFile(char *path, size_t *sizePtr) {
    FILE *f = fopen(path, "r+");
    if (!f) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    char *buffer = malloc(size + 1);
    rewind(f);
    size_t numread = fread(buffer, sizeof(char), size, f);
    fclose(f);
    assert(numread == size);
    buffer[size] = 0;
    if (sizePtr) {
        *sizePtr = size;
    }
    return buffer;
}

void nodoka_writeFile(char *path, char *buffer, size_t size) {
    FILE *f = fopen(path, "w+");
    size_t numwrite = fwrite(buffer, sizeof(char), size, f);
    fclose(f);
    assert(numwrite == size);
}

static nodoka_string *readConstString(char *buffer, size_t *ptr) {
    size_t length = read16(buffer, ptr);
    uint16_t *str = malloc(length * sizeof(uint16_t));
    for (int i = 0; i < length; i++) {
        str[i] = read16(buffer, ptr);
    }
    return nodoka_new_string((utf16_string_t) {
        .len = length,
         .str = str
    });
}

static size_t countString(nodoka_string *str) {
    return 2 + str->value.len * 2;
}

static void writeConstString(char *buffer, size_t *ptr, nodoka_string *string) {
    write16(buffer, ptr, string->value.len);
    for (int i = 0; i < string->value.len; i++) {
        write16(buffer, ptr, string->value.str[i]);
    }
}

static nodoka_code *readConstCodeSegment(char *buffer, size_t *ptr) {
    nodoka_code *code = (nodoka_code *)nodoka_new_data(NODOKA_CODE);
    code->strPoolLength = read16(buffer, ptr);
    code->codePoolLength = read16(buffer, ptr);
    code->bytecodeLength = read16(buffer, ptr);
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
    *ptr += code->bytecodeLength;
    return code;
}

static void writeConstCode(char *buffer, size_t *ptr, nodoka_code *code) {
    write16(buffer, ptr, code->strPoolLength);
    write16(buffer, ptr, code->codePoolLength);
    write16(buffer, ptr, code->bytecodeLength);
    for (int i = 0; i < code->strPoolLength; i++) {
        writeConstString(buffer, ptr, code->stringPool[i]);
    }
    for (int i = 0; i < code->codePoolLength; i++) {
        writeConstCode(buffer, ptr, code->codePool[i]);
    }
    memcpy(&buffer[*ptr], code->bytecode, code->bytecodeLength);
    *ptr += code->bytecodeLength;
}

static size_t countCode(nodoka_code *code) {
    size_t size = 6;
    for (int i = 0; i < code->strPoolLength; i++) {
        size += countString(code->stringPool[i]);
    }
    for (int i = 0; i < code->codePoolLength; i++) {
        size += countCode(code->codePool[i]);
    }
    size += code->bytecodeLength;
    return size;
}

nodoka_code *nodoka_loadBytecode(char *path) {
    size_t size;
    char *buffer = nodoka_readFile(path, &size);
    if (!buffer) {
        return NULL;
    }
    size_t ptr = 0;
    nodoka_code *code = readConstCodeSegment(buffer, &ptr);
    free(buffer);
    assert(ptr == size);
    return code;
}

void nodoka_storeBytecode(char *path, nodoka_code *code) {
    size_t size = countCode(code);
    char *buffer = malloc(size);
    size_t ptr = 0;
    writeConstCode(buffer, &ptr, code);
    assert(ptr == size);
    nodoka_writeFile(path, buffer, size);
    free(buffer);
}