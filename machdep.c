/*
 * machdep.c
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  No portion of this notice shall be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 */

#ifndef lint
static char sccsid[] __attribute__((unused)) = "@(#)machdep.c	5.2 (Berkeley) 11/25/87";
#endif /* not lint */

/* Included in this file are all system dependent routines.  Extensive use
 * of #ifdef's will be used to compile the appropriate code on each system:
 *
 *    UNIX:        all UNIX systems.
 *    UNIX_BSD4_2: UNIX BSD 4.2 and later, UTEK, (4.1 BSD too?)
 *    UNIX_SYSV:   UNIX system V
 *    UNIX_V7:     UNIX version 7
 *
 * All UNIX code should be included between the single "#ifdef UNIX" at the
 * top of this file, and the "#endif" at the bottom.
 * 
 * To change a routine to include a new UNIX system, simply #ifdef the
 * existing routine, as in the following example:
 *
 *   To make a routine compatible with UNIX system 5, change the first
 *   function to the second:
 *
 *      md_function()
 *      {
 *         code;
 *      }
 *
 *      md_function()
 *      {
 *      #ifdef UNIX_SYSV
 *         sys5code;
 *      #else
 *         code;
 *      #endif
 *      }
 *
 * Appropriate variations of this are of course acceptible.
 * The use of "#elseif" is discouraged because of non-portability.
 * If the correct #define doesn't exist, "UNIX_SYSV" in this case, make it up
 * and insert it in the list at the top of the file.  Alter the CFLAGS
 * in you Makefile appropriately.
 *
 */

#ifdef UNIX

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <pwd.h>

#ifdef MODERN_UNIX
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#endif

#ifdef UNIX_BSD4_2
#include <sys/time.h>
#include <sgtty.h>
#endif

#ifdef UNIX_SYSV
#include <time.h>
#include <termio.h>
#endif

#include <signal.h>
#include "rogue.h"

#ifdef MODERN_UNIX
static boolean md_tty_saved = 0;
static struct termios md_tty_original;
#endif

void md_save_tty(void)
{
#ifdef MODERN_UNIX
	if (!md_tty_saved) {
		if (tcgetattr(0, &md_tty_original) == 0) {
			md_tty_saved = 1;
		}
	}
#endif
}

void md_restore_tty(void)
{
#ifdef MODERN_UNIX
	if (md_tty_saved) {
		(void) tcsetattr(0, TCSANOW, &md_tty_original);
	}
#endif
}


/* md_slurp:
 *
 * This routine throws away all keyboard input that has not
 * yet been read.  It is used to get rid of input that the user may have
 * typed-ahead.
 *
 * This function is not necessary, so it may be stubbed.  The might cause
 * message-line output to flash by because the game has continued to read
 * input without waiting for the user to read the message.  Not such a
 * big deal.
 */

void md_slurp(void)
{
	long ln = 0;

#ifdef MODERN_UNIX
	if (ioctl(0, FIONREAD, &ln) == -1) {
		ln = 0;
	}
#else
#ifdef UNIX_BSD4_2
	ioctl(0, FIONREAD, &ln);
#endif
#ifdef UNIX_SYSV
	ioctl(0, TCFLSH, &ln);
	ln = 0;
#endif

	ln += stdin->_cnt;
#endif

	for (; ln > 0; ln--) {
		(void) getchar();
	}
}

/* md_control_keyboard():
 *
 * This routine is much like md_control_keybord().  It sets up the
 * keyboard for appropriate input.  Specifically, it prevents the tty driver
 * from stealing characters.  For example, ^Y is needed as a command
 * character, but the tty driver intercepts it for another purpose.  Any
 * such behavior should be stopped.  This routine could be avoided if
 * we used RAW mode instead of CBREAK.  But RAW mode does not allow the
 * generation of keyboard signals, which the program uses.
 *
 * The parameter 'mode' when true, indicates that the keyboard should
 * be set up to play rogue.  When false, it should be restored if
 * necessary.
 *
 * This routine is not strictly necessary and may be stubbed.  This may
 * cause certain command characters to be unavailable.
 */

