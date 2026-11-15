/* Windows PDCursesMod keyboard, UTF-8 text output, wizard Ctrl-C,
 * and xterm-to-PDCurses palette translation (VT backend only).
 */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#define ROGUE_COLOR_INTERNAL
#include "rogue.h"
#include "rogue_windows.h"
#include "rogue_windows_console.h"

/* PDCursesMod's wget_wch() reports a Unicode scalar via wint_t.  On
 * MinGW-w64, wint_t is only 16 bits, so U+1F34E (apple emoji) becomes
 * U+F34E -- a completely different, private-use character.  PDCursesMod's
 * wgetch() also passes supplementary codepoints through a 16-bit wchar_t
 * while converting them to UTF-8.  Neither route can reliably preserve
 * emojis on this platform.
 *
 * While the options editor can access a Windows console input handle,
 * read UTF-16 KEY_EVENT records directly.  This preserves
 * surrogate pairs and keeps the curses rendering and color paths intact.
 * If there is no console input handle, retain the original PDCurses path
 * as a fallback for redirected/alternate terminals.
 */
static unsigned char input_utf8[4];
static int input_utf8_len = 0;
static int input_utf8_next = 0;
static unsigned long input_high_surrogate = 0;
static unsigned long input_keyup_high_surrogate = 0;
static unsigned long input_last_keydown_emoji = 0;
static unsigned long input_last_keydown_bmp = 0;
static int input_allow_keyup_surrogates = 0;
static int input_option_mode = 0;

/* With the VT backend, symbol/emoji-panel commits can appear in KeyUp
 * records under mintty. Recover both BMP symbols and surrogate-pair emoji
 * in the options editor, but ignore ordinary ASCII key releases and
 * suppress matching KeyDown/KeyUp duplicates. */
void rogue_win_set_option_input(int enabled)
{
    input_option_mode = enabled;
    input_high_surrogate = 0;
    input_keyup_high_surrogate = 0;
    input_last_keydown_emoji = 0;
    input_last_keydown_bmp = 0;
    input_allow_keyup_surrogates = enabled;
    input_utf8_next = input_utf8_len = 0;
}

/* Both the KeyDown and VT KeyUp paths use the same UTF-16
 * surrogate-pair arithmetic. Call this only after checking both halves. */
static unsigned long win_surrogate_scalar(unsigned long high, unsigned long low)
{
    return 0x10000UL + ((high - 0xd800UL) << 10) + (low - 0xdc00UL);
}

/* Convert common cursor/editing keys into the same Rogue codes as the
 * PDCurses input handler. Return -1 for an irrelevant key event.
 *
 * A supplementary Unicode character may arrive as either KeyDown or,
 * in VT sessions, KeyUp UTF-16 halves. Keep two independent
 * surrogate buffers so empty KeyDown events do not discard KeyUp data.
 * If both down and up contain the same character, suppress the echo. */
