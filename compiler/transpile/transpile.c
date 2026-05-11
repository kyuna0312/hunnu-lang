#include "transpile.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void transpile_node(ASTNode* node, FILE* out, int indent, int is_expr);

static struct {
    char* name;
    char** fields;
    size_t field_count;
    char** method_names;
    size_t* method_param_counts;
    size_t method_count;
    int has_constructor;
} class_info[64];
static size_t class_info_count = 0;

static char* reg_init = NULL;
static size_t reg_init_cap = 0;
static size_t reg_init_len = 0;

static void reg_init_append(const char* s) {
    size_t len = strlen(s);
    if (reg_init_len + len + 1 > reg_init_cap) {
        reg_init_cap = reg_init_cap ? reg_init_cap * 2 : 4096;
        reg_init = realloc(reg_init, reg_init_cap);
    }
    memcpy(reg_init + reg_init_len, s, len);
    reg_init_len += len;
    reg_init[reg_init_len] = '\0';
}

static struct {
    char** names;
    size_t* field_counts;
    char*** field_names;
    size_t count;
    size_t cap;
} type_registry;

static void type_registry_init(void) {
    type_registry.names = NULL;
    type_registry.field_counts = NULL;
    type_registry.field_names = NULL;
    type_registry.count = 0;
    type_registry.cap = 0;
}

static void type_registry_add(const char* name, char** fields, size_t field_count) {
    if (type_registry.count >= type_registry.cap) {
        size_t new_cap = type_registry.cap ? type_registry.cap * 2 : 8;
        type_registry.names = realloc(type_registry.names, sizeof(char*) * new_cap);
        type_registry.field_counts = realloc(type_registry.field_counts, sizeof(size_t) * new_cap);
        type_registry.field_names = realloc(type_registry.field_names, sizeof(char**) * new_cap);
        type_registry.cap = new_cap;
    }
    type_registry.names[type_registry.count] = strdup(name);
    type_registry.field_counts[type_registry.count] = field_count;
    type_registry.field_names[type_registry.count] = malloc(sizeof(char*) * field_count);
    for (size_t i = 0; i < field_count; i++) {
        type_registry.field_names[type_registry.count][i] = strdup(fields[i]);
    }
    type_registry.count++;
}

static char* sanitize_name(const char* name) {
    char* s = strdup(name);
    for (char* p = s; *p; p++) {
        if (*p == '.') *p = '_';
    }
    return s;
}

static void write_indent(FILE* out, int indent) {
    for (int i = 0; i < indent; i++) fprintf(out, "    ");
}

