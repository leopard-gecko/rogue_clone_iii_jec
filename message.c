/*
 * message.c
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
static char sccsid[] __attribute__((unused)) = "@(#)message.c	5.1 (Berkeley) 11/25/87";
#endif /* not lint */

#include <stdio.h>
#include "rogue.h"

char msgs[NMESSAGES][ROGUE_DESC_LEN] = {"", "", "", "", ""};
short msg_col = 0, imsg = -1;
boolean msg_cleared = 1, rmsg = 0;
char hunger_str[8] = "";
char *more = EN_STATIC("-more-", " --続く-- ");

extern boolean cant_int, did_int, interrupted, save_is_interactive;
extern short add_strength;
extern short cur_level;

/* Repaint supplementary-plane glyphs after the Windows VT curses refresh. */
static void refresh_message_text(void)
{
    refresh();
#ifdef _WIN32
    rogue_win_vt_restore_nonbmp_rows(1);
#endif
}

void message(char *msg, boolean intrpt)
{
	cant_int = 1;

	if (!save_is_interactive) {
		return;
	}
	if (intrpt) {
		interrupted = 1;
		md_slurp();
	}

	if (!msg_cleared) {
		mvaddstr(MIN_ROW-1, msg_col, more);
#ifdef _WIN32
		/* PDCursesMod's UTF-8 rendering may leave its logical cursor at
		 * a different position from the number of terminal cells printed.
		 * Put the input cursor after the More prompt explicitly; otherwise
		 * it can appear at the extreme right edge of the message row. */
		{
			int after_more = msg_col + jrogue_text_width(more);
			if (after_more >= DCOLS - 1)
				after_more = DCOLS - 2;
			move(MIN_ROW - 1, after_more);
		}
#endif
		refresh_message_text();
		wait_for_ack();
		check_message();
	}
	if (!rmsg) {
		imsg = (imsg + 1) % NMESSAGES;
		(void) snprintf(msgs[imsg], ROGUE_DESC_LEN, "%s", msg);
	}
	move(MIN_ROW-1, 0);
	rogue_addstr_fit(msg, DCOLS - 1);
	addch(' ');
	refresh_message_text();
	msg_cleared = 0;
	msg_col = jrogue_text_width(msg);
#ifdef _WIN32
	/* Leave a free cell to the right of the More acknowledgement cursor.
	 * The final terminal column can wrap/scroll on some VT hosts. */
	if (msg_col > DCOLS - jrogue_text_width(more) - 2)
		msg_col = DCOLS - jrogue_text_width(more) - 2;
#else
	if (msg_col > (DCOLS - jrogue_text_width(more) - 1)) {
		msg_col = DCOLS - jrogue_text_width(more) - 1;
	}
#endif

	cant_int = 0;

	if (did_int) {
		did_int = 0;
		onintr(0);
	}
	if (!show_cursor) {
		rogue_show_cursor(0);
	}
}

void remessage(short c)
{
	if (imsg != -1) {
		check_message();
		rmsg = 1;
		while (c > imsg) {
			c -= NMESSAGES;
		}
		message(msgs[((imsg - c) % NMESSAGES)], 0);
		rmsg = 0;
		move(rogue.row, rogue.col);
		refresh_message_text();
	}
}

void check_message(void)
{
	if (msg_cleared) {
		return;
	}
	move(MIN_ROW-1, 0);
	clrtoeol();
	refresh_message_text();
	msg_cleared = 1;
	if (!show_cursor) {
		rogue_show_cursor(0);
	}
}

