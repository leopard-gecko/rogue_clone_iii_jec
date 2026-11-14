/*
 * score.c
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
static char sccsid[] __attribute__((unused)) = "@(#)score.c	5.2 (Berkeley) 11/25/87";
#endif /* not lint */

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif
#include "rogue.h"

#define OLD_SCORE_LINE_LEN 80
#define SCORE_RECORD_LEN (SCORE_LINE_LEN + SCORE_NAME_LEN)
#define OLD_SCORE_RECORD_LEN (OLD_SCORE_LINE_LEN + SCORE_NAME_LEN)

extern char login_name[];
extern char *m_names[];
static void format_score_reason(char *buf, int len, int other, int level, int mchar, int amulet);
extern short max_level;
extern boolean score_only, no_skull, msg_cleared;
extern char *byebye_string, *nick_name;

static void score_cat(char *dest, int len, char *src)
{
	size_t used = strlen(dest);

	if ((int) used >= len - 1) {
		return;
	}
	(void) snprintf(dest + used, len - used, "%s", src);
}


static void skull_addstr(short row, short col, char *s)
{
	short i;

	mvaddstr(row, col, s);
	for (i = 0; s[i]; i++) {
		if ((s[i] == 'X') || (s[i] == '@')) {
			rogue_mvaddch_skull_dark(row, col + i, s[i]);
		}
	}
}

static void pad_score_line(char *score)
{
	size_t used;

	score[SCORE_LINE_LEN] = 0;
	used = strlen(score);
	if (used >= SCORE_LINE_LEN) {
		score[SCORE_LINE_LEN - 1] = 0;
		used = SCORE_LINE_LEN - 1;
	}
	for (; used < SCORE_LINE_LEN - 1; used++) {
		score[used] = ' ';
	}
	score[SCORE_LINE_LEN - 1] = 0;
}

void killed_by(object *monster, short other)
{
	char buf[ROGUE_DESC_LEN];
	boolean show_skull;

	md_ignore_signals();

	if (other != QUIT) {
		rogue.gold = ((rogue.gold * 9) / 10);
	}

	if (other) {
		switch(other) {
		case HYPOTHERMIA:
			(void) strcpy(buf, EN("died of hypothermia", "寒さにより死す"));
			break;
		case STARVATION:
			(void) strcpy(buf, EN("died of starvation", "飢えにより死す"));
			break;
		case POISON_DART:
			(void) strcpy(buf, EN("killed by a dart", "毒矢により死す"));
			break;
		case QUIT:
			(void) strcpy(buf, EN("quit", "逃亡す"));
			break;
		case KFIRE:
			(void) strcpy(buf, EN("killed by fire", "炎により死す"));
			break;
		}
	} else {
if (rogue_lang_is_japanese) {
			(void) snprintf(buf, sizeof(buf), "%s", m_names[monster->m_char - 'A']);
			score_cat(buf, sizeof(buf), "と戦いて死す");
		} else {
			(void) snprintf(buf, sizeof(buf), "%s", "Killed by ");
			if (is_vowel(m_names[monster->m_char - 'A'][0])) {
				score_cat(buf, sizeof(buf), "an ");
			} else {
				score_cat(buf, sizeof(buf), "a ");
			}
			score_cat(buf, sizeof(buf), m_names[monster->m_char - 'A']);
		}
	}
	score_cat(buf, sizeof(buf), EN(" with ", "。 所持金は"));
	(void) snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf),
		EN("%ld gold", "＄%ld"), rogue.gold);
	show_skull = ((other != QUIT) && (!no_skull));
	if (show_skull) {
		check_message();
		clear();
		mvaddstr(4, 32, "__---------__");
		mvaddstr(5, 30, "_~             ~_");
		mvaddstr(6, 29, "/                 \\");
		mvaddstr(7, 28, "~                   ~");
		mvaddstr(8, 27, "/                     \\");
		skull_addstr(9, 27, "|    XXXX     XXXX    |");
		skull_addstr(10, 27, "|    XXXX     XXXX    |");
		skull_addstr(11, 27, "|    XXX       XXX    |");
		skull_addstr(12, 28, "\\         @         /");
		skull_addstr(13, 29, "--\\     @@@     /--");
		skull_addstr(14, 30, "| |    @@@    | |");
		mvaddstr(15, 30, "| |           | |");
		mvaddstr(16, 30, "| vvVvvvvvvvVvv |");
		mvaddstr(17, 30, "|  ^^^^^^^^^^^  |");
		mvaddstr(18, 31, "\\_           _/");
		mvaddstr(19, 33, "~---------~");
		center(21, nick_name);
		center(22, buf);
		move(DROWS-1, 0);
		clrtoeol();
		rogue_addstr_fit(press_space, DCOLS - 1);
		md_slurp();
		refresh();
		wait_for_ack();
		msg_cleared = 1;
	} else {
		message(buf, 0);
	}
	if (!show_skull) {
		message("", 0);
	}
	put_scores(monster, other);
}

