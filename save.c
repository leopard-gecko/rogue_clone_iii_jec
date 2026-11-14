/*
 * save.c
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
static char sccsid[] __attribute__((unused)) = "@(#)save.c	5.1 (Berkeley) 11/25/87";
#endif /* not lint */

#include <stdio.h>
#include "rogue.h"

short write_failed = 0;
char *save_file = (char *) 0;

extern boolean detect_monster;
extern short cur_level, max_level;
extern char hunger_str[];
extern char login_name[];
extern short party_room;
extern short foods;
extern boolean is_wood[];
extern short cur_room;
extern boolean being_held;
extern short bear_trap;
extern short halluc;
extern short blind;
extern short confused;
extern short levitate;
extern short haste_self;
extern boolean see_invisible;
extern boolean detect_monster;
extern boolean wizard;
extern boolean score_only;
extern short m_moves;

extern boolean msg_cleared;

static void rw_generated_title_ids(FILE *fp, boolean wr);
static void restore_object_after_read(object *obj, boolean is_monster);
/* The saved screen uses one byte per cell, which cannot preserve UTF-8
 * filenames or curses color attributes.  Restore the message row from
 * the pathname rather than replaying its byte snapshot. */

void save_game(void)
{
	char fname[ROGUE_DESC_LEN];

	if (!get_input_line(EN("file name?", "ファイル名は？"), save_file, fname,
		EN("game not saved", "ゲームはセーブされませんでした。"),
			0, 1)) {
		return;
	}
	check_message();
	message(fname, 0);
	/* Keep the saved filename visible briefly before a successful save
	 * closes the curses screen.  This is only a display delay. */
	(void) napms(200);
	save_into_file(fname);
}

void save_into_file(char *sfile)
{
	FILE *fp;
	int file_id, n;
	char name_buffer[ROGUE_DESC_LEN];
	char *hptr;
	struct rogue_time rt_buf;

	if (sfile[0] == '~') {
		if ((hptr = md_getenv("HOME"))) {
			n = snprintf(name_buffer, sizeof(name_buffer), "%s%s", hptr, sfile+1);
			if ((n < 0) || (n >= (int) sizeof(name_buffer))) {
				message(EN("save file name is too long",
					"セーブファイル名が長すぎます。"), 0);
				return;
			}
			sfile = name_buffer;
		}
	}
	if (	((fp = fopen(sfile, "wb")) == NULL) ||
			((file_id = md_get_file_id(sfile)) == -1)) {
		message(EN("problem accessing the save file", "セーブファイルにアクセスできません。"), 0);
		return;
	}
	md_ignore_signals();
	write_failed = 0;
	(void) xxx(1);
	r_write(fp, (char *) &detect_monster, sizeof(detect_monster));
	r_write(fp, (char *) &cur_level, sizeof(cur_level));
	r_write(fp, (char *) &max_level, sizeof(max_level));
	write_string(hunger_str, fp);
	write_string(login_name, fp);
	r_write(fp, (char *) &party_room, sizeof(party_room));
	write_pack(&level_monsters, fp);
	write_pack(&level_objects, fp);
	r_write(fp, (char *) &file_id, sizeof(file_id));
	rw_dungeon(fp, 1);
	r_write(fp, (char *) &foods, sizeof(foods));
	r_write(fp, (char *) &rogue, sizeof(fighter));
	write_pack(&rogue.pack, fp);
	rw_id(id_potions, fp, POTIONS, 1);
	rw_id(id_scrolls, fp, SCROLS, 1);
	rw_id(id_wands, fp, WANDS, 1);
	rw_id(id_rings, fp, RINGS, 1);
	rw_generated_title_ids(fp, 1);
	r_write(fp, (char *) traps, (MAX_TRAPS * sizeof(trap)));
	r_write(fp, (char *) is_wood, (WANDS * sizeof(boolean)));
	r_write(fp, (char *) &cur_room, sizeof(cur_room));
	rw_rooms(fp, 1);
	r_write(fp, (char *) &being_held, sizeof(being_held));
	r_write(fp, (char *) &bear_trap, sizeof(bear_trap));
	r_write(fp, (char *) &halluc, sizeof(halluc));
	r_write(fp, (char *) &blind, sizeof(blind));
	r_write(fp, (char *) &confused, sizeof(confused));
	r_write(fp, (char *) &levitate, sizeof(levitate));
	r_write(fp, (char *) &haste_self, sizeof(haste_self));
	r_write(fp, (char *) &see_invisible, sizeof(see_invisible));
	r_write(fp, (char *) &detect_monster, sizeof(detect_monster));
	r_write(fp, (char *) &wizard, sizeof(wizard));
	r_write(fp, (char *) &score_only, sizeof(score_only));
	r_write(fp, (char *) &m_moves, sizeof(m_moves));
	write_string(rogue_language_code(), fp);
	md_gct(&rt_buf);
	rt_buf.second += 10;		/* allow for some processing time */
	r_write(fp, (char *) &rt_buf, sizeof(rt_buf));
	fclose(fp);

	if (write_failed) {
		(void) md_df(sfile);	/* delete file */
	} else {
		clean_up("");
	}
}

