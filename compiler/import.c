#include "import.h"
#include "i18n/i18n.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_SOURCE_SIZE (1024 * 1024)
#define MAX_IMPORT_DEPTH 16

static char* resolve_import_path(const char* import_path, const char* base_dir) {
    if (import_path[0] == '/') {
        return strdup(import_path);
    }

    int has_slash = (strchr(import_path, '/') != NULL);
    const char* first_dot = strchr(import_path, '.');
    char* path_to_use = NULL;

    if (!has_slash && first_dot != NULL) {
        const char* rest = first_dot + 1;
        size_t rest_len = strlen(rest);
        size_t len = 7 + rest_len + 3 + 1;
        char* module_path = (char*)malloc(len);
        char* out = module_path;

        memcpy(out, "stdlib/", 7);
        out += 7;

        const char* p = rest;
        while (*p) {
            if (*p == '.') {
                *out++ = '/';
            } else {
                *out++ = *p;
            }
            p++;
        }

        memcpy(out, ".hn", 3);
        out += 3;
        *out = '\0';

        path_to_use = module_path;
    } else {
        path_to_use = strdup(import_path);
    }

    char* cwd = getcwd(NULL, 0);
    if (cwd) {
        size_t full_len = strlen(cwd) + 1 + strlen(path_to_use) + 1;
        char* full = (char*)malloc(full_len);
        snprintf(full, full_len, "%s/%s", cwd, path_to_use);
        free(cwd);
        FILE* test = fopen(full, "r");
        if (test) {
            fclose(test);
            free(path_to_use);
            return full;
        }
        free(full);
    }

    if (base_dir && base_dir[0]) {
        size_t full_len = strlen(base_dir) + 1 + strlen(path_to_use) + 1;
        char* full = (char*)malloc(full_len);
        snprintf(full, full_len, "%s/%s", base_dir, path_to_use);
        FILE* test = fopen(full, "r");
        if (test) {
            fclose(test);
            free(path_to_use);
            return full;
        }
        free(full);
    }

    {
        const char* stdlib_path = getenv("HUNNU_STDLIB_PATH");
        if (stdlib_path && stdlib_path[0]) {
            size_t full_len = strlen(stdlib_path) + 1 + strlen(path_to_use) + 1;
            char* full = (char*)malloc(full_len);
            snprintf(full, full_len, "%s/%s", stdlib_path, path_to_use);
            FILE* test = fopen(full, "r");
            if (test) {
                fclose(test);
                free(path_to_use);
                return full;
            }
            free(full);
        }
    }

    free(path_to_use);
    size_t len = strlen(base_dir) + 1 + strlen(import_path) + 1;
    char* full = (char*)malloc(len);
    snprintf(full, len, "%s/%s", base_dir, import_path);
    return full;
}

static char* read_raw_file(const char* filename, long* out_size) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* source = (char*)malloc(file_size + 1);
    fread(source, 1, file_size, fp);
    source[file_size] = '\0';
    fclose(fp);

    *out_size = file_size;
    return source;
}

static char* expand_imports_recursive(const char* source, long source_size,
                                       const char* base_dir, int depth, int* out_size);