void win(void)
{
	short message_col = (rogue_lang_is_japanese ? 2 : 11);

	unwield(rogue.weapon);		/* disarm and relax */
	unwear(rogue.armor);
	un_put_on(rogue.left_ring);
	un_put_on(rogue.right_ring);

	clear();
	rogue_set_score_header_color();
	mvaddstr(10, 11, "@   @  @@@   @   @      @  @  @   @@@   @   @   @");
	mvaddstr(11, 11, " @ @  @   @  @   @      @  @  @  @   @  @@  @   @");
	mvaddstr(12, 11, "  @   @   @  @   @      @  @  @  @   @  @ @ @   @");
	mvaddstr(13, 11, "  @   @   @  @   @      @  @  @  @   @  @  @@");
	mvaddstr(14, 11, "  @    @@@    @@@        @@ @@    @@@   @   @   @");
	rogue_set_score_title_color();
	mvaddstr(17, message_col, EN("Congratulations,  you have  been admitted  to  the",
		"おめでとう！君は戦士ギルドの一員として認められた。"));
	mvaddstr(18, message_col, EN("Fighters' Guild.   You return home,  sell all your",
		"故郷へ帰り、すべての宝物を素晴らしい値段で売却し、"));
	mvaddstr(19, message_col, EN("treasures at great profit and retire into comfort.",
		"悠々自適な暮らしを手に入れたということである・・・"));
	rogue_reset_score_color();
	message("", 0);
	message("", 0);
	id_all();
	sell_pack();
	put_scores((object *) 0, WIN);
}

void quit(boolean from_intrpt)
{
	char buf[ROGUE_DESC_LEN];
	short i, orow, ocol;
	boolean mc;

	md_ignore_signals();

	if (from_intrpt) {
		orow = rogue.row;
		ocol = rogue.col;

		mc = msg_cleared;

		for (i = 0; i < DCOLS; i++) {
			buf[i] = ROGUE_MVINCH_CHAR(0, i);
		}
	}
	check_message();
	message(EN("really quit?", "本当にやめますか？"), 1);
	rogue_show_cursor(1);
	if (rgetchar() != 'y') {
		rogue_show_cursor(show_cursor);
		md_heed_signals();
		check_message();
		if (from_intrpt) {
			for (i = 0; i < DCOLS; i++) {
				mvaddch(0, i, buf[i]);
			}
			msg_cleared = mc;
			move(orow, ocol);
			refresh();
		}
		return;
	}
	rogue_show_cursor(show_cursor);
	if (from_intrpt) {
		clean_up(byebye_string);
	}
	check_message();
	killed_by((object *) 0, QUIT);
}

