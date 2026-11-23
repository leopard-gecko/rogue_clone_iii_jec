/*
 * init.c
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
static char sccsid[] __attribute__((unused)) = "@(#)init.c	5.1 (Berkeley) 11/25/87";
#endif /* not lint */

#include <stdio.h>
#include "rogue.h"

char login_name[MAX_OPT_LEN];
char *nick_name = (char *) 0;
char *rest_file = 0;
boolean cant_int = 0;
boolean did_int = 0;
boolean score_only;
boolean init_curses = 0;
boolean save_is_interactive = 1;
boolean ask_quit = 1;
boolean no_skull = 0;
boolean passgo = DEFAULT_PASSGO;
boolean show_cursor = DEFAULT_CURSOR;
boolean use_color = DEFAULT_COLOR;
char *error_file = "rogue.esave";
char *byebye_string = EN_STATIC("Okay, bye bye!", "了解、またね！");

extern char *fruit;
extern char *save_file;
extern short party_room;
extern boolean jump;


int init(int argc, char **argv)
{
	char *pn;
#ifdef MODERN_UNIX
	char *term;
#endif
	int seed;

	rogue_detect_language();
	rogue_apply_language();

	pn = md_gln();
	if ((!pn) || (strlen(pn) >= MAX_OPT_LEN)) {
		clean_up(EN("Hey!  Who are you?", "ちょっと、あなたは誰？"));
	}
	(void) strcpy(login_name, pn);

	do_args(argc, argv);
	do_opts();

	md_save_tty();
#ifdef MODERN_UNIX
	term = md_getenv("TERM");
	if ((!term) || (!*term) || !strcmp(term, "dumb") ||
			!strcmp(term, "vt100")) {
		(void) setenv("TERM", "xterm-256color", 1);
	}
#endif
	/* PDCursesMod VT initializes the console itself, as in 5.4.5 JEC. */
	initscr();
#ifdef _WIN32
	rogue_win_retry_direct_launch_resize();
	/* The VT resize above can change the real window AFTER initscr() has
	 * sampled its dimensions.  If we keep stale LINES/COLS (e.g. 35x80)
	 * and subsequently clear the screen, the rows beyond the new 24-line
	 * viewport can scroll the entire dungeon off the top of the terminal.
	 * Match 5.4.5's 80x24 game area by updating PDCurses' logical screen
	 * before any game drawing.  Only resize an approved direct launch;
	 * existing cmd/PowerShell/MSYS2 terminals are not resized. */
	if (rogue_win_is_direct_launch() &&
	    (LINES != DROWS || COLS != DCOLS)) {
		if (resize_term(DROWS, DCOLS) == ERR) {
			clean_up(EN("cannot resize the game screen",
			            "ゲーム画面の大きさを変更できません。"));
		}
	}
	/* After a terminal resize, fully repaint using the new geometry. */
	if (rogue_win_is_direct_launch()) {
		clearok(stdscr, TRUE);
	}
	(void) keypad(stdscr, TRUE);
#endif
	if ((LINES < DROWS) || (COLS < DCOLS)) {
		clean_up(EN("must be played on 24 x 80 screen",
		            "画面の大きさは24x80にしましょう。"));
	}
	start_window();
	init_curses = 1;
	if (use_color) {
		rogue_init_colors();
		rogue_apply_color_mode();
	}
#ifdef _WIN32
    /* Rogue 3.6-style Explorer launch: offer resume/new/file/quit only
     * when the default save actually exists.  An explicit command-line
     * filename (including drag & drop) always takes priority. */
    if (!score_only && !rest_file && rogue_win_is_direct_launch() &&
        rogue_win_save_exists(save_file)) {
        rogue_win_startup_save_menu();
    }
#endif
	if (!score_only && !rest_file) {
		char msg[ROGUE_DESC_LEN];

		(void) snprintf(msg, sizeof(msg),
			EN("Hello %s, just a moment while I dig the dungeon...",
			   "やあ、%s。 ダンジョンを掘っている間、ちょっと待っててね..."),
			nick_name);
		rogue_clear();
		(void) attrset(A_NORMAL);
		move(0, 0);
		addstr(msg);
		clrtoeol();
		if (!show_cursor) {
			rogue_show_cursor(0);
		}
		refresh();
#ifdef MODERN_UNIX
		(void) usleep(1000000);
#else
		md_sleep(1);
#endif
		rogue_clear();
		refresh();
	}

	md_heed_signals();

	if (score_only) {
		put_scores((object *) 0, 0);
	}
	seed = md_gseed();
	(void) srrandom(seed);
	if (rest_file) {
		restore(rest_file);
		return(1);
	}
	mix_colors();
	get_wand_and_ring_materials();
	make_scroll_titles();

	level_objects.next_object = (object *) 0;
	level_monsters.next_monster = (object *) 0;
	player_init();
	ring_stats(0);
	return(0);
}