int get_input_line(char *prompt, char *insert, char *buf, char *if_cancelled, boolean add_blank, boolean do_echo)
{
	short ch;
	short i = 0, n;
	char uch[8];
	int ulen, k;

	message(prompt, 0);
	n = jrogue_text_width(prompt);

	if (insert[0]) {
		move(MIN_ROW - 1, n + 1);
		rogue_addstr_fit(insert, DCOLS - n - 2);
		(void) strcpy(buf, insert);
		i = strlen(insert);
		move(MIN_ROW-1, ((n + jrogue_text_width(buf) + 1 < DCOLS) ?
			(n + jrogue_text_width(buf) + 1) : DCOLS - 1));
		refresh_message_text();
	}

	rogue_show_cursor(1);

	while (((ch = rgetchar()) != '\r') && (ch != '\n') && (ch != CANCEL)) {
		if ((ch == '\b') || (ch == 0177)) {
			/* Never treat DEL as input, even when the line is empty.
			 * Otherwise curses renders the control byte as "^?". */
			if (i > 0) {
				i = (short) jrogue_text_prev(buf, (size_t) i);
				buf[i] = 0;
				if (do_echo) {
					move(MIN_ROW-1, n + 1);
					clrtoeol();
					rogue_addstr_fit(buf, DCOLS - n - 2);
				}
			}
		} else if ((ulen = rogue_read_utf8_char(ch, uch, sizeof(uch))) > 0) {
			if (((uch[0] != ' ') || (i > 0)) && ((i + ulen) < MAX_TITLE_LENGTH-2)) {
				for (k = 0; k < ulen; k++) {
					buf[i++] = uch[k];
				}
				buf[i] = 0;
				if (do_echo) {
					move(MIN_ROW-1, n + 1);
					clrtoeol();
					rogue_addstr_fit(buf, DCOLS - n - 2);
				}
			}
		}
		refresh_message_text();
	}
	check_message();
	rogue_show_cursor(show_cursor);
	if (add_blank) {
		buf[i++] = ' ';
	} else {
		while ((i > 0) && (buf[i-1] == ' ')) {
			i--;
		}
	}

	buf[i] = 0;

	if ((ch == CANCEL) || (i == 0) || ((i == 1) && add_blank)) {
		if (if_cancelled) {
			message(if_cancelled, 0);
		}
		return(0);
	}
	return(i);
}

int rogue_utf8_edit_char(int ch)
{
	return(((ch >= ' ') && (ch <= '~')) || ((ch & 0x80) != 0));
}

int rogue_read_utf8_char(int first, char *out, int out_size)
{
	int len, i, ch;
	unsigned char c;

	/* Input must be one Unicode scalar, never a control byte. */
	if (first < 0 || first > 255 || first < ' ' || first == 0x7f) {
		return(0);
	}
	c = (unsigned char) first;
	if (c < 0x80) len = 1;
	else if (c >= 0xc2 && c <= 0xdf) len = 2;
	else if (c >= 0xe0 && c <= 0xef) len = 3;
	else if (c >= 0xf0 && c <= 0xf4) len = 4;
	else return(0);

	if (!out || out_size <= len) return(0);
	out[0] = (char)c;
	for (i = 1; i < len; i++) {
		ch = rgetchar();
		if (ch < 0 || ch > 255 || ((ch & 0xc0) != 0x80)) {
			return(0);
		}
		out[i] = (char)ch;
	}
	if (jrogue_utf8_scalar_length(out, (size_t)len) != (size_t)len) {
		return(0);
	}
	out[len] = 0;
	return(len);
}

/* Draw at most max_cols terminal cells, never splitting a UTF-8 grapheme.
 * Keep the temporary string bounded as well (messages can be long). */
void rogue_addstr_fit(char *s, int max_cols)
{
	char buf[ROGUE_DESC_LEN];
	size_t at = 0, next;
	int width = 0, cells;

	if (!s || max_cols <= 0) {
		return;
	}
	while (s[at]) {
		next = jrogue_text_next(s, at);
		if (next <= at || next >= sizeof(buf)) {
			break;
		}
		cells = jrogue_text_width_n(s + at, next - at);
		if (cells > max_cols - width) {
			break;
		}
		width += cells;
		at = next;
	}
	memcpy(buf, s, at);
	buf[at] = 0;
#ifdef _WIN32
    /* Keep complete UTF-8 runs and supplementary scalars on the same
     * Windows output path as the 5.4.5 JEC implementation. */
    rogue_win_addstr_utf8(buf);
#else
	addstr(buf);
#endif
}