static int win_console_key(const KEY_EVENT_RECORD *ev)
{
    unsigned long cp;
    unsigned long hi;
    unsigned long scalar;
    WORD vk;
    DWORD modifiers;

    cp = (unsigned long)(unsigned short)ev->uChar.UnicodeChar;
    if (!ev->bKeyDown) {
        if (!input_allow_keyup_surrogates) return -1;
        if (cp >= 0xd800UL && cp <= 0xdbffUL) {
            input_keyup_high_surrogate = cp;
            return -1;
        }
        if (cp >= 0xdc00UL && cp <= 0xdfffUL) {
            hi = input_keyup_high_surrogate;
            input_keyup_high_surrogate = 0;
            if (!hi) return -1;
            scalar = win_surrogate_scalar(hi, cp);
            if (scalar == input_last_keydown_emoji) {
                input_last_keydown_emoji = 0;
                return -1;
            }
            input_last_keydown_emoji = 0;
            return (int)scalar;
        }
        /* Some mintty/ConPTY emoji-picker commits place a BMP symbol
         * (for example U+2694) only in a KeyUp record.  Previously only
         * supplementary characters were recovered this way.  Suppress a
         * matching KeyDown/KeyUp echo, but accept a release-only symbol.
         * ASCII releases remain ignored so navigation/editing keys are
         * never applied twice. */
        if (cp >= 0x80UL && (cp < 0xd800UL || cp > 0xdfffUL)) {
            if (cp == input_last_keydown_bmp) {
                input_last_keydown_bmp = 0;
                return -1;
            }
            input_last_keydown_bmp = 0;
            return (int)cp;
        }
        return -1; /* Never treat an ordinary key release as text. */
    }

    vk = ev->wVirtualKeyCode;
    modifiers = ev->dwControlKeyState;

    /* IME/emoji picker Unicode data has priority over its virtual key.
     * In particular, VK_RETURN can accompany a committed character. */
    if (cp >= 0x80UL) {
        if (cp >= 0xd800UL && cp <= 0xdbffUL) {
            input_high_surrogate = cp;
            return -1;
        }
        if (cp >= 0xdc00UL && cp <= 0xdfffUL) {
            hi = input_high_surrogate;
            input_high_surrogate = 0;
            if (!hi) return -1;
            scalar = win_surrogate_scalar(hi, cp);
            input_last_keydown_emoji = scalar;
            return (int)scalar;
        }
        input_high_surrogate = 0;
        input_last_keydown_emoji = 0;
        input_last_keydown_bmp = cp;
        return (int)cp;
    }

    /* These keys are for actual KeyDown events only. Do not clear the
     * KeyUp surrogate buffer: mintty can send empty KeyDown records
     * between the two KeyUp halves of a single emoji. */
    input_high_surrogate = 0;
    if (cp != 0) {
        /* A real text character commits the preceding emoji. Leave
         * empty KeyDown events alone: they can occur between mintty's
         * KeyUp surrogate halves, and may carry a nonzero VK_PACKET. */
        input_last_keydown_emoji = 0;
        input_last_keydown_bmp = 0;
    }
    switch (vk) {
    case VK_LEFT:   return 'h';
    case VK_RIGHT:  return 'l';
    case VK_UP:     return 'k';
    case VK_DOWN:   return 'j';
    case VK_HOME:   return 'y';
    case VK_END:    return 'b';
    case VK_PRIOR:  return 'u';
    case VK_NEXT:   return 'n';
    case VK_DELETE: return 127;
    case VK_BACK:   return '\b';
    case VK_RETURN: return '\n';
    case VK_ESCAPE: return '\033';
    default: break;
    }

    if (cp == 0) {
        if (modifiers & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
            if (vk >= 'A' && vk <= 'Z')
                return (int)(vk - 'A' + 1);
        }
        return -1;
    }
    return (int)cp;
}

/* wget_wch() returns KEY_CODE_YES for special keys and stores the
 * keycode in wc. This is the fallback when direct console input cannot
 * be used; MinGW-w64's 16-bit wint_t cannot preserve all emoji here. */
static int win_pdc_key(void)
{
    wint_t wc;
    int result;
    for (;;) {
        result = wget_wch(stdscr, &wc);
        if (result == ERR) continue;
        if (result == KEY_CODE_YES) {
            /* Some PDCursesMod input paths report Ctrl-J as CTL_ENTER. */
#ifdef CTL_ENTER
            if ((int)wc == CTL_ENTER) return '\n';
#endif
            if ((int)wc == 0xED11) return '\n';
            switch ((int)wc) {
#ifdef KEY_RESIZE
            case KEY_RESIZE: continue;
#endif
#ifdef KEY_LEFT
            case KEY_LEFT: return 'h';
#endif
#ifdef KEY_DOWN
            case KEY_DOWN: return 'j';
#endif
#ifdef KEY_UP
            case KEY_UP: return 'k';
#endif
#ifdef KEY_RIGHT
            case KEY_RIGHT: return 'l';
#endif
#ifdef KEY_HOME
            case KEY_HOME: return 'y';
#endif
#ifdef KEY_END
            case KEY_END: return 'b';
#endif
#ifdef KEY_PPAGE
            case KEY_PPAGE: return 'u';
#endif
#ifdef KEY_NPAGE
            case KEY_NPAGE: return 'n';
#endif
#ifdef KEY_DC
            case KEY_DC: return 127;
#endif
#ifdef KEY_BACKSPACE
            case KEY_BACKSPACE: return '\b';
#endif
#ifdef KEY_ENTER
            case KEY_ENTER: return '\n';
#endif
            default: continue;
            }
        }
        if (result == OK) return (int)wc;
    }
}

