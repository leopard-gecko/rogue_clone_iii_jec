/* Windows PDCurses console input, Unicode drawing and color mapping.
 * No Win32 handles or curses types cross the interface.
 */
#ifndef ROGUE_CLONE_WINDOWS_CONSOLE_H
#define ROGUE_CLONE_WINDOWS_CONSOLE_H
#ifdef _WIN32
int rogue_win_getchar(void);
void rogue_win_set_option_input(int enabled);
void rogue_win_set_wizard_ctrl_c(int enabled);
int rogue_win_addstr_utf8(const char *text);
void rogue_win_vt_restore_nonbmp_rows(int max_rows);
void rogue_win_init_palette(void);
short rogue_win_platform_color(short color);
#endif /* _WIN32 */
#endif /* ROGUE_CLONE_WINDOWS_CONSOLE_H */