int rgetchar(void)
{
	register int ch;

	for(;;) {
#ifndef _WIN32
		/* Restore the same input attributes as Rogue 5.4.5 JEC. */
		rogue_prepare_input_attr(stdscr);
#endif
		ch = getchar();
		if (ch == EOF) {
			return(CANCEL);
		}

		switch(ch) {
		case '\022':
			wrefresh(curscr);
			break;
#ifdef UNIX_BSD4_2
		case '\032':
			printf(CL);
			fflush(stdout);
			tstp();
			break;
#endif
		case '&':
			save_screen();
			break;
		default:
			return(ch);
		}
	}
}
/*
Level: 99 Gold: 999999 Hp: 999(999) Str: 99(99) Arm: 99 Exp: 21/10000000 Hungry
0    5    1    5    2    5    3    5    4    5    5    5    6    5    7    5
*/

void print_stats(register int stat_mask)
{
	char buf[32];
	boolean label;
	int row = DROWS - 1;
char line[ROGUE_DESC_LEN];

	if (rogue_lang_is_japanese) {

	if (rogue.gold > MAX_GOLD) {
		rogue.gold = MAX_GOLD;
	}
	if (rogue.hp_max > MAX_HP) {
		rogue.hp_current -= (rogue.hp_max - MAX_HP);
		rogue.hp_max = MAX_HP;
	}
	if (rogue.str_max > MAX_STRENGTH) {
		rogue.str_current -= (rogue.str_max - MAX_STRENGTH);
		rogue.str_max = MAX_STRENGTH;
	}
	if (rogue.armor && (rogue.armor->d_enchant > MAX_ARMOR)) {
		rogue.armor->d_enchant = MAX_ARMOR;
	}
	if (rogue.exp_points > MAX_EXP) {
		rogue.exp_points = MAX_EXP;
	}
	if (rogue.exp > MAX_EXP_LEVEL) {
		rogue.exp = MAX_EXP_LEVEL;
	}
	if (hunger_str[0]) {
		(void) snprintf(line, sizeof(line),
			"%s%d  %s%ld    %s%d(%d)  %s%d(%d) %s%d  %s%d/%ld %s",
			ROGUE_STAT_LEVEL, cur_level,
			ROGUE_STAT_GOLD, rogue.gold,
			ROGUE_STAT_HP, rogue.hp_current, rogue.hp_max,
			ROGUE_STAT_STRENGTH, (rogue.str_current + add_strength), rogue.str_max,
			ROGUE_STAT_ARMOR, get_armor_class(rogue.armor),
			ROGUE_STAT_EXP, rogue.exp, rogue.exp_points,
			hunger_str);
	} else {
		(void) snprintf(line, sizeof(line),
			"%s%d  %s%ld    %s%d(%d)  %s%d(%d) %s%d  %s%d/%ld",
			ROGUE_STAT_LEVEL, cur_level,
			ROGUE_STAT_GOLD, rogue.gold,
			ROGUE_STAT_HP, rogue.hp_current, rogue.hp_max,
			ROGUE_STAT_STRENGTH, (rogue.str_current + add_strength), rogue.str_max,
			ROGUE_STAT_ARMOR, get_armor_class(rogue.armor),
			ROGUE_STAT_EXP, rogue.exp, rogue.exp_points);
	}
	move(row, 0);
	clrtoeol();
	rogue_addstr_fit(line, DCOLS - 1);
	refresh();
		return;
	}

	label = (stat_mask & STAT_LABEL) ? 1 : 0;
	if (label) {
		move(row, 0);
		clrtoeol();
	}

	if (stat_mask & STAT_LEVEL) {
		if (label) {
			mvaddstr(row, ROGUE_STAT_LEVEL_LABEL_COL, ROGUE_STAT_LEVEL);
		}
		/* max level taken care of in make_level() */
		sprintf(buf, "%d", cur_level);
		mvaddstr(row, ROGUE_STAT_LEVEL_VALUE_COL, buf);
		pad(buf, 2);
	}
	if (stat_mask & STAT_GOLD) {
		if (label) {
			mvaddstr(row, ROGUE_STAT_GOLD_LABEL_COL, ROGUE_STAT_GOLD);
		}
		if (rogue.gold > MAX_GOLD) {
			rogue.gold = MAX_GOLD;
		}
		sprintf(buf, "%ld", rogue.gold);
		mvaddstr(row, ROGUE_STAT_GOLD_VALUE_COL, buf);
		pad(buf, 6);
	}
	if (stat_mask & STAT_HP) {
		if (label) {
			mvaddstr(row, ROGUE_STAT_HP_LABEL_COL, ROGUE_STAT_HP);
		}
		if (rogue.hp_max > MAX_HP) {
			rogue.hp_current -= (rogue.hp_max - MAX_HP);
			rogue.hp_max = MAX_HP;
		}
		sprintf(buf, "%d(%d)", rogue.hp_current, rogue.hp_max);
		mvaddstr(row, ROGUE_STAT_HP_VALUE_COL, buf);
		pad(buf, 8);
	}
	if (stat_mask & STAT_STRENGTH) {
		if (label) {
			mvaddstr(row, ROGUE_STAT_STRENGTH_LABEL_COL, ROGUE_STAT_STRENGTH);
		}
		if (rogue.str_max > MAX_STRENGTH) {
			rogue.str_current -= (rogue.str_max - MAX_STRENGTH);
			rogue.str_max = MAX_STRENGTH;
		}
		sprintf(buf, "%d(%d)", (rogue.str_current + add_strength),
			rogue.str_max);
		mvaddstr(row, ROGUE_STAT_STRENGTH_VALUE_COL, buf);
		pad(buf, 6);
	}
	if (stat_mask & STAT_ARMOR) {
		if (label) {
			mvaddstr(row, ROGUE_STAT_ARMOR_LABEL_COL, ROGUE_STAT_ARMOR);
		}
		if (rogue.armor && (rogue.armor->d_enchant > MAX_ARMOR)) {
			rogue.armor->d_enchant = MAX_ARMOR;
		}
		sprintf(buf, "%d", get_armor_class(rogue.armor));
		mvaddstr(row, ROGUE_STAT_ARMOR_VALUE_COL, buf);
		pad(buf, 2);
	}
	if (stat_mask & STAT_EXP) {
		if (label) {
			mvaddstr(row, ROGUE_STAT_EXP_LABEL_COL, ROGUE_STAT_EXP);
		}
		if (rogue.exp_points > MAX_EXP) {
			rogue.exp_points = MAX_EXP;
		}
		if (rogue.exp > MAX_EXP_LEVEL) {
			rogue.exp = MAX_EXP_LEVEL;
		}
		sprintf(buf, "%d/%ld", rogue.exp, rogue.exp_points);
		mvaddstr(row, ROGUE_STAT_EXP_VALUE_COL, buf);
		pad(buf, 11);
	}
	if (stat_mask & STAT_HUNGER) {
		mvaddstr(row, 73, hunger_str);
		clrtoeol();
	}
	refresh();
}