int rogue_win_getchar(void)
{
    HANDLE input;
    DWORD mode, received;
    INPUT_RECORD ev;
    unsigned long cp;
    int key;

    if (input_utf8_next < input_utf8_len)
        return input_utf8[input_utf8_next++];
    input_utf8_next = input_utf8_len = 0;

    input = GetStdHandle(STD_INPUT_HANDLE);
    if (!input_option_mode || input == INVALID_HANDLE_VALUE ||
        input == NULL || !GetConsoleMode(input, &mode)) {
        /* PDCurses fallback for redirected input/non-console backends. */
        key = win_pdc_key();
        if (key < 0) return key;
        if (key <= 0xff) return key;
        cp = (unsigned long)key;
    } else {
        for (;;) {
            if (!ReadConsoleInputW(input, &ev, 1, &received) ||
                received != 1)
                return EOF;
            if (ev.EventType != KEY_EVENT) continue;
            key = win_console_key(&ev.Event.KeyEvent);
            if (key < 0) continue;
            if (key <= 0x7f) return key;
            cp = (unsigned long)key;
            break;
        }
    }

    if (cp >= 0xd800UL && cp <= 0xdfffUL) return EOF;
    if (cp < 0x80UL) return (int)cp;
    if (cp < 0x800UL) {
        input_utf8[0] = 0xc0 | (unsigned char)(cp >> 6);
        input_utf8[1] = 0x80 | (unsigned char)(cp & 0x3f);
        input_utf8_len = 2;
    } else if (cp < 0x10000UL) {
        input_utf8[0] = 0xe0 | (unsigned char)(cp >> 12);
        input_utf8[1] = 0x80 | (unsigned char)((cp >> 6) & 0x3f);
        input_utf8[2] = 0x80 | (unsigned char)(cp & 0x3f);
        input_utf8_len = 3;
    } else if (cp <= 0x10ffffUL) {
        input_utf8[0] = 0xf0 | (unsigned char)(cp >> 18);
        input_utf8[1] = 0x80 | (unsigned char)((cp >> 12) & 0x3f);
        input_utf8[2] = 0x80 | (unsigned char)((cp >> 6) & 0x3f);
        input_utf8[3] = 0x80 | (unsigned char)(cp & 0x3f);
        input_utf8_len = 4;
    } else {
        return EOF;
    }
    input_utf8_next = 1;
    return input_utf8[0];
}

/* Keep ordinary Japanese (BMP) text on the same UTF-8 string output path
 * as Rogue 5.4.5 JEC.  PDCursesMod is built with PDC_FORCE_UTF8; its
 * waddstr() decodes a complete UTF-8 sequence and handles wide characters.
 * In particular, do not convert every Japanese character to wchar_t and
 * feed it separately to waddch(), which uses a different drawing path.
 *
 * Supplementary-plane characters (e.g. the apple emoji in the options)
 * still need a single 21-bit chtype: forwarding their four UTF-8 bytes to
 * a narrow curses string function can split them into mojibake.  Flush
 * preceding UTF-8 text, write the scalar, then continue the text run.
 * This is the Windows VT text path; it does not alter Unix or game rules.
 */
