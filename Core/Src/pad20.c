/*
 * pad20.c
 *
 *  Created on: Mar 16, 2026
 *      Author: igor
 */
#include "pad20.h"
#include <string.h>

void Pad20(char *s, size_t cap)
{
    size_t len = strnlen(s, cap);
    if (len > 20) len = 20;

    for (size_t i = len; i < 20; i++) s[i] = ' ';
    s[20] = '\0';
}

