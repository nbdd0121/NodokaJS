#include "c/assert.h"

#include "js/object.h"
#include "js/builtin.h"

#include "unicode/hash.h"

enum nodoka_completion nodoka_call(nodoka_context *C, nodoka_object *O, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    return O->call(C, O, this, ret, argc, argv);
}

enum nodoka_completion nodoka_construct(nodoka_context *C, nodoka_object *O, nodoka_data **ret, int argc, nodoka_data **argv) {
    return O->construct(C, O, ret, argc, argv);
}

static enum nodoka_completion function_call(nodoka_context *C, nodoka_object *O, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv) {
    nodoka_object *thisBinding;
    if (true/*!strict*/) {
        if (this->type == NODOKA_NULL || this->type == NODOKA_UNDEF) {
            thisBinding = C->global->global;
        } else if (this->type != NODOKA_OBJECT) {
            thisBinding = nodoka_toObject(C, this);
        } else {
            thisBinding = (nodoka_object *)this;
        }
    }
    nodoka_code *code = O->code;
    nodoka_envRec *rec = nodoka_newDeclEnvRecord(O->scope);
    nodoka_context *context = nodoka_newContext(C->global, rec, code, thisBinding);
    for (int i = 0; i < code->formalParameters.length; i++) {
        nodoka_setMutableBinding(rec, code->formalParameters.array[i], i >= argc ? nodoka_undefined : argv[i]);
    }
    if (code->name && code->name->value.len) {
        nodoka_setMutableBinding(rec, code->name, (nodoka_data *)O);
    }
    /* FunctionDeclaration */
    /* Argument object */
    /* Variable list */
    enum nodoka_completion comp = nodoka_exec(context, ret);
    return comp;
}

static enum nodoka_completion function_construct(nodoka_context *C, nodoka_object *O, nodoka_data **ret, int argc, nodoka_data **argv) {
    nodoka_object *obj = nodoka_newObject(C->global);
    nodoka_data *proto = nodoka_get(O, nodoka_newStringFromUtf8("prototype"));
    if (proto->type == NODOKA_OBJECT) {
        obj->prototype = (nodoka_object *)proto;
    }
    enum nodoka_completion comp = nodoka_call(C, O, (nodoka_data *)obj, ret, argc, argv);
    if ((*ret)->type != NODOKA_OBJECT) {
        *ret = (nodoka_data *)obj;
    }
    return comp;
}

nodoka_object *nodoka_newFunction(nodoka_context *context, nodoka_code *code) {
    nodoka_object *F = nodoka_newNativeFunction(context->global, function_call, code->formalParameters.length);
    //F->GET
    F->construct = function_construct;
    //F->HasInstance
    F->scope = context->env;
    F->code = code;
    nodoka_object *proto = nodoka_newObject(context->global);
    nodoka_global_defineValue(proto, "constructor", (nodoka_data *)F, true, false, true);
    nodoka_global_defineValue(F, "prototype", (nodoka_data *)proto, true, false, false);
    /* strict blah */
    return F;
}