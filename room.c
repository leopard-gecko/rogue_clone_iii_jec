/*
 * room.c
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
static char sccsid[] __attribute__((unused)) = "@(#)room.c	5.1 (Berkeley) 11/25/87";
#endif /* not lint */

#include "rogue.h"

room rooms[MAXROOMS];
boolean rooms_visited[MAXROOMS];

extern short blind, cur_room;
extern boolean detect_monster, jump, passgo, no_skull, ask_quit, wizard;
extern char *nick_name, *fruit, *save_file, *press_space;

#define NOPTS 10

struct option {
	char *prompt_en;
	char *prompt_ja;
	boolean is_bool;
	char **strval;
	boolean *bval;
} options[NOPTS] = {
	{
		"Show position only at end of run (\"jump\"): ",
		"移動中の表示を行わない (\"jump\"): ",
		1, (char **) 0, &jump
	},
	{
		"Follow turnings in passageways (\"passgo\"): ",
		"通路の角で止まらずに進む (\"passgo\"): ",
		1, (char **) 0, &passgo
	},
	{
		"Don't print skull when killed (\"noskull\" or \"notombstone\"): ",
		"死亡時に墓を表示しない (\"noskull\"/\"notombstone\"): ",
		1, (char **) 0, &no_skull
	},
	{
		"Ask player before saying 'Okay, bye-bye!' (\"askquit\"): ",
		"終了前に確認する (\"askquit\"): ",
		1, (char **) 0, &ask_quit
	},
	{
		"Name (\"name\"): ", "プレイヤー名 (\"name\"): ",
		0, &nick_name, (boolean *) 0
	},
	{
		"Fruit (\"fruit\"): ", "好きな果物 (\"fruit\"): ",
		0, &fruit, (boolean *) 0
	},
	{
		"Use colors (\"color\"): ",
		"カラーで表示 (\"color\"): ",
		1, (char **) 0, &use_color
	},
	{
		"Show cursor during play (\"cursor\"): ",
		"プレイ中のカーソルを表示 (\"cursor\"): ",
		1, (char **) 0, &show_cursor
	},
	{
		"Language (\"language\", J/E): ",
		"表示する言語 (\"language\", J/E): ",
		0, &language_option, (boolean *) 0
	},
	{
		"Save file (\"file\"): ", "セーブファイル名 (\"file\"): ",
		0, &save_file, (boolean *) 0
	}
};

static char *opt_prompt(int i)
{
	return(EN(options[i].prompt_en, options[i].prompt_ja));
}

static int opt_is_language(int i)
{
	return((!options[i].is_bool) && (options[i].strval == &language_option));
}

static void opt_redraw_all(void)
{
	short i;

	for (i = 0; i < NOPTS; i++) {
		opt_show(i);
	}
}

static void opt_change_language(int i, char *code)
{
	rogue_set_language_code(code);
	rogue_apply_language();
	opt_redraw_all();
	print_stats(STAT_ALL);
	opt_go(i);
	refresh();
}

static char *opt_language_display(void)
{
	return((!strcmp(language_option, "ja")) ? "日本語(J)" : "English(E)");
}

static void opt_draw_edit(int row, char *buf, int cursor)
{
	int prompt_width = jrogue_text_width(opt_prompt(row));
	int col = prompt_width + jrogue_text_width_n(buf, (size_t) cursor);

	/* Avoid wrapping the 80-column option screen for long UTF-8 names. */
	if (prompt_width >= DCOLS) {
		prompt_width = DCOLS - 1;
	}
	move(row, prompt_width);
	clrtoeol();
	rogue_addstr_fit(buf, DCOLS - prompt_width - 1);
	if (col >= DCOLS) {
		col = DCOLS - 1;
	}
	move(row, col);
}

static int opt_read_escape(void)
{
	int ch;

	ch = rgetchar();
	if (ch == '[') {
		ch = rgetchar();
		switch(ch) {
		case 'C':
			return('R');
		case 'D':
			return('L');
		case '3':
			if (rgetchar() == '~') {
				return('D');
			}
			break;
		}
	}
	return('\033');
}

