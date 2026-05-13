/**
 * @file i18n.c
 * @brief Internationalization support implementation
 */

#include "i18n.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static Language current_lang = LANG_EN;

static const char* error_messages_en[] = {
    [ERR_UNKNOWN] = "Unknown error",
    [ERR_DIVISION_BY_ZERO] = "Division by zero",
    [ERR_UNDEFINED_VARIABLE] = "Undefined variable '%s'",
    [ERR_INDEX_OUT_OF_BOUNDS] = "Index out of bounds (index %ld, length %zu)",
    [ERR_CAN_ONLY_INDEX_ARRAYS] = "Can only index into arrays",
    [ERR_UNKNOWN_FUNCTION] = "Unknown function '%s'",
    [ERR_TOO_MANY_ARGS_EXTERN] = "Too many arguments for extern function '%s'",
    [ERR_EXTERN_NOT_LOADED] = "Extern function '%s' not loaded",
    [ERR_EXTERN_TOO_MANY_ARGS] = "Extern function '%s' has too many args",
    [ERR_CANNOT_LOAD_LIBRARY] = "Cannot load library: %s",
    [ERR_CANNOT_FIND_SYMBOL] = "Cannot find symbol '%s': %s",
    [ERR_TOO_MANY_EXTERN_DECLS] = "Too many extern declarations",
    [ERR_FAILED_PARSE] = "Failed to parse source file",
    [ERR_CANNOT_OPEN_FILE] = "Cannot open file '%s'",
    [ERR_CANNOT_OPEN_IMPORT] = "Cannot open imported file '%s'",
    [ERR_IMPORT_DEPTH_EXCEEDED] = "Maximum import depth exceeded (%d)",
    [ERR_UNKNOWN_COMMAND] = "Unknown command '%s'",
    [ERR_MISSING_FILENAME] = "Missing filename argument",
    [ERR_MEMORY_ALLOCATION] = "Memory allocation failed",
    [ERR_UNKNOWN_CHARACTER] = "Unknown character '%s'",
    [ERR_MAX] = NULL
};

static const char* error_messages_mn[] = {
    [ERR_UNKNOWN] = "Үл мэдэгдэх алдаа",
    [ERR_DIVISION_BY_ZERO] = "Тэгээр хуваах оролдлого",
    [ERR_UNDEFINED_VARIABLE] = "Тодорхойгүй хувьсагч '%s'",
    [ERR_INDEX_OUT_OF_BOUNDS] = "Индекс хүрээсээ гарсан (индекс %ld, урт %zu)",
    [ERR_CAN_ONLY_INDEX_ARRAYS] = "Зөвхөн массив руу индекс ашиглаж болно",
    [ERR_UNKNOWN_FUNCTION] = "Үл мэдэгдэх функц '%s'",
    [ERR_TOO_MANY_ARGS_EXTERN] = "Гадаад функц '%s'-д хэт олон аргумент",
    [ERR_EXTERN_NOT_LOADED] = "Гадаад функц '%s' ачаалагдсангүй",
    [ERR_EXTERN_TOO_MANY_ARGS] = "Гадаад функц '%s'-д хэт олон аргумент байна",
    [ERR_CANNOT_LOAD_LIBRARY] = "Сан ачаалж чадсангүй: %s",
    [ERR_CANNOT_FIND_SYMBOL] = "Симбол '%s' олдсонгүй: %s",
    [ERR_TOO_MANY_EXTERN_DECLS] = "Хэт олон гадаад зарлага",
    [ERR_FAILED_PARSE] = "Эх кодыг задлахад амжилтгүй боллоо",
    [ERR_CANNOT_OPEN_FILE] = "Файл '%s' нээгдсэнгүй",
    [ERR_CANNOT_OPEN_IMPORT] = "Импортлогдсон файл '%s' нээгдсэнгүй",
    [ERR_IMPORT_DEPTH_EXCEEDED] = "Импортын дээд шатны хязгаарт хүрлээ (%d)",
    [ERR_UNKNOWN_COMMAND] = "Үл мэдэгдэх команд '%s'",
    [ERR_MISSING_FILENAME] = "Файлын нэр өгөгдөөгүй",
    [ERR_MEMORY_ALLOCATION] = "Санах ой хуваарилахад алдаа гарлаа",
    [ERR_UNKNOWN_CHARACTER] = "Үл мэдэгдэх тэмдэгт '%s'",
    [ERR_MAX] = NULL
};

static const char* value_strings_en[] = {
    "true",
    "false",
    "nil"
};

static const char* value_strings_mn[] = {
    "үнэн",
    "худал",
    "хоосон"
};

void i18n_init(void) {
    const char* lang_env = getenv("HUNNU_LANG");
    if (lang_env) {
        i18n_set_language(lang_env);
    }
}

void i18n_set_language(const char* lang) {
    if (strcmp(lang, "mn") == 0 || strcmp(lang, "mongolian") == 0) {
        current_lang = LANG_MN;
    } else if (strcmp(lang, "en") == 0 || strcmp(lang, "english") == 0) {
        current_lang = LANG_EN;
    }
}

Language i18n_get_language(void) {
    return current_lang;
}

void i18n_error(ErrorKey key, ...) {
    const char* fmt = (current_lang == LANG_MN) 
        ? error_messages_mn[key] 
        : error_messages_en[key];
    
    if (!fmt) fmt = "Unknown error";
    
    va_list args;
    va_start(args, key);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

const char* i18n_get_value_string(const char* value) {
    if (current_lang == LANG_MN) {
        if (strcmp(value, "true") == 0) return value_strings_mn[0];
        if (strcmp(value, "false") == 0) return value_strings_mn[1];
        if (strcmp(value, "nil") == 0) return value_strings_mn[2];
    }
    return value;
}

const char* i18n_get_mn_keyword(const char* english_keyword) {
    static const char* keyword_map[][2] = {
        {"let", "хувьсагч"},
        {"fn", "функц"},
        {"if", "хэрвээ"},
        {"else", "бусад"},
        {"true", "үнэн"},
        {"false", "худал"},
        {"print", "хэвлэх"},
        {"while", "давталт"},
        {"for", "тооллого"},
        {"return", "буцаах"},
        {"break", "зогсоох"},
        {"continue", "үргэлжлүүлэх"},
        {"match", "тохирох"},
        {"null", "хоосон"},
        {"import", "импорт"},
        {"extern", "гаднах"},
        {"try", "турших"},
        {"catch", "барих"},
        {"finally", "эцэст"},
        {"type", "төрөл"},
        {"class", "класс"},
        {"new", "шинэ"},
        {"pub", "нийт"},
        {"self", "өөрөө"},
        {"trait", "шинж"},
        {"impl", "хэрэгжүүлэх"},
        {"unsafe", "аюулгүйбус"},
        {"enum", "тоолол"},
        {"mut", "өөрчлөгдөх"},
        {"and", "мөн"},
        {"or", "эсвэл"},
        {"not", "үгүй"},
        {NULL, NULL}
    };

    for (int i = 0; keyword_map[i][0] != NULL; i++) {
        if (strcmp(english_keyword, keyword_map[i][0]) == 0) {
            return keyword_map[i][1];
        }
    }
    return NULL;
}

int i18n_is_mn_supported(void) {
    return 1;
}