static void write_runtime_header(FILE* out) {
    fprintf(out, "#define _GNU_SOURCE\n");
    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "#include <stdlib.h>\n");
    fprintf(out, "#include <string.h>\n");
    fprintf(out, "#include <stdint.h>\n");
    fprintf(out, "#include <stdarg.h>\n\n");
    fprintf(out, "// Hunnu runtime types\n");
    fprintf(out, "typedef enum { HUNNU_INT, HUNNU_FLOAT, HUNNU_STRING, HUNNU_BOOL, HUNNU_NONE, HUNNU_ARRAY, HUNNU_STRUCT } HunnuType;\n");
    fprintf(out, "typedef struct HunnuValue_s { HunnuType type; int64_t i; double f; char* s; int b; struct HunnuValue_s* elems; size_t count; char* struct_type; struct HunnuValue_s* fields; char** field_names; size_t field_count; } HunnuValue;\n\n");
    fprintf(out, "static HunnuValue hunnu_int(int64_t v) { HunnuValue r; r.type = HUNNU_INT; r.i = v; return r; }\n");
    fprintf(out, "static HunnuValue hunnu_float(double v) { HunnuValue r; r.type = HUNNU_FLOAT; r.f = v; return r; }\n");
    fprintf(out, "static HunnuValue hunnu_bool(int v) { HunnuValue r; r.type = HUNNU_BOOL; r.b = v; return r; }\n");
    fprintf(out, "static HunnuValue hunnu_none() { HunnuValue r; r.type = HUNNU_NONE; return r; }\n");
    fprintf(out, "static HunnuValue hunnu_string(const char* v) { HunnuValue r; r.type = HUNNU_STRING; r.s = (v ? strdup(v) : strdup(\"\")); return r; }\n");
    fprintf(out, "static void hunnu_print(HunnuValue v) { switch(v.type) { case HUNNU_INT: printf(\"%%lld\\n\", (long long)v.i); break; case HUNNU_FLOAT: printf(\"%%g\\n\", v.f); break; case HUNNU_STRING: printf(\"%%s\\n\", v.s?v.s:\"\"); break; case HUNNU_BOOL: printf(v.b?\"true\\n\":\"false\\n\"); break; case HUNNU_NONE: printf(\"nil\\n\"); break; case HUNNU_ARRAY: printf(\"[array %%zu items]\\n\", v.count); break; case HUNNU_STRUCT: printf(\"{struct %%s}\\n\", v.struct_type?v.struct_type:\"\"); break; } }\n");
    fprintf(out, "static int hunnu_truthy(HunnuValue v) { switch(v.type) { case HUNNU_BOOL: return v.b; case HUNNU_NONE: return 0; default: return 1; } }\n");
    fprintf(out, "static HunnuValue hunnu_field_get(HunnuValue obj, const char* name) { for (size_t i = 0; i < obj.field_count; i++) { if (strcmp(obj.field_names[i], name) == 0) return obj.fields[i]; } return hunnu_none(); }\n\n");
    fprintf(out, "static HunnuValue hunnu_value_copy(HunnuValue v) {\n");
    fprintf(out, "    HunnuValue r; r.type = v.type; r.i = v.i; r.f = v.f; r.b = v.b; r.s = NULL; r.elems = NULL; r.count = 0; r.struct_type = NULL; r.fields = NULL; r.field_names = NULL; r.field_count = 0;\n");
    fprintf(out, "    if (v.type == HUNNU_STRING && v.s) r.s = strdup(v.s);\n");
    fprintf(out, "    r.struct_type = v.struct_type ? strdup(v.struct_type) : NULL;\n");
    fprintf(out, "    r.field_count = v.field_count;\n");
    fprintf(out, "    if (v.field_count > 0 && v.fields) {\n");
    fprintf(out, "        r.fields = malloc(sizeof(HunnuValue) * v.field_count);\n");
    fprintf(out, "        for (size_t _i = 0; _i < v.field_count; _i++) r.fields[_i] = hunnu_value_copy(v.fields[_i]);\n");
    fprintf(out, "    }\n");
    fprintf(out, "    if (v.type == HUNNU_ARRAY && v.elems && v.count > 0) {\n");
    fprintf(out, "        r.elems = malloc(sizeof(HunnuValue) * v.count); r.count = v.count;\n");
    fprintf(out, "        for (size_t _i = 0; _i < v.count; _i++) r.elems[_i] = hunnu_value_copy(v.elems[_i]);\n");
    fprintf(out, "    }\n");
    fprintf(out, "    return r;\n");
    fprintf(out, "}\n\n");
    fprintf(out, "static HunnuValue hunnu_field_set(HunnuValue obj, const char* name, HunnuValue val) {\n");
    fprintf(out, "    for (size_t i = 0; i < obj.field_count; i++) {\n");
    fprintf(out, "        if (strcmp(obj.field_names[i], name) == 0) { obj.fields[i] = val; return obj; }\n");
    fprintf(out, "    }\n");
    fprintf(out, "    fprintf(stderr, \"Runtime error: field '%%s' not found\\n\", name); exit(1); return obj;\n");
    fprintf(out, "}\n\n");
    fprintf(out, "typedef HunnuValue (*MethodFunc)(size_t, HunnuValue*);\n");
    fprintf(out, "#define MAX_METHODS 512\n");
    fprintf(out, "static struct { const char* type; const char* method; MethodFunc func; } _methods[MAX_METHODS];\n");
    fprintf(out, "static size_t _methods_count = 0;\n");
    fprintf(out, "static void _reg_method(const char* type, const char* method, MethodFunc func) {\n");
    fprintf(out, "    if (_methods_count < MAX_METHODS) { _methods[_methods_count].type = type; _methods[_methods_count].method = method; _methods[_methods_count].func = func; _methods_count++; }\n");
    fprintf(out, "}\n");
    fprintf(out, "static HunnuValue hunnu_method_call(HunnuValue obj, const char* method_name, size_t arg_count, ...) {\n");
    fprintf(out, "    va_list ap; va_start(ap, arg_count);\n");
    fprintf(out, "    HunnuValue args[16]; args[0] = obj;\n");
    fprintf(out, "    for (size_t i = 0; i < arg_count && i < 15; i++) args[i+1] = va_arg(ap, HunnuValue);\n");
    fprintf(out, "    va_end(ap);\n");
    fprintf(out, "    for (size_t i = 0; i < _methods_count; i++) {\n");
    fprintf(out, "        if (strcmp(_methods[i].type, obj.struct_type) == 0 && strcmp(_methods[i].method, method_name) == 0) {\n");
    fprintf(out, "            return _methods[i].func(arg_count + 1, args);\n");
    fprintf(out, "        }\n");
    fprintf(out, "    }\n");
    fprintf(out, "    fprintf(stderr, \"Runtime error: method '%%s' not found on type '%%s'\\n\", method_name, obj.struct_type ? obj.struct_type : \"?\"); exit(1); return hunnu_none();\n");
    fprintf(out, "}\n\n");
}