static int edit_option_string(int row, int first_ch)
{
	char buf[MAX_OPT_LEN + 2];
	char uch[8];
	int len = 0, cursor = 0;
	int ch = first_ch, key, ulen, k;

	/* Copy only whole Unicode characters into the edit buffer. */
	len = (int) jrogue_text_prefix(*(options[row].strval), sizeof(buf) - 1);
	memcpy(buf, *(options[row].strval), (size_t) len);
	buf[len] = 0;
	cursor = len;
	opt_draw_edit(row, buf, cursor);
	refresh();
	rogue_show_cursor(1);

	for (;;) {
		if ((ch == '\012') || (ch == '\015')) {
			break;
		}
		if (ch == '\033') {
			key = opt_read_escape();
			if (key == 'L') {
				if (cursor > 0) {
					cursor = (int) jrogue_text_prev(buf, (size_t) cursor);
				}
			} else if (key == 'R') {
				if (cursor < len) {
					cursor = (int) jrogue_text_next(buf, (size_t) cursor);
				}
			} else if (key == 'D') {
				if (cursor < len) {
					int next = (int) jrogue_text_next(buf, (size_t) cursor);
					memmove(buf + cursor, buf + next, len - next + 1);
					len -= (next - cursor);
				}
			} else {
				return('\033');
			}
		} else if ((ch == '\010') || (ch == 0177)) {
			if (cursor > 0) {
				int prev = (int) jrogue_text_prev(buf, (size_t) cursor);
				memmove(buf + prev, buf + cursor, len - cursor + 1);
				len -= (cursor - prev);
				cursor = prev;
			}
		} else if ((ulen = rogue_read_utf8_char(ch, uch, sizeof(uch))) > 0) {
			if ((len + ulen) < MAX_OPT_LEN-1) {
				memmove(buf + cursor + ulen, buf + cursor, len - cursor + 1);
				for (k = 0; k < ulen; k++) {
					buf[cursor + k] = uch[k];
				}
				cursor += ulen;
				len += ulen;
				/* Keep the cursor after any newly completed combined character. */
				cursor = (int) jrogue_text_next(buf, (size_t)(cursor - 1));
			}
		}
		opt_draw_edit(row, buf, cursor);
		refresh();
		rogue_show_cursor(1);
		ch = rgetchar();
	}
	(void) strcpy(*(options[row].strval), buf);
	opt_show(row);
	return(ch);
}

void light_up_room(int rn)
{
	short i, j;

	if (!blind) {
		for (i = rooms[rn].top_row;
			i <= rooms[rn].bottom_row; i++) {
			for (j = rooms[rn].left_col;
				j <= rooms[rn].right_col; j++) {
				if (dungeon[i][j] & MONSTER) {
					object *monster;

					if ((monster = object_at(&level_monsters, i, j))) {
						dungeon[monster->row][monster->col] &= (~MONSTER);
						monster->trail_char =
							get_dungeon_char(monster->row, monster->col);
						dungeon[monster->row][monster->col] |= MONSTER;
					}
				}
				mvaddch(i, j, get_dungeon_char(i, j));
			}
		}
		mvaddch(rogue.row, rogue.col, rogue.fchar);
	}
}

void light_passage(int row, int col)
{
	short i, j, i_end, j_end;

	if (blind) {
		return;
	}
	i_end = (row < (DROWS-2)) ? 1 : 0;
	j_end = (col < (DCOLS-1)) ? 1 : 0;

	for (i = ((row > MIN_ROW) ? -1 : 0); i <= i_end; i++) {
		for (j = ((col > 0) ? -1 : 0); j <= j_end; j++) {
			if (can_move(row, col, row+i, col+j)) {
				mvaddch(row+i, col+j, get_dungeon_char(row+i, col+j));
			}
		}
	}
}

void darken_room(short rn)
{
	short i, j;

	for (i = rooms[rn].top_row + 1; i < rooms[rn].bottom_row; i++) {
		for (j = rooms[rn].left_col + 1; j < rooms[rn].right_col; j++) {
			if (blind) {
				mvaddch(i, j, ' ');
			} else {
				if (!(dungeon[i][j] & (OBJECT | STAIRS)) &&
					!(detect_monster && (dungeon[i][j] & MONSTER))) {
					if (!imitating(i, j)) {
						mvaddch(i, j, ' ');
					}
					if ((dungeon[i][j] & TRAP) && (!(dungeon[i][j] & HIDDEN))) {
						mvaddch(i, j, '^');
					}
				}
			}
		}
	}
}

