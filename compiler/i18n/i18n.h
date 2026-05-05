/**
 * @file i18n.h
 * @brief Internationalization support for Hunnu language
 */

#ifndef HUNNU_I18N_H
#define HUNNU_I18N_H

#include <stdint.h>
#include <stdarg.h>

/** Supported languages */
typedef enum {
    LANG_EN = 0,    /**< English */
    LANG_MN = 1     /**< Mongolian */
} Language;

/** Error message keys */
typedef enum {
    ERR_UNKNOWN,
    ERR_DIVISION_BY_ZERO,
    ERR_UNDEFINED_VARIABLE,
    ERR_INDEX_OUT_OF_BOUNDS,
    ERR_CAN_ONLY_INDEX_ARRAYS,
    ERR_UNKNOWN_FUNCTION,
    ERR_TOO_MANY_ARGS_EXTERN,
    ERR_EXTERN_NOT_LOADED,
    ERR_EXTERN_TOO_MANY_ARGS,
    ERR_CANNOT_LOAD_LIBRARY,
    ERR_CANNOT_FIND_SYMBOL,
    ERR_TOO_MANY_EXTERN_DECLS,
    ERR_FAILED_PARSE,
    ERR_CANNOT_OPEN_FILE,
    ERR_CANNOT_OPEN_IMPORT,
    ERR_IMPORT_DEPTH_EXCEEDED,
    ERR_UNKNOWN_COMMAND,
    ERR_MISSING_FILENAME,
    ERR_MEMORY_ALLOCATION,
    ERR_UNKNOWN_CHARACTER,
    ERR_MAX
} ErrorKey;

/**
 * @brief Initialize i18n system
 * Checks HUNNU_LANG environment variable
 */
void i18n_init(void);

/**
 * @brief Set current language
 * @param lang Language code ("en" or "mn")
 */
void i18n_set_language(const char* lang);

/**
 * @brief Get current language
 * @return Current language enum
 */
Language i18n_get_language(void);

/**
 * @brief Print translated error message with formatting
 * @param key Error message key
 * @param ... Variable arguments for format string
 */
void i18n_error(ErrorKey key, ...);

/**
 * @brief Get translated value string (true/false/nil)
 * @param value Value name ("true", "false", "nil")
 * @return Translated string
 */
const char* i18n_get_value_string(const char* value);

/**
 * @brief Get Mongolian keyword equivalent
 * @param english_keyword English keyword
 * @return Mongolian keyword or NULL if not found
 */
const char* i18n_get_mn_keyword(const char* english_keyword);

/**
 * @brief Check if Mongolian keyword is supported
 * @return 1 if supported, 0 otherwise
 */
int i18n_is_mn_supported(void);

#endif