int rogue_win_addstr_utf8(const char *utf8)
{
    const unsigned char *p;
    char *segment;
    size_t total, used = 0;
    int result = OK;

    if (!utf8) return ERR;
    total = strlen(utf8);
    segment = (char *)malloc(total + 1);
    if (!segment) return waddstr(stdscr, utf8);

    p = (const unsigned char *)utf8;
    while (*p) {
        unsigned int cp = 0;
        size_t left = total - (size_t)(p - (const unsigned char *)utf8);

        if (left >= 4 && p[0] >= 0xf0 && p[0] <= 0xf4 &&
            (p[1] & 0xc0) == 0x80 && (p[2] & 0xc0) == 0x80 &&
            (p[3] & 0xc0) == 0x80) {
            cp = ((unsigned int)(p[0] & 7) << 18) |
                 ((unsigned int)(p[1] & 0x3f) << 12) |
                 ((unsigned int)(p[2] & 0x3f) << 6) |
                 (unsigned int)(p[3] & 0x3f);
        }
        /* U+2694 is text-presentation by default. The native Windows
         * backend can omit it, while Windows Terminal draws the VT repair
         * as a two-cell emoji. Reserve two curses cells so subsequent text
         * and cursor positioning agree with the repaired glyph. */
        if (left >= 3 && p[0] == 0xe2 && p[1] == 0x9a && p[2] == 0x94) {
            if (used) {
                segment[used] = 0;
                if (waddstr(stdscr, segment) == ERR) result = ERR;
                used = 0;
            }
            if (waddch(stdscr, (chtype)0x2694U) == ERR) result = ERR;
            if (waddch(stdscr, (chtype)' ') == ERR) result = ERR;
            p += 3;
        } else if (cp >= 0x10000 && cp <= 0x10ffff) {
            if (used) {
                segment[used] = 0;
                if (waddstr(stdscr, segment) == ERR) result = ERR;
                used = 0;
            }
            if (waddch(stdscr, (chtype)cp) == ERR) result = ERR;
            p += 4;
        } else {
            segment[used++] = (char)*p++;
        }
    }
    if (used) {
        segment[used] = 0;
        if (waddstr(stdscr, segment) == ERR) result = ERR;
    }
    free(segment);
    return result;
}

/*
 * PDCursesMod's Windows VT backend currently narrows the Unicode scalar
 * to a 16-bit wchar_t when converting a screen cell to UTF-8.  It can
 * therefore show '?' or an unrelated character for an emoji such as U+1F34E.
 *
 * Use the Rogue 5.4.5 JEC option-screen workaround for explicit text
 * screens (options, messages, inventory and scores). After curses
 * refresh, repaint supplementary-plane cells using UTF-8 VT output.  Keep the curses screen as the source
 * of truth and restore both its logical cursor and the terminal cursor.
 * Normal Japanese (BMP) and all dungeon drawing stay with PDCursesMod.
 */
/* The VT repaint is an out-of-band write relative to PDCurses.  Preserve
 * the terminal's REAL cursor with DECSC/DECRC rather than guessing its
 * position from getyx(stdscr): the latter is not necessarily the physical
 * cursor location under mintty/ConPTY.  Never move the curses cursor here.
 */
/* Return true if a scalar is a Unicode symbol that the Windows VT
 * backend may replace or omit. Supplementary-plane scalars need repair
 * because the backend may narrow them; U+2694 is a reproducible BMP
 * symbol failure. Do not change Japanese and ordinary BMP text. */
static int rogue_needs_vt_repaint(unsigned int cp)
{
    return (cp >= 0x10000U && cp <= 0x10ffffU) || cp == 0x2694U;
}

/* Write to the *same terminal* that PDCurses uses. Under ConPTY the
 * Win32 handle is a console; under MSYS2 mintty it is often a pipe and
 * GetConsoleMode() fails. In that case, use the pipe only when TERM is
 * set to a VT terminal, rather than silently skipping repaint. */
static int rogue_vt_write(HANDLE output, const char *s, size_t n)
{
    DWORD written = 0;
    return WriteFile(output, s, (DWORD)n, &written, NULL) &&
           written == (DWORD)n;
}

/* The console's VT parser decodes byte writes using its output code page.
 * A successful WriteFile is not evidence that the bytes were decoded as
 * UTF-8.  PDCurses may have restored a non-UTF-8 page during init. */
static int rogue_vt_ensure_utf8(int is_console)
{
    if (!is_console) return 1; /* mintty consumes UTF-8 bytes itself */
    if (GetConsoleOutputCP() == CP_UTF8) return 1;
    if (!SetConsoleOutputCP(CP_UTF8)) return 0;
    return GetConsoleOutputCP() == CP_UTF8;
}

