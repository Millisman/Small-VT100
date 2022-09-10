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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include "vt100_print.h"

struct termios orig_termios;

void die(const char *s) {
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

typedef enum  {
    VT100_Term_State_IDLE,
    VT100_Term_State_ESCS,               // escape sequence
    VT100_Term_State_ANSI_Key_Mode_Reset,// escape sequence + [
    VT100_Term_State_ANSI_Key_Mode_Set,  // escape sequence + O
    VT100_Term_State_Skip_Char
} VT100_Term_State;


VT100_Term_State vt100_ts = VT100_Term_State_IDLE;

typedef enum  {
    KBD_BTN_NULL        = 0,
    KBD_BTN_KEY_ENTER,
    KBD_BTN_KEY_TAB,
    KBD_BTN_KEY_ESC,
    KBD_BTN_KEY_BACKSPACE,
    KBD_BTN_KEY_UP,
    KBD_BTN_KEY_DOWN,
    KBD_BTN_KEY_RIGHT,
    KBD_BTN_KEY_LEFT,
    KBD_BTN_KEY_F1,
    KBD_BTN_KEY_F2,
    KBD_BTN_KEY_F3,
    KBD_BTN_KEY_F4,
    KBD_BTN_KEY_HOME,
    KBD_BTN_KEY_END,
    KBD_BTN_CTRL_C,
    KBD_BTN_KEY_INSERT,
    KBD_BTN_KEY_DELETE,
    KBD_BTN_KEY_PG_UP,
    KBD_BTN_KEY_PG_DN
} KeyboardButtons;

static KeyboardButtons Keyboard_Pressed = KBD_BTN_NULL;
static KeyboardButtons Keyboard_Temp    = KBD_BTN_NULL;
u_int8_t UDR0;

void UDR_INT() {
    u_int8_t c = UDR0;

    switch (vt100_ts) {
        case VT100_Term_State_IDLE:
            switch (c) {
                case 0x1B: vt100_ts = VT100_Term_State_ESCS; break;
                case 0x03: Keyboard_Pressed = KBD_BTN_CTRL_C;        break;  // CTRL + C
                case 0x09: Keyboard_Pressed = KBD_BTN_KEY_TAB;       break;  // TAB
                case 0x0D: Keyboard_Pressed = KBD_BTN_KEY_ENTER;     break;
                case 0x7F: Keyboard_Pressed = KBD_BTN_KEY_BACKSPACE; break;
                default: {
                    if ((c != 127) && (c > 0x1F)) {
                        printf_P(PSTR("Load_Buffer: %d ('%c')\r\n"), c, c);
                    } else { printf_P(PSTR("Unk not printable char: %d ('%c')\r\n"), c, c); }
                }
            }
            break;

                case VT100_Term_State_ESCS:
                    vt100_ts = VT100_Term_State_IDLE;
                    switch (c) {
                        case 0x1B: Keyboard_Pressed = KBD_BTN_KEY_ESC; break;
                        case '[': { vt100_ts = VT100_Term_State_ANSI_Key_Mode_Reset; } break;
                        case 'O': { vt100_ts = VT100_Term_State_ANSI_Key_Mode_Set; } break;
                        default: printf_P(PSTR("UNKNOWN GROUP %d ('%c')\r\n"), c, c);
                    }
                    break;

                        case VT100_Term_State_ANSI_Key_Mode_Reset: // '['
                        case VT100_Term_State_ANSI_Key_Mode_Set:   // 'O'
                            vt100_ts = VT100_Term_State_IDLE;
                            switch (c) {
                                case 'A': Keyboard_Pressed = KBD_BTN_KEY_UP;    break; // BT_KEY_UP;
                                case 'B': Keyboard_Pressed = KBD_BTN_KEY_DOWN;  break; // BT_KEY_DOWN;
                                case 'C': Keyboard_Pressed = KBD_BTN_KEY_RIGHT; break; // BT_KEY_RIGHT;
                                case 'D': Keyboard_Pressed = KBD_BTN_KEY_LEFT;  break; // BT_KEY_LEFT;
                                case 'P': Keyboard_Pressed = KBD_BTN_KEY_F1;    break; // BT_KEY_F1;
                                case 'Q': Keyboard_Pressed = KBD_BTN_KEY_F2;    break; // BT_KEY_F2;
                                case 'R': Keyboard_Pressed = KBD_BTN_KEY_F3;    break; // BT_KEY_F3;
                                case 'S': Keyboard_Pressed = KBD_BTN_KEY_F4;    break; // BT_KEY_F4;
                                case 'H': Keyboard_Pressed = KBD_BTN_KEY_HOME;  break; // 
                                case 'F': Keyboard_Pressed = KBD_BTN_KEY_END;   break; // 
                                case '2': Keyboard_Temp = KBD_BTN_KEY_INSERT;   vt100_ts = VT100_Term_State_Skip_Char;  break; // 
                                case '3': Keyboard_Temp = KBD_BTN_KEY_DELETE;   vt100_ts = VT100_Term_State_Skip_Char;  break; // 
                                case '5': Keyboard_Temp = KBD_BTN_KEY_PG_UP;    vt100_ts = VT100_Term_State_Skip_Char;  break; // 
                                case '6': Keyboard_Temp = KBD_BTN_KEY_PG_DN;    vt100_ts = VT100_Term_State_Skip_Char;  break; //
                                default: printf_P(PSTR("WTF %d ('%c')\r\n"), c, c);
                            }
                            break;

                        case VT100_Term_State_Skip_Char:
                            if (c == 0x7E) {
                                switch (Keyboard_Temp) {
                                    case KBD_BTN_KEY_INSERT:   Keyboard_Pressed = KBD_BTN_KEY_INSERT;   break;
                                    case KBD_BTN_KEY_DELETE:   Keyboard_Pressed = KBD_BTN_KEY_DELETE;   break;
                                    case KBD_BTN_KEY_PG_UP:    Keyboard_Pressed = KBD_BTN_KEY_PG_UP;    break;
                                    case KBD_BTN_KEY_PG_DN:    Keyboard_Pressed = KBD_BTN_KEY_PG_DN;    break;
                                    default: printf_P(PSTR("VT100_Term_State_Skip_Char %d ('%c')\r\n"), c, c);
                                }
                            } else { printf_P(PSTR("wtf! Tilda\r\n")); }
                            Keyboard_Temp = KBD_BTN_NULL;
                            vt100_ts = VT100_Term_State_IDLE;
                            //printf_P(PSTR("VT100_Term_State_Skip_Char>>>VT100_Term_State_IDLE %d ('%c')\r\n"), c, c);
                            break;
    }
}





void Print_Pressed_Decoder(KeyboardButtons _key) {
    switch (_key) {
        case KBD_BTN_NULL:      break;
        case KBD_BTN_KEY_ENTER:     printf_P(PSTR("KBD_BTN_KEY_ENTER\r\n"));        break;
        case KBD_BTN_KEY_TAB:       printf_P(PSTR("KBD_BTN_KEY_TAB\r\n"));          break;
        case KBD_BTN_KEY_ESC:       printf_P(PSTR("KBD_BTN_KEY_ESC\r\n"));          break;
        case KBD_BTN_KEY_BACKSPACE: printf_P(PSTR("KBD_BTN_KEY_BACKSPACE\r\n"));    break;
        case KBD_BTN_KEY_UP:        printf_P(PSTR("KBD_BTN_KEY_UP\r\n"));           break;
        case KBD_BTN_KEY_DOWN:      printf_P(PSTR("KBD_BTN_KEY_DOWN\r\n"));         break;
        case KBD_BTN_KEY_RIGHT:     printf_P(PSTR("KBD_BTN_KEY_RIGHT\r\n"));        break;
        case KBD_BTN_KEY_LEFT:      printf_P(PSTR("KBD_BTN_KEY_LEFT\r\n"));         break;
        case KBD_BTN_KEY_F1:        printf_P(PSTR("KBD_BTN_KEY_F1\r\n"));           break;
        case KBD_BTN_KEY_F2:        printf_P(PSTR("KBD_BTN_KEY_F2\r\n"));           break;
        case KBD_BTN_KEY_F3:        printf_P(PSTR("KBD_BTN_KEY_F3\r\n"));           break;
        case KBD_BTN_KEY_F4:        printf_P(PSTR("KBD_BTN_KEY_F4\r\n"));           break;
        case KBD_BTN_KEY_HOME:      printf_P(PSTR("KBD_BTN_KEY_HOME\r\n"));         break;
        case KBD_BTN_KEY_END:       printf_P(PSTR("KBD_BTN_KEY_END\r\n"));          break;
        case KBD_BTN_KEY_INSERT:    printf_P(PSTR("KBD_BTN_KEY_INSERT\r\n"));       break;
        case KBD_BTN_KEY_DELETE:    printf_P(PSTR("KBD_BTN_KEY_DELETE\r\n"));       break;
        case KBD_BTN_KEY_PG_UP:     printf_P(PSTR("KBD_BTN_KEY_PG_UP\r\n"));        break;
        case KBD_BTN_KEY_PG_DN:     printf_P(PSTR("KBD_BTN_KEY_PG_DN\r\n"));        break;
        case KBD_BTN_CTRL_C:        printf_P(PSTR("KBD_BTN_CTRL_C\r\n"));           break;
    }
}

#define CEL_COUNT 15

u_int16_t cells_mv[CEL_COUNT] = {3854,3940,3901,3899,3988,3964,3978,3887,3899,3754,3999,3797,3992,3910,3959};

const char pStr_BMS[]               PROGMEM = {"BMS"};
const char pStr_Up_Time[]           PROGMEM = {"Up Time :"};
const char pStr_Build_on[]          PROGMEM = {"Build on : " __DATE__ " " __TIME__};
const char pStr_Cell[]              PROGMEM = {"Cell"};
const char pStr_Raw[]               PROGMEM = {"Raw"};
const char pStr_Volts[]             PROGMEM = {"Volts"};
const char pStr_Name[]              PROGMEM = {"Name"};
const char pStr_Count[]             PROGMEM = {"Count"};
const char pStr_Last_Time[]         PROGMEM = {"Last Time"};
const char pStr_Monitor_XREADY[]    PROGMEM = {"Monitor XREADY"};
const char pStr_Monitor_ALERT[]     PROGMEM = {"Monitor ALERT"};
const char pStr_Under_Voltage[]     PROGMEM = {"Under Voltage"};
const char pStr_Over_Voltage[]      PROGMEM = {"Over Voltage"};
const char pStr_Load_Short_Circuit[] PROGMEM = {"Load Short Circuit"};
const char pStr_Load_Overcurrent[]  PROGMEM = {"Load Overcurrent"};
const char pStr_USER_SWITCH[]       PROGMEM = {"USER SWITCH"};
const char pStr_USER_DISCHG_TEMP[]  PROGMEM = {"USER DISCHG_TEMP"};
const char pStr_USER_CHG_TEMP[]     PROGMEM = {"USER CHG_TEMP"};
const char pStr_USER_CHG_OCD[]      PROGMEM = {"USER CHG_OCD"};



vt100_instance_t vt;

void Print_Background() {
    vt.x1 = 1; vt.y1 = 1; vt.x2 = 27; vt.y2 = 71; vt100_draw_box(&vt);
    vt.x1 = 2; vt.y1 = 27;  vt100_print_text_P(&vt, (char *)pStr_BMS);
    vt.x1 = 2; vt.y1 = 3;   vt100_print_text_P(&vt, (char *)pStr_Up_Time);
    vt.x1 = 2; vt.y1 = 37;  vt100_print_text_P(&vt, (char *)pStr_Build_on);
    vt.x1 = 3; vt.y1 = 1; vt.x2 = 3; vt.y2 = 71; vt100_draw_divider(&vt, Horizontal, true);
    vt.x1 = 4; vt.y1 = 2; vt.x2 = 22; vt.y2 = 24; vt100_draw_box(&vt);
    vt.x1 = 5; vt.y1 = 3;   vt100_print_text_P(&vt, (char *)pStr_Cell);
    vt.x1 = 5; vt.y1 = 10;  vt100_print_text_P(&vt, (char *)pStr_Raw);
    vt.x1 = 5; vt.y1 = 18;  vt100_print_text_P(&vt, (char *)pStr_Volts);
    vt.x1 = 6; vt.y1 = 2;  vt.x2 = 6;  vt.y2 = 24; vt100_draw_divider(&vt, Horizontal, true);
    vt.x1 = 6; vt.y1 = 7;  vt.x2 = 22; vt.y2 = 7;  vt100_draw_divider(&vt, Vertical, true);
    vt.x1 = 6; vt.y1 = 16; vt.x2 = 22; vt.y2 = 16; vt100_draw_divider(&vt, Vertical, true);
    vt.x1 = 4; vt.y1 = 25; vt.x2 = 22; vt.y2 = 70; vt100_draw_box(&vt);
    vt.x1 = 5; vt.y1 = 31;  vt100_print_text_P(&vt, (char *)pStr_Name);
    vt.x1 = 5; vt.y1 = 48;  vt100_print_text_P(&vt, (char *)pStr_Count);
    vt.x1 = 5; vt.y1 = 59;  vt100_print_text_P(&vt, (char *)pStr_Last_Time);
    vt.x1 = 6; vt.y1 = 25;  vt.x2 = 6;  vt.y2 = 70;  vt100_draw_divider(&vt, Horizontal, true);
    vt.x1 = 6; vt.y1 = 46;  vt.x2 = 22; vt.y2 = 46;  vt100_draw_divider(&vt, Vertical, true);
    vt.x1 = 6; vt.y1 = 56;  vt.x2 = 22; vt.y2 = 56;  vt100_draw_divider(&vt, Vertical, true);
    vt.set.Default_Color = RED; vt.set.Format = BLINKING;
    vt.x1 = 7;  vt.y1 = 27; vt100_print_text_P(&vt, (char *)pStr_Monitor_XREADY);
    vt.x1 = 8;  vt.y1 = 27; vt100_print_text_P(&vt, (char *)pStr_Monitor_ALERT);
    vt.x1 = 9;  vt.y1 = 27; vt100_print_text_P(&vt, (char *)pStr_Under_Voltage);
    vt.x1 = 10; vt.y1 = 27; vt100_print_text_P(&vt, (char *)pStr_Over_Voltage);
    vt.x1 = 11; vt.y1 = 27; vt100_print_text_P(&vt, (char *)pStr_Load_Short_Circuit);
    vt.x1 = 12; vt.y1 = 27; vt100_print_text_P(&vt, (char *)pStr_Load_Overcurrent);
    vt.x1 = 13; vt.y1 = 27; vt100_print_text_P(&vt, (char *)pStr_USER_SWITCH);
    vt.x1 = 14; vt.y1 = 27; vt100_print_text_P(&vt, (char *)pStr_USER_DISCHG_TEMP);
    vt.x1 = 15; vt.y1 = 27; vt100_print_text_P(&vt, (char *)pStr_USER_CHG_TEMP);
    vt.x1 = 16; vt.y1 = 27; vt100_print_text_P(&vt, (char *)pStr_USER_CHG_OCD);
    vt.x1 = 23; vt.y1 = 2; vt.x2 = 26; vt.y2 = 70; vt100_draw_box(&vt);
}

void Print_Values() {
    for (uint8_t i = 0; i < CEL_COUNT; i++) {
        vt100_pos_x_y(&vt, 7+i, 4);  fprintf_P(vt.f, PSTR("%u"), i+1);
        vt100_pos_x_y(&vt, 7+i, 9);  fprintf_P(vt.f, PSTR("%u"), cells_mv[i]);
        vt100_pos_x_y(&vt, 7+i, 18); fprintf_P(vt.f, PSTR("%1u.%03u"), cells_mv[i]/1000, cells_mv[i]%1000);
    }
}

int main(int argc, char **argv) {
    enableRawMode();

    vt100_init(&vt);
    vt100_begin(&vt);
    Print_Background();

    while (1) {
        for (uint8_t i = 0; i < CEL_COUNT; i++) {
            cells_mv[i] += rand()/100000000;
            cells_mv[i] -= rand()/100000000;
        }

        if (read(STDIN_FILENO, &UDR0, 1) == -1 && errno != EAGAIN) die("read");
        if (UDR0) UDR_INT();
        UDR0 = 0;

        Print_Pressed_Decoder(Keyboard_Pressed);

        if (Keyboard_Pressed == KBD_BTN_CTRL_C) {

            vt100_end(&vt);

            printf_P(PSTR("CTRL + C, Bye!\r\n"));
            exit(0);
        }
        Keyboard_Pressed = KBD_BTN_NULL;
        Print_Values();
    }
    return 0;
}