void put_scores(object *monster, short other)
{
	short i, n, rank = 10, x, ne = 0, found_player = -1;
	char scores[10][SCORE_LINE_LEN+2];
	char n_names[10][SCORE_NAME_LEN+1];
	char buf[ROGUE_DESC_LEN];
	FILE *fp;
	long s, file_size;
	int score_line_len = SCORE_LINE_LEN;
	boolean pause = score_only;

	md_lock(1);

	/*
	 * Do not use "a+" here.  Modern Unix/macOS append mode forces all
	 * writes to the end of the file, even after rewind(), which corrupts
	 * the fixed-size score file after the first rewrite.
	 */
	if ((fp = fopen(SCORE_FILE, "r+b")) == NULL) {
		if ((fp = fopen(SCORE_FILE, "w+b")) == NULL) {
			message(EN("cannot read/write/create score file", "得点ファイルがオープンできません。"), 0);
			sf_error();
		}
	}
	if (fseek(fp, 0L, SEEK_END) == 0) {
		file_size = ftell(fp);
		if (file_size > 0) {
			if ((file_size % SCORE_RECORD_LEN) == 0) {
				score_line_len = SCORE_LINE_LEN;
			} else if ((file_size % OLD_SCORE_RECORD_LEN) == 0) {
				score_line_len = OLD_SCORE_LINE_LEN;
			} else {
				sf_error();
			}
		}
	}
	rewind(fp);
	(void) xxx(1);

	for (i = 0; i < 10; i++) {
		(void) memset(scores[i], 0, sizeof(scores[i]));
		if (((n = fread(scores[i], sizeof(char), score_line_len, fp)) < score_line_len) && (n != 0)) {
			sf_error();
		} else if (n != 0) {
			xxxx(scores[i], score_line_len);
			scores[i][score_line_len] = 0;
			if ((n = fread(n_names[i], sizeof(char), SCORE_NAME_LEN, fp)) < SCORE_NAME_LEN) {
				sf_error();
			}
			xxxx(n_names[i], SCORE_NAME_LEN);
			n_names[i][SCORE_NAME_LEN] = 0;
		} else {
			break;
		}
		ne++;
#ifndef ALLSCORES
		if ((!score_only) && (found_player == -1)) {
			if (!name_cmp(scores[i]+15, login_name)) {
				x = 5;
				while (scores[i][x] == ' ') {
					x++;
				}
				s = lget_number(scores[i] + x);
				if (rogue.gold < s) {
					score_only = 1;
				} else {
					found_player = i;
				}
			}
		}
#endif
	}
	if (found_player != -1) {
		ne--;
		for (i = found_player; i < ne; i++) {
			(void) strcpy(scores[i], scores[i+1]);
			(void) strcpy(n_names[i], n_names[i+1]);
		}
	}
	if (!score_only) {
		for (i = 0; i < ne; i++) {
			x = 5;
			while (scores[i][x] == ' ') {
				x++;
			}
			s = lget_number(scores[i] + x);

			if (rogue.gold >= s) {
				rank = i;
				break;
			}
		}
		if (ne == 0) {
			rank = 0;
		} else if ((ne < 10) && (rank == 10)) {
			rank = ne;
		}
		if (rank < 10) {
			insert_score(scores, n_names, nick_name, rank, ne, monster,
				other);
			if (ne < 10) {
				ne++;
			}
		}
		rewind(fp);
	}

	clear();
	rogue_set_score_title_color();
	mvaddstr(3, 30, EN("Top  Ten  Rogueists", "最 高 得 点 者"));
	rogue_reset_score_color();
	rogue_set_score_header_color();
	mvaddstr(8, 0, EN("Rank   Score   Name", "順位    得点   名前"));
	rogue_reset_score_color();

	md_ignore_signals();

	(void) xxx(1);

	for (i = 0; i < ne; i++) {
		if (i == rank) {
			rogue_set_score_reverse_color();
		}
		if (i == 9) {
			scores[i][0] = '1';
			scores[i][1] = '0';
		} else {
			scores[i][0] = ' ';
			scores[i][1] = i + '1';
			}
			nickize(buf, scores[i], n_names[i]);
			move(i+10, 0);
			rogue_addstr_fit(buf, DCOLS);
			if (rank < 10) {
				pad_score_line(scores[i]);
				xxxx(scores[i], SCORE_LINE_LEN);
				if (fwrite(scores[i], sizeof(char), SCORE_LINE_LEN, fp) != SCORE_LINE_LEN) {
					sf_error();
				}
				xxxx(n_names[i], SCORE_NAME_LEN);
			if (fwrite(n_names[i], sizeof(char), SCORE_NAME_LEN, fp) != SCORE_NAME_LEN) {
				sf_error();
			}
		}
		if (i == rank) {
			rogue_end_score_reverse_color();
		}
	}
	if (rank < 10) {
		fflush(fp);
		#ifdef _WIN32
		(void) _chsize_s(_fileno(fp), (long long) ne * SCORE_RECORD_LEN);
#else
		(void) ftruncate(fileno(fp), (off_t) ne * SCORE_RECORD_LEN);
#endif
	}
	md_lock(0);
	refresh();
	fclose(fp);
	move(DROWS-1, 0);
	clrtoeol();
	rogue_addstr_fit(press_space, DCOLS - 1);
	refresh();
	wait_for_ack();
	(void) pause;
	clean_up("");
}

/* Keep the original fixed-size score field, but never cut in the middle
 * of a grapheme (including combining accents or emoji sequences). */
static void copy_score_name_utf8(char *dest, const char *src)
{
	size_t bytes;

	if (!src) {
		dest[0] = 0;
		return;
	}
	bytes = jrogue_text_prefix(src, SCORE_NAME_LEN);
	memcpy(dest, src, bytes);
	dest[bytes] = 0;
}