static void transpile_literal(ASTNode* node, FILE* out) {
    switch (node->data.literal.literal_type) {
        case TOKEN_INT_LITERAL:
            fprintf(out, "hunnu_int(%lld)", (long long)node->data.literal.value.int_value);
            break;
        case TOKEN_FLOAT_LITERAL:
            fprintf(out, "hunnu_float(%g)", node->data.literal.value.float_value);
            break;
        case TOKEN_STRING_LITERAL: {
            char* s = node->data.literal.value.string_value;
            fprintf(out, "hunnu_string(");
            if (s) {
                fprintf(out, "\"");
                for (char* p = s; *p; p++) {
                    if (*p == '\\') fprintf(out, "\\\\");
                    else if (*p == '"') fprintf(out, "\\\"");
                    else if (*p == '\n') fprintf(out, "\\n");
                    else if (*p == '\t') fprintf(out, "\\t");
                    else fputc(*p, out);
                }
                fprintf(out, "\"");
            } else {
                fprintf(out, "\"\"");
            }
            fprintf(out, ")");
            break;
        }
        case TOKEN_TRUE:
            fprintf(out, "hunnu_bool(1)");
            break;
        case TOKEN_FALSE:
            fprintf(out, "hunnu_bool(0)");
            break;
        case TOKEN_NULL: case TOKEN_NIL_KEYWORD:
            fprintf(out, "hunnu_none()");
            break;
        default:
            fprintf(out, "hunnu_none()");
            break;
    }
}

static void transpile_call_expr(ASTNode* node, FILE* out) {
    char* sfname = sanitize_name(node->data.call_expr.name);
    fprintf(out, "hunnu_call_%s(", sfname);
    free(sfname);
    for (size_t i = 0; i < node->data.call_expr.arg_count; i++) {
        if (i > 0) fprintf(out, ", ");
        transpile_node(node->data.call_expr.args[i], out, 0, 1);
    }
    fprintf(out, ")");
}

static void transpile_binary_expr(ASTNode* node, FILE* out) {
    ASTNode* left = node->data.binary_expr.left;
    ASTNode* right = node->data.binary_expr.right;

    int is_cmp = 0;
    const char* op_str = "";
    switch (node->data.binary_expr.operator) {
        case TOKEN_PLUS: op_str = "+"; break;
        case TOKEN_MINUS: op_str = "-"; break;
        case TOKEN_STAR: op_str = "*"; break;
        case TOKEN_SLASH: op_str = "/"; break;
        case TOKEN_PERCENT: op_str = "%%"; break;
        case TOKEN_EQ: op_str = "=="; is_cmp = 1; break;
        case TOKEN_NEQ: op_str = "!="; is_cmp = 1; break;
        case TOKEN_LT: op_str = "<"; is_cmp = 1; break;
        case TOKEN_LE: op_str = "<="; is_cmp = 1; break;
        case TOKEN_GT: op_str = ">"; is_cmp = 1; break;
        case TOKEN_GE: op_str = ">="; is_cmp = 1; break;
        default: op_str = "+"; break;
    }

    if (is_cmp) {
        fprintf(out, "({ HunnuValue _l = ");
        transpile_node(left, out, 0, 1);
        fprintf(out, "; HunnuValue _r = ");
        transpile_node(right, out, 0, 1);
        fprintf(out, "; hunnu_bool(_l.i %s _r.i); })", op_str);
    } else {
        fprintf(out, "({ HunnuValue _l = ");
        transpile_node(left, out, 0, 1);
        fprintf(out, "; HunnuValue _r = ");
        transpile_node(right, out, 0, 1);
        fprintf(out, "; HunnuValue _res; _res.type = HUNNU_INT; _res.i = _l.i %s _r.i; _res; })", op_str);
    }
}