void player_init(void)
{
	object *obj;

	rogue.pack.next_object = (object *) 0;

	obj = alloc_object();
	get_food(obj, 1);
	(void) add_to_pack(obj, &rogue.pack, 1);

	obj = alloc_object();		/* initial armor */
	obj->what_is = ARMOR;
	obj->which_kind = RINGMAIL;
	obj->class = RINGMAIL+2;
	obj->is_protected = 0;
	obj->d_enchant = 1;
	(void) add_to_pack(obj, &rogue.pack, 1);
	do_wear(obj);

	obj = alloc_object();		/* initial weapons */
	obj->what_is = WEAPON;
	obj->which_kind = MACE;
	obj->damage = "2d3";
	obj->hit_enchant = obj->d_enchant = 1;
	obj->identified = 1;
	(void) add_to_pack(obj, &rogue.pack, 1);
	do_wield(obj);

	obj = alloc_object();
	obj->what_is = WEAPON;
	obj->which_kind = BOW;
	obj->damage = "1d2";
	obj->hit_enchant = 1;
	obj->d_enchant = 0;
	obj->identified = 1;
	(void) add_to_pack(obj, &rogue.pack, 1);

	obj = alloc_object();
	obj->what_is = WEAPON;
	obj->which_kind = ARROW;
	obj->quantity = get_rand(25, 35);
	obj->damage = "1d2";
	obj->hit_enchant = 0;
	obj->d_enchant = 0;
	obj->identified = 1;
	(void) add_to_pack(obj, &rogue.pack, 1);
}

void clean_up(char *estr)
{
	if (save_is_interactive) {
		if (init_curses) {
			move(DROWS-1, 0);
			refresh();
			stop_window();
		}
		if (estr && *estr) {
			printf("%s\n", estr);
		}
	}
	md_exit(0);
}

void start_window(void)
{
	/*
	 * Save/alter special control characters before curses switches the
	 * terminal into cbreak/noecho/nonl.  In v9 this was done after the
	 * curses mode changes, so md_control_keybord() could save an already
	 * modified tty state and later restore that bad state on exit.
	 * Reapply it after curses setup too, because cbreak() can reinstate
	 * characters such as VINTR.
	 */
	md_control_keybord(0);
	cbreak();
	noecho();
#ifndef BAD_NONL
	nonl();
#endif
	md_control_keybord(0);
#ifdef _WIN32
	/* Reapply after initscr() or reset_prog_mode() (shell return). */
	rogue_win_cursor_invalidate();
	rogue_show_cursor(show_cursor);
#endif
}

void stop_window(void)
{
#ifdef _WIN32
	/* Do not leak wizard-mode console input settings to the shell. */
	rogue_win_set_wizard_ctrl_c(0);
#endif
	/* Restore control characters first; then let curses restore its modes. */
	rogue_show_cursor(1);
	rogue_restore_terminal_colors();
	md_control_keybord(1);
	/*
	 * System curses/ncurses keeps its own terminal state.  Make sure
	 * modes changed by cbreak(), noecho(), and nonl() are explicitly
	 * restored before returning to the user's shell.
	 */
	nl();
	echo();
	nocbreak();
	reset_shell_mode();
	endwin();
#ifdef _WIN32
	/* PDCursesMod's VT PDC_scr_close() forces a blinking block cursor.
	 * VT has already been disabled by endwin(), so temporarily enable it
	 * only to reset the terminal's cursor shape to its configured default. */
	rogue_win_cursor_restore_default();
#endif
	rogue_restore_terminal_colors();
	/* Last-resort exact restore of the tty state captured before initscr(). */
	md_restore_tty();
}

void byebye(int sig)
{
	(void) sig;
	md_ignore_signals();
	if (ask_quit) {
		quit(1);
	} else {
		clean_up(byebye_string);
	}
	md_heed_signals();
}

void onintr(int sig)
{
	(void) sig;
	md_ignore_signals();
	if (cant_int) {
		did_int = 1;
	} else {
		check_message();
		message(EN("interrupt", "割り込みがあった。"), 1);
	}
	md_heed_signals();
}