void insert_score(char scores[][SCORE_LINE_LEN+2], char n_names[][SCORE_NAME_LEN+1], char *n_name, short rank, short n, object *monster, int other)
{
	short i;
	char buf[ROGUE_DESC_LEN];

	if (n > 0) {
		for (i = n; i > rank; i--) {
			if ((i < 10) && (i > 0)) {
				(void) strcpy(scores[i], scores[i-1]);
				(void) strcpy(n_names[i], n_names[i-1]);
			}
		}
	}
	(void) snprintf(buf, sizeof(buf), "%2d    %6ld   %s: @RC3:%d:%d:%c:%d",
		rank+1, rogue.gold, login_name, other, max_level,
		(other ? '-' : monster->m_char), ((other != WIN) && has_amulet()) ? 1 : 0);
	for (i = strlen(buf); i < SCORE_LINE_LEN - 1; i++) {
		buf[i] = ' ';
	}
	buf[SCORE_LINE_LEN - 1] = 0;
	(void) strcpy(scores[rank], buf);
	(void) memset(n_names[rank], 0, SCORE_NAME_LEN + 1);
	copy_score_name_utf8(n_names[rank], n_name);
}

int is_vowel(short ch)
{
	return( (ch == 'a') ||
		(ch == 'e') ||
		(ch == 'i') ||
		(ch == 'o') ||
		(ch == 'u') );
}

void sell_pack(void)
{
	object *obj;
	short row = 2, val;
	char buf[ROGUE_DESC_LEN];

	obj = rogue.pack.next_object;

	clear();
	mvaddstr(1, 0, EN("Value      Item", " 価格      持ちもの"));

	while (obj) {
		if (obj->what_is != FOOD) {
			obj->identified = 1;
			val = get_value(obj);
			rogue.gold += val;

			if (row < DROWS) {
				sprintf(buf, "%5d      ", val);
				get_desc_into(obj, buf+11, ROGUE_DESC_LEN-11);
				mvaddstr(row++, 0, buf);
			}
		}
		obj = obj->next_object;
	}
	refresh();
	if (rogue.gold > MAX_GOLD) {
		rogue.gold = MAX_GOLD;
	}
	message("", 0);
}

int get_value(object *obj)
{
	short wc;
	int val = 10;

	wc = obj->which_kind;

	switch(obj->what_is) {
	case WEAPON:
		val = id_weapons[wc].value;
		if ((wc == ARROW) || (wc == DAGGER) || (wc == SHURIKEN) ||
			(wc == DART)) {
			val *= obj->quantity;
		}
		val += (obj->d_enchant * 85);
		val += (obj->hit_enchant * 85);
		break;
	case ARMOR:
		val = id_armors[wc].value;
		val += (obj->d_enchant * 75);
		if (obj->is_protected) {
			val += 200;
		}
		break;
	case WAND:
		val = id_wands[wc].value * (obj->class + 1);
		break;
	case SCROL:
		val = id_scrolls[wc].value * obj->quantity;
		break;
	case POTION:
		val = id_potions[wc].value * obj->quantity;
		break;
	case AMULET:
		val = 5000;
		break;
	case RING:
		val = id_rings[wc].value * (obj->class + 1);
		break;
	}
	if (val <= 0) {
		val = 10;
	}
	return(val);
}