static void transpile_unary_expr(ASTNode* node, FILE* out) {
    if (node->data.unary_expr.operator == TOKEN_MINUS) {
        fprintf(out, "({ HunnuValue _v = ");
        transpile_node(node->data.unary_expr.operand, out, 0, 1);
        fprintf(out, "; HunnuValue _r; _r.type = HUNNU_INT; _r.i = -_v.i; _r; })");
    } else if (node->data.unary_expr.operator == TOKEN_NOT) {
        fprintf(out, "({ HunnuValue _v = ");
        transpile_node(node->data.unary_expr.operand, out, 0, 1);
        fprintf(out, "; hunnu_bool(!hunnu_truthy(_v)); })");
    } else {
        transpile_node(node->data.unary_expr.operand, out, 0, 1);
    }
}

static void transpile_array_expr(ASTNode* node, FILE* out) {
    fprintf(out, "({ HunnuValue _arr; _arr.type = HUNNU_ARRAY; _arr.count = %zu; _arr.elems = malloc(sizeof(HunnuValue)*_arr.count); ", node->data.array_expr.count);
    for (size_t i = 0; i < node->data.array_expr.count; i++) {
        fprintf(out, "_arr.elems[%zu] = ", i);
        transpile_node(node->data.array_expr.elements[i], out, 0, 1);
        fprintf(out, "; ");
    }
    fprintf(out, "_arr; })");
}

static void transpile_index_expr(ASTNode* node, FILE* out) {
    fprintf(out, "({ HunnuValue _arr = ");
    transpile_node(node->data.index_expr.array, out, 0, 1);
    fprintf(out, "; HunnuValue _idx = ");
    transpile_node(node->data.index_expr.index, out, 0, 1);
    fprintf(out, "; _arr.elems[_idx.i]; })");
}

static void transpile_field_access(ASTNode* node, FILE* out) {
    fprintf(out, "hunnu_field_get(");
    transpile_node(node->data.field_access.object, out, 0, 1);
    fprintf(out, ", \"%s\")", node->data.field_access.field);
}

static void transpile_struct_instance(ASTNode* node, FILE* out) {
    const char* tn = node->data.struct_instance.type_name;
    fprintf(out, "({ HunnuValue _s; _s.type = HUNNU_STRUCT; _s.struct_type = strdup(\"%s\"); _s.field_count = %zu; _s.fields = malloc(sizeof(HunnuValue) * %zu); _s.field_names = malloc(sizeof(char*) * %zu); ", tn, node->data.struct_instance.field_count, node->data.struct_instance.field_count, node->data.struct_instance.field_count);
    for (size_t i = 0; i < node->data.struct_instance.field_count; i++) {
        fprintf(out, "_s.fields[%zu] = ", i);
        transpile_node(node->data.struct_instance.field_values[i], out, 0, 1);
        fprintf(out, "; _s.field_names[%zu] = strdup(\"%s\"); ", i, node->data.struct_instance.field_names[i]);
    }
    fprintf(out, "_s; })");
}

static void transpile_method_call(ASTNode* node, FILE* out) {
    char* smethod = sanitize_name(node->data.method_call.method);
    int is_static = 0;
    char* type_name = NULL;
    if (node->data.method_call.object->type == AST_IDENTIFIER) {
        type_name = node->data.method_call.object->data.identifier.name;
        for (size_t i = 0; i < type_registry.count; i++) {
            if (strcmp(type_registry.names[i], type_name) == 0) {
                is_static = 1;
                break;
            }
        }
    }
    if (is_static) {
        char* full_name = malloc(strlen(type_name) + 1 + strlen(smethod) + 1);
        sprintf(full_name, "%s_%s", type_name, smethod);
        fprintf(out, "hunnu_call_%s(", full_name);
        free(full_name);
        for (size_t i = 0; i < node->data.method_call.arg_count; i++) {
            if (i > 0) fprintf(out, ", ");
            transpile_node(node->data.method_call.args[i], out, 0, 1);
        }
        fprintf(out, ")");
    } else {
        fprintf(out, "hunnu_method_call(");
        transpile_node(node->data.method_call.object, out, 0, 1);
        fprintf(out, ", \"%s\", %zu", smethod, node->data.method_call.arg_count);
        for (size_t i = 0; i < node->data.method_call.arg_count; i++) {
            fprintf(out, ", ");
            transpile_node(node->data.method_call.args[i], out, 0, 1);
        }
        fprintf(out, ")");
    }
    free(smethod);
}