static char* expand_imports_recursive(const char* source, long source_size,
                                       const char* base_dir, int depth, int* out_size) {
    if (depth >= MAX_IMPORT_DEPTH) {
        i18n_error(ERR_IMPORT_DEPTH_EXCEEDED, MAX_IMPORT_DEPTH);
        fprintf(stderr, "\n");
        *out_size = (int)source_size;
        return strdup(source);
    }

    char* output = (char*)malloc(MAX_SOURCE_SIZE);
    size_t out_pos = 0;
    size_t pos = 0;
    size_t file_size = (size_t)source_size;

    while (pos < file_size) {
        if (source[pos] == '"') {
            size_t end = pos + 1;
            while (end < file_size && source[end] != '"') {
                if (source[end] == '\\' && end + 1 < file_size) {
                    end++;
                }
                end++;
            }
            if (end < file_size) {
                end++;
            }
            size_t len = end - pos;
            if (out_pos + len < MAX_SOURCE_SIZE) {
                memcpy(output + out_pos, source + pos, len);
                out_pos += len;
            }
            pos = end;
            continue;
        }

        int is_import = 0;
        size_t import_start = pos;

        while (import_start < file_size && (source[import_start] == ' ' || source[import_start] == '\t')) {
            import_start++;
        }

        if (import_start + 7 <= file_size && strncmp(source + import_start, "import ", 7) == 0) {
            if (pos == 0 || source[pos-1] == '\n' || source[pos-1] == '\r') {
                is_import = 1;
            }
        } else if (import_start + 13 <= file_size && strncmp(source + import_start, "\xD0\xB8\xD0\xBC\xD0\xBF\xD0\xBE\xD1\x80\xD1\x82 ", 13) == 0) {
            if (pos == 0 || source[pos-1] == '\n' || source[pos-1] == '\r') {
                is_import = 1;
            }
        }

        if (is_import) {
            size_t path_start = import_start;
            if (strncmp(source + import_start, "import ", 7) == 0) {
                path_start += 7;
            } else {
                path_start += 13;
            }

            while (path_start < file_size && (source[path_start] == ' ' || source[path_start] == '\t')) {
                path_start++;
            }

            if (path_start < file_size && source[path_start] == '"') {
                path_start++;
                size_t path_end = path_start;
                while (path_end < file_size && source[path_end] != '"') {
                    path_end++;
                }

                if (path_end < file_size) {
                    size_t path_len = path_end - path_start;
                    char* import_path = (char*)malloc(path_len + 1);
                    memcpy(import_path, source + path_start, path_len);
                    import_path[path_len] = '\0';

                    char* full_path = resolve_import_path(import_path, base_dir);

                    long imp_file_size = 0;
                    char* imp_raw = read_raw_file(full_path, &imp_file_size);

                    if (imp_raw) {
                        char* imp_dir_copy = strdup(full_path);
                        char* imp_last_slash = strrchr(imp_dir_copy, '/');
                        char* imp_base_dir = "";
                        if (imp_last_slash) {
                            *imp_last_slash = '\0';
                            imp_base_dir = imp_dir_copy;
                        }

                        int expanded_size = 0;
                        char* expanded = expand_imports_recursive(imp_raw, imp_file_size,
                                                                   imp_base_dir, depth + 1,
                                                                   &expanded_size);

                        if (out_pos + expanded_size + 1 < MAX_SOURCE_SIZE) {
                            memcpy(output + out_pos, expanded, expanded_size);
                            out_pos += expanded_size;
                            output[out_pos++] = '\n';
                        }

                        free(expanded);
                        free(imp_dir_copy);
                        free(imp_raw);
                    } else {
                        fprintf(stderr, "Cannot open imported file '%s'\n", full_path);
                    }

                    free(full_path);
                    free(import_path);

                    pos = path_end + 1;
                    while (pos < file_size && source[pos] != '\n' && source[pos] != ';') {
                        pos++;
                    }
                    if (pos < file_size && source[pos] == ';') {
                        pos++;
                    }
                    continue;
                }
            } else {
                size_t path_end = path_start;
                while (path_end < file_size && source[path_end] != '\n' && source[path_end] != ';' && source[path_end] != ' ') {
                    path_end++;
                }

                size_t path_len = path_end - path_start;
                char* import_path = (char*)malloc(path_len + 1);
                memcpy(import_path, source + path_start, path_len);
                import_path[path_len] = '\0';

                char* full_path = resolve_import_path(import_path, base_dir);

                long imp_file_size = 0;
                char* imp_raw = read_raw_file(full_path, &imp_file_size);

                if (imp_raw) {
                    char* imp_dir_copy = strdup(full_path);
                    char* imp_last_slash = strrchr(imp_dir_copy, '/');
                    char* imp_base_dir = "";
                    if (imp_last_slash) {
                        *imp_last_slash = '\0';
                        imp_base_dir = imp_dir_copy;
                    }

                    int expanded_size = 0;
                    char* expanded = expand_imports_recursive(imp_raw, imp_file_size,
                                                               imp_base_dir, depth + 1,
                                                               &expanded_size);

                    if (out_pos + expanded_size + 1 < MAX_SOURCE_SIZE) {
                        memcpy(output + out_pos, expanded, expanded_size);
                        out_pos += expanded_size;
                        output[out_pos++] = '\n';
                    }

                    free(expanded);
                    free(imp_dir_copy);
                    free(imp_raw);
                } else {
                    fprintf(stderr, "Cannot open imported file '%s'\n", full_path);
                }

                free(full_path);
                free(import_path);

                pos = path_end;
                while (pos < file_size && source[pos] != '\n' && source[pos] != ';') {
                    pos++;
                }
                if (pos < file_size && source[pos] == ';') {
                    pos++;
                }
                continue;
            }
        }

        if (out_pos < MAX_SOURCE_SIZE - 1) {
            output[out_pos++] = source[pos];
        }
        pos++;
    }

    output[out_pos] = '\0';
    *out_size = (int)out_pos;
    return output;
}

char* read_source_with_imports(const char* filename, int* out_size) {
    long file_size = 0;
    char* source = read_raw_file(filename, &file_size);
    if (!source) {
        i18n_error(ERR_CANNOT_OPEN_FILE, filename);
        fprintf(stderr, "\n");
        return NULL;
    }

    char* dir_copy = strdup(filename);
    char* last_slash = strrchr(dir_copy, '/');
    char* base_dir = "";
    if (last_slash) {
        *last_slash = '\0';
        base_dir = dir_copy;
    }

    char* result = expand_imports_recursive(source, file_size, base_dir, 0, out_size);

    free(source);
    free(dir_copy);
    return result;
}
