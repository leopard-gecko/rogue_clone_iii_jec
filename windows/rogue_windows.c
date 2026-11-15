/* Windows launch, Unicode path operations and console startup for
 * Rogue Clone III JEC.  Compiled only by Makefile.win.
 * This module keeps OS-specific file/launch APIs separate from the
 * original Rogue md_* platform interface and PDCurses console I/O.
 */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <commdlg.h>
#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif
#include <wchar.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "rogue.h"
#include "rogue_windows.h"

/* Like Rogue 3.6 JEC: request a 80x24 terminal only when the game was
 * started directly (e.g., double-clicked in Explorer).  Do NOT resize an
 * already-running cmd, PowerShell, mintty/MSYS2 or other shell terminal.
 * The dungeon's 80x24 geometry is fixed independently of this window size.
 *
 * Classic Console Host uses the Win32 console API; also issue a VT
 * CSI 8;24;80t request in ALL direct-launch cases and repeat it after
 * PDCurses initialization. Windows Terminal's proxy HWND visibility is
 * not a reliable indication of its ability to process VT resize requests.
 * This matches the proven Rogue 3.6 v138+ approach. A terminal may still
 * decline the request (e.g. multiple panes or unsupported versions).
 */
static int rogue_direct_launch_vt_retry = 0;

static int rogue_resize_shell_name(const wchar_t *name)
{
    static const wchar_t *names[] = {
        L"cmd.exe", L"powershell.exe", L"pwsh.exe",
        L"bash.exe", L"sh.exe", L"mintty.exe", L"wsl.exe",
        L"zsh.exe", L"fish.exe", L"nu.exe", L"fish-shell.exe"
    };
    size_t i;
    for (i = 0; i < sizeof(names)/sizeof(names[0]); ++i)
        if (lstrcmpiW(name, names[i]) == 0) return 1;
    return 0;
}

/* 1 = known direct-launch host, -1 = interactive shell, 0 = unknown.
 * Windows Terminal's default-terminal handoff can leave an exited or
 * otherwise unidentifiable parent, so unknown does not mean direct. */
