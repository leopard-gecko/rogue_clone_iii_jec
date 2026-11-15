/* Native Windows md_* platform layer for Rogue Clone III JEC.
 * This retains the original game's machine-dependent interface while
 * launch/file selection and PDCurses console I/O live in dedicated modules.
 */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif
#include <lmcons.h>
#include <wchar.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#define ROGUE_COLOR_INTERNAL
#include "rogue.h"
#include "rogue_windows.h"

void md_save_tty(void) { }
void md_restore_tty(void) { }
void md_control_keybord(boolean mode) { (void)mode; }
void md_slurp(void) { (void) flushinp(); }

void md_heed_signals(void)
{
    (void) signal(SIGINT, onintr);
#ifdef SIGBREAK
    (void) signal(SIGBREAK, error_save);
#endif
}
void md_ignore_signals(void)
{
    (void) signal(SIGINT, SIG_IGN);
#ifdef SIGBREAK
    (void) signal(SIGBREAK, SIG_IGN);
#endif
}

static HANDLE open_read_utf8(const char *filename)
{
    HANDLE file = INVALID_HANDLE_VALUE;
    wchar_t *wide = rogue_win_to_wide(filename);
    if (wide) {
        file = CreateFileW(wide, FILE_READ_ATTRIBUTES,
                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        free(wide);
    }
    return file;
}
int md_get_file_id(char *filename)
{
    HANDLE file = open_read_utf8(filename);
    BY_HANDLE_FILE_INFORMATION info;
    unsigned long id;
    if (file == INVALID_HANDLE_VALUE) return -1;
    if (!GetFileInformationByHandle(file, &info)) {
        CloseHandle(file);
        return -1;
    }
    CloseHandle(file);
    id = info.nFileIndexLow ^ info.nFileIndexHigh ^ info.dwVolumeSerialNumber;
    /* -1 is the not-found sentinel used by the original game. */
    if (id == (unsigned long)-1) id ^= 0x40000000UL;
    return (int)id;
}
int md_link_count(char *filename)
{
    HANDLE file = open_read_utf8(filename);
    BY_HANDLE_FILE_INFORMATION info;
    int count;
    if (file == INVALID_HANDLE_VALUE) return 1;
    if (!GetFileInformationByHandle(file, &info)) {
        CloseHandle(file);
        return 1;
    }
    count = (int)info.nNumberOfLinks;
    CloseHandle(file);
    return count > 0 ? count : 1;
}
void md_gct(struct rogue_time *out)
{
    SYSTEMTIME now;
    GetLocalTime(&now);
    out->year = now.wYear;
    out->month = now.wMonth;
    out->day = now.wDay;
    out->hour = now.wHour;
    out->minute = now.wMinute;
    out->second = now.wSecond;
}
void md_gfmt(char *filename, struct rogue_time *out)
{
    HANDLE file = open_read_utf8(filename);
    FILETIME modified, local;
    SYSTEMTIME dt;
    if (file == INVALID_HANDLE_VALUE ||
        !GetFileTime(file, NULL, NULL, &modified) ||
        !FileTimeToLocalFileTime(&modified, &local) ||
        !FileTimeToSystemTime(&local, &dt)) {
        if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
        memset(out, 0, sizeof(*out));
        return;
    }
    CloseHandle(file);
    out->year = dt.wYear;
    out->month = dt.wMonth;
    out->day = dt.wDay;
    out->hour = dt.wHour;
    out->minute = dt.wMinute;
    out->second = dt.wSecond;
}
boolean md_df(char *filename)
{
    wchar_t *wide = rogue_win_to_wide(filename);
    int ok;
    if (!wide) return 0;
    ok = _wremove(wide) == 0;
    free(wide);
    return (boolean)ok;
}

char *md_gln(void)
{
    static char username[256];
    wchar_t wname[UNLEN+1];
    DWORD chars = (DWORD)(sizeof(wname)/sizeof(wname[0]));
    int n;
    if (!GetUserNameW(wname, &chars)) return NULL;
    n = WideCharToMultiByte(CP_UTF8, 0, wname, -1, username,
                            (int)sizeof(username), NULL, NULL);
    return n ? username : NULL;
}
void md_sleep(int n) { if (n > 0) Sleep((DWORD)n * 1000); }
void rogue_delay(void) { Sleep(24); }
void rogue_show_cursor(boolean show) { (void)curs_set(show ? 1 : 0); }

/* The Windows C runtime's narrow getenv() uses the active ANSI code
 * page and can corrupt Unicode ROGUEOPTS, including Japanese and emoji.
 * Read the Windows Unicode environment and convert to UTF-8 instead. */
static char *wide_env_utf8(const wchar_t *key)
{
    static char value[8192];
    DWORD needed = GetEnvironmentVariableW(key, NULL, 0);
    wchar_t *wide;
    int count;
    if (!needed) return NULL;
    if (needed > 4096) return NULL;
    wide = (wchar_t *) malloc((size_t)needed * sizeof(wchar_t));
    if (!wide) return NULL;
    if (!GetEnvironmentVariableW(key, wide, needed)) {
        free(wide);
        return NULL;
    }
    count = WideCharToMultiByte(CP_UTF8, 0, wide, -1,
                                value, (int)sizeof(value), NULL, NULL);
    free(wide);
    return count > 0 ? value : NULL;
}

char *md_getenv(char *name)
{
    wchar_t *wide;
    char *result;
    if (!strcmp(name, "HOME")) {
        result = wide_env_utf8(L"USERPROFILE");
        if (result) return result;
    }
    wide = rogue_win_to_wide(name);
    if (!wide) return NULL;
    result = wide_env_utf8(wide);
    free(wide);
    return result;
}

char *md_malloc(int n) { return (char *)malloc((size_t)n); }
int md_gseed(void) { return (int)(GetTickCount() ^ GetCurrentProcessId()); }
void md_exit(int status) { exit(status); }

/* Lock the first byte of the score stream's own file handle.
 * Read/write and LockFileEx must use the same HANDLE. */
static HANDLE score_lock_handle = INVALID_HANDLE_VALUE;
static OVERLAPPED score_lock_overlapped;

int md_lock(FILE *fp, boolean acquire)
{
    HANDLE handle;
    intptr_t os_handle;
    DWORD error;
    int attempt;

    if (!acquire) {
        if (score_lock_handle != INVALID_HANDLE_VALUE) {
            (void) UnlockFileEx(score_lock_handle, 0, 1, 0,
                                &score_lock_overlapped);
            score_lock_handle = INVALID_HANDLE_VALUE;
        }
        return 1;
    }
    if (score_lock_handle != INVALID_HANDLE_VALUE) {
        errno = EBUSY;
        return 0;
    }
    if (fp == NULL || _fileno(fp) < 0 ||
        (os_handle = _get_osfhandle(_fileno(fp))) == -1) {
        errno = EBADF;
        return 0;
    }
    handle = (HANDLE) os_handle;
    memset(&score_lock_overlapped, 0, sizeof score_lock_overlapped);
    for (attempt = 0; attempt <= 50; ++attempt) {
        if (LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK |
                       LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0,
                       &score_lock_overlapped)) {
            score_lock_handle = handle;
            return 1;
        }
        error = GetLastError();
        if (error != ERROR_LOCK_VIOLATION && error != ERROR_IO_PENDING) {
            errno = EIO;
            return 0;
        }
        if (attempt < 50)
            Sleep(100);
    }
    errno = EWOULDBLOCK;
    return 0;
}