void id_all(void)
{
	short i;

	for (i = 0; i < SCROLS; i++) {
		id_scrolls[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < WEAPONS; i++) {
		id_weapons[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < ARMORS; i++) {
		id_armors[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < WANDS; i++) {
		id_wands[i].id_status = IDENTIFIED;
	}
	for (i = 0; i < POTIONS; i++) {
		id_potions[i].id_status = IDENTIFIED;
	}
}

int name_cmp(char *s1, char *s2)
{
	short i = 0;
	int r;

	while(s1[i] != ':') {
		i++;
	}
	s1[i] = 0;
	r = strcmp(s1, s2);
	s1[i] = ':';
	return(r);
}

void xxxx(char *buf, short n)
{
	short i;
	unsigned char c;

	for (i = 0; i < n; i++) {

		/* It does not matter if accuracy is lost during this assignment */
		c = (unsigned char) xxx(0);

		buf[i] ^= c;
	}
}

long xxx(boolean st)
{
	static long f, s;
	long r;

	if (st) {
		f = 37;
		s = 7;
		return(0L);
	}
	r = ((f * s) + 9337) % 8887;
	f = s;
	s = r;
	return(r);
}

void nickize(char *buf, char *score, char *n_name)
{
	short i = 15, j;
	char *reason;
	int other, level, amulet;
	char mchar;
	char rbuf[ROGUE_DESC_LEN];

	while ((i < SCORE_LINE_LEN) && score[i] && (score[i] != ':')) {
		i++;
	}
	if ((i >= SCORE_LINE_LEN) || !score[i]) {
		(void) snprintf(buf, ROGUE_DESC_LEN, "%.*s", ROGUE_DESC_LEN - 1, score);
		return;
	}

	reason = score + i + 1;
	while (*reason == ' ') {
		reason++;
	}
	if (!strncmp(reason, "@RC3:", 5)) {
		other = level = amulet = 0;
		mchar = 0;
		if (sscanf(reason, "@RC3:%d:%d:%c:%d", &other, &level, &mchar, &amulet) >= 3) {
			format_score_reason(rbuf, sizeof(rbuf), other, level, mchar, amulet);
			(void) snprintf(buf, ROGUE_DESC_LEN, "%.*s%s: %s", 15, score,
				(n_name[0] ? n_name : score + 15), rbuf);
			buf[ROGUE_DESC_LEN - 1] = 0;
			return;
		}
	}

	if (!n_name[0]) {
		(void) snprintf(buf, ROGUE_DESC_LEN, "%s", score);
	} else {
		(void) snprintf(buf, ROGUE_DESC_LEN, "%.*s%s", 15, score, n_name);
		j = strlen(buf);
		while ((i < SCORE_LINE_LEN) && score[i] && (j < ROGUE_DESC_LEN - 1)) {
			buf[j++] = score[i++];
		}
		buf[j] = 0;
		buf[ROGUE_DESC_LEN - 1] = 0;
	}
}

static void format_score_reason(char *buf, int len, int other, int level, int mchar, int amulet)
{
	buf[0] = 0;
	if (rogue_lang_is_japanese) {
		(void) snprintf(buf, len, "地下%d階にて", level);
		if (other) {
			switch(other) {
			case HYPOTHERMIA: score_cat(buf, len, "寒さにより死す"); break;
			case STARVATION: score_cat(buf, len, "飢えにより死す"); break;
			case POISON_DART: score_cat(buf, len, "毒矢により死す"); break;
			case QUIT: score_cat(buf, len, "逃亡す"); break;
			case WIN: score_cat(buf, len, "勝利者"); break;
			case KFIRE: score_cat(buf, len, "炎により死す"); break;
			}
		} else if ((mchar >= 'A') && (mchar <= 'Z')) {
			score_cat(buf, len, m_names[mchar - 'A']);
			score_cat(buf, len, "と戦いて死す");
		}
		if (amulet && (other != WIN)) {
			score_cat(buf, len, " 魔除けを手に");
		}
	} else {
		if (other) {
			switch(other) {
			case HYPOTHERMIA: score_cat(buf, len, "died of hypothermia"); break;
			case STARVATION: score_cat(buf, len, "died of starvation"); break;
			case POISON_DART: score_cat(buf, len, "killed by a dart"); break;
			case QUIT: score_cat(buf, len, "quit"); break;
			case WIN: score_cat(buf, len, "a total winner"); break;
			case KFIRE: score_cat(buf, len, "killed by fire"); break;
			}
		} else if ((mchar >= 'A') && (mchar <= 'Z')) {
			score_cat(buf, len, "killed by ");
			if (is_vowel(m_names[mchar - 'A'][0])) {
				score_cat(buf, len, "an ");
			} else {
				score_cat(buf, len, "a ");
			}
			score_cat(buf, len, m_names[mchar - 'A']);
		}
		(void) snprintf(buf + strlen(buf), len - strlen(buf), " on level %d ", level);
		if (amulet && (other != WIN)) {
			score_cat(buf, len, "with amulet");
		}
	}
}

void center(short row, char *buf)
{
	short margin;

	margin = ((DCOLS - jrogue_text_width(buf)) / 2);
	if (margin < 0) {
		margin = 0;
	}
	move(row, margin);
	rogue_addstr_fit(buf, DCOLS - margin);
}

void sf_error(void)
{
	md_lock(0);
	message("", 1);
	clean_up(EN("sorry, score file is out of order", "得点ファイルが正しくありません。"));
}