void restore(char *fname)
{
	FILE *fp;
	struct rogue_time saved_time, mod_time;
	char buf[4];
	char tbuf[MAX_OPT_LEN+1];
	char saved_lang[8];
	int new_file_id, saved_file_id;

	if (	((new_file_id = md_get_file_id(fname)) == -1) ||
			((fp = fopen(fname, "rb")) == NULL)) {
		clean_up(EN("cannot open file", "ファイルを開けません。"));
	}
	if (md_link_count(fname) > 1) {
		clean_up(EN("file has link", "ファイルにリンクがあります。"));
	}
	(void) xxx(1);
	r_read(fp, (char *) &detect_monster, sizeof(detect_monster));
	r_read(fp, (char *) &cur_level, sizeof(cur_level));
	r_read(fp, (char *) &max_level, sizeof(max_level));
	read_string(hunger_str, 8, fp);

	(void) snprintf(tbuf, sizeof(tbuf), "%s", login_name);
	read_string(login_name, MAX_OPT_LEN, fp);
	if (strcmp(tbuf, login_name)) {
		clean_up(EN("you're not the original player", "元のプレイヤーではありません。"));
	}

	r_read(fp, (char *) &party_room, sizeof(party_room));
	read_pack(&level_monsters, fp, 0, 1);
	read_pack(&level_objects, fp, 0, 0);
	r_read(fp, (char *) &saved_file_id, sizeof(saved_file_id));
	if (new_file_id != saved_file_id) {
		clean_up(EN("sorry, saved game is not in the same file",
			"保存されたゲームのファイルが一致しません。"));
	}
	/* The top message row is redrawn after the dungeon is restored. */
	rw_dungeon(fp, 0);
	r_read(fp, (char *) &foods, sizeof(foods));
	r_read(fp, (char *) &rogue, sizeof(fighter));
	read_pack(&rogue.pack, fp, 1, 0);
	rw_id(id_potions, fp, POTIONS, 0);
	rw_id(id_scrolls, fp, SCROLS, 0);
	rw_id(id_wands, fp, WANDS, 0);
	rw_id(id_rings, fp, RINGS, 0);
	rw_generated_title_ids(fp, 0);
	r_read(fp, (char *) traps, (MAX_TRAPS * sizeof(trap)));
	r_read(fp, (char *) is_wood, (WANDS * sizeof(boolean)));
	r_read(fp, (char *) &cur_room, sizeof(cur_room));
	rw_rooms(fp, 0);
	r_read(fp, (char *) &being_held, sizeof(being_held));
	r_read(fp, (char *) &bear_trap, sizeof(bear_trap));
	r_read(fp, (char *) &halluc, sizeof(halluc));
	r_read(fp, (char *) &blind, sizeof(blind));
	r_read(fp, (char *) &confused, sizeof(confused));
	r_read(fp, (char *) &levitate, sizeof(levitate));
	r_read(fp, (char *) &haste_self, sizeof(haste_self));
	r_read(fp, (char *) &see_invisible, sizeof(see_invisible));
	r_read(fp, (char *) &detect_monster, sizeof(detect_monster));
	r_read(fp, (char *) &wizard, sizeof(wizard));
	r_read(fp, (char *) &score_only, sizeof(score_only));
	r_read(fp, (char *) &m_moves, sizeof(m_moves));
	read_string(saved_lang, sizeof(saved_lang), fp);
	r_read(fp, (char *) &saved_time, sizeof(saved_time));

	if (fread(buf, sizeof(char), 1, fp) > 0) {
		clear();
		clean_up(EN("extra characters in file", "ファイルに余分な文字があります。"));
	}

	md_gfmt(fname, &mod_time);	/* get file modification time */

	if (has_been_touched(&saved_time, &mod_time)) {
		clear();
		clean_up(EN("sorry, file has been touched", "ファイルが変更されています。"));
	}
	/* Win32 cannot reliably remove an open CRT file.  Close the save
	 * after verifying the complete contents, but before removing it. */
	(void) fclose(fp);
	if ((!wizard) && !md_df(fname)) {
		clean_up(EN("cannot delete file", "ファイルを削除できません。"));
	}
	rogue_apply_language();
	print_stats(STAT_ALL);
	move(rogue.row, rogue.col);
	refresh();
	msg_cleared = 0;
	ring_stats(0);
	/* Repaint the entire save filename in the normal message color.
	 * Replaying individual saved screen bytes could retain black or other
	 * attributes on macOS curses, even for an ASCII filename. */
	rogue_reset_score_color();
	move(MIN_ROW - 1, 0);
	clrtoeol();
	msg_cleared = 1;
	message(fname, 0);
	move(rogue.row, rogue.col);
	refresh();
}