void md_control_keybord(boolean mode)
{
	static boolean called_before = 0;
#ifdef MODERN_UNIX
	static struct termios tty_orig;
	struct termios tty_temp;
#else
#ifdef UNIX_BSD4_2
	static struct ltchars ltc_orig;
	static struct tchars tc_orig;
	struct ltchars ltc_temp;
	struct tchars tc_temp;
#endif
#ifdef UNIX_SYSV
	static struct termio _oldtty;
	struct termio _tty;
#endif
#endif

	if (!called_before) {
		called_before = 1;
#ifdef MODERN_UNIX
		md_save_tty();
		if (md_tty_saved) {
			tty_orig = md_tty_original;
		} else {
			tcgetattr(0, &tty_orig);
		}
#else
#ifdef UNIX_BSD4_2
		ioctl(0, TIOCGETC, &tc_orig);
		ioctl(0, TIOCGLTC, &ltc_orig);
#endif
#ifdef UNIX_SYSV
		ioctl(0, TCGETA, &_oldtty);
#endif
#endif
	}
#ifdef MODERN_UNIX
	if (!mode) {
		if (tcgetattr(0, &tty_temp) != 0) {
			tty_temp = tty_orig;
		}
	} else {
		tty_temp = tty_orig;
	}
#else
#ifdef UNIX_BSD4_2
	ltc_temp = ltc_orig;
	tc_temp = tc_orig;
#endif
#ifdef UNIX_SYSV
	_tty = _oldtty;
#endif
#endif

	if (!mode) {
#ifdef MODERN_UNIX
#ifdef VSUSP
		tty_temp.c_cc[VSUSP] = _POSIX_VDISABLE;
#endif
#ifdef VDSUSP
		tty_temp.c_cc[VDSUSP] = _POSIX_VDISABLE;
#endif
#ifdef VREPRINT
		tty_temp.c_cc[VREPRINT] = _POSIX_VDISABLE;
#endif
#ifdef VDISCARD
		tty_temp.c_cc[VDISCARD] = _POSIX_VDISABLE;
#endif
#ifdef VWERASE
		tty_temp.c_cc[VWERASE] = _POSIX_VDISABLE;
#endif
#ifdef VLNEXT
		tty_temp.c_cc[VLNEXT] = _POSIX_VDISABLE;
#endif
#ifdef VSTART
		tty_temp.c_cc[VSTART] = _POSIX_VDISABLE;
#endif
#ifdef VSTOP
		tty_temp.c_cc[VSTOP] = _POSIX_VDISABLE;
#endif
#ifdef VINTR
		tty_temp.c_cc[VINTR] = _POSIX_VDISABLE;
#endif
#ifdef VSTATUS
		tty_temp.c_cc[VSTATUS] = _POSIX_VDISABLE;
#endif
#ifdef IXON
		tty_temp.c_iflag &= ~IXON;
#endif
#ifdef IXOFF
		tty_temp.c_iflag &= ~IXOFF;
#endif
#ifdef IXANY
		tty_temp.c_iflag &= ~IXANY;
#endif
#else
#ifdef UNIX_BSD4_2
		ltc_temp.t_suspc = ltc_temp.t_dsuspc = -1;
		ltc_temp.t_rprntc = ltc_temp.t_flushc = -1;
		ltc_temp.t_werasc = ltc_temp.t_lnextc = -1;
		tc_temp.t_startc = tc_temp.t_stopc = -1;
#endif
#ifdef UNIX_SYSV
#ifdef VSWTCH
		_tty.c_cc[VSWTCH] = CNSWTCH;
#endif
#endif
#endif
	}
#ifdef MODERN_UNIX
	tcsetattr(0, TCSANOW, &tty_temp);
#else
#ifdef UNIX_BSD4_2
	ioctl(0, TIOCSETC, &tc_temp);
	ioctl(0, TIOCSLTC, &ltc_temp);
#endif
#ifdef UNIX_SYSV
	ioctl(0, TCSETA, &_tty);
#endif
#endif
}

/* md_heed_signals():
 *
 * This routine tells the program to call particular routines when
 * certain interrupts/events occur:
 *
 *      SIGINT: call onintr() to interrupt fight with monster or long rest.
 *      SIGQUIT: call byebye() to check for game termination.
 *      SIGHUP: call error_save() to save game when terminal hangs up.
 *
 *		On VMS, SIGINT and SIGQUIT correspond to ^C and ^Y.
 *
 * This routine is not strictly necessary and can be stubbed.  This will
 * mean that the game cannot be interrupted properly with keyboard
 * input, this is not usually critical.
 */

void md_heed_signals(void)
{
	signal(SIGINT, onintr);
	signal(SIGQUIT, byebye);
	signal(SIGHUP, error_save);
}

