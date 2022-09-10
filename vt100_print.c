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

#include "vt100_print.h"
#include <stdio.h>
#include <string.h>

const char pStr_send_format[] PROGMEM = { "\x1B[%um" };

void vt100_Printer_Text_Color(FILE *f, vt100_color_t _Color) {
    if (_Color > BLACK) { fprintf_P(f, pStr_send_format, _Color); }
}

void vt100_Printer_Text_Format(FILE *f, vt100_format_t _Format) {
    if (_Format > F_NOT_SET) { fprintf_P(f, pStr_send_format, (uint8_t)_Format - 1); }
}

void vt100_Printer_Background_Color(FILE *f, vt100_color_t _Color) {
    if (_Color >= BLACK) { fprintf_P(f, pStr_send_format, (uint8_t)_Color + 10); }
}

const char pStr_send_clear[] PROGMEM = { "\x1B[" };

void vt100_clear(vt100_instance_t *i, vt100_clear_t t) {
    fprintf_P(i->f, pStr_send_clear);
    switch (t) {
        case LINE_AFTER_CURSOR: fprintf_P(i->f, PSTR("K"));  break;
        case LINE_TO_CURSOR:    fprintf_P(i->f, PSTR("1K")); break;
        case LINE:              fprintf_P(i->f, PSTR("2K")); break;
        case SCREEN:            fprintf_P(i->f, PSTR("2J")); break;
        default: {
            fprintf_P(i->f, PSTR("1;1H"));
            fprintf_P(i->f, pStr_send_clear);
            fprintf_P(i->f, PSTR("2J"));
        }
    }
}

void vt100_init(vt100_instance_t *i) {
    i->f = stdout;
    i->def.Default_Color = WHITE;
    i->def.Background_Color = BLUE;
    i->def.Format = BOLD;
    i->set.Default_Color = C_NOT_SET;
    i->set.Background_Color = C_NOT_SET;
    i->set.Format = F_NOT_SET;
}

void vt100_cursor(vt100_instance_t *i, bool cursor) {
    (cursor == true) ? fprintf_P(i->f, PSTR("\x1B[?25h")) : fprintf_P(i->f, PSTR("\x1B[?25l"));
}

void vt100_begin(vt100_instance_t *i) {
    vt100_format_restore(i);
    vt100_cursor(i, false);
    vt100_clear(i, ALL);
}


void vt100_end(vt100_instance_t *i) {
    vt100_Printer_Text_Format(i->f, MODESOFF);
    vt100_Printer_Text_Color(i->f, DEFAULT);
    vt100_Printer_Background_Color(i->f, DEFAULT);
    vt100_cursor(i, true);
    vt100_clear(i, ALL);
}

void vt100_format_set(vt100_instance_t *i) {
    if (i->set.Default_Color != C_NOT_SET) {
        vt100_Printer_Text_Color(i->f,  i->set.Default_Color);
        i->set.Default_Color = C_NOT_SET;
    }
    if (i->set.Background_Color != C_NOT_SET) {
        vt100_Printer_Background_Color(i->f, i->set.Background_Color);
        i->set.Background_Color = C_NOT_SET;
    }
    if (i->set.Format != F_NOT_SET) {
        vt100_Printer_Text_Format(i->f, i->set.Format);
        i->set.Format = F_NOT_SET;
    }
}

void vt100_format_restore(vt100_instance_t *i) {
    vt100_Printer_Text_Format(i->f, MODESOFF);
    vt100_Printer_Text_Format(i->f, i->def.Format);
    vt100_Printer_Background_Color(i->f, i->def.Background_Color);
    vt100_Printer_Text_Color(i->f, i->def.Default_Color);
}

void vt100_beep(vt100_instance_t *i) {
    fprintf_P(i->f, PSTR("\x07"));
}

void vt100_pos_x_y(vt100_instance_t *i, uint8_t x, uint8_t y) {
    fprintf_P(i->f, PSTR("\x1B[%u;%uH"), x , y);
}

void vt100_print_text(vt100_instance_t *i, char *txt) {
    vt100_pos_x_y(i, i->x1, i->y1);
    vt100_format_set(i);
    fputs(txt, i->f);
    vt100_format_restore(i);
}

void vt100_print_text_P(vt100_instance_t *i, char *txt) {
    vt100_pos_x_y(i, i->x1, i->y1);
    vt100_format_set(i);
    fputs_P(txt, i->f);
    vt100_format_restore(i);
}


void vt100_draw_box(vt100_instance_t *i) {
    vt100_format_set(i);
    vt100_pos_x_y(i, i->x1, i->y1); fprintf_P(i->f, PSTR("┌"));
    vt100_pos_x_y(i, i->x1, i->y2); fprintf_P(i->f, PSTR("┐"));
    vt100_pos_x_y(i, i->x2, i->y1); fprintf_P(i->f, PSTR("└"));
    vt100_pos_x_y(i, i->x2, i->y2); fprintf_P(i->f, PSTR("┘"));
    uint8_t p;
    for (p = i->x1 + 1; p <= i->x2 - 1; ++p) {
        vt100_pos_x_y(i, p, i->y1);
        fprintf_P(i->f, PSTR("│"));
    }
    for (p = i->x1 + 1; p <= i->x2 - 1; ++p) {
        vt100_pos_x_y(i, p, i->y2);
        fprintf_P(i->f, PSTR("│"));
    }
    for (p = i->y1 + 1; p <= i->y2 - 1; ++p) {
        vt100_pos_x_y(i, i->x1, p);
        fprintf_P(i->f, PSTR("─"));
    }
    for (p = i->y1 + 1; p <= i->y2 - 1; ++p) {
        vt100_pos_x_y(i, i->x2, p);
        fprintf_P(i->f, PSTR("─"));
    }
    vt100_format_restore(i);
}

void vt100_draw_divider(vt100_instance_t *i, vt100_divider_t dt, bool _End) {
    vt100_format_set(i);
    if (_End == true) {
        if (dt == Horizontal) {
            vt100_pos_x_y(i,i->x1, i->y1); fprintf_P(i->f, PSTR("├"));
            vt100_pos_x_y(i,i->x1, i->y2); fprintf_P(i->f, PSTR("┤"));
        } else if (dt == Vertical) {
            vt100_pos_x_y(i,i->x1, i->y1); fprintf_P(i->f, PSTR("┬"));
            vt100_pos_x_y(i,i->x2, i->y1); fprintf_P(i->f, PSTR("┴"));
        }
    }
    uint8_t p;
    if (dt == Horizontal) {
        for (p = i->y1 + 1; p <= i->y2 - 1; p++) {
            vt100_pos_x_y(i, i->x1, p);
            fprintf_P(i->f, PSTR("─"));
        }
    } else if (dt == Vertical) {
        for (p = i->x1 + 1; p <= i->x2 - 1; p++) {
            vt100_pos_x_y(i, p, i->y1);
            fprintf_P(i->f, PSTR("│"));
        }
    }
    vt100_format_restore(i);
}