static void transpile_new_expr(ASTNode* node, FILE* out) {
    const char* cn = node->data.new_expr.class_name;
    char** fields = NULL;
    size_t field_count = 0;
    int has_ctor = 0;
    for (size_t i = 0; i < class_info_count; i++) {
        if (strcmp(class_info[i].name, cn) == 0) {
            fields = class_info[i].fields;
            field_count = class_info[i].field_count;
            has_ctor = class_info[i].has_constructor;
            break;
        }
    }
    fprintf(out, "({ HunnuValue _s; _s.type = HUNNU_STRUCT; _s.struct_type = strdup(\"%s\"); _s.field_count = %zu; _s.fields = malloc(sizeof(HunnuValue) * %zu); _s.field_names = malloc(sizeof(char*) * %zu); ", cn, field_count, field_count, field_count);
    for (size_t i = 0; i < field_count; i++) {
        fprintf(out, "_s.fields[%zu] = hunnu_none(); _s.field_names[%zu] = strdup(\"%s\"); ", i, i, fields[i]);
    }
    if (has_ctor) {
        char* scn = sanitize_name(cn);
        fprintf(out, "hunnu_call_%s_new(_s", scn);
        for (size_t i = 0; i < node->data.new_expr.arg_count; i++) {
            fprintf(out, ", ");
            transpile_node(node->data.new_expr.args[i], out, 0, 1);
        }
        fprintf(out, "); ");
        free(scn);
    }
    fprintf(out, "_s; })");
}

