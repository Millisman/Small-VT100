/*
 * MIT License
 *
 * Copyright (c) 2022 Sergey Kostyanoy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define F(x)    (x)
#define PSTR(x) (x)
#define PROGMEM
#define printf_P printf
#define fprintf_P fprintf
#define fputs_P fputs

typedef enum  {
    C_NOT_SET     = 0,
    BLACK         = 30,
    RED           = 31,
    GREEN         = 32,
    YELLOW        = 33,
    BLUE          = 34,
    MAGENTA       = 35,
    CYAN          = 36,
    WHITE         = 37,
    DEFAULT       = 39
} vt100_color_t;


typedef enum  {
    F_NOT_SET       = 0,
    MODESOFF        = 1,
    BOLD            = 2,
    LOW_INTENSITY   = 3,
    UNDERLINE       = 5,
    BLINKING        = 6,
    REVERSE         = 8,
    INVISIBLE       = 9
} vt100_format_t;

typedef enum  {
    LINE_AFTER_CURSOR   = 0,
    LINE_TO_CURSOR      = 1,
    LINE                = 2,
    SCREEN              = 3,
    ALL                 = 4
} vt100_clear_t;

typedef enum  {
    Horizontal,
    Vertical
} vt100_divider_t;

typedef struct {
    vt100_color_t Default_Color;
    vt100_color_t Background_Color;
    vt100_format_t Format;
} vt100_ccf_t;

typedef struct {
    FILE    *f;
    vt100_ccf_t def;
    vt100_ccf_t set;
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
} vt100_instance_t;

void vt100_init(vt100_instance_t *i);
void vt100_begin(vt100_instance_t *i);
void vt100_end(vt100_instance_t *i);
void vt100_clear(vt100_instance_t *i, vt100_clear_t t);
void vt100_cursor(vt100_instance_t *i, bool cursor);
void vt100_format_set(vt100_instance_t *i);
void vt100_format_restore(vt100_instance_t *i);
void vt100_beep(vt100_instance_t *i);
void vt100_pos_x_y(vt100_instance_t *i, uint8_t x, uint8_t y);
void vt100_print_text(vt100_instance_t *i, char *txt);
void vt100_print_text_P(vt100_instance_t *i, char *txt);
void vt100_draw_box(vt100_instance_t *i);
void vt100_draw_divider(vt100_instance_t *i, vt100_divider_t dt, bool _End);