/* md_ignore_signals():
 *
 * This routine tells the program to completely ignore the events mentioned
 * in md_heed_signals() above.  The event handlers will later be turned on
 * by a future call to md_heed_signals(), so md_heed_signals() and
 * md_ignore_signals() need to work together.
 *
 * This function should be implemented or the user risks interrupting
 * critical sections of code, which could cause score file, or saved-game
 * file, corruption.
 */

void md_ignore_signals(void)
{
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
}

/* md_get_file_id():
 *
 * This function returns an integer that uniquely identifies the specified
 * file.  It need not check for the file's existence.  In UNIX, the inode
 * number is used.
 *
 * This function is used to identify saved-game files.
 */

int md_get_file_id(char *fname)
{
	struct stat sbuf;

	if (stat(fname, &sbuf)) {
		return(-1);
	}
	return((int) sbuf.st_ino);
}

/* md_link_count():
 *
 * This routine returns the number of hard links to the specified file.
 *
 * This function is not strictly necessary.  On systems without hard links
 * this routine can be stubbed by just returning 1.
 */

int md_link_count(char *fname)
{
	struct stat sbuf;

	stat(fname, &sbuf);
	return((int) sbuf.st_nlink);
}

/* md_gct(): (Get Current Time)
 *
 * This function returns the current year, month(1-12), day(1-31), hour(0-23),
 * minute(0-59), and second(0-59).  This is used for identifying the time
 * at which a game is saved.
 *
 * This function is not strictly necessary.  It can be stubbed by returning
 * zeros instead of the correct year, month, etc.  If your operating
 * system doesn't provide all of the time units requested here, then you
 * can provide only those that it does, and return zeros for the others.
 * If you cannot provide good time values, then users may be able to copy
 * saved-game files and play them.  
 */

void md_gct(struct rogue_time *rt_buf)
{
	struct tm *t;
	long seconds;

	time(&seconds);
	t = localtime(&seconds);

	rt_buf->year = t->tm_year + 1900;
	rt_buf->month = t->tm_mon + 1;
	rt_buf->day = t->tm_mday;
	rt_buf->hour = t->tm_hour;
	rt_buf->minute = t->tm_min;
	rt_buf->second = t->tm_sec;
}

/* md_gfmt: (Get File Modification Time)
 *
 * This routine returns a file's date of last modification in the same format
 * as md_gct() above.
 *
 * This function is not strictly necessary.  It is used to see if saved-game
 * files have been modified since they were saved.  If you have stubbed the
 * routine md_gct() above by returning constant values, then you may do
 * exactly the same here.
 * Or if md_gct() is implemented correctly, but your system does not provide
 * file modification dates, you may return some date far in the past so
 * that the program will never know that a saved-game file being modified.  
 * You may also do this if you wish to be able to restore games from
 * saved-games that have been modified.
 */

void md_gfmt(char *fname, struct rogue_time *rt_buf)
{
	struct stat sbuf;
	long seconds;
	struct tm *t;

	stat(fname, &sbuf);
	seconds = (long) sbuf.st_mtime;
	t = localtime(&seconds);

	rt_buf->year = t->tm_year + 1900;
	rt_buf->month = t->tm_mon + 1;
	rt_buf->day = t->tm_mday;
	rt_buf->hour = t->tm_hour;
	rt_buf->minute = t->tm_min;
	rt_buf->second = t->tm_sec;
}

/* md_df: (Delete File)
 *
 * This function deletes the specified file, and returns true (1) if the
 * operation was successful.  This is used to delete saved-game files
 * after restoring games from them.
 *
 * Again, this function is not strictly necessary, and can be stubbed
 * by simply returning 1.  In this case, saved-game files will not be
 * deleted and can be replayed.
 */

boolean md_df(char *fname)
{
	if (unlink(fname)) {
		return(0);
	}
	return(1);
}

/* md_gln: (Get login name)
 *
 * This routine returns the login name of the user.  This string is
 * used mainly for identifying users in score files.
 *
 * A dummy string may be returned if you are unable to implement this
 * function, but then the score file would only have one name in it.
 */

char * md_gln(void)
{
	struct passwd *p;

	if (!(p = getpwuid(getuid())))
		return((char *)NULL);
	return(p->pw_name);
}

