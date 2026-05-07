/**
 * @file version.h
 * @brief Version constants using authentic Mongolian women names (А-Я)
 * @source https://www.behindthename.com/names/gender/feminine/usage/mongolian
 */

#ifndef HUNNU_VERSION_H
#define HUNNU_VERSION_H

#include <stdint.h>

/**
 * Authentic Mongolian women names for versioning
 * Each name represents a milestone in the language development
 */
typedef enum {
    VERSION_ALTAN = 0,        /**< 0.1.0 - Алтан (golden) - MVP/initial release */
    VERSION_ALTANGEREL,         /**< 0.2.0 - Алтангэрэл (golden light) */
    VERSION_ALTANTSETSEG,        /**< 0.3.0 - Алтанцэцэг (golden flower) */
    VERSION_ANAR,              /**< 0.4.0 - Анар (pomegranate) */
    VERSION_BATTSETSEG,        /**< 0.5.0 - Батцэцэг (strong flower) */
    VERSION_BOLORMAA,          /**< 0.6.0 - Болормаа (crystal woman) */
    VERSION_BOLORTSETSEG,      /**< 0.7.0 - Болорцэцэг (crystal flower) */
    VERSION_ENKH,             /**< 0.8.0 - Энх (peace, calm) */
    VERSION_ENKHJARGAL,        /**< 0.9.0 - Энхжаргал (peace blessing) */
    VERSION_ERDENE,            /**< 1.0.0 - Эрдэнэ (jewel, treasure) */
    VERSION_ERDENECHIMEG,      /**< 1.1.0 - Эрдэнэчимэг (jewel ornament) */
    VERSION_GEREL,            /**< 1.2.0 - Гэрэл (light) - stable */
    VERSION_JARGAL,            /**< 1.3.0 - Жаргал (happiness) */
    VERSION_MARAL,            /**< 1.4.0 - Марал (deer) */
    VERSION_MONKHSETSEG,      /**< 1.5.0 - Мөнхцэцэг (eternal flower) */
    VERSION_MONKHTUYA,       /**< 1.6.0 - Мөнхтуяа (eternal ray) */
    VERSION_NARANGEREL,       /**< 0.17 - Нарангэрэл (sunlight) */
    VERSION_NARANTSETSEG,       /**< 0.18 - Наранцэцэг (sun flower) */
    VERSION_ODONCHIMEG,      /**< 0.19 - Одончимэг (star ornament) */
    VERSION_ODTSETSEG,        /**< 0.20 - Одцэцэг (star flower) */
    VERSION_OYUNCHIMEG,       /**< 0.21 - Оюунчимэг (wisdom ornament) */
    VERSION_SARANGEREL,         /**< 0.22 - Сарангэрэл (moonlight) */
    VERSION_SARANTUYA,         /**< 0.23 - Сарантуяа (moonbeam) */
    VERSION_SARNAI,           /**< 0.24 - Сарнай (rose) */
    VERSION_SOLONGO,          /**< 0.25 - Солонго (rainbow) */
    VERSION_TSETSEG,           /**< 0.26 - Цэцэг (flower) */
    VERSION_UYANGA,          /**< 0.27 - Уянга (melody) */
    VERSION_ZAYA,            /**< 0.28 - Заяа (fate, destiny) */
    VERSION_COUNT
} HunnuVersion;

/** Current version - Эрдэнэ (Jewel, treasure) - v1.0.0 */
#ifndef HUNNU_VERSION_CURRENT
#define HUNNU_VERSION_CURRENT VERSION_ERDENE
#endif

/** Version string for display */
#define HUNNU_VERSION_STRING "1.0.0"

/** Version data defined in version.c */
extern const char* version_names[];
extern const char* version_descriptions[];
extern const uint32_t version_majors[];
extern const uint32_t version_minors[];
extern const uint32_t version_patches[];

/**
 * @brief Gets current version name
 * @return Version name in Mongolian
 */
const char* version_get_name(void);

/**
 * @brief Gets current version description
 * @return Version description in Mongolian
 */
const char* version_get_description(void);

/**
 * @brief Gets major version number
 * @return Major version
 */
uint32_t version_get_major(void);

/**
 * @brief Gets minor version number
 * @return Minor version
 */
uint32_t version_get_minor(void);

/**
 * @brief Gets patch version number
 * @return Patch version
 */
uint32_t version_get_patch(void);

/**
 * @brief Gets full version string
 * @return Full version string (e.g., "0.2.0 (Алтангэрэл)")
 */
const char* version_get_string(void);

#endif