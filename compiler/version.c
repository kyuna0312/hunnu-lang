/**
 * @file version.c
 * @brief Version implementation using authentic Mongolian women names
 */

#include "version.h"
#include <stdio.h>
#include <string.h>

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