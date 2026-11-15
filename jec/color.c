/*
 * color.c
 *
 * Optional ncurses color support for modern terminals.
 */

#define ROGUE_COLOR_INTERNAL
#include "rogue.h"

#define ROGUE_PAIR_NORMAL 1
#define ROGUE_PAIR_PLAYER 2
#define ROGUE_PAIR_TERRAIN 3
#define ROGUE_PAIR_TUNNEL 4
#define ROGUE_PAIR_MONSTER 5
#define ROGUE_PAIR_ITEM 6
#define ROGUE_PAIR_DOOR 7
#define ROGUE_PAIR_EFFECT_COLD 8
#define ROGUE_PAIR_EFFECT_FIRE 9
#define ROGUE_PAIR_TRAP 10
#define ROGUE_PAIR_SCORE_TITLE 11
#define ROGUE_PAIR_SCORE_HEADER 12
#define ROGUE_PAIR_SCORE_REVERSE 13

/* The game color values below use ANSI/xterm indexes, while Windows
 * PDCursesMod has its own indices for the first 16 curses colors, including
 * with the VT backend.  Keep the Unix definitions untouched. */
#ifdef _WIN32
#define ROGUE_BASE_RED     1
#define ROGUE_BASE_GREEN   2
#define ROGUE_BASE_YELLOW  3
#define ROGUE_BASE_MAGENTA 5
#define ROGUE_BASE_CYAN    6
#else
#define ROGUE_BASE_RED     COLOR_RED
#define ROGUE_BASE_GREEN   COLOR_GREEN
#define ROGUE_BASE_YELLOW  COLOR_YELLOW
#define ROGUE_BASE_MAGENTA COLOR_MAGENTA
#define ROGUE_BASE_CYAN    COLOR_CYAN
#endif

static boolean color_ready = 0;
static boolean color_started = 0;
static short effect_color = ROGUE_EFFECT_NONE;
static boolean terminal_background_changed = 0;
static boolean terminal_foreground_changed = 0;

static short color_background = COLOR_BLACK;
static short color_player = ROGUE_BASE_GREEN;
static short color_terrain = COLOR_WHITE;
static short color_tunnel = COLOR_BLACK;
static short color_monster = ROGUE_BASE_MAGENTA;
static short color_item = ROGUE_BASE_YELLOW;
static short color_door = ROGUE_BASE_YELLOW;
static short color_effect_cold = ROGUE_BASE_CYAN;
static short color_effect_fire = ROGUE_BASE_RED;
static short color_trap = ROGUE_BASE_RED;
static short color_score_title = ROGUE_BASE_YELLOW;
static short color_score_header = ROGUE_BASE_GREEN;
static short color_score_highlight = ROGUE_BASE_CYAN;
static boolean use_precise_colors = 0;

static void write_ansi(char *s)
{
#ifdef _WIN32
	/* The VT backend owns the curses screen.  Do not write escapes
	 * behind its back and desynchronize the curses display. */
	(void) s;
#else
	(void) write(1, s, strlen(s));
#endif
}

#ifndef _WIN32
static void write_background_ansi(void)
{
	if (use_precise_colors && (color_background == 16)) {
		write_ansi("\033[48;5;16m");
	} else {
		write_ansi("\033[40m");
	}
}

static void write_clear_background_ansi(void)
{
	write_background_ansi();
	write_ansi("\033[2J\033[H");
}

/*
 * Like Rogue 5.4.5 JEC's jrogue_prepare_input_attr(): prepare the
 * curses window and the real terminal attributes before reading a key.
 * macOS Terminal's IME may draw uncommitted text with the terminal's
 * current/default attributes rather than the curses color pair.
 * Reset those attributes to the game's normal COLOR_WHITE on black,
 * without changing the other game colors or the terminal palette.
 * Windows/PDCursesMod intentionally keeps its existing input path.
 */
void rogue_prepare_input_attr(WINDOW *wn)
{
	if (wn != NULL) {
		if (use_color && color_ready) {
			(void) wattrset(wn, COLOR_PAIR(ROGUE_PAIR_NORMAL));
		} else {
			(void) wattrset(wn, A_NORMAL);
		}
		(void) wrefresh(wn);
	}
	if (use_color && color_ready) {
		/* Conventional ANSI COLOR_WHITE (index 7), as in Rogue 5.4.5. */
		write_ansi("\033]10;#c0c0c0\007");
		terminal_foreground_changed = 1;
		write_ansi("\033]11;#000000\007");
		terminal_background_changed = 1;
		write_ansi("\033[0m\033[37m");
		write_background_ansi();
	}
}
#endif

/* The game's palettes use ANSI/xterm indices.  Windows needs
 * translation to PDCursesMod's color indices even with the VT backend. */
#ifdef _WIN32
#define rogue_platform_color rogue_win_platform_color
#else
static short rogue_platform_color(short color)
{
    return color;
}
#endif

static void init_chatgpt_colors(void)
{
	/* Even on 8/16-color Windows consoles, keep the desired xterm RGB
	 * indexes so rogue_platform_color() selects the nearest basic color.
	 * Do not alter the Unix/macOS behavior below. */
#ifdef _WIN32
	if (COLORS >= 8) {
#else
	if (COLORS >= 256) {
#endif
		color_background = 16;
		color_player = 107;  /* #87af5f, near #6a9955 */
		color_terrain = 250; /* #bcbcbc */
		color_tunnel = 242;  /* #6c6c6c */
		color_monster = 176; /* #d787d7, near #c586c0 */
		color_item = 179;    /* #d7af5f */
		color_door = 173;    /* #d7875f, near #ce9178 */
		color_effect_cold = 81;  /* #5fd7ff, near #4fc1ff */
		color_effect_fire = 167; /* #d75f5f, near #f44747 */
		color_trap = 167;    /* #d75f5f, near #f44747 */
		/* Match the Hybrid Edition score screen colors. */
		color_score_title = 221;
		color_score_header = 114;
		color_score_highlight = 153;
		use_precise_colors = (COLORS >= 256);
	}
}

static void init_pairs(void)
{
	(void) init_pair(ROGUE_PAIR_NORMAL, rogue_platform_color(COLOR_WHITE), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_PLAYER, rogue_platform_color(color_player), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_TERRAIN, rogue_platform_color(color_terrain), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_TUNNEL, rogue_platform_color(color_tunnel), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_MONSTER, rogue_platform_color(color_monster), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_ITEM, rogue_platform_color(color_item), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_DOOR, rogue_platform_color(color_door), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_EFFECT_COLD, rogue_platform_color(color_effect_cold), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_EFFECT_FIRE, rogue_platform_color(color_effect_fire), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_TRAP, rogue_platform_color(color_trap), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_SCORE_TITLE, rogue_platform_color(color_score_title), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_SCORE_HEADER, rogue_platform_color(color_score_header), rogue_platform_color(color_background));
	(void) init_pair(ROGUE_PAIR_SCORE_REVERSE, rogue_platform_color(COLOR_BLACK), rogue_platform_color(color_score_highlight));
}

static int item_char(int ch)
{
	return((ch == '?') || (ch == '!') || (ch == '*') || (ch == ':') ||
		(ch == '/') || (ch == ']') || (ch == ')') || (ch == '=') ||
		(ch == ','));
}

static short pair_for_char(int ch)
{
	ch &= 0xff;
	if (effect_color == ROGUE_EFFECT_COLD) {
		return(ROGUE_PAIR_EFFECT_COLD);
	}
	if (effect_color == ROGUE_EFFECT_FIRE) {
		return(ROGUE_PAIR_EFFECT_FIRE);
	}
	if ((ch == rogue.fchar) || (ch == '@')) {
		return(ROGUE_PAIR_PLAYER);
	}
	if ((ch >= 'A') && (ch <= 'Z')) {
		return(ROGUE_PAIR_MONSTER);
	}
	if (item_char(ch)) {
		return(ROGUE_PAIR_ITEM);
	}
	switch(ch) {
	case '-':
	case '|':
	case '%':
		return(ROGUE_PAIR_TERRAIN);
	case '^':
		return(ROGUE_PAIR_TRAP);
	case '.':
	case '#':
		return(ROGUE_PAIR_TUNNEL);
	case '+':
		return(ROGUE_PAIR_DOOR);
	default:
		return(ROGUE_PAIR_NORMAL);
	}
}

static int color_addch_at(int row, int col, int ch, boolean do_move)
{
	int r;
	short pair;
	attr_t attr = A_NORMAL;

	if ((!use_color) || (!color_ready)) {
		if (do_move) {
			return(mvaddch(row, col, ch));
		}
		return(addch(ch));
	}
	pair = pair_for_char(ch);
	if ((!use_precise_colors) && (pair == ROGUE_PAIR_TUNNEL)) {
		attr |= A_BOLD;
	}
	(void) attrset(COLOR_PAIR(pair) | attr);
	if (do_move) {
		r = mvaddch(row, col, ch & 0xff);
	} else {
		r = addch(ch & 0xff);
	}
	(void) attrset(COLOR_PAIR(ROGUE_PAIR_NORMAL));
	return(r);
}

static void recolor_cell(int row, int col)
{
	chtype cell;
	int ch;
	short pair = ROGUE_PAIR_NORMAL;
	attr_t attr = A_NORMAL;

	if (!color_ready) {
		return;
	}
	cell = mvinch(row, col);
	ch = (int) (cell & A_CHARTEXT);
	if (use_color) {
		if ((row == 0) || (row == DROWS - 1)) {
			pair = ROGUE_PAIR_NORMAL;
		} else {
			pair = pair_for_char(ch);
			if ((!use_precise_colors) && (pair == ROGUE_PAIR_TUNNEL)) {
				attr |= A_BOLD;
			}
		}
		(void) mvchgat(row, col, 1, attr, pair, NULL);
		if ((ch == ' ') && (row > 0) && (row < DROWS - 1)) {
			(void) attrset(COLOR_PAIR(ROGUE_PAIR_NORMAL));
			(void) mvaddch(row, col, ' ');
			(void) attrset(COLOR_PAIR(ROGUE_PAIR_NORMAL));
		}
	} else {
		(void) mvchgat(row, col, 1, A_NORMAL, 0, NULL);
		if ((ch == ' ') && (row > 0) && (row < DROWS - 1)) {
			(void) attrset(A_NORMAL);
			(void) mvaddch(row, col, ' ');
			(void) attrset(A_NORMAL);
		}
	}
}

void rogue_init_colors(void)
{
	int start_result;

	if (color_started) {
		return;
	}
	start_result = start_color();
	color_started = 1;
	if (start_result == ERR) {
		color_ready = 0;
		return;
	}
#ifndef _WIN32
	(void) use_default_colors();
#endif
	if ((!has_colors()) || (COLORS < 8) ||
			(COLOR_PAIRS <= ROGUE_PAIR_SCORE_REVERSE)) {
		color_ready = 0;
		return;
	}
	init_chatgpt_colors();
#ifdef _WIN32
	rogue_win_init_palette();
#endif
	init_pairs();
	color_ready = 1;
}

void rogue_apply_color_mode(void)
{
	if (use_color && !color_ready) {
		rogue_init_colors();
	}
	if (use_color && color_ready) {
#ifndef _WIN32
		/* The input method may draw preedit/marked text using the terminal's
		 * default foreground rather than the curses color pair.  On a
		 * light-background macOS Terminal profile this default is black,
		 * which becomes invisible after the game selects a black background.
		 * Keep the terminal's default foreground and background consistent
		 * while color mode is active, and restore both on exit. */
		/* Rogue 5.4.5-style conventional COLOR_WHITE (palette index 7). */
		write_ansi("\033]10;#c0c0c0\007");
		terminal_foreground_changed = 1;
		write_ansi("\033]11;#000000\007");
		terminal_background_changed = 1;
		write_background_ansi();
#endif
		(void) bkgd(' ' | COLOR_PAIR(ROGUE_PAIR_NORMAL));
		(void) attrset(COLOR_PAIR(ROGUE_PAIR_NORMAL));
	} else {
		(void) bkgd(' ' | A_NORMAL);
		(void) attrset(A_NORMAL);
		rogue_restore_terminal_colors();
	}
}

void rogue_restore_terminal_colors(void)
{
	if (terminal_foreground_changed) {
		write_ansi("\033]110\007");
		terminal_foreground_changed = 0;
	}
	if (terminal_background_changed) {
		write_ansi("\033]111\007");
		terminal_background_changed = 0;
	}
	write_ansi("\033[39;49m");
}

void rogue_recolor_screen(void)
{
	int row, col, old_row, old_col;

	if (!color_ready) {
		return;
	}
	getyx(stdscr, old_row, old_col);
	for (row = 0; row < DROWS; row++) {
		for (col = 0; col < DCOLS; col++) {
			recolor_cell(row, col);
		}
	}
	move(old_row, old_col);
}


void rogue_set_score_title_color(void)
{
	if (use_color && color_ready) {
		(void) attrset(COLOR_PAIR(ROGUE_PAIR_SCORE_TITLE));
	}
}

void rogue_set_score_header_color(void)
{
	if (use_color && color_ready) {
		(void) attrset(COLOR_PAIR(ROGUE_PAIR_SCORE_HEADER));
	}
}

void rogue_set_score_reverse_color(void)
{
	if (use_color && color_ready) {
		(void) attrset(COLOR_PAIR(ROGUE_PAIR_SCORE_REVERSE));
		return;
	}
	standout();
}

void rogue_reset_score_color(void)
{
	if (use_color && color_ready) {
		(void) attrset(COLOR_PAIR(ROGUE_PAIR_NORMAL));
		return;
	}
	(void) attrset(A_NORMAL);
}

void rogue_end_score_reverse_color(void)
{
	if (use_color && color_ready) {
		(void) attrset(COLOR_PAIR(ROGUE_PAIR_NORMAL));
		return;
	}
	standend();
}

void rogue_set_effect_color(short effect)
{
	effect_color = effect;
}

int rogue_addch(int ch)
{
	return(color_addch_at(0, 0, ch, 0));
}

int rogue_clear(void)
{
#ifndef _WIN32
	int row, col;
#endif
	int r;

	/* On Windows, PDCursesMod VT uses the current background color when
	 * clear() erases the window.  Do not also refresh and paint LINES x
	 * COLS blank cells individually: the bottom-right cell may trigger
	 * terminal autowrap/scroll, especially just after a VT resize.
	 * Unix retains its original explicit background clearing behavior. */
#ifdef _WIN32
	r = clear();
	if (use_color && color_ready)
		(void) attrset(COLOR_PAIR(ROGUE_PAIR_NORMAL));
	(void) move(0, 0);
	return r;
#else
	r = clear();
	if (use_color && color_ready) {
		(void) refresh();
		write_clear_background_ansi();
		(void) attrset(COLOR_PAIR(ROGUE_PAIR_NORMAL));
		for (row = 0; row < LINES; row++) {
			for (col = 0; col < COLS; col++) {
				(void) mvaddch(row, col, ' ');
			}
		}
		move(0, 0);
	}
	return(r);
#endif
}


int rogue_mvaddch_skull_dark(short row, short col, int ch)
{
	int r;
	attr_t attr = A_NORMAL;

	if ((!use_color) || (!color_ready)) {
		return(mvaddch(row, col, ch));
	}
	if (!use_precise_colors) {
		attr |= A_BOLD;
	}
	(void) attrset(COLOR_PAIR(ROGUE_PAIR_TUNNEL) | attr);
	r = mvaddch(row, col, ch & 0xff);
	(void) attrset(COLOR_PAIR(ROGUE_PAIR_NORMAL));
	return(r);
}

int rogue_mvaddch(short row, short col, int ch)
{
	return(color_addch_at(row, col, ch, 1));
}
