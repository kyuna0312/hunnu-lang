/**
 * @file version.c
 * @brief Version implementation using authentic Mongolian women names
 */

#include "version.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

const char* version_names[] = {
    "Алтан", "Алтангэрэл", "Алтанцэцэг", "Анар",
    "Батцэцэг", "Болормаа", "Болорцэцэг", "Энх",
    "Энхжаргал", "Эрдэнэ", "Эрдэнэчимэг", "Гэрэл",
    "Жаргал", "Марал", "Мөнхцэцэг", "Мөнхтуяа",
    "Нарангэрэл", "Наранцэцэг", "Одончимэг", "Одцэцэг",
    "Оюунчимэг", "Сарангэрэл", "Сарантуяа", "Сарнай",
    "Солонго", "Цэцэг", "Уянга", "Заяа"
};

const char* version_descriptions[] = {
    "Анхны хувилбар - Алтан",
    "Үндсэн функцууд - Алтангэрэл",
    "Сайжруулсан хувилбар - Алтанцэцэг",
    "Гурван үндсэн функц - Анар",
    "Тогтмол хувилбар - Батцэцэг",
    "Өнгөлөг хувилбар - Болормаа",
    "Ухаалаг хувилбар - Болорцэцэг",
    "Төгс хувилбар - Энх",
    "Төгсөг хувилбар - Энхжаргал",
    "Тогтворол хувилбар - Эрдэнэ",
    "Тогтвортой хувилбар - Эрдэнэчимэг",
    "Тогтмол хувилбар - Гэрэл",
    "Бүрэн хувилбар - Жаргал",
    "Сайн хувилбар - Марал",
    "Янз бүрийн хувилбар - Мөнхцэцэг",
    "Шинээрэл хувилбар - Мөнхтуяа",
    "Гэрэлт хувилбар - Нарангэрэл",
    "Улаан цайх - Наранцэцэг",
    "Оддын хувилбар - Одончимэг",
    "Оддын цэцэг - Одцэцэг",
    "Оюуны чимэг - Оюунчимэг",
    "Сарны гэрэл - Сарангэрэл",
    "Сарны туяа - Сарантуяа",
    "Сарнайн хувилбар - Сарнай",
    "Рашагын хувилбар - Солонго",
    "Цэцгийн хувилбар - Цэцэг",
    "Дууны хувилбар - Уянга",
    "Бүхнийг төгсгөх - Заяа"
};

const uint32_t version_majors[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const uint32_t version_minors[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6,
    7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
};

const uint32_t version_patches[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char* version_get_name(void) {
    return version_names[HUNNU_VERSION_CURRENT];
}

const char* version_get_description(void) {
    return version_descriptions[HUNNU_VERSION_CURRENT];
}

uint32_t version_get_major(void) {
    return version_majors[HUNNU_VERSION_CURRENT];
}

uint32_t version_get_minor(void) {
    return version_minors[HUNNU_VERSION_CURRENT];
}

uint32_t version_get_patch(void) {
    return version_patches[HUNNU_VERSION_CURRENT];
}

const char* version_get_string(void) {
    static char buffer[64];
    snprintf(buffer, sizeof(buffer), "%u.%u.%u (%s)",
            version_get_major(),
            version_get_minor(),
            version_get_patch(),
            version_get_name());
    return buffer;
}