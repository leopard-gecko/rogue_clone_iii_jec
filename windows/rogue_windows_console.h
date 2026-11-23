/* Windows PDCurses console input, Unicode drawing and color mapping.
 * No Win32 handles cross the interface; WINDOW is defined by curses.h.
 */
#ifndef ROGUE_CLONE_WINDOWS_CONSOLE_H
#define ROGUE_CLONE_WINDOWS_CONSOLE_H
#ifdef _WIN32
int rogue_win_getchar(void);
void rogue_win_set_option_input(int enabled);
void rogue_win_set_wizard_ctrl_c(int enabled);
int rogue_win_addstr_utf8(const char *text);
void rogue_win_vt_restore_nonbmp_rows(int max_rows);
/* Windows VT cursor shape and flicker control. */
int rogue_win_refresh(WINDOW *win);
void rogue_win_cursor_set_visible(int visible);
void rogue_win_cursor_invalidate(void);
void rogue_win_cursor_restore_default(void);
void rogue_win_init_palette(void);
short rogue_win_platform_color(short color);
#endif /* _WIN32 */
#endif /* ROGUE_CLONE_WINDOWS_CONSOLE_H */