void write_pack(object *pack, FILE *fp)
{
	object t;

	while ((pack = pack->next_object)) {
		r_write(fp, (char *) pack, sizeof(object));
	}
	t.ichar = t.what_is = 0;
	r_write(fp, (char *) &t, sizeof(object));
}

void read_pack(object *pack, FILE *fp, boolean is_rogue, boolean is_monsters)
{
	object read_obj, *new_obj;

	for (;;) {
		r_read(fp, (char *) &read_obj, sizeof(object));
		if (read_obj.ichar == 0) {
			pack->next_object = (object *) 0;
			break;
		}
		new_obj = alloc_object();
		*new_obj = read_obj;
		restore_object_after_read(new_obj, is_monsters);
		if (is_rogue) {
			if (new_obj->in_use_flags & BEING_WORN) {
					do_wear(new_obj);
			} else if (new_obj->in_use_flags & BEING_WIELDED) {
					do_wield(new_obj);
			} else if (new_obj->in_use_flags & (ON_EITHER_HAND)) {
				do_put_on(new_obj,
					((new_obj->in_use_flags & ON_LEFT_HAND) ? 1 : 0));
			}
		}
		pack->next_object = new_obj;
		pack = new_obj;
	}
}


static char *weapon_damage_for_kind(short which_kind)
{
	switch(which_kind) {
	case BOW:
	case DART:
		return("1d1");
	case ARROW:
		return("1d2");
	case DAGGER:
		return("1d3");
	case SHURIKEN:
		return("1d4");
	case MACE:
		return("2d3");
	case LONG_SWORD:
		return("3d4");
	case TWO_HANDED_SWORD:
		return("4d5");
	}
	return("1d1");
}

static void restore_object_after_read(object *obj, boolean is_monster)
{
	short m;

	obj->next_object = (object *) 0;
	if (is_monster) {
		m = obj->m_char - 'A';
		if ((m >= 0) && (m < MONSTERS)) {
			obj->m_damage = mon_tab[m].m_damage;
		} else {
			obj->m_damage = "0d0";
		}
		return;
	}
	if (obj->what_is == WEAPON) {
		obj->damage = weapon_damage_for_kind(obj->which_kind);
	}
}

void rw_dungeon(FILE *fp, boolean rw)
{
	short i, j;
	char buf[DCOLS];

	for (i = 0; i < DROWS; i++) {
		if (rw) {
			r_write(fp, (char *) dungeon[i], (DCOLS * sizeof(dungeon[0][0])));
			for (j = 0; j < DCOLS; j++) {
				buf[j] = ROGUE_MVINCH_CHAR(i, j);
			}
			r_write(fp, buf, DCOLS);
		} else {
			r_read(fp, (char *) dungeon[i], (DCOLS * sizeof(dungeon[0][0])));
			r_read(fp, buf, DCOLS);
			if (i == MIN_ROW - 1)
				continue;
			for (j = 0; j < DCOLS; j++) {
				mvaddch(i, j, buf[j]);
			}
		}
	}
}