static int rogue_resize_parent_kind(void)
{
    HANDLE snap;
    PROCESSENTRY32W pe;
    DWORD my_pid = GetCurrentProcessId(), parent_pid = 0;
    int kind = 0;
    snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    memset(&pe, 0, sizeof(pe));
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            if (pe.th32ProcessID == my_pid) {
                parent_pid = pe.th32ParentProcessID;
                break;
            }
        } while (Process32NextW(snap, &pe));
    }
    if (parent_pid && Process32FirstW(snap, &pe)) {
        do {
            const wchar_t *name;
            if (pe.th32ProcessID != parent_pid) continue;
            name = pe.szExeFile;
            if (rogue_resize_shell_name(name))
                kind = -1;
            else if (lstrcmpiW(name, L"explorer.exe") == 0 ||
                     lstrcmpiW(name, L"WindowsTerminal.exe") == 0 ||
                     lstrcmpiW(name, L"WindowsTerminalPreview.exe") == 0 ||
                     lstrcmpiW(name, L"OpenConsole.exe") == 0 ||
                     lstrcmpiW(name, L"conhost.exe") == 0 ||
                     lstrcmpiW(name, L"wt.exe") == 0)
                kind = 1;
            break;
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return kind;
}

/* Only shrink the classic Console Host when this EXE owns its console. */
static int rogue_resize_sole_client(void)
{
    DWORD pids[64];
    DWORD n = GetConsoleProcessList(pids, 64);
    int result = n == 1 && pids[0] == GetCurrentProcessId();
    return result;
}

/* Safety check for ConPTY: reject a console whose client list includes an
 * interactive shell, even when the immediate parent is a terminal host. */
static int rogue_resize_console_has_shell(void)
{
    DWORD pids[64], n, i;
    HANDLE snap;
    PROCESSENTRY32W pe;
    int found = 0;
    n = GetConsoleProcessList(pids, 64);
    if (n == 0 || n > 64) return 0;
    snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    memset(&pe, 0, sizeof(pe));
    pe.dwSize = sizeof(pe);
    if (Process32FirstW(snap, &pe)) {
        do {
            if (!rogue_resize_shell_name(pe.szExeFile)) continue;
            for (i = 0; i < n; ++i) {
                if (pids[i] == pe.th32ProcessID) {
                    found = 1;
                    break;
                }
            }
            if (found) break;
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
}

/* Set both viewport and scrollback buffer to 80x24. Windows requires
 * shrinking the viewport before shrinking the buffer, and growing the
 * buffer before enlarging the viewport. */
static void rogue_resize_console_host(HANDLE output)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD buffer;
    SMALL_RECT window;
    COORD maximum;
    if (!GetConsoleScreenBufferInfo(output, &info)) return;
    maximum = GetLargestConsoleWindowSize(output);
    if (maximum.X < 80 || maximum.Y < 24) return;

    window.Left = 0;
    window.Top = 0;
    window.Right = (SHORT)((info.srWindow.Right - info.srWindow.Left + 1 < 80)
        ? info.srWindow.Right - info.srWindow.Left : 79);
    window.Bottom = (SHORT)((info.srWindow.Bottom - info.srWindow.Top + 1 < 24)
        ? info.srWindow.Bottom - info.srWindow.Top : 23);
    if (!SetConsoleWindowInfo(output, TRUE, &window)) return;

    buffer.X = info.dwSize.X > 80 ? info.dwSize.X : 80;
    buffer.Y = info.dwSize.Y > 24 ? info.dwSize.Y : 24;
    if (!SetConsoleScreenBufferSize(output, buffer)) return;

    window.Right = 79;
    window.Bottom = 23;
    if (!SetConsoleWindowInfo(output, TRUE, &window)) return;
    buffer.X = 80;
    buffer.Y = 24;
    (void)SetConsoleScreenBufferSize(output, buffer);
}

/* Send the VT terminal-window resize request. Some hosts ignore it; the
 * request is retried after PDCurses initialization for Windows Terminal. */
static void rogue_resize_conpty(HANDLE output, DWORD mode)
{
    static const char seq[] = "\033[8;24;80t";
    DWORD written;
    if (!(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        if (!SetConsoleMode(output, mode | ENABLE_PROCESSED_OUTPUT |
                            ENABLE_VIRTUAL_TERMINAL_PROCESSING))
            return;
    }
    (void)WriteFile(output, seq, (DWORD)(sizeof(seq) - 1), &written, NULL);
}

void rogue_win_resize_direct_launch(void)
{
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    HWND window;
    int kind, sole;
    rogue_direct_launch_vt_retry = 0;
    if (output == INVALID_HANDLE_VALUE || output == NULL ||
        !GetConsoleMode(output, &mode)) return;

    kind = rogue_resize_parent_kind();
    if (kind < 0) return; /* An interactive shell launched the game. */
    sole = rogue_resize_sole_client();
    if (rogue_resize_console_has_shell() || (kind == 0 && !sole)) return;

    /* Windows Terminal/ConPTY can report a visible console window. Never
     * gate the VT request on HWND visibility: doing so broke double-click
     * resizing. Retain the classic Console Host API for a sole client. */
    window = GetConsoleWindow();
    if (window != NULL && IsWindowVisible(window) && sole)
        rogue_resize_console_host(output);

    rogue_direct_launch_vt_retry = 1;
    rogue_resize_conpty(output, mode);
}

void rogue_win_retry_direct_launch_resize(void)
{
    HANDLE output;
    DWORD mode;
    if (!rogue_direct_launch_vt_retry) return;
    output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output == INVALID_HANDLE_VALUE || output == NULL ||
        !GetConsoleMode(output, &mode)) return;
    rogue_resize_conpty(output, mode);
}

/* The same direct-launch detection also controls the save chooser.  A
 * shell-started game (including MSYS2 UCRT64/mintty) keeps its normal CLI. */
int rogue_win_is_direct_launch(void)
{
    return rogue_direct_launch_vt_retry;
}

/* Windows passes dragged file names in its Unicode command line, whereas
 * the narrow CRT argv can replace Japanese characters with question marks.
 * Convert every argument to UTF-8 before init()/do_args() sees it.  The
 * allocations deliberately live for the whole game: rest_file points into
 * this array until restore() has completed. */
void rogue_win_utf8_argv(int *argc, char ***argv)
{
    int i, count, size;
    LPWSTR *wide_argv;
    char **converted;

    wide_argv = CommandLineToArgvW(GetCommandLineW(), &count);
    if (wide_argv == NULL || count <= 0) {
        if (wide_argv != NULL) LocalFree(wide_argv);
        return;
    }
    converted = (char **)calloc((size_t)count + 1, sizeof(char *));
    if (converted == NULL) {
        LocalFree(wide_argv);
        return;
    }
    for (i = 0; i < count; i++) {
        size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                   wide_argv[i], -1, NULL, 0, NULL, NULL);
        if (size <= 0) break;
        converted[i] = (char *)malloc((size_t)size);
        if (converted[i] == NULL ||
            !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                 wide_argv[i], -1, converted[i], size,
                                 NULL, NULL)) break;
    }
    LocalFree(wide_argv);
    if (i != count) {
        int j;
        for (j = 0; j < count; j++) free(converted[j]);
        free(converted);
        return;
    }
    *argc = count;
    *argv = converted;
}

/* The game saves relative and absolute UTF-8 paths.  Windows CRT fopen()
 * otherwise interprets paths in the current ANSI code page. */
wchar_t *rogue_win_to_wide(const char *utf8)
{
    int len;
    wchar_t *dest;
    if (!utf8) return NULL;
    len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, NULL, 0);
    if (!len) return NULL;
    dest = (wchar_t *) malloc((size_t)len * sizeof(wchar_t));
    if (!dest) return NULL;
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, dest, len)) {
        free(dest);
        return NULL;
    }
    return dest;
}

/* While curses is suspended, printf() may use a legacy output code
 * page.  Write the Unicode message directly when a console is attached;
 * pipes and redirected files should instead receive the original UTF-8. */
