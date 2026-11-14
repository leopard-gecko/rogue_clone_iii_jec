/*
 * use.c
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
static char sccsid[] __attribute__((unused)) = "@(#)use.c	5.1 (Berkeley) 11/25/87";
#endif /* not lint */

#include "rogue.h"

short halluc = 0;
short blind = 0;
short confused = 0;
short levitate = 0;
short haste_self = 0;
boolean see_invisible = 0;
short extra_hp = 0;
boolean detect_monster = 0;
boolean con_mon = 0;
char *strange_feeling = EN_STATIC("you have a strange feeling for a moment, then it passes",
	"不思議な気分におそわれたが、すぐに消えていった。");

extern short bear_trap;
extern char hunger_str[];
extern short cur_room;
extern long level_points[];
extern boolean being_held;
extern char *fruit, *you_can_move_again;
extern boolean sustain_strength;

void quaff(void)
{
	short ch;
	char buf[ROGUE_DESC_LEN];
	object *obj;

	ch = pack_letter(EN("quaff what?", "飲む水薬は？"), POTION);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(EN("no such item.", "それは持っていない。"), 0);
		return;
	}
	if (obj->what_is != POTION) {
		message(EN("you can't drink that", "それは飲めない。"), 0);
		return;
	}
	switch(obj->which_kind) {
		case INCREASE_STRENGTH:
			message(EN("you feel stronger now, what bulging muscles!",
				"強くなった気がするぞ。このはちきれそうな筋肉！"),
			0);
			rogue.str_current++;
			if (rogue.str_current > rogue.str_max) {
				rogue.str_max = rogue.str_current;
			}
			break;
		case RESTORE_STRENGTH:
			rogue.str_current = rogue.str_max;
			message(EN("this tastes great, you feel warm all over",
				"これはうまい。身体中があたたまる。"), 0);
			break;
		case HEALING:
			message(EN("you begin to feel better", "気分がよくなってきた。"), 0);
			potion_heal(0);
			break;
		case EXTRA_HEALING:
			message(EN("you begin to feel much better", "とても気分がよくなってきた。"), 0);
			potion_heal(1);
			break;
		case POISON:
			if (!sustain_strength) {
				rogue.str_current -= get_rand(1, 3);
				if (rogue.str_current < 1) {
					rogue.str_current = 1;
				}
			}
			message(EN("you feel very sick now", "すごく具合が悪くなった。"), 0);
			if (halluc) {
				unhallucinate();
			}
			break;
		case RAISE_LEVEL:
			rogue.exp_points = level_points[rogue.exp - 1];
			message(EN("you suddenly feel much more skillful",
				"突然に腕前がすごく上がったようだ。"), 0);
			add_exp(1, 1);
			break;
		case BLINDNESS:
			go_blind();
			break;
		case HALLUCINATION:
			message(EN("oh wow, everything seems so cosmic",
				"わお！ 何もかもが宇宙っぽく見える！"), 0);
			halluc += get_rand(500, 800);
			break;
		case DETECT_MONSTER:
			show_monsters();
			if (!(level_monsters.next_monster)) {
				message(strange_feeling, 0);
			}
			break;
		case DETECT_OBJECTS:
			if (level_objects.next_object) {
				if (!blind) {
					show_objects();
				}
			} else {
				message(strange_feeling, 0);
			}
			break;
		case CONFUSION:
			message((halluc ? EN("what a trippy feeling", "トリップしている気分だ！") :
			EN("you feel confused", "ちょっと待った、何が起こったんだ？え？なに？だれ？")), 0);
			cnfs();
			break;
		case LEVITATION:
			message(EN("you start to float in the air", "体が宙に浮き始めた。"), 0);
			levitate += get_rand(15, 30);
			being_held = bear_trap = 0;
			break;
		case HASTE_SELF:
			message(EN("you feel yourself moving much faster", "動きが素早くなったようだ。"), 0);
			haste_self += get_rand(11, 21);
			if (!(haste_self % 2)) {
				haste_self++;
			}
			break;
		case SEE_INVISIBLE:
			sprintf(buf, EN("hmm, this potion tastes like %sjuice",
				"う〜む、%sジュースのような味がする。"), fruit);
			message(buf, 0);
			if (blind) {
				unblind();
			}
			see_invisible = 1;
			relight();
			break;
	}
	print_stats((STAT_STRENGTH | STAT_HP));
	if (id_potions[obj->which_kind].id_status != CALLED) {
		id_potions[obj->which_kind].id_status = IDENTIFIED;
	}
	vanish(obj, 1, &rogue.pack);
}