int get_dungeon_char(register int row, register int col)
{
	register unsigned short mask = dungeon[row][col];

	if (mask & MONSTER) {
		return(gmc_row_col(row, col));
	}
	if (mask & OBJECT) {
		object *obj;

		obj = object_at(&level_objects, row, col);
		return(get_mask_char(obj->what_is));
	}
	if (mask & (TUNNEL | STAIRS | HORWALL | VERTWALL | FLOOR | DOOR)) {
		if ((mask & (TUNNEL| STAIRS)) && (!(mask & HIDDEN))) {
			return(((mask & STAIRS) ? '%' : '#'));
		}
		if (mask & HORWALL) {
			return('-');
		}
		if (mask & VERTWALL) {
			return('|');
		}
		if (mask & FLOOR) {
			if (mask & TRAP) {
				if (!(dungeon[row][col] & HIDDEN)) {
					return('^');
				}
			}
			return('.');
		}
		if (mask & DOOR) {
			if (mask & HIDDEN) {
				if (((col > 0) && (dungeon[row][col-1] & HORWALL)) ||
					((col < (DCOLS-1)) && (dungeon[row][col+1] & HORWALL))) {
					return('-');
				} else {
					return('|');
				}
			} else {
				return('+');
			}
		}
	}
	return(' ');
}

int get_mask_char(register unsigned short mask)
{
		switch(mask) {
		case SCROL:
			return('?');
		case POTION:
			return('!');
		case GOLD:
			return('*');
		case FOOD:
			return(':');
		case WAND:
			return('/');
		case ARMOR:
			return(']');
		case WEAPON:
			return(')');
		case RING:
			return('=');
		case AMULET:
			return(',');
		default:
			return('~');	/* unknown, something is wrong */
		}
}

static int gr_candidate_ok(short r, short c, unsigned short mask)
{
	short rn;

	if (!ROGUE_IN_DUNGEON(r, c)) {
		return(0);
	}
	rn = get_room_number(r, c);
	if (rn == NO_ROOM) {
		return(0);
	}
	if (!(rooms[rn].is_room & (R_ROOM | R_MAZE))) {
		return(0);
	}
	if (!(dungeon[r][c] & mask) || (dungeon[r][c] & (~mask))) {
		return(0);
	}
	if ((r == rogue.row) && (c == rogue.col)) {
		return(0);
	}
	return(1);
}

static int gr_scan_cell(short *row, short *col, unsigned short mask)
{
	short r, c, sr, sc;
	short rr, cc;

	sr = get_rand(MIN_ROW, DROWS-2);
	sc = get_rand(0, DCOLS-1);
	for (r = MIN_ROW; r <= DROWS-2; r++) {
		rr = MIN_ROW + ((sr - MIN_ROW + r - MIN_ROW) % (DROWS - 2));
		for (c = 0; c < DCOLS; c++) {
			cc = (sc + c) % DCOLS;
			if (gr_candidate_ok(rr, cc, mask)) {
				*row = rr;
				*col = cc;
				return(1);
			}
		}
	}
	return(0);
}

int gr_row_col(short *row, short *col, unsigned short mask)
{
	short rn;
	short r, c;
	short tries;

	for (tries = 0; tries < 1000; tries++) {
		r = get_rand(MIN_ROW, DROWS-2);
		c = get_rand(0, DCOLS-1);
		rn = get_room_number(r, c);
		if ((rn != NO_ROOM) &&
			(dungeon[r][c] & mask) &&
			(!(dungeon[r][c] & (~mask))) &&
			(rooms[rn].is_room & (R_ROOM | R_MAZE)) &&
			(!((r == rogue.row) && (c == rogue.col)))) {
			*row = r;
			*col = c;
			return(1);
		}
	}
	if (gr_scan_cell(row, col, mask)) {
		return(1);
	}
	/* Never destroy stairs, traps or other terrain to make room. */
	*row = *col = -1;
	return(0);
}

int gr_room(void)
{
	short i, tries;

	for (tries = 0; tries < 100; tries++) {
		i = get_rand(0, MAXROOMS-1);
		if (rooms[i].is_room & (R_ROOM | R_MAZE)) {
			return(i);
		}
	}
	for (i = 0; i < MAXROOMS; i++) {
		if (rooms[i].is_room & (R_ROOM | R_MAZE)) {
			return(i);
		}
	}
	return(0);
}

