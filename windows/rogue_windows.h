/* Windows launch and UTF-8 file services for Rogue Clone III JEC.
 * Win32 types are deliberately hidden from the portable code.
 */
#ifndef ROGUE_CLONE_WINDOWS_H
#define ROGUE_CLONE_WINDOWS_H
#ifdef _WIN32
#include <stdio.h>
#include <wchar.h>

void rogue_win_setup_console_encoding(void);
void rogue_win_startup_save_menu(void);
void rogue_win_resize_direct_launch(void);
void rogue_win_retry_direct_launch_resize(void);
int rogue_win_is_direct_launch(void);
void rogue_win_utf8_argv(int *argc, char ***argv);
int rogue_win_save_exists(const char *utf8_path);
FILE *rogue_win_fopen(const char *path, const char *mode);
/* Shared UTF-8 -> UTF-16 conversion; caller must free the result. */
wchar_t *rogue_win_to_wide(const char *utf8);
/* Print UTF-8 text as UTF-16 on a Windows console; preserve UTF-8 when
 * output is redirected to a pipe or file. */
void rogue_win_write_utf8(FILE *stream, const char *utf8);
#endif /* _WIN32 */
#endif /* ROGUE_CLONE_WINDOWS_H */