void read_scroll(void)
{
	short ch;
	object *obj;
	char msg[ROGUE_DESC_LEN];

	ch = pack_letter(EN("read what?", "読む巻き物は？"), SCROL);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(EN("no such item.", "それは持っていない。"), 0);
		return;
	}
	if (obj->what_is != SCROL) {
		message(EN("you can't read that", "それは読めない。"), 0);
		return;
	}
	switch(obj->which_kind) {
		case SCARE_MONSTER:
			message(EN("you hear a maniacal laughter in the distance",
				"遠くから、狂ったような笑い声が聞こえてくる。"),
			0);
			break;
		case HOLD_MONSTER:
			hold_monster();
			break;
		case ENCH_WEAPON:
			if (rogue.weapon) {
				if (rogue.weapon->what_is == WEAPON) {
if (rogue_lang_is_japanese) {
						sprintf(msg, "手に持った%sが、少しのあいだ%s輝きに包まれた。",
							name_of(rogue.weapon), get_ench_color());
					} else {
						sprintf(msg, "your %sglow%s %sfor a moment",
							name_of(rogue.weapon),
							((rogue.weapon->quantity <= 1) ? "s" : ""),
							get_ench_color());
					}
					message(msg, 0);
					if (coin_toss()) {
						rogue.weapon->hit_enchant++;
					} else {
						rogue.weapon->d_enchant++;
					}
				}
				rogue.weapon->is_cursed = 0;
			} else {
				message(EN("your hands tingle", "手がぴりぴりする。"), 0);
			}
			break;
		case ENCH_ARMOR:
			if (rogue.armor) {
				sprintf(msg, EN("your armor glows %sfor a moment",
					"着ているよろいが、少しのあいだ%s輝きに包まれた。"),
				get_ench_color());
				message(msg, 0);
				rogue.armor->d_enchant++;
				rogue.armor->is_cursed = 0;
				print_stats(STAT_ARMOR);
			} else {
				message(EN("your skin crawls", "身の毛がよだった。"), 0);
			}
			break;
		case IDENTIFY:
			message(EN("this is a scroll of identify", "これは持ちものを調べる巻き物だった。"), 0);
			obj->identified = 1;
			id_scrolls[obj->which_kind].id_status = IDENTIFIED;
			idntfy();
			break;
		case TELEPORT:
			tele();
			break;
		case SLEEP:
			message(EN("you fall asleep", "知らないうちに眠り込んでしまった。"), 0);
			take_a_nap();
			break;
		case PROTECT_ARMOR:
			if (rogue.armor) {
				message(EN("your armor is covered by a shimmering gold shield",
					"着ているよろいは、輝く金色のシールドに包まれた。"),0);
				rogue.armor->is_protected = 1;
				rogue.armor->is_cursed = 0;
			} else {
				message(EN("your acne seems to have disappeared",
					"ニキビが消えたようだ。"), 0);
			}
			break;
		case REMOVE_CURSE:
				message((!halluc) ?
					EN("you feel as though someone is watching over you",
						"誰かに見守られているような気がした。") :
					EN("you feel in touch with the universal oneness",
						"宇宙の一体性に触れた気がした。"), 0);
			uncurse_all();
			break;
		case CREATE_MONSTER:
			create_monster();
			break;
		case AGGRAVATE_MONSTER:
			aggravate();
			break;
		case MAGIC_MAPPING:
			message(EN("this scroll seems to have a map on it",
				"この巻き物には地図が描かれているようだ。"), 0);
			draw_magic_map();
			break;
		case CON_MON:
			con_mon = 1;
			sprintf(msg, EN("your hands glow %sfor a moment",
				"手が少しのあいだ%s輝きに包まれた。"), get_ench_color());
			message(msg, 0);
			break;
	}
	if (id_scrolls[obj->which_kind].id_status != CALLED) {
		id_scrolls[obj->which_kind].id_status = IDENTIFIED;
	}
	vanish(obj, (obj->which_kind != SLEEP), &rogue.pack);
}

/* vanish() does NOT handle a quiver of weapons with more than one
 *  arrow (or whatever) in the quiver.  It will only decrement the count.
 */