static void transpile_node(ASTNode* node, FILE* out, int indent, int is_expr) {
    if (!node) return;

    switch (node->type) {
        case AST_LITERAL:
            transpile_literal(node, out);
            break;

        case AST_IDENTIFIER: {
            char* name = node->data.identifier.name;
            if (strcmp(name, "true") == 0) { fprintf(out, "hunnu_bool(1)"); break; }
            if (strcmp(name, "false") == 0) { fprintf(out, "hunnu_bool(0)"); break; }
            if (strcmp(name, "null") == 0 || strcmp(name, "nil") == 0) { fprintf(out, "hunnu_none()"); break; }
            fprintf(out, "_var_%s", name);
            break;
        }

        case AST_CALL_EXPR:
            transpile_call_expr(node, out);
            break;

        case AST_BINARY_EXPR:
            transpile_binary_expr(node, out);
            break;

        case AST_UNARY_EXPR:
            transpile_unary_expr(node, out);
            break;

        case AST_ARRAY_EXPR:
            transpile_array_expr(node, out);
            break;

        case AST_INDEX_EXPR:
            transpile_index_expr(node, out);
            break;

        case AST_FIELD_ACCESS:
            transpile_field_access(node, out);
            break;

        case AST_STRUCT_INSTANCE:
            transpile_struct_instance(node, out);
            break;

        case AST_METHOD_CALL:
            transpile_method_call(node, out);
            break;

        case AST_ASSIGN:
            write_indent(out, indent);
            fprintf(out, "_var_%s = ", node->data.assign.name);
            transpile_node(node->data.assign.value, out, 0, 1);
            fprintf(out, ";\n");
            break;

        case AST_VAR_DECL:
            write_indent(out, indent);
            fprintf(out, "HunnuValue _var_%s = ", node->data.var_decl.name);
            if (node->data.var_decl.initializer) {
                transpile_node(node->data.var_decl.initializer, out, 0, 1);
            } else {
                fprintf(out, "hunnu_none()");
            }
            fprintf(out, ";\n");
            break;

        case AST_FN_DECL: {
            char* fname = node->data.fn_decl.name;
            char* sfname = sanitize_name(fname);
            int is_main = (strcmp(fname, "main") == 0);
            int is_method = (strchr(fname, '.') != NULL);

            if (is_main) {
                fprintf(out, "\nint main(int argc, char** argv) {\n");
                if (reg_init && reg_init_len > 0) {
                    fprintf(out, "%s", reg_init);
                }
            } else {
                fprintf(out, "\nHunnuValue hunnu_call_%s(", sfname);
                for (size_t i = 0; i < node->data.fn_decl.param_count; i++) {
                    if (i > 0) fprintf(out, ", ");
                    fprintf(out, "HunnuValue _var_%s", node->data.fn_decl.params[i]);
                }
                fprintf(out, ") {\n");
            }

            if (node->data.fn_decl.body) {
                transpile_node(node->data.fn_decl.body, out, 1, 0);
            }

            if (is_main) {
                fprintf(out, "    return 0;\n");
            } else {
                fprintf(out, "    return hunnu_none();\n");
            }
            fprintf(out, "}\n");
            if (is_method && !is_main) {
                fprintf(out, "static HunnuValue _wrap_%s(size_t n, HunnuValue* args) {\n    (void)n; return hunnu_call_%s(", sfname, sfname);
                for (size_t i = 0; i < node->data.fn_decl.param_count; i++) {
                    if (i > 0) fprintf(out, ", ");
                    fprintf(out, "args[%zu]", i);
                }
                fprintf(out, ");\n}\n");
            }
            free(sfname);
            break;
        }

        case AST_BLOCK:
            for (size_t i = 0; i < node->data.block.count; i++) {
                transpile_node(node->data.block.statements[i], out, indent, 0);
            }
            break;

        case AST_PRINT_STMT:
            write_indent(out, indent);
            fprintf(out, "hunnu_print(");
            transpile_node(node->data.print_stmt.argument, out, 0, 1);
            fprintf(out, ");\n");
            break;

        case AST_RETURN_STMT:
            write_indent(out, indent);
            fprintf(out, "return ");
            if (node->data.return_stmt.value) {
                transpile_node(node->data.return_stmt.value, out, 0, 1);
            } else {
                fprintf(out, "hunnu_none()");
            }
            fprintf(out, ";\n");
            break;

        case AST_EXPR_STMT:
            write_indent(out, indent);
            transpile_node(node->data.expr_stmt.expression, out, 0, 1);
            fprintf(out, ";\n");
            break;

        case AST_IF_STMT:
            write_indent(out, indent);
            fprintf(out, "if (hunnu_truthy(");
            transpile_node(node->data.if_stmt.condition, out, 0, 1);
            fprintf(out, ")) {\n");
            transpile_node(node->data.if_stmt.then_branch, out, indent + 1, 0);
            if (node->data.if_stmt.else_branch) {
                write_indent(out, indent);
                fprintf(out, "} else {\n");
                transpile_node(node->data.if_stmt.else_branch, out, indent + 1, 0);
            }
            write_indent(out, indent);
            fprintf(out, "}\n");
            break;

        case AST_WHILE_STMT:
            write_indent(out, indent);
            fprintf(out, "while (hunnu_truthy(");
            transpile_node(node->data.while_stmt.condition, out, 0, 1);
            fprintf(out, ")) {\n");
            transpile_node(node->data.while_stmt.body, out, indent + 1, 0);
            write_indent(out, indent);
            fprintf(out, "}\n");
            break;

        case AST_FOR_STMT: {
            write_indent(out, indent);
            fprintf(out, "for (HunnuValue _for_var = ");
            if (node->data.for_stmt.initializer) {
                transpile_node(node->data.for_stmt.initializer, out, 0, 1);
            } else {
                fprintf(out, "hunnu_none()");
            }
            fprintf(out, "; hunnu_truthy(");
            if (node->data.for_stmt.condition) {
                transpile_node(node->data.for_stmt.condition, out, 0, 1);
            } else {
                fprintf(out, "hunnu_bool(1)");
            }
            fprintf(out, "); ");
            if (node->data.for_stmt.update) {
                transpile_node(node->data.for_stmt.update, out, 0, 1);
            } else {
                fprintf(out, "hunnu_none()");
            }
            fprintf(out, ") {\n");
            transpile_node(node->data.for_stmt.body, out, indent + 1, 0);
            write_indent(out, indent);
            fprintf(out, "}\n");
            break;
        }

        case AST_TYPE_DECL:
            break;

        case AST_EXTERN_FN: {
            char* fname = node->data.extern_fn.name;
            char* sfname = sanitize_name(fname);
            fprintf(out, "\nextern HunnuValue hunnu_call_%s(", sfname);
            free(sfname);
            for (size_t i = 0; i < node->data.extern_fn.param_count; i++) {
                if (i > 0) fprintf(out, ", ");
                fprintf(out, "HunnuValue _var_%s", node->data.extern_fn.param_names[i]);
            }
            fprintf(out, ");\n");
            break;
        }

        case AST_ADDRESS_OF:
        case AST_DEREFERENCE:
        case AST_STRING_CONCAT:
        case AST_MATCH_EXPR:
        case AST_TRY_STMT:
        case AST_INDEX_ASSIGN:
        case AST_TRAIT_DECL:
        case AST_IMPL_DECL:
        case AST_UNSAFE_BLOCK:
        case AST_ENUM_DECL:
        case AST_ENUM_VARIANT:
            write_indent(out, indent);
            fprintf(out, "// Unsupported: %s\n", ast_node_type_to_string(node->type));
            break;

        case AST_CLASS_DECL: {
            const char* cn = node->data.class_decl.name;
            char* scn = sanitize_name(cn);
            if (node->data.class_decl.constructor) {
                ASTNode* ctor = node->data.class_decl.constructor;
                fprintf(out, "\nstatic HunnuValue hunnu_call_%s_new(", scn);
                for (size_t ci = 0; ci < ctor->data.fn_decl.param_count; ci++) {
                    if (ci > 0) fprintf(out, ", ");
                    fprintf(out, "HunnuValue _var_%s", ctor->data.fn_decl.params[ci]);
                }
                fprintf(out, ") {\n");
                if (ctor->data.fn_decl.body) transpile_node(ctor->data.fn_decl.body, out, 1, 0);
                fprintf(out, "    return hunnu_none();\n}\n");
                fprintf(out, "static HunnuValue _wrap_%s_new(size_t n, HunnuValue* args) {\n    (void)n; return hunnu_call_%s_new(", scn, scn);
                for (size_t ci = 0; ci < ctor->data.fn_decl.param_count; ci++) {
                    if (ci > 0) fprintf(out, ", ");
                    fprintf(out, "args[%zu]", ci);
                }
                fprintf(out, ");\n}\n");
                char rbuf[512];
                snprintf(rbuf, sizeof(rbuf), "    _reg_method(\"%s\", \"new\", _wrap_%s_new);\n", cn, scn);
                reg_init_append(rbuf);
            }
            for (size_t mi = 0; mi < node->data.class_decl.method_count; mi++) {
                ASTNode* method = node->data.class_decl.methods[mi];
                char* smethod = sanitize_name(method->data.fn_decl.name);
                /* Extract just the method name (strip "ClassName." prefix) */
                const char* short_method = method->data.fn_decl.name;
                char* dot = strchr(short_method, '.');
                if (dot) short_method = dot + 1;
                fprintf(out, "\nstatic HunnuValue hunnu_call_%s_%s(", scn, smethod);
                for (size_t mj = 0; mj < method->data.fn_decl.param_count; mj++) {
                    if (mj > 0) fprintf(out, ", ");
                    fprintf(out, "HunnuValue _var_%s", method->data.fn_decl.params[mj]);
                }
                fprintf(out, ") {\n");
                if (method->data.fn_decl.body) transpile_node(method->data.fn_decl.body, out, 1, 0);
                fprintf(out, "    return hunnu_none();\n}\n");
                fprintf(out, "static HunnuValue _wrap_%s_%s(size_t n, HunnuValue* args) {\n    (void)n; return hunnu_call_%s_%s(", scn, smethod, scn, smethod);
                for (size_t mj = 0; mj < method->data.fn_decl.param_count; mj++) {
                    if (mj > 0) fprintf(out, ", ");
                    fprintf(out, "args[%zu]", mj);
                }
                fprintf(out, ");\n}\n");
                char rbuf[512];
                snprintf(rbuf, sizeof(rbuf), "    _reg_method(\"%s\", \"%s\", _wrap_%s_%s);\n", cn, short_method, scn, smethod);
                reg_init_append(rbuf);
                free(smethod);
            }
            free(scn);
            break;
        }

        case AST_NEW_EXPR:
            transpile_new_expr(node, out);
            if (!is_expr) fprintf(out, ";\n");
            break;

        case AST_FIELD_ASSIGN: {
            write_indent(out, indent);
            ASTNode* fa_obj = node->data.field_assign.object;
            const char* fa_field = node->data.field_assign.field;
            if (fa_obj->type == AST_IDENTIFIER) {
                fprintf(out, "_var_%s = hunnu_field_set(_var_%s, \"%s\", ", fa_obj->data.identifier.name, fa_obj->data.identifier.name, fa_field);
            } else {
                fprintf(out, "hunnu_field_set(");
                transpile_node(fa_obj, out, 0, 1);
                fprintf(out, ", \"%s\", ", fa_field);
            }
            transpile_node(node->data.field_assign.value, out, 0, 1);
            fprintf(out, ");\n");
            break;
        }

        case AST_BREAK_STMT:
            write_indent(out, indent);
            fprintf(out, "break;\n");
            break;

        case AST_CONTINUE_STMT:
            write_indent(out, indent);
            fprintf(out, "continue;\n");
            break;

        default:
            break;
    }
}

