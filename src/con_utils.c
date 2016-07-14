/*
 * Copyright (C) 2019, Syntacore Ltd.
 * All Rights Reserved.
 */

/// Syntacore SCR* framework
///
/// @copyright Copyright (C) 2019, Syntacore Ltd. All rights reserved.
/// @author mn-sc
///
/// @brief console utilites

#include "uart.h"
#include "con_utils.h"

#define TTY_COLOR_NC       "\033[0m"
#define TTY_COLOR_REVERSE  "\033[7m"

#define TTY_COLOR_RED_Y    "\033[1;31m"
#define TTY_COLOR_GREEN    "\033[0;32m"
#define TTY_COLOR_GREEN_Y  "\033[1;32m"
#define TTY_COLOR_YELLOW   "\033[0;33m"
#define TTY_COLOR_YELLOW_Y "\033[1;33m"
#define TTY_COLOR_BLUE_Y   "\033[1;34m"
#define TTY_COLOR_MAGENTA  "\033[0;35m"
#define TTY_COLOR_CYAN     "\033[0;36m"
#define TTY_COLOR_CYAN_Y   "\033[1;36m"
#define TTY_COLOR_WHITE_Y  "\033[1;37m"

/* #define TTY_COLOR_MAIN_TEXT "\033[38;5;99m" */

#define TTY_COLOR_TIME_BRACE TTY_COLOR_CYAN
#define TTY_COLOR_TIME_TEXT TTY_COLOR_CYAN //TTY_COLOR_GREEN

#define TTY_COLOR_INFO2_BRACE TTY_COLOR_NC
#define TTY_COLOR_INFO2_TEXT TTY_COLOR_NC

#define TTY_COLOR_LINUX_TEXT TTY_COLOR_YELLOW_Y

/* #define TTY_COLOR_TEXT_MAIN_BRACE TTY_COLOR_YELLOW_Y */
/* #define TTY_COLOR_TEXT_REST_BRACE TTY_COLOR_YELLOW_Y //TTY_COLOR_YELLOW */

/* #define TTY_COLOR_TEXT_MAIN_BRACE TTY_COLOR_YELLOW_Y */
#define TTY_COLOR_TEXT_MAIN_BRACE TTY_COLOR_CYAN_Y
#define TTY_COLOR_TEXT_REST_BRACE TTY_COLOR_CYAN

/* #define TTY_COLOR_TEXT_MAIN_DIGIT TTY_COLOR_GREEN_Y */
#define TTY_COLOR_TEXT_MAIN_DIGIT TTY_COLOR_YELLOW_Y
#define TTY_COLOR_TEXT_REST_DIGIT TTY_COLOR_YELLOW

/* #define TTY_COLOR_MAIN_TEXT TTY_COLOR_GREEN_Y */
/* #define TTY_COLOR_MAIN_REST TTY_COLOR_GREEN */
#define TTY_COLOR_MAIN_TEXT TTY_COLOR_WHITE_Y
#define TTY_COLOR_MAIN_TEXT2 TTY_COLOR_CYAN_Y
#define TTY_COLOR_MAIN_REST TTY_COLOR_GREEN