void vanish(object *obj, short rm, object *pack)
{
	if (obj->quantity > 1) {
		obj->quantity--;
	} else {
		if (obj->in_use_flags & BEING_WIELDED) {
			unwield(obj);
		} else if (obj->in_use_flags & BEING_WORN) {
			unwear(obj);
		} else if (obj->in_use_flags & ON_EITHER_HAND) {
			un_put_on(obj);
		}
		take_from_pack(obj, pack);
		free_object(obj);
	}
	if (rm) {
		(void) reg_move();
	}
}

void potion_heal(int extra)
{
	float ratio;
	short add;

	rogue.hp_current += rogue.exp;

	ratio = ((float)rogue.hp_current) / rogue.hp_max;

	if (ratio >= 1.00) {
		rogue.hp_max += (extra ? 2 : 1);
		extra_hp += (extra ? 2 : 1);
		rogue.hp_current = rogue.hp_max;
	} else if (ratio >= 0.90) {
		rogue.hp_max += (extra ? 1 : 0);
		extra_hp += (extra ? 1 : 0);
		rogue.hp_current = rogue.hp_max;
	} else {
		if (ratio < 0.33) {
			ratio = 0.33;
		}
		if (extra) {
			ratio += ratio;
		}
		add = (short)(ratio * ((float)rogue.hp_max - rogue.hp_current));
		rogue.hp_current += add;
		if (rogue.hp_current > rogue.hp_max) {
			rogue.hp_current = rogue.hp_max;
		}
	}
	if (blind) {
		unblind();
	}
	if (confused && extra) {
			unconfuse();
	} else if (confused) {
		confused = (confused / 2) + 1;
	}
	if (halluc && extra) {
		unhallucinate();
	} else if (halluc) {
		halluc = (halluc / 2) + 1;
	}
}

void idntfy(void)
{
	short ch;
	object *obj;
	struct id *id_table;
	char desc[ROGUE_DESC_LEN];
AGAIN:
	ch = pack_letter(EN("what would you like to identify?", "調べる持ちものは？"), ALL_OBJECTS);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(EN("no such item, try again", "それは持っていない。"), 0);
		message("", 0);
		check_message();
		goto AGAIN;
	}
	obj->identified = 1;
	if (obj->what_is & (SCROL | POTION | WEAPON | ARMOR | WAND | RING)) {
		id_table = get_id_table(obj);
		id_table[obj->which_kind].id_status = IDENTIFIED;
	}
	get_desc(obj, desc);
	message(desc, 0);
}

void eat(void)
{
	short ch;
	short moves;
	object *obj;
	char buf[ROGUE_DESC_LEN];

	ch = pack_letter(EN("eat what?", "食べる物は？"), FOOD);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(EN("no such item.", "それは持っていない。"), 0);
		return;
	}
	if (obj->what_is != FOOD) {
		message(EN("you can't eat that", "それは食べられない。"), 0);
		return;
	}
	if ((obj->which_kind == FRUIT) || rand_percent(60)) {
		moves = get_rand(950, 1150);
		if (obj->which_kind == RATION) {
			message(EN("yum, that tasted good", "ああ、おいしかった。"), 0);
		} else {
			(void) snprintf(buf, sizeof(buf),
				EN("my, that was a yummy %s", "ああ、美味しい%sだった。"), fruit);
			message(buf, 0);
		}
	} else {
		moves = get_rand(750, 950);
		message(EN("yuk, that food tasted awful", "げげっ、ひどい味だ。"), 0);
		add_exp(2, 1);
	}
	rogue.moves_left /= 3;
	rogue.moves_left += moves;
	hunger_str[0] = 0;
	print_stats(STAT_HUNGER);

	vanish(obj, 1, &rogue.pack);
}