char* transpile_to_c(ASTNode* program) {
    char* buffer = malloc(1024 * 1024);
    if (!buffer) return NULL;
    buffer[0] = '\0';

    FILE* out = fmemopen(buffer, 1024 * 1024, "w");
    if (!out) { free(buffer); return NULL; }

    type_registry_init();
    reg_init = NULL;
    reg_init_cap = 0;
    reg_init_len = 0;
    class_info_count = 0;

    write_runtime_header(out);

    if (program->type == AST_PROGRAM) {
        for (size_t i = 0; i < program->data.program.count; i++) {
            ASTNode* stmt = program->data.program.statements[i];
            if (stmt->type == AST_TYPE_DECL) {
                type_registry_add(stmt->data.type_decl.name, stmt->data.type_decl.fields, stmt->data.type_decl.field_count);
                    (void)stmt->data.type_decl.is_pub;
            } else if (stmt->type == AST_CLASS_DECL) {
                type_registry_add(stmt->data.class_decl.name, stmt->data.class_decl.fields, stmt->data.class_decl.field_count);
                    (void)stmt->data.class_decl.is_pub;
                if (class_info_count < 64) {
                    class_info[class_info_count].name = stmt->data.class_decl.name;
                    class_info[class_info_count].fields = stmt->data.class_decl.fields;
                    class_info[class_info_count].field_count = stmt->data.class_decl.field_count;
                    class_info[class_info_count].method_count = stmt->data.class_decl.method_count;
                    class_info[class_info_count].has_constructor = (stmt->data.class_decl.constructor != NULL);
                    class_info[class_info_count].method_names = malloc(sizeof(char*) * stmt->data.class_decl.method_count);
                    class_info[class_info_count].method_param_counts = malloc(sizeof(size_t) * stmt->data.class_decl.method_count);
                    for (size_t mi = 0; mi < stmt->data.class_decl.method_count; mi++) {
                        class_info[class_info_count].method_names[mi] = stmt->data.class_decl.methods[mi]->data.fn_decl.name;
                        class_info[class_info_count].method_param_counts[mi] = stmt->data.class_decl.methods[mi]->data.fn_decl.param_count;
                    }
                    class_info_count++;
                }
            } else if (stmt->type == AST_FN_DECL) {
                /* Register standalone methods (fn Type.method) in the dispatch table */
                char* fname = stmt->data.fn_decl.name;
                char* dot = strchr(fname, '.');
                if (dot) {
                    size_t type_name_len = dot - fname;
                    char* sname = sanitize_name(fname);
                    char type_buf[256];
                    snprintf(type_buf, sizeof(type_buf), "%.*s", (int)type_name_len, fname);
                    char* method_buf = strdup(dot + 1);
                    char rbuf[512];
                    snprintf(rbuf, sizeof(rbuf), "    _reg_method(\"%s\", \"%s\", _wrap_%s);\n", type_buf, method_buf, sname);
                    reg_init_append(rbuf);
                    free(method_buf);
                    free(sname);
                }
            }
        }
        for (size_t i = 0; i < program->data.program.count; i++) {
            transpile_node(program->data.program.statements[i], out, 0, 0);
        }
    }

    fclose(out);
    return buffer;
}