void rogue_win_vt_restore_nonbmp_rows(int max_rows)
{
    HANDLE output;
    DWORD mode;
    int rows, cols, row, col, saved = 0;
    int is_console = 0;
    char seq[96];

    if (!stdscr || max_rows <= 0)
        return;
    output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output == NULL || output == INVALID_HANDLE_VALUE)
        return;
    if (GetConsoleMode(output, &mode)) {
        is_console = 1;
        if (!(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) &&
            !SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
            return;
    } else {
        const char *term = getenv("TERM");
        /* mintty's pseudo-terminal is a pipe to a native Windows program.
         * Do not emit VT sequences to redirected files or ordinary pipes. */
        if (GetFileType(output) != FILE_TYPE_PIPE || !term ||
            (!strstr(term, "xterm") && !strstr(term, "mintty") &&
             !strstr(term, "screen") && !strstr(term, "tmux")))
            return;
    }
    if (!rogue_vt_ensure_utf8(is_console)) return;

    getmaxyx(stdscr, rows, cols);
    if (rows > max_rows) rows = max_rows;
    if (cols > COLS) cols = COLS;
    for (row = 0; row < rows; ++row) {
        for (col = 0; col < cols - 1; ++col) {
            unsigned int cp = (unsigned int)(mvwinch(stdscr, row, col) & A_CHARTEXT);
            unsigned char utf8[4];
            int len, n;
            if (!rogue_needs_vt_repaint(cp))
                continue;
            if (!saved) {
                if (!rogue_vt_write(output, "\0337", 2)) return;
                saved = 1;
            }
            if (cp <= 0x7ffU) {
                utf8[0] = (unsigned char)(0xc0U | (cp >> 6));
                utf8[1] = (unsigned char)(0x80U | (cp & 0x3fU));
                len = 2;
            } else if (cp <= 0xffffU) {
                utf8[0] = (unsigned char)(0xe0U | (cp >> 12));
                utf8[1] = (unsigned char)(0x80U | ((cp >> 6) & 0x3fU));
                utf8[2] = (unsigned char)(0x80U | (cp & 0x3fU));
                len = 3;
            } else {
                utf8[0] = (unsigned char)(0xf0U | (cp >> 18));
                utf8[1] = (unsigned char)(0x80U | ((cp >> 12) & 0x3fU));
                utf8[2] = (unsigned char)(0x80U | ((cp >> 6) & 0x3fU));
                utf8[3] = (unsigned char)(0x80U | (cp & 0x3fU));
                len = 4;
            }
            n = snprintf(seq, sizeof seq, "\033[%d;%dH", row + 1, col + 1);
            if (n <= 0 || (size_t)n + (size_t)len >= sizeof seq)
                continue;
            memcpy(seq + n, utf8, (size_t)len);
            /* Select the emoji presentation of crossed swords in VT mode.
             * This also helps mintty choose the same two-cell glyph as
             * Windows Terminal. No change to the saved UTF-8 string. */
            if (cp == 0x2694U) {
                seq[n + len++] = (char)0xef;
                seq[n + len++] = (char)0xb8;
                seq[n + len++] = (char)0x8f;
            }
            (void)rogue_vt_write(output, seq, (size_t)n + (size_t)len);
        }
    }
    if (saved) (void)rogue_vt_write(output, "\0338", 2);
}

/* On Windows, ENABLE_PROCESSED_INPUT turns Ctrl-C into a console
 * CTRL_C_EVENT before PDCurses can read the key.  In wizard mode Ctrl-C
 * means "create object" (ASCII 3), so allow that key through as input.
 * Leave normal gameplay's Ctrl-C interrupt behavior unchanged.
 *
 * Save only the previous processed-input bit, not the whole input mode:
 * PDCurses and the console host may legitimately change other bits.
 * Restore the bit when wizard mode ends or curses shuts down.
 */
static int wizard_ctrl_c_mode_saved = 0;
static int wizard_ctrl_c_was_processed = 0;

void rogue_win_set_wizard_ctrl_c(int enabled)
{
    HANDLE input;
    DWORD mode, new_mode;

    /* Normal games do not need any extra Win32 console calls. */
    if (!enabled && !wizard_ctrl_c_mode_saved) return;
    input = GetStdHandle(STD_INPUT_HANDLE);

    if (input == INVALID_HANDLE_VALUE || input == NULL ||
        !GetConsoleMode(input, &mode)) return;

    if (enabled) {
        if (!wizard_ctrl_c_mode_saved) {
            wizard_ctrl_c_was_processed =
                (mode & ENABLE_PROCESSED_INPUT) != 0;
            wizard_ctrl_c_mode_saved = 1;
        }
        new_mode = mode & ~ENABLE_PROCESSED_INPUT;
    } else {
        if (!wizard_ctrl_c_mode_saved) return;
        new_mode = wizard_ctrl_c_was_processed
            ? (mode | ENABLE_PROCESSED_INPUT)
            : (mode & ~ENABLE_PROCESSED_INPUT);
    }

    if (new_mode != mode && !SetConsoleMode(input, new_mode)) {
        /* Retry on the next turn if setting the wizard mode failed. */
        if (enabled) wizard_ctrl_c_mode_saved = 0;
        return;
    }
    if (!enabled) wizard_ctrl_c_mode_saved = 0;
}