void hold_monster(void)
{
	short i, j;
	short mcount = 0;
	object *monster;
	short row, col;

	for (i = -2; i <= 2; i++) {
		for (j = -2; j <= 2; j++) {
			row = rogue.row + i;
			col = rogue.col + j;
			if ((row < MIN_ROW) || (row > (DROWS-2)) || (col < 0) ||
				 (col > (DCOLS-1))) {
				continue;
			}
			if (dungeon[row][col] & MONSTER) {
				monster = object_at(&level_monsters, row, col);
				if (!monster) {
					dungeon[row][col] &= ~MONSTER;
					continue;
				}
				monster->m_flags |= ASLEEP;
				monster->m_flags &= (~WAKENS);
				mcount++;
			}
		}
	}
	if (mcount == 0) {
		message(EN("you feel a strange sense of loss",
			"急に、体から力が抜けていくような気がした。"), 0);
	} else if (mcount == 1) {
		message(EN("the monster freezes", "怪物は動けなくなった。"), 0);
	} else {
		message(EN("the monsters around you freeze", "怪物どもは動けなくなった。"), 0);
	}
}

void tele(void)
{
	mvaddch(rogue.row, rogue.col, get_dungeon_char(rogue.row, rogue.col));

	if (cur_room >= 0) {
		darken_room(cur_room);
	}
	put_player(get_room_number(rogue.row, rogue.col));
	being_held = 0;
	bear_trap = 0;
}

void hallucinate(void)
{
	object *obj, *monster;
	short ch;

	if (blind) return;

	obj = level_objects.next_object;

	while (obj) {
		ch = ROGUE_MVINCH_SHORT(obj->row, obj->col);
		if (((ch < 'A') || (ch > 'Z')) &&
			((obj->row != rogue.row) || (obj->col != rogue.col)))
		if ((ch != ' ') && (ch != '.') && (ch != '#') && (ch != '+')) {
			addch(gr_obj_char());
		}
		obj = obj->next_object;
	}
	monster = level_monsters.next_monster;

	while (monster) {
		ch = ROGUE_MVINCH_SHORT(monster->row, monster->col);
		if ((ch >= 'A') && (ch <= 'Z')) {
			addch(get_rand('A', 'Z'));
		}
		monster = monster->next_monster;
	}
}

void unhallucinate(void)
{
	halluc = 0;
	relight();
	message(EN("everything looks SO boring now", "すべてがとてもつまらなく見える。"), 1);
}

void unblind(void)
{
	blind = 0;
	message(EN("the veil of darkness lifts", "暗闇のとばりが上がってゆく。"), 1);
	relight();
	if (halluc) {
		hallucinate();
	}
	if (detect_monster) {
		show_monsters();
	}
}

void relight(void)
{
	if (cur_room == PASSAGE) {
		light_passage(rogue.row, rogue.col);
	} else {
		light_up_room(cur_room);
	}
	mvaddch(rogue.row, rogue.col, rogue.fchar);
}

void take_a_nap(void)
{
	short i;

	i = get_rand(2, 5);
	md_sleep(1);

	while (i--) {
		mv_mons();
	}
	md_sleep(1);
	message(you_can_move_again, 0);
}

void go_blind(void)
{
	short i, j;

	if (!blind) {
		message(EN("a cloak of darkness falls around you",
			"深い暗闇のとばりがあたりをおおってゆく。"), 0);
	}
	blind += get_rand(500, 800);

	if (detect_monster) {
		object *monster;

		monster = level_monsters.next_monster;

		while (monster) {
			mvaddch(monster->row, monster->col, monster->trail_char);
			monster = monster->next_monster;
		}
	}
	if (cur_room >= 0) {
		for (i = rooms[cur_room].top_row + 1;
			 i < rooms[cur_room].bottom_row; i++) {
			for (j = rooms[cur_room].left_col + 1;
				 j < rooms[cur_room].right_col; j++) {
				mvaddch(i, j, ' ');
			}
		}
	}
	mvaddch(rogue.row, rogue.col, rogue.fchar);
}

char * get_ench_color(void)
{
	if (halluc) {
		return(id_potions[get_rand(0, POTIONS-1)].title);
	} else if (con_mon) {
		return(EN("red ", "赤い"));
	} 
	return(EN("blue ", "青い"));
}

void cnfs(void)
{
	confused += get_rand(12, 22);
}

void unconfuse(void)
{
	char msg[ROGUE_DESC_LEN];

	confused = 0;
	sprintf(msg, EN("you feel less %s now", "ようやく、%sおさまってきた。"),
		(halluc ? EN("trippy", "トリップ感覚が") : EN("confused", "混乱が")));
	message(msg, 1);
}

void uncurse_all(void)
{
	object *obj;

	obj = rogue.pack.next_object;

	while (obj) {
		obj->is_cursed = 0;
		obj = obj->next_object;
	}
}