void error_save(int sig)
{
	(void) sig;
	save_is_interactive = 0;
	save_into_file(error_file);
	clean_up("");
}

void do_args(int argc, char **argv)
{
	short i, j;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			for (j = 1; argv[i][j]; j++) {
				switch(argv[i][j]) {
				case 's':
					score_only = 1;
					break;
				}
			}
		} else {
			rest_file = argv[i];
		}
	}
}

void do_opts(void)
{
	char *eptr;

	if ((eptr = md_getenv("ROGUEOPTS"))) {
		for (;;) {
			while ((*eptr) == ' ') {
				eptr++;
			}
			if (!(*eptr)) {
				break;
			}
			if (!strncmp(eptr, "fruit=", 6)) {
				eptr += 6;
				env_get_value(&fruit, eptr, 1);
			} else if (!strncmp(eptr, "file=", 5)) {
				eptr += 5;
				env_get_value(&save_file, eptr, 0);
			} else if (!strncmp(eptr, "language=", 9)) {
				char code[MAX_OPT_LEN + 2];
				const char *value;
				int length;

				/* Parse one complete, comma-delimited value.  Never write
				 * "auto" into the in-game language option string.
				 */
				eptr += 9;
				value = eptr;
				while (*value == ' ' || *value == '\t') {
					value++;
				}
				length = 0;
				while (value[length] && value[length] != ',') {
					length++;
				}
				while (length > 0 &&
				       (value[length - 1] == ' ' ||
				        value[length - 1] == '\t')) {
					length--;
				}
				if (length < 1 || length >= (int) sizeof(code)) {
					fprintf(stderr, "invalid ROGUEOPTS language (use ja, en, or auto)\n");
					exit(EXIT_FAILURE);
				}
				(void) memcpy(code, value, length);
				code[length] = '\0';
				if (!rogue_set_initial_language_code(code)) {
					fprintf(stderr, "invalid ROGUEOPTS language '%s' (use ja, en, or auto)\n", code);
					exit(EXIT_FAILURE);
				}
				rogue_apply_language();
			} else if (!strncmp(eptr, "jump", 4)) {
				jump = 1;
			} else if (!strncmp(eptr, "name=", 5)) {
				eptr += 5;
				env_get_value(&nick_name, eptr, 0);
			} else if (!strncmp(eptr, "noaskquit", 9)) {
				ask_quit = 0;
			} else if (!strncmp(eptr, "nocolor", 7)) {
				use_color = 0;
			} else if (!strncmp(eptr, "nocursor", 8)) {
				show_cursor = 0;
			} else if (!strncmp(eptr, "color", 5)) {
				use_color = 1;
			} else if (!strncmp(eptr, "noskull", 5) ||
					!strncmp(eptr,"notomb", 6)) {
				no_skull = 1;
			} else if (!strncmp(eptr, "nopassgo", 8)) {
				passgo = 0;
			} else if (!strncmp(eptr, "passgo", 5)) {
				passgo = 1;
			} else if (!strncmp(eptr, "showcursor", 10) ||
					!strncmp(eptr, "cursor", 6)) {
				show_cursor = 1;
			}
			while ((*eptr) && (*eptr != ',')) {
				eptr++;
			}
			if (!(*(eptr++))) {
				break;
			}
		}
	}
	/* If some strings have not been set through ROGUEOPTS, assign defaults
	 * to them so that the options editor has data to work with.
	 */
	init_str(&nick_name, login_name);
	init_str(&save_file, DEFAULT_SAVE_FILE);
	init_str(&language_option, rogue_language_code());
	init_str(&fruit, ROGUE_FRUIT);
	rogue_apply_language();
}

void env_get_value(char **s, char *e, boolean add_blank)
{
	short i = 0;
	char *t;

	t = e;

	while ((*e) && (*e != ',')) {
		if (*e == ':') {
			*e = ';';		/* ':' reserved for score file purposes */
		}
		e++;
		if (++i >= MAX_OPT_LEN) {
			break;
		}
	}
	*s = md_malloc(MAX_OPT_LEN + 2);
	(void) strncpy(*s, t, i);
	if (add_blank) {
		(*s)[i++] = ' ';
	}
	(*s)[i] = '\0';
}

void init_str(char **str, char *dflt)
{
	if (!(*str)) {
		*str = md_malloc(MAX_OPT_LEN + 2);
		(void) strcpy(*str, dflt);
	}
}