static void rw_generated_title_ids(FILE *fp, boolean wr)
{
	if (wr) {
		r_write(fp, (char *) potion_title_id, POTIONS * sizeof(short));
		r_write(fp, (char *) wand_material_id, WANDS * sizeof(short));
		r_write(fp, (char *) ring_gem_id, RINGS * sizeof(short));
		r_write(fp, (char *) scroll_title_syllable_count, SCROLS * sizeof(short));
		r_write(fp, (char *) scroll_title_syllables,
			SCROLS * MAX_SCROLL_TITLE_SYLLABLES * sizeof(short));
	} else {
		r_read(fp, (char *) potion_title_id, POTIONS * sizeof(short));
		r_read(fp, (char *) wand_material_id, WANDS * sizeof(short));
		r_read(fp, (char *) ring_gem_id, RINGS * sizeof(short));
		r_read(fp, (char *) scroll_title_syllable_count, SCROLS * sizeof(short));
		r_read(fp, (char *) scroll_title_syllables,
			SCROLS * MAX_SCROLL_TITLE_SYLLABLES * sizeof(short));
	}
}

void rw_id(struct id *id_table, FILE *fp, int n, boolean wr)
{
	short i;

	for (i = 0; i < n; i++) {
		if (wr) {
			r_write(fp, (char *) &(id_table[i].value), sizeof(short));
			r_write(fp, (char *) &(id_table[i].id_status),
				sizeof(unsigned short));
			write_string(id_table[i].title, fp);
		} else {
			r_read(fp, (char *) &(id_table[i].value), sizeof(short));
			r_read(fp, (char *) &(id_table[i].id_status),
				sizeof(unsigned short));
			read_string(id_table[i].title, ID_TITLE_SIZE, fp);
		}
	}
}

void write_string(char *s, FILE *fp)
{
	short n;
	char *buf;

	n = strlen(s) + 1;
	buf = (char *) malloc(n);
	if (buf == (char *) 0) {
		message(EN("out of memory", "メモリが足りません。"), 0);
		write_failed = 1;
		return;
	}
	(void) memcpy(buf, s, n);
	xxxx(buf, n);
	r_write(fp, (char *) &n, sizeof(short));
	r_write(fp, buf, n);
	free(buf);
}

void read_string(char *s, int maxlen, FILE *fp)
{
	short n;
	char discard[128];
	int remaining, chunk;

	r_read(fp, (char *) &n, sizeof(short));
	if (n <= 0) {
		if (maxlen > 0) {
			s[0] = '\0';
		}
		return;
	}
	if (n <= maxlen) {
		r_read(fp, s, n);
		xxxx(s, n);
	} else {
		r_read(fp, s, maxlen - 1);
		xxxx(s, maxlen - 1);
		s[maxlen - 1] = '\0';
		remaining = n - (maxlen - 1);
		while (remaining > 0) {
			chunk = (remaining > (int) sizeof(discard)) ? (int) sizeof(discard) : remaining;
			r_read(fp, discard, chunk);
			xxxx(discard, chunk);
			remaining -= chunk;
		}
	}
}

void rw_rooms(FILE *fp, boolean rw)
{
	short i;

	for (i = 0; i < MAXROOMS; i++) {
		rw ? r_write(fp, (char *) (rooms + i), sizeof(room)) :
			r_read(fp, (char *) (rooms + i), sizeof(room));
	}
}

void r_read(FILE *fp, char *buf, int n)
{
	if ((n < 0) || (fread(buf, sizeof(char), (size_t)n, fp) != (size_t)n)) {
		clean_up(EN("read() failed, don't know why", "読み込みに失敗しました。"));
	}
}

void r_write(FILE *fp, char *buf, int n)
{
	if (!write_failed) {
		if ((n < 0) || (fwrite(buf, sizeof(char), (size_t)n, fp) != (size_t)n)) {
			message(EN("write() failed, don't know why", "書き込みに失敗しました。"), 0);
			sound_bell();
			write_failed = 1;
		}
	}
}

boolean has_been_touched(struct rogue_time *saved_time, struct rogue_time *mod_time)
{
	if (saved_time->year < mod_time->year) {
		return(1);
	} else if (saved_time->year > mod_time->year) {
		return(0);
	}
	if (saved_time->month < mod_time->month) {
		return(1);
	} else if (saved_time->month > mod_time->month) {
		return(0);
	}
	if (saved_time->day < mod_time->day) {
		return(1);
	} else if (saved_time->day > mod_time->day) {
		return(0);
	}
	if (saved_time->hour < mod_time->hour) {
		return(1);
	} else if (saved_time->hour > mod_time->hour) {
		return(0);
	}
	if (saved_time->minute < mod_time->minute) {
		return(1);
	} else if (saved_time->minute > mod_time->minute) {
		return(0);
	}
	if (saved_time->second < mod_time->second) {
		return(1);
	}
	return(0);
}