/* md_sleep:
 *
 * This routine causes the game to pause for the specified number of
 * seconds.
 *
 * This routine is not particularly necessary at all.  It is used for
 * delaying execution, which is useful to this program at some times.
 */

void md_sleep(int nsecs)
{
	(void) sleep(nsecs);
}

void rogue_delay(void)
{
#ifdef MODERN_UNIX
	(void) usleep(24000);
#else
	md_sleep(1);
#endif
}

void rogue_show_cursor(boolean show)
{
	(void) curs_set(show ? 1 : 0);
#ifdef UNIX
	(void) write(1, show ? "\033[?25h" : "\033[?25l", 6);
#else
	(void) show;
#endif
}

/* md_getenv()
 *
 * This routine gets certain values from the user's environment.  These
 * values are strings, and each string is identified by a name.  The names
 * of the values needed, and their use, is as follows:
 *
 *   TERM
 *     The name of the user's terminal.
 *   ROGUEOPTS
 *     A string containing the various game options.  This need not be
 *     defined.
 *   HOME
 *     The user's home directory.  This is only used when the user specifies
 *     '~' as the first character of a saved-game file.  This string need
 *     not be defined.
 *   SHELL
 *     The user's favorite shell.  If not found, "/bin/sh" is assumed.
 *
 * Returning zero indicates that there is no defined value for the given
 * string.
 */

char * md_getenv(char *name)
{
	char *value;

	value = getenv(name);

	return(value);
}

/* md_malloc()
 *
 * This routine allocates, and returns a pointer to, the specified number
 * of bytes.  This routines absolutely MUST be implemented for your
 * particular system or the program will not run at all.  Return zero
 * when no more memory can be allocated.
 */

char * md_malloc(int n)
{
	char *t;

	t = (char *) malloc(n);
	return(t);
}

/* md_gseed() (Get Seed)
 *
 * This function returns a seed for the random number generator (RNG).  This
 * seed causes the RNG to begin generating numbers at some point in it's
 * sequence.  Without a random seed, the RNG will generate the same set
 * of numbers, and every game will start out exactly the same way.  A good
 * number to use is the process id, given by getpid() on most UNIX systems.
 *
 * You need to find some single random integer, such as:
 *   process id.
 *   current time (minutes + seconds) returned from md_gct(), if implemented.
 *   
 * It will not help to return "get_rand()" or "rand()" or the return value of
 * any pseudo-RNG.  If you don't have a random number, you can just return 1,
 * but this means your games will ALWAYS start the same way, and will play
 * exactly the same way given the same input.
 */

int md_gseed(void)
{
	return(getpid());
}

/* md_exit():
 *
 * This function causes the program to discontinue execution and exit.
 * This function must be implemented or the program will continue to
 * hang when it should quit.
 */

void md_exit(int status)
{
	exit(status);
}

/* md_lock():
 *
 * This function is intended to give the user exclusive access to the
 * score file.  It does so by "creat"ing a lock file, which can only
 * be created if it does not already exist.  The file is deleted when
 * score file processing is finished.  The lock file should be located
 * in the same directory as the score file.  These full path names should
 * be defined for any particular site in rogue.h.  The constants SCORE_FILE
 * and LOCK_FILE define these file names.
 *
 * When the parameter 'l' is non-zero (true), a lock is requested.  Otherwise
 * the lock is released by removing the lock file.
 */

void md_lock(boolean l)
{
	short tries;
	char *lock_file = LOCK_FILE;

	if (l) {
		for (tries = 0; tries < 5; tries++) {
			if (md_get_file_id(lock_file) == -1) {
				if (creat(lock_file, 0444) != -1) {
					break;
				} else {
					message(EN("cannot lock score file", "得点ファイルをロックできません。"), 0);
				}
			} else {
				message(EN("waiting to lock score file", "得点ファイルのロックを待っています。"), 0);
			}
			sleep(2);
		}
	} else {
		(void) unlink(lock_file);
	}
}

/* md_shell():
 *
 * This function spawns a shell for the user to use.  When this shell is
 * terminated, the game continues.  Since this program may often be run
 * setuid to gain access to privileged files, care is taken that the shell
 * is run with the user's REAL user id, and not the effective user id.
 * The effective user id is restored after the shell completes.
 */

void md_shell(char *shell)
{
	int status;

	if (!fork()) {
		int uid;

		uid = getuid();
		setuid(uid);
		execl(shell, shell, 0);
	}
	wait(&status);
}


#endif