/* Print colorized Syntacore header */
void  con_print_scr_header(void)
{
    static const char logo[] =
        "\n"
        /* "              vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "                  vvvvvvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrr       vvvvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrrrrr      vvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrrrrrrr    vvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrrrrrrr    vvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrrrrrrr    vvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrrrrr      vvvvvvvvvvvvvvvvvvvvvv  \n" */
        /* "rrrrrrrrrrrrr       vvvvvvvvvvvvvvvvvvvvvv    \n" */
        /* "rr                vvvvvvvvvvvvvvvvvvvvvv      \n" */
        /* "rr            vvvvvvvvvvvvvvvvvvvvvvvv      rr\n" */
        /* "rrrr      vvvvvvvvvvvvvvvvvvvvvvvvvv      rrrr\n" */
        /* "rrrrrr      vvvvvvvvvvvvvvvvvvvvvv      rrrrrr\n" */
        /* "rrrrrrrr      vvvvvvvvvvvvvvvvvv      rrrrrrrr\n" */
        /* "rrrrrrrrrr      vvvvvvvvvvvvvv      rrrrrrrrrr\n" */
        /* "rrrrrrrrrrrr      vvvvvvvvvv      rrrrrrrrrrrr\n" */
        /* "rrrrrrrrrrrrrr      vvvvvv      rrrrrrrrrrrrrr\n" */
        /* "rrrrrrrrrrrrrrrr      vv      rrrrrrrrrrrrrrrr\n" */
        /* "rrrrrrrrrrrrrrrrrr          rrrrrrrrrrrrrrrrrr\n" */
        /* "rrrrrrrrrrrrrrrrrrrr      rrrrrrrrrrrrrrrrrrrr\n" */

        /* ";;;;;;;;;;;;;;vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* ";;;;;;;;;;;;;;;;;;vvvvvvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrr;;;;;;;vvvvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrrrrr;;;;;;vvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrrrrrrr;;;;vvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrrrrrrr;;;;vvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrrrrrrr;;;;vvvvvvvvvvvvvvvvvvvvvvvv\n" */
        /* "rrrrrrrrrrrrrrrr;;;;;;vvvvvvvvvvvvvvvvvvvvvv;;\n" */
        /* "rrrrrrrrrrrrr;;;;;;;vvvvvvvvvvvvvvvvvvvvvv;;;;\n" */
        /* "rr;;;;;;;;;;;;;;;;vvvvvvvvvvvvvvvvvvvvvv;;;;;;\n" */
        /* "rr;;;;;;;;;;;;vvvvvvvvvvvvvvvvvvvvvvvv;;;;;;rr\n" */
        /* "rrrr;;;;;;vvvvvvvvvvvvvvvvvvvvvvvvvv;;;;;;rrrr\n" */
        /* "rrrrrr;;;;;;vvvvvvvvvvvvvvvvvvvvvv;;;;;;rrrrrr\n" */
        /* "rrrrrrrr;;;;;;vvvvvvvvvvvvvvvvvv;;;;;;rrrrrrrr\n" */
        /* "rrrrrrrrrr;;;;;;vvvvvvvvvvvvvv;;;;;;rrrrrrrrrr\n" */
        /* "rrrrrrrrrrrr;;;;;;vvvvvvvvvv;;;;;;rrrrrrrrrrrr\n" */
        /* "rrrrrrrrrrrrrr;;;;;;vvvvvv;;;;;;rrrrrrrrrrrrrr\n" */
        /* "rrrrrrrrrrrrrrrr;;;;;;vv;;;;;;rrrrrrrrrrrrrrrr\n" */
        /* "rrrrrrrrrrrrrrrrrr;;;;;;;;;;rrrrrrrrrrrrrrrrrr\n" */
        /* "rrrrrrrrrrrrrrrrrrrr;;;;;;rrrrrrrrrrrrrrrrrrrr\n" */

        /* ";;;;;;;vvvvvvvvvvvvvvvv\n" */
        /* ";;;;;;;;;vvvvvvvvvvvvvv\n" */
        /* "rrrrrr;;;;vvvvvvvvvvvvv\n" */
        /* "rrrrrrrr;;;vvvvvvvvvvvv\n" */
        /* "rrrrrrrrr;;vvvvvvvvvvvv\n" */
        /* "rrrrrrrrr;;vvvvvvvvvvvv\n" */
        /* "rrrrrrrrr;;vvvvvvvvvvvv\n" */
        /* "rrrrrrrr;;;vvvvvvvvvvv;\n" */
        /* "rrrrrr;;;;vvvvvvvvvvv;;\n" */
        /* "r;;;;;;;;vvvvvvvvvvv;;;\n" */
        /* "r;;;;;;vvvvvvvvvvvv;;;r\n" */
        /* "rr;;;vvvvvvvvvvvvv;;;rr\n" */
        /* "rrr;;;vvvvvvvvvvv;;;rrr\n" */
        /* "rrrr;;;vvvvvvvvv;;;rrrr\n" */
        /* "rrrrr;;;vvvvvvv;;;rrrrr\n" */
        /* "rrrrrr;;;vvvvv;;;rrrrrr\n" */
        /* "rrrrrrr;;;vvv;;;rrrrrrr\n" */
        /* "rrrrrrrr;;;v;;;rrrrrrrr\n" */
        /* "rrrrrrrrr;;;;;rrrrrrrrr\n" */
        /* "rrrrrrrrrr;;;rrrrrrrrrr\n" */

        /* ";;;;;vvvvvvvvvv\n" */
        /* "rrrr;;;vvvvvvvv\n" */
        /* "rrrrrr;;vvvvvvv\n" */
        /* "rrrrrr;;vvvvvv;\n" */
        /* "rrrr;;;vvvvvv;;\n" */
        /* "rr;;;vvvvvvv;;r\n" */
        /* "rrr;;vvvvvv;;rr\n" */
        /* "rrrr;;vvvv;;rrr\n" */
        /* "rrrrr;;vv;;rrrr\n" */
        /* "rrrrrrr;;rrrrrr\n" */

        ";;;;;vvvvvvvvv    SSSSSSS                                                    \n"
        "rrrr;;;vvvvvvv  SS                        s                                 TM\n"
        "rrrrrr;;vvvvvv  SS          s   s   sss  ssss  sss    sss   sss    sss  sss \n"
        "rrrrrr;;vvvvv;    SSSSSS    s   s  s   s  s       s  s     s   s  s    s   s\n"
        "rrrr;;;vvvvv;;          SS  s   s  s   s  s   sssss  s     s   s  s    sssss\n"
        "rr;;;vvvvvv;;r          SS   ssss  s   s  s   s   s  s     s   s  s    s    \n"
        "rrr;;vvvvv;;rr   SSSSSSS        s  s   s   ss  sss    sss   sss   s     sss \n"
        "rrrr;;vvv;;rrr               sss                                            \n"
        "rrrrr;;v;;rrrr\n"
        "rrrrrr;;;rrrrr                                           RISC-V CORES AND TOOLS\n"
        /* "       INSTRUCTION SETS WANT TO BE FREE\n" */
        ;

    const char *p = logo;

    /* uart_puts(TTY_COLOR_REVERSE "\n"); // reverse video */

#ifdef PLF_UART0_SCR_RTL
    uart_puts(p);
#else // PLF_UART0_SCR_RTL
    while (*p) {
        char ch = *p++;
        switch (ch) {
        case '\n':
            uart_putc(ch);
            break;
        case ' ':
            /* uart_puts(TTY_COLOR_WHITE_Y " "); */
            uart_putc(ch);
            break;
        case 'r':
            /* /\* uart_puts(TTY_COLOR_NC TTY_COLOR_REVERSE TTY_COLOR_BLUE_Y " "); *\/ */
            /* uart_puts(TTY_COLOR_BLUE_Y "r"); */
            uart_puts(TTY_COLOR_RED_Y "r");
            break;
        case 'v':
            /* /\* uart_puts(TTY_COLOR_YELLOW_Y " "); *\/ */
            /* uart_puts(TTY_COLOR_YELLOW_Y "v"); */
            uart_puts(TTY_COLOR_WHITE_Y "v");
            break;
        case ';':
            /* uart_puts(TTY_COLOR_WHITE_Y ";"); */
            uart_puts(TTY_COLOR_BLUE_Y ";");
            break;
        default:
            uart_puts(TTY_COLOR_WHITE_Y);
            uart_putc(ch);
            break;
        }
    }
    uart_puts(TTY_COLOR_NC "\n");
#endif // PLF_UART0_SCR_RTL
}