void pad(char *s, short n)
{
	short i;

	for (i = strlen(s); i < n; i++) {
		addch(' ');
	}
}

void save_screen(void)
{
	FILE *fp;
	short i, j;
	char buf[DCOLS+2];
	boolean found_non_blank;

	if ((fp = fopen("rogue.screen", "w")) != NULL) {
		for (i = 0; i < DROWS; i++) {
			found_non_blank = 0;
			for (j = (DCOLS - 1); j >= 0; j--) {
				buf[j] = ROGUE_MVINCH_CHAR(i, j);
				if (!found_non_blank) {
					if ((buf[j] != ' ') || (j == 0)) {
						buf[j + ((j == 0) ? 0 : 1)] = 0;
						found_non_blank = 1;
					}
				}
			}
			fputs(buf, fp);
			putc('\n', fp);
		}
		fclose(fp);
	} else {
		sound_bell();
	}
}

void sound_bell(void)
{
	putchar(7);
	fflush(stdout);
}

boolean is_digit(short ch)
{
	return((ch >= '0') && (ch <= '9'));
}

int r_index(char *str, int ch, boolean last)
{
	int i = 0;

	if (last) {
		for (i = strlen(str) - 1; i >= 0; i--) {
			if (str[i] == ch) {
				return(i);
			}
		}
	} else {
		for (i = 0; str[i]; i++) {
			if (str[i] == ch) {
				return(i);
			}
		}
	}
	return(-1);
}