/* Open/create atomically, without truncating scores created by another process. */
FILE *md_score_open(const char *path)
{
    wchar_t *wide = rogue_win_to_wide(path);
    HANDLE handle;
    int fd;
    FILE *fp;
    if (wide == NULL)
        return NULL;
    handle = CreateFileW(wide, GENERIC_READ | GENERIC_WRITE,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    free(wide);
    if (handle == INVALID_HANDLE_VALUE)
        return NULL;
    fd = _open_osfhandle((intptr_t)handle, _O_RDWR | _O_BINARY);
    if (fd < 0) {
        CloseHandle(handle);
        return NULL;
    }
    fp = _fdopen(fd, "r+b");
    if (fp == NULL)
        _close(fd);
    return fp;
}

/* Keep Rogue attached to its console while cmd.exe is running.  A
 * temporary console-control handler prevents Ctrl-C meant for cmd.exe
 * from interrupting the suspended game.  Unlike SIG_IGN, this handler
 * is not inherited by the child process. */
static BOOL WINAPI rogue_shell_control(DWORD control)
{
    return (control == CTRL_C_EVENT || control == CTRL_BREAK_EVENT) ? TRUE : FALSE;
}

static void rogue_shell_error(DWORD error)
{
    char line[256];
    int ch;
    (void) snprintf(line, sizeof(line),
                    EN("Cannot start the Windows command prompt (error %lu).\n",
                       "Windowsのコマンドプロンプトを起動できません (エラー %lu)。\n"),
                    (unsigned long)error);
    rogue_win_write_utf8(stderr, line);
    rogue_win_write_utf8(stderr,
                         EN("Press Enter to return to Rogue...",
                            "EnterキーでRogueに戻ります..."));
    /* curses has already released the terminal, so read from the CRT,
     * not from the game's Windows/PDCurses keyboard hook. */
    do {
        ch = fgetc(stdin);
    } while (ch != '\n' && ch != EOF);
}

void md_shell(char *shell)
{
    wchar_t *wide_shell = NULL;
    wchar_t fallback[MAX_PATH];
    wchar_t command_line[] = L"cmd.exe";
    const wchar_t *program;
    DWORD length;
    DWORD failure = 0;
    STARTUPINFOW startup;
    PROCESS_INFORMATION process;
    BOOL handler_installed;

    /* The game's md_getenv() returns UTF-8, not the active ANSI code page. */
    if (shell != NULL && *shell != '\0') {
        wide_shell = rogue_win_to_wide(shell);
    }
    if (wide_shell != NULL) {
        program = wide_shell;
    } else {
        /* Use the actual System32 cmd.exe if COMSPEC was not supplied. */
        length = GetSystemDirectoryW(fallback, MAX_PATH);
        if (!length || length >= MAX_PATH - 9) {
            rogue_shell_error(GetLastError());
            return;
        }
        wcscat(fallback, L"\\cmd.exe");
        program = fallback;
    }

    handler_installed = SetConsoleCtrlHandler(rogue_shell_control, TRUE);
    if (!handler_installed) {
        failure = GetLastError();
    } else {
        memset(&startup, 0, sizeof(startup));
        memset(&process, 0, sizeof(process));
        startup.cb = sizeof(startup);
        /* No CREATE_NEW_CONSOLE, CREATE_NO_WINDOW, or detached flag: the
         * child uses the same console/ConPTY as Rogue. */
        if (!CreateProcessW(program, command_line, NULL, NULL, TRUE, 0,
                            NULL, NULL, &startup, &process)) {
            failure = GetLastError();
        } else {
            (void)WaitForSingleObject(process.hProcess, INFINITE);
            CloseHandle(process.hThread);
            CloseHandle(process.hProcess);
        }
        (void)SetConsoleCtrlHandler(rogue_shell_control, FALSE);
    }
    free(wide_shell);
    if (failure != 0) {
        rogue_shell_error(failure);
    }
}
#endif /* _WIN32 */