void rogue_win_write_utf8(FILE *stream, const char *utf8)
{
    HANDLE output;
    DWORD mode;
    DWORD written = 0;
    wchar_t *wide;
    size_t length;

    if (stream == NULL || utf8 == NULL) return;
    (void) fflush(stream);
    output = GetStdHandle(stream == stderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    if (output != NULL && output != INVALID_HANDLE_VALUE &&
        GetConsoleMode(output, &mode)) {
        wide = rogue_win_to_wide(utf8);
        if (wide != NULL) {
            length = wcslen(wide);
            if (length <= 0xffffffffUL &&
                WriteConsoleW(output, wide, (DWORD) length, &written, NULL) &&
                written == (DWORD) length) {
                free(wide);
                return;
            }
            free(wide);
            /* Do not print a second copy if a partial write succeeded. */
            if (written != 0) return;
        }
    }
    (void) fputs(utf8, stream);
    (void) fflush(stream);
}

/* Only a regular existing file triggers the direct-launch menu. */
int rogue_win_save_exists(const char *utf8_path)
{
    DWORD attrs;
    wchar_t *path = rogue_win_to_wide(utf8_path);
    if (!path) return 0;
    attrs = GetFileAttributesW(path);
    free(path);
    return attrs != INVALID_FILE_ATTRIBUTES &&
           (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

/* The F command uses Windows' Unicode file picker.  OFN_NOCHANGEDIR is
 * essential: changing the game working directory would relocate default
 * save and score files after the dialog closes.  NULL means Cancel. */
static char *rogue_win_choose_save(void)
{
    OPENFILENAMEW ofn;
    wchar_t filename[32768];
    static const wchar_t filter[] =
        L"Rogue save files (*.save)\0*.save\0All files (*.*)\0*.*\0\0";
    static const wchar_t title[] = L"Select a saved game / セーブファイルを選択";
    char *utf8;
    int size;

    memset(&ofn, 0, sizeof(ofn));
    filename[0] = L'\0';
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = (DWORD)(sizeof(filename) / sizeof(filename[0]));
    ofn.lpstrTitle = title;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
    if (!GetOpenFileNameW(&ofn)) return NULL;

    size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                               filename, -1, NULL, 0, NULL, NULL);
    if (size <= 0) return NULL;
    utf8 = (char *)malloc((size_t)size);
    if (utf8 == NULL) return NULL;
    if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                             filename, -1, utf8, size, NULL, NULL)) {
        free(utf8);
        return NULL;
    }
    return utf8;
}

FILE *rogue_win_fopen(const char *path, const char *mode)
{
    FILE *fp;
    wchar_t *wpath = rogue_win_to_wide(path);
    wchar_t *wmode = rogue_win_to_wide(mode);
    if (!wpath || !wmode) {
        free(wpath); free(wmode);
        errno = EINVAL;
        return NULL;
    }
    fp = _wfopen(wpath, wmode);
    free(wpath); free(wmode);
    return fp;
}


/* cmd.exe can change the code pages while the ! shell command is active.
 * Restore UTF-8 after PDCurses has restored its own console mode.  Do not
 * change output handles or VT processing flags. */
void rogue_win_restore_shell_encoding(void)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (out && out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode)) {
        (void)SetConsoleOutputCP(CP_UTF8);
        (void)SetConsoleCP(CP_UTF8);
    }
}

/* Direct-launch save selection, previously kept in init.c.
 * Use the same UTF-8 curses drawing macros as the original call site. */
extern char *rest_file;
extern char *save_file;

void rogue_win_startup_save_menu(void)
{
    int key;
    char *chosen;
    for (;;) {
        rogue_clear();
        (void) attrset(A_NORMAL);
        (void) mvaddstr(2, 2, "Rogue Clone III");
        (void) mvaddstr(4, 2, EN("A saved adventure was found.",
                                "セーブした冒険が見つかりました。"));
        (void) mvaddstr(6, 2, EN("[R/Enter] Resume the saved adventure",
                                "[R/Enter] セーブした冒険を再開する"));
        (void) mvaddstr(7, 2, EN("[N] Start a new adventure",
                                "[N] 新しい冒険を始める"));
        (void) mvaddstr(8, 2, EN("[F] Choose another save file",
                                "[F] 別のセーブファイルを選ぶ"));
        (void) mvaddstr(9, 2, EN("[Q] Quit", "[Q] 終了する"));
        (void) mvaddstr(11, 2, EN("Press a key:", "キーを押してください:"));
        refresh();
        key = getch();
        switch (key) {
        case 'r': case 'R': case '\n': case '\r':
#ifdef KEY_ENTER
        case KEY_ENTER:
#endif
            rest_file = save_file;
            rogue_clear();
            refresh();
            return;
        case 'n': case 'N':
            /* Starting a new game must not delete the existing save. */
            rogue_clear();
            refresh();
            return;
        case 'f': case 'F':
            chosen = rogue_win_choose_save();
            if (chosen != NULL) {
                rest_file = chosen;
                rogue_clear();
                refresh();
                return;
            }
            /* Cancel returns to the chooser without changing CWD. */
            break;
        case 'q': case 'Q':
            clean_up("");
            return;
        default:
            break;
        }
    }
}

#endif /* _WIN32 */
