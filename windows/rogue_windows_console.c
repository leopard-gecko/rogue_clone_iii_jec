/* Windows PDCursesMod keyboard, UTF-8 text output, wizard Ctrl-C,
 * and xterm-to-PDCurses palette translation.  No game-rule changes.
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
static int input_allow_keyup_surrogates = 0;
static int input_option_mode = 0;

/* Under MSYS2/mintty, some emoji-panel commits deliver their UTF-16
 * surrogate halves on KeyUp, with empty KeyDown events between them.
 * Native Windows consoles normally supply the halves on KeyDown.
 * Recognize VT/mintty sessions without enabling KeyUp for ordinary keys. */
static int win_accept_keyup_surrogates(void)
{
    const char *term = getenv("TERM");
    const char *msystem = getenv("MSYSTEM");
    const char *term_program = getenv("TERM_PROGRAM");
#ifdef CLONE_PDC_VT
    return 1;
#else
    if (term_program && strstr(term_program, "mintty")) return 1;
    if (msystem && *msystem && term &&
        (!strncmp(term, "xterm", 5) || !strncmp(term, "screen", 6) ||
         !strncmp(term, "tmux", 4))) return 1;
    return 0;
#endif
}

void rogue_win_set_option_input(int enabled)
{
    input_option_mode = enabled;
    input_high_surrogate = 0;
    input_keyup_high_surrogate = 0;
    input_last_keydown_emoji = 0;
    input_allow_keyup_surrogates = enabled && win_accept_keyup_surrogates();
    input_utf8_next = input_utf8_len = 0;
}

/* Both the KeyDown and (mintty-only) KeyUp paths use the same UTF-16
 * surrogate-pair arithmetic. Call this only after checking both halves. */
static unsigned long win_surrogate_scalar(unsigned long high, unsigned long low)
{
    return 0x10000UL + ((high - 0xd800UL) << 10) + (low - 0xdc00UL);
}

/* Convert common cursor/editing keys into the same Rogue codes as the
 * PDCurses input handler. Return -1 for an irrelevant key event.
 *
 * A supplementary Unicode character may arrive as either KeyDown or,
 * in mintty/VT sessions, KeyUp UTF-16 halves. Keep two independent
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
        return -1; /* Never treat a normal key release as text. */
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
            /* PDCursesMod WinCon: Ctrl-J may be reported as CTL_ENTER. */
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

/* Convert UTF-8 to Unicode scalar values before passing text to PDCursesMod.
 * On Windows, wchar_t is UTF-16.  waddwstr() passes each surrogate half to
 * waddch() separately; PDCursesMod may then combine the halves into a single
 * cell without allocating the second display cell for a wide emoji.  The
 * following fullwidth character can be overwritten (e.g. "🍎だった。" loses
 * "だ").  Feeding the complete scalar via waddch() lets PDCursesMod
 * calculate its full display width and reserve both cells.
 *
 * All BMP characters follow the same waddch() path as waddwstr() used;
 * supplementary characters are now passed as one code point, not two halves.
 */
int rogue_win_addstr_utf8(const char *utf8)
{
    wchar_t *wide = rogue_win_to_wide(utf8);
    const wchar_t *p;
    int result = OK;

    if (!wide) return ERR;
    for (p = wide; *p; ++p) {
        unsigned int cp = (unsigned int)*p;
        if (cp >= 0xd800U && cp <= 0xdbffU &&
            (unsigned int)p[1] >= 0xdc00U &&
            (unsigned int)p[1] <= 0xdfffU) {
            cp = 0x10000U + ((cp - 0xd800U) << 10) +
                 ((unsigned int)p[1] - 0xdc00U);
            ++p;
        }
        if (waddch(stdscr, (chtype)cp) == ERR) {
            result = ERR;
            break;
        }
    }
    free(wide);
    return result;
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


/* Windows-only palette mapping from color.c.  This stays aligned
 * with Rogue 5.4.5 JEC, without changing Clone III RGB values. */
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
    /* Do not write raw ANSI escape sequences here: PDCursesMod handles
     * the console drawing for WinCon and VT backends. */
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