/* Windows/PDCursesMod color-index mapping for the VT backend.
 * Keep Clone III's existing RGB selections and curses color indices. */
static const short rogue_ansi_to_pdc[16] = {
    COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
    COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE,
    COLOR_BLACK + 8, COLOR_RED + 8, COLOR_GREEN + 8,
    COLOR_YELLOW + 8, COLOR_BLUE + 8, COLOR_MAGENTA + 8,
    COLOR_CYAN + 8, COLOR_WHITE + 8
};

static const unsigned char rogue_ansi_rgb[16][3] = {
    {  0,   0,   0}, {128,   0,   0}, {  0, 128,   0}, {128, 128,   0},
    {  0,   0, 128}, {128,   0, 128}, {  0, 128, 128}, {192, 192, 192},
    {128, 128, 128}, {255,   0,   0}, {  0, 255,   0}, {255, 255,   0},
    {  0,   0, 255}, {255,   0, 255}, {  0, 255, 255}, {255, 255, 255}
};

static void rogue_xterm_rgb(int index, int *red, int *green, int *blue)
{
    static const unsigned char steps[6] = {0, 95, 135, 175, 215, 255};
    int n;

    if ((index >= 0) && (index < 16)) {
        *red = rogue_ansi_rgb[index][0];
        *green = rogue_ansi_rgb[index][1];
        *blue = rogue_ansi_rgb[index][2];
    } else if ((index >= 16) && (index < 232)) {
        n = index - 16;
        *red = steps[n / 36];
        *green = steps[(n / 6) % 6];
        *blue = steps[n % 6];
    } else if ((index >= 232) && (index < 256)) {
        *red = *green = *blue = 8 + (index - 232) * 10;
    } else {
        *red = *green = *blue = 0;
    }
}

static short rogue_nearest_windows_color(int index)
{
    int red, green, blue, best, distance;
    int i, dr, dg, db, d;
    int count = (COLORS >= 16) ? 16 : ((COLORS >= 8) ? 8 : COLORS);

    if (count <= 0) {
        return COLOR_BLACK;
    }
    rogue_xterm_rgb(index, &red, &green, &blue);
    best = 0;
    distance = 0x7fffffff;
    for (i = 0; i < count; ++i) {
        dr = red - (int) rogue_ansi_rgb[i][0];
        dg = green - (int) rogue_ansi_rgb[i][1];
        db = blue - (int) rogue_ansi_rgb[i][2];
        d = dr * dr + dg * dg + db * db;
        if (d < distance) {
            distance = d;
            best = i;
        }
    }
    return rogue_ansi_to_pdc[best];
}

short rogue_win_platform_color(short color)
{
    if (color < 0) {
        return COLOR_BLACK;
    }
    if (color < 16) {
        return (color < COLORS) ? rogue_ansi_to_pdc[color]
                                : rogue_nearest_windows_color(color);
    }
    if (color >= COLORS) {
        return rogue_nearest_windows_color(color);
    }
    return color;
}

void rogue_win_init_palette(void)
{
    int index, red, green, blue, target;
    int count = (COLORS < 256) ? COLORS : 256;

    if (!can_change_color()) {
        return;
    }
    /* PDCursesMod VT owns screen output and its color palette; do not
     * bypass curses with raw ANSI escape sequences here. */
    for (index = 0; index < count; ++index) {
        rogue_xterm_rgb(index, &red, &green, &blue);
        target = (index < 16) ? rogue_ansi_to_pdc[index] : index;
        if (target >= COLORS) {
            continue;
        }
        (void) init_color((short)target,
                          (short)((red   * 1000 + 127) / 255),
                          (short)((green * 1000 + 127) / 255),
                          (short)((blue  * 1000 + 127) / 255));
    }
}
#endif /* _WIN32 */
