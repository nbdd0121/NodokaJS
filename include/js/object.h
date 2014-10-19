#ifndef JS_OBJECT_H
#define JS_OBJECT_H

#include "js/js.h"
#include "js/bytecode.h"
#include "data-struct/hashmap.h"

struct nodoka_prop_desc {
    nodoka_data base;
    nodoka_data *value;
    nodoka_data *get;
    nodoka_data *set;
    nodoka_data *writable;
    nodoka_data *enumerable;
    nodoka_data *configurable;
};

typedef nodoka_prop_desc *(*nodoka_getOwnProperty_func)(nodoka_object *O, nodoka_string *P);
typedef enum nodoka_completion(*nodoka_construct_func)(nodoka_context *C, nodoka_object *O, nodoka_data **ret, int argc, nodoka_data **argv);
typedef enum nodoka_completion(*nodoka_call_func)(nodoka_context *C, nodoka_object *O, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv);


struct nodoka_object {
    nodoka_data base;
    hashmap_t *prop;
    nodoka_object *prototype;
    nodoka_string *_class;
    nodoka_getOwnProperty_func getOwnProperty;

    nodoka_data *primitiveValue;
    nodoka_construct_func construct;
    nodoka_call_func call;
    // bool HasInstance(any)
    nodoka_envRec *scope;

    struct {
        size_t length;
        nodoka_string **array;
    } formalParameters;
    nodoka_code *code;
    nodoka_object *targetFunction;
    nodoka_data *boundThis;
    struct {
        size_t length;
        nodoka_data **array;
    } boundArguments;

    nodoka_string *codeString;
    bool extensible;
};

nodoka_prop_desc *nodoka_getOwnProperty(nodoka_object *O, nodoka_string *P);
nodoka_prop_desc *nodoka_getProperty(nodoka_object *O, nodoka_string *P);
nodoka_data *nodoka_get(nodoka_object *O, nodoka_string *P);
bool nodoka_canPut(nodoka_object *O, nodoka_string *P);
void nodoka_put(nodoka_object *O, nodoka_string *P, nodoka_data *V, bool throw);
bool nodoka_hasProperty(nodoka_object *O, nodoka_string *P);
bool nodoka_delete(nodoka_object *O, nodoka_string *P, bool throw);
nodoka_data *nodoka_defaultValue(nodoka_context *C, nodoka_object *O, enum nodoka_data_type hint);
bool nodoka_defineOwnProperty(nodoka_object *O, nodoka_string *P, nodoka_prop_desc *desc, bool throw);

enum nodoka_completion nodoka_call(nodoka_context *global, nodoka_object *O, nodoka_data *this, nodoka_data **ret, int argc, nodoka_data **argv);
enum nodoka_completion nodoka_construct(nodoka_context *global, nodoka_object *O, nodoka_data **ret, int argc, nodoka_data **argv);


/* prop.c */
bool nodoka_isDataDescriptor(nodoka_prop_desc *desc);
bool nodoka_isAccessorDescriptor(nodoka_prop_desc *desc);
bool nodoka_isGenericDescriptor(nodoka_prop_desc *desc);

nodoka_prop_desc *nodoka_newPropertyDesc(void);

nodoka_object *nodoka_newNativeObject(void);

#endif