int party_objects(int rn)
{
	short i, j, nf = 0;
	object *obj;
	short n, N, row, col;
	boolean found;

	if ((rn < 0) || (rn >= MAXROOMS) ||
		(!(rooms[rn].is_room & (R_ROOM | R_MAZE))) ||
		(rooms[rn].top_row+1 > rooms[rn].bottom_row-1) ||
		(rooms[rn].left_col+1 > rooms[rn].right_col-1)) {
		return(0);
	}
	N = ((rooms[rn].bottom_row - rooms[rn].top_row) - 1) *
		((rooms[rn].right_col - rooms[rn].left_col) - 1);
	n =  get_rand(5, 10);
	if (n > N) {
		n = N - 2;
	}
	if (n < 0) {
		n = 0;
	}
	for (i = 0; i < n; i++) {
		for (j = found = 0; ((!found) && (j < 250)); j++) {
			row = get_rand(rooms[rn].top_row+1,
					   rooms[rn].bottom_row-1);
			col = get_rand(rooms[rn].left_col+1,
					   rooms[rn].right_col-1);
			if (ROGUE_IN_DUNGEON(row, col) &&
				((dungeon[row][col] == FLOOR) ||
				 (dungeon[row][col] == TUNNEL))) {
				found = 1;
			}
		}
		if (found) {
			obj = gr_object();
			place_at(obj, row, col);
			nf++;
		}
	}
	return(nf);
}

int get_room_number(register int row, register int col)
{
	short i;

	for (i = 0; i < MAXROOMS; i++) {
		if ((row >= rooms[i].top_row) && (row <= rooms[i].bottom_row) &&
			(col >= rooms[i].left_col) && (col <= rooms[i].right_col)) {
			return(i);
		}
	}
	return(NO_ROOM);
}

int is_all_connected(void)
{
	short i, starting_room;

	starting_room = NO_ROOM;
	for (i = 0; i < MAXROOMS; i++) {
		rooms_visited[i] = 0;
		if (rooms[i].is_room & (R_ROOM | R_MAZE)) {
			starting_room = i;
		}
	}
	if (starting_room == NO_ROOM) {
		return(0);
	}

	visit_rooms(starting_room);

	for (i = 0; i < MAXROOMS; i++) {
		if ((rooms[i].is_room & (R_ROOM | R_MAZE)) && (!rooms_visited[i])) {
			return(0);
		}
	}
	return(1);
}

void visit_rooms(int rn)
{
	short i;
	short oth_rn;

	if ((rn < 0) || (rn >= MAXROOMS)) {
		return;
	}
	rooms_visited[rn] = 1;

	for (i = 0; i < 4; i++) {
		oth_rn = rooms[rn].doors[i].oth_room;
		if ((oth_rn >= 0) && (oth_rn < MAXROOMS) && (!rooms_visited[oth_rn])) {
			visit_rooms(oth_rn);
		}
	}
}

void draw_magic_map(void)
{
	short i, j, ch, och;
	unsigned short mask = (HORWALL | VERTWALL | DOOR | TUNNEL | TRAP | STAIRS |
			MONSTER);
	unsigned short s;

	for (i = 0; i < DROWS; i++) {
		for (j = 0; j < DCOLS; j++) {
			s = dungeon[i][j];
			if (s & mask) {
				if (((ch = ROGUE_MVINCH_SHORT(i, j)) == ' ') ||
					((ch >= 'A') && (ch <= 'Z')) || (s & (TRAP | HIDDEN))) {
					och = ch;
					dungeon[i][j] &= (~HIDDEN);
					if (s & HORWALL) {
						ch = '-';
					} else if (s & VERTWALL) {
						ch = '|';
					} else if (s & DOOR) {
						ch = '+';
					} else if (s & TRAP) {
						ch = '^';
					} else if (s & STAIRS) {
						ch = '%';
					} else if (s & TUNNEL) {
						ch = '#';
					} else {
						continue;
					}
					if ((!(s & MONSTER)) || (och == ' ')) {
						addch(ch);
					}
					if (s & MONSTER) {
						object *monster;

						if ((monster = object_at(&level_monsters, i, j))) {
							monster->trail_char = ch;
						}
					}
				}
			}
		}
	}
}

void dr_course(object *monster, boolean entering, short row, short col)
{
	short i, j, k, rn;
	short r, rr;

	monster->row = row;
	monster->col = col;

	if (mon_sees(monster, rogue.row, rogue.col)) {
		monster->trow = NO_ROOM;
		return;
	}
	rn = get_room_number(row, col);

	if (entering) {		/* entering room */
		/* look for door to some other room */
		r = get_rand(0, MAXROOMS-1);
		for (i = 0; i < MAXROOMS; i++) {
			rr = (r + i) % MAXROOMS;
			if ((!(rooms[rr].is_room & (R_ROOM | R_MAZE))) || (rr == rn)) {
				continue;
			}
			for (k = 0; k < 4; k++) {
				if (rooms[rr].doors[k].oth_room == rn) {
					monster->trow = rooms[rr].doors[k].oth_row;
					monster->tcol = rooms[rr].doors[k].oth_col;
					if ((monster->trow == row) &&
						(monster->tcol == col)) {
						continue;
					}
					return;
				}
			}
		}
		/* look for door to dead end */
		for (i = rooms[rn].top_row; i <= rooms[rn].bottom_row; i++) {
			for (j = rooms[rn].left_col; j <= rooms[rn].right_col; j++) {
				if ((i != monster->row) && (j != monster->col) &&
					(dungeon[i][j] & DOOR)) {
					monster->trow = i;
					monster->tcol = j;
					return;
				}
			}
		}
		/* return monster to room that he came from */
		for (i = 0; i < MAXROOMS; i++) {
			for (j = 0; j < 4; j++) {
				if (rooms[i].doors[j].oth_room == rn) {
					for (k = 0; k < 4; k++) {
						if (rooms[rn].doors[k].oth_room == i) {
							monster->trow = rooms[rn].doors[k].oth_row;
							monster->tcol = rooms[rn].doors[k].oth_col;
							return;
						}
					}
				}
			}
		}
		/* no place to send monster */
		monster->trow = NO_ROOM;
	} else {		/* exiting room */
		if (!get_oth_room(rn, &row, &col)) {
			monster->trow = NO_ROOM;
		} else {
			monster->trow = row;
			monster->tcol = col;
		}
	}
}

int get_oth_room(short rn, short *row, short *col)
{
	short d = -1;

	if (*row == rooms[rn].top_row) {
		d = UPWARD/2;
	} else if (*row == rooms[rn].bottom_row) {
		d = DOWN/2;
	} else if (*col == rooms[rn].left_col) {
		d = LEFT/2;
	} else if (*col == rooms[rn].right_col) {
		d = RIGHT/2;
	}
	if ((d != -1) && (rooms[rn].doors[d].oth_room >= 0)) {
		*row = rooms[rn].doors[d].oth_row;
		*col = rooms[rn].doors[d].oth_col;
		return(1);
	}
	return(0);
}

void edit_opts(void)
{
	char save[NOPTS+1][DCOLS];
	short i, j;
	short ch;
	boolean done = 0;
	boolean old_use_color = use_color;
	rogue_show_cursor(1);
	for (i = 0; i < NOPTS+1; i++) {
		for (j = 0; j < DCOLS; j++) {
			save[i][j] = ROGUE_MVINCH_CHAR(i, j);
		}
		if (i < NOPTS) {
			opt_show(i);
		}
	}
	opt_go(0);
	i = 0;
#ifdef _WIN32
	/* Bypass PDCursesMod's 16-bit wint_t while editing Unicode options. */
	rogue_win_set_option_input(1);
#endif

	while (!done) {
		refresh();
		rogue_show_cursor(1);
		ch = rgetchar();
CH:
		switch(ch) {
		case '\033':
			if (options[i].is_bool || opt_is_language(i)) {
				done = 1;
			} else {
				ch = edit_option_string(i, ch);
				if (ch == '\033') {
					done = 1;
				} else {
					goto CH;
				}
			}
			break;
		case '\012':
		case '\015':
			if (i == (NOPTS - 1)) {
				mvaddstr(NOPTS, 0, press_space);
				refresh();
				wait_for_ack();
				done = 1;
			} else {
				i++;
				opt_go(i);
			}
			break;
		case '-':
			if (i > 0) {
				opt_go(--i);
			} else {
				sound_bell();
			}
			break;
		case 'j':
		case 'J':
			if (opt_is_language(i)) {
				opt_change_language(i, "ja");
				break;
			}
			if (options[i].is_bool) {
				sound_bell();
				break;
			}
			ch = edit_option_string(i, ch);
			goto CH;
		case 'e':
		case 'E':
			if (opt_is_language(i)) {
				opt_change_language(i, "en");
				break;
			}
			if (options[i].is_bool) {
				sound_bell();
				break;
			}
			ch = edit_option_string(i, ch);
			goto CH;
		case 't':
		case 'T':
		case 'f':
		case 'F':
			if (options[i].is_bool) {
				*(options[i].bval) = (((ch == 't') || (ch == 'T')) ? 1 : 0);
				opt_show(i);
				opt_go(++i);
				break;
			}
			if (opt_is_language(i)) {
				sound_bell();
				break;
			}
			ch = edit_option_string(i, ch);
			goto CH;
		default:
			if (options[i].is_bool || opt_is_language(i)) {
				sound_bell();
				break;
			}
			if ((ch == '\010') || (ch == 0177) || rogue_utf8_edit_char(ch)) {
				ch = edit_option_string(i, ch);
				goto CH;
			} else {
				sound_bell();
			}
			break;
		}
	}
#ifdef _WIN32
	rogue_win_set_option_input(0);
#endif
	for (i = 0; i < NOPTS+1; i++) {
		move(i, 0);
		for (j = 0; j < DCOLS; j++) {
			addch(save[i][j]);
		}
	}
	if (old_use_color != use_color) {
		rogue_apply_color_mode();
	}
	print_stats(STAT_ALL);
	if (old_use_color != use_color) {
		rogue_recolor_screen();
	}
	move(rogue.row, rogue.col);
	refresh();
	rogue_show_cursor(show_cursor);
}

void opt_show(int i)
{
	char *s;
	struct option *opt = &options[i];

	opt_erase(i);

	if (opt->is_bool) {
		s = *(opt->bval) ? ROGUE_TRUE : ROGUE_FALSE;
	} else if (opt_is_language(i)) {
		s = opt_language_display();
	} else {
		s = *(opt->strval);
	}
	/* Keep long names from spilling into the next option line. */
	rogue_addstr_fit(s, DCOLS - jrogue_text_width(opt_prompt(i)) - 1);
}

void opt_erase(int i)
{
	mvaddstr(i, 0, opt_prompt(i));
	clrtoeol();
}

void opt_go(int i)
{
	int col;

	col = jrogue_text_width(opt_prompt(i));
	if (opt_is_language(i)) {
		col += jrogue_text_width(opt_language_display());
	} else if ((!options[i].is_bool) && options[i].strval && *(options[i].strval)) {
		col += jrogue_text_width(*(options[i].strval));
	}
	if (col >= DCOLS) {
		col = DCOLS - 1;
	}
	move(i, col);
}

void rogue_redraw_screen(void)
{
	clear();
	if (cur_room == PASSAGE) {
		light_passage(rogue.row, rogue.col);
	} else if (cur_room != NO_ROOM) {
		light_up_room(cur_room);
	}
	if (detect_monster) {
		show_monsters();
	}
	mvaddch(rogue.row, rogue.col, rogue.fchar);
	print_stats(STAT_ALL);
	move(rogue.row, rogue.col);
	refresh();
}

void do_shell(void)
{
#ifdef _WIN32
	WINDOW *saved;
	int saved_row, saved_col;

	/* The shell must not erase previously explored terrain or its color
	 * attributes.  Keep a copy of the curses window, not a regenerated map. */
	saved = dupwin(stdscr);
	if (saved == NULL) {
		message(EN("cannot save screen for shell", "シェル用の画面を保存できません。"), 0);
		return;
	}
	getyx(stdscr, saved_row, saved_col);
	move(LINES - 1, 0);
	refresh();
	stop_window();
	/* Match the original shell message.  Write UTF-16 to the Windows
	 * console because curses may have restored a legacy output code page. */
	rogue_win_write_utf8(stdout, EN("\nCreating new shell...\n",
	                               "\n新しいシェルを起動します...\n"));
	/* COMSPEC is UTF-8 here; md_shell() converts it to UTF-16 and starts
	 * cmd.exe in our existing console, without creating a new window. */
	md_shell(md_getenv("COMSPEC"));

	/* Restore PDCurses modes after the child has finished.  The Windows
	 * console code page may have been changed by a shell command. */
	rogue_win_setup_console_encoding();
	reset_prog_mode();
	start_window();
	rogue_apply_color_mode();
	(void) overwrite(saved, stdscr);
	(void) delwin(saved);
	move(saved_row, saved_col);
	clearok(stdscr, TRUE);
	refresh();
	if (!show_cursor) {
		rogue_show_cursor(0);
	}
	/* stop_window() intentionally restored the normal Ctrl-C setting for
	 * cmd.exe. Reapply the game's wizard Ctrl-C binding after returning. */
	rogue_win_set_wizard_ctrl_c(wizard);
#elif defined(UNIX)
	char *sh;

	md_ignore_signals();
	if (!(sh = md_getenv("SHELL"))) {
		sh = "/bin/sh";
	}
	move(LINES-1, 0);
	refresh();
	stop_window();
	printf("%s", EN("\nCreating new shell...\n",
	          "\n新しいシェルを起動します...\n"));
	md_shell(sh);
	start_window();
	wrefresh(curscr);
	md_heed_signals();
#endif
}