#ifdef PLF_KERNEL_ENTRY
enum Log_color_state {
    LCS_INIT = 0,
    LCS_TIME,
    LCS_INFO2,
    LCS_SPACE1,
    LCS_LINE_START,
    LCS_LINUX_HEADER,
    LCS_LINUX_HDR_REST,
    LCS_TEXT_MAIN,
    LCS_TEXT_REST,
    LCS_PARSE_ERR,
};

enum Log_color_sub_state {
    LCS_NONE = 0,
    LCS_BRACES = 1,
    LCS_DIGIT = 2,
    LCS_XDIGIT = 4,
};


#if PLF_MMODE_ONLY == 0
/* Linux kernel log colorizer */
void con_colorize_putchar(int ch)
{
    static unsigned lcs_pos = 0;
    static unsigned lcs = LCS_INIT;
    static unsigned lcs_submode = LCS_NONE;
    static const char linux_hdr_pattern[] = "Linux ";

    switch (lcs) {
    case LCS_INIT:
        if (ch == '[') {
            uart_puts(TTY_COLOR_TIME_BRACE "[" TTY_COLOR_TIME_TEXT);
            lcs = LCS_TIME;
            lcs_submode = LCS_NONE;
            lcs_pos = 0;
        } else {
            if (ch != '\n' && ch != '\r') {
                lcs = LCS_PARSE_ERR;
                uart_puts(TTY_COLOR_NC);
            }
            uart_putc(ch);
        }
        break;
    case LCS_TIME:
        if (ch == ']') {
            uart_puts(TTY_COLOR_TIME_BRACE "]" TTY_COLOR_MAIN_TEXT);
            lcs = LCS_SPACE1;
        } else {
            if (++lcs_pos > 30) {
                lcs = LCS_PARSE_ERR;
                uart_puts(TTY_COLOR_NC);
            }
            uart_putc(ch);
        }
        break;
    case LCS_SPACE1:
        if (ch == ' ') {
            uart_putc(ch);
            lcs = LCS_LINE_START;
        } else if (ch == '[') {
            uart_puts(TTY_COLOR_INFO2_BRACE "[" TTY_COLOR_INFO2_TEXT);
            lcs = LCS_INFO2;
            lcs_submode = LCS_NONE;
            lcs_pos = 0;
        } else {
            lcs = LCS_PARSE_ERR;
            uart_puts(TTY_COLOR_NC);
            uart_putc(ch);
        }
        break;
    case LCS_INFO2:
        if (ch == ']') {
            uart_puts(TTY_COLOR_INFO2_BRACE "]" TTY_COLOR_MAIN_TEXT);
            lcs = LCS_SPACE1;
        } else {
            if (++lcs_pos > 30) {
                lcs = LCS_PARSE_ERR;
                uart_puts(TTY_COLOR_NC);
            }
            uart_putc(ch);
        }
        break;
    case LCS_LINE_START:
        if (ch == linux_hdr_pattern[0]) {
            lcs = LCS_LINUX_HEADER;
            lcs_pos = 1;
        } else {
            if (ch == ' ' || ch == '\t')
                uart_puts(TTY_COLOR_MAIN_TEXT2);
            lcs = LCS_TEXT_MAIN;
            uart_putc(ch);
        }
        break;
    case LCS_LINUX_HEADER:
        if (linux_hdr_pattern[lcs_pos] == '\0') {
            uart_puts(TTY_COLOR_LINUX_TEXT);
            uart_puts(linux_hdr_pattern);
            /* uart_puts(TTY_COLOR_MAIN_TEXT); */
            lcs = LCS_LINUX_HDR_REST;
            uart_putc(ch);
        } else if (ch == linux_hdr_pattern[lcs_pos]) {
            ++lcs_pos;
        } else {
            lcs = LCS_TEXT_MAIN;
            for (unsigned i = 0; i < lcs_pos; ++i)
                uart_putc(linux_hdr_pattern[i]);
            uart_putc(ch);
        }
        break;
    case LCS_LINUX_HDR_REST:
        if (ch == '\n') {
            lcs = LCS_INIT;
            uart_puts(TTY_COLOR_NC);
        }
        uart_putc(ch);
        break;
    case LCS_TEXT_MAIN:
        if (ch == '\n') {
            lcs = LCS_INIT;
            uart_puts(TTY_COLOR_NC);
        } else if (ch == '[') {
            uart_puts(TTY_COLOR_TEXT_MAIN_BRACE);
            lcs_submode = LCS_BRACES;
        } else if (ch >= '0' && ch <= '9' && !(lcs_submode & LCS_DIGIT) && !(lcs_submode & LCS_XDIGIT)) {
            lcs_submode |= LCS_DIGIT;
            uart_puts(TTY_COLOR_TEXT_MAIN_DIGIT);
        } else if ((lcs_submode & LCS_DIGIT) && (ch == 'x' || ch == 'X')) {
            lcs_submode &= ~LCS_DIGIT;
            lcs_submode |= LCS_XDIGIT;
            uart_puts(TTY_COLOR_TEXT_MAIN_DIGIT);
        } else if ((lcs_submode & LCS_DIGIT) && !(ch >= '0' && ch <= '9')) {
            lcs_submode &= ~LCS_DIGIT;
            uart_puts(lcs_submode & LCS_BRACES ? TTY_COLOR_TEXT_MAIN_BRACE : TTY_COLOR_MAIN_TEXT);
        } else if ((lcs_submode & LCS_XDIGIT) && !(ch >= '0' && ch <= '9')
                   && !((ch | ('a' ^ 'A')) >= 'a' && (ch | ('a' ^ 'A')) <= 'f')) {
            lcs_submode &= ~LCS_XDIGIT;
            uart_puts(lcs_submode & LCS_BRACES ? TTY_COLOR_TEXT_MAIN_BRACE : TTY_COLOR_MAIN_TEXT);
        }
        /* if (ch == '\033') { */
        /*     uart_puts("***ESC***"); */
        /* } */
        uart_putc(ch);
        if (ch == ']') {
            lcs_submode = LCS_NONE;
            uart_puts(TTY_COLOR_MAIN_TEXT);
        }
        if (ch == ':') {
            lcs = LCS_TEXT_REST;
            lcs_submode = LCS_NONE;
            uart_puts(TTY_COLOR_MAIN_REST);
        }
        break;
    case LCS_TEXT_REST:
        if (ch == '\n') {
            lcs = LCS_INIT;
            uart_puts(TTY_COLOR_NC);
        } else if (ch == '[') {
            uart_puts(TTY_COLOR_TEXT_REST_BRACE);
            lcs_submode = LCS_BRACES;
        } else if (ch >= '0' && ch <= '9' && !(lcs_submode & LCS_DIGIT) && !(lcs_submode & LCS_XDIGIT)) {
            lcs_submode |= LCS_DIGIT;
            uart_puts(TTY_COLOR_TEXT_REST_DIGIT);
        } else if ((lcs_submode & LCS_DIGIT) && (ch == 'x' || ch == 'X')) {
            lcs_submode &= ~LCS_DIGIT;
            lcs_submode |= LCS_XDIGIT;
            uart_puts(TTY_COLOR_TEXT_REST_DIGIT);
        } else if ((lcs_submode & LCS_DIGIT) && !(ch >= '0' && ch <= '9')) {
            lcs_submode &= ~LCS_DIGIT;
            uart_puts(lcs_submode & LCS_BRACES ? TTY_COLOR_TEXT_REST_BRACE : TTY_COLOR_MAIN_REST);
        } else if ((lcs_submode & LCS_XDIGIT) && !(ch >= '0' && ch <= '9')
                   && !((ch | ('a' ^ 'A')) >= 'a' && (ch | ('a' ^ 'A')) <= 'f')) {
            lcs_submode &= ~LCS_XDIGIT;
            uart_puts(lcs_submode & LCS_BRACES ? TTY_COLOR_TEXT_REST_BRACE : TTY_COLOR_MAIN_REST);
        }
        uart_putc(ch);
        if (ch == ']') {
            uart_puts(TTY_COLOR_MAIN_REST);
        }
        break;
    case LCS_PARSE_ERR:
        if (ch == '\n') {
            lcs = LCS_INIT;
            uart_puts(TTY_COLOR_NC);
        }
        /* if (ch == '\033') { */
        /*     uart_puts("***ESC***"); */
        /* } */
        uart_putc(ch);
        break;
    default:
        uart_putc(ch);
        break;
    }
}
#endif // PLF_MMODE_ONLY

#endif // PLF_KERNEL_ENTRY
