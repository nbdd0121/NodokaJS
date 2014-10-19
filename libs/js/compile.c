#include "unicode/convert.h"

#include "js/js.h"
#include "js/bytecode.h"
#include "js/lex.h"
#include "js/pass.h"
#include "js/object.h"

nodoka_code *nodoka_compile(utf16_string_t str) {
    nodoka_lex *lex = lex_new(str);
    nodoka_grammar *grammar = grammar_new(lex);
    nodoka_lex_class *ast = grammar_program(grammar);
    grammar_dispose(grammar);
    lex_dispose(lex);

    nodoka_code_emitter *emitter = nodoka_newCodeEmitter();
    nodoka_declgen(emitter, ast);
    nodoka_emitBytecode(emitter, NODOKA_BC_UNDEF);
    nodoka_codegen(emitter, ast);
    nodoka_disposeLexNode(ast);
    nodoka_emitBytecode(emitter, NODOKA_BC_RET);

    nodoka_optimizer(emitter);
    nodoka_code *code = nodoka_packCode(emitter);
    return code;
}