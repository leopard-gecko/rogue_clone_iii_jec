/*
 * object.c
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
static char sccsid[] __attribute__((unused)) = "@(#)object.c	5.1 (Berkeley) 11/25/87";
#endif /* not lint */

#include "rogue.h"

object level_objects;
unsigned short dungeon[DROWS][DCOLS];
short foods = 0;
object *free_list = (object *) 0;
char *fruit = (char *) 0;


fighter rogue = {
	INIT_AW,	/* armor, weapon */
	INIT_RINGS,	/* rings */
	INIT_HP,	/* Hp current,max */
	INIT_STR,	/* Str current,max */
	INIT_PACK,	/* pack */
	INIT_GOLD,	/* gold */
	INIT_EXP,	/* exp level,points */
	0, 0,		/* row, col */
	INIT_CHAR,	/* char */
	INIT_MOVES	/* moves */
};

struct id id_potions[POTIONS] = {
{100, EN_STATIC("blue \0                           ", "青い\0                            "), EN_STATIC("of increase strength ", "強さが増す"), 0},
{250, EN_STATIC("red \0                            ", "赤い\0                            "), EN_STATIC("of restore strength ", "強さが元にもどる"), 0},
{100, EN_STATIC("green \0                          ", "緑の\0                            "), EN_STATIC("of healing ", "傷を治す"), 0},
{200, EN_STATIC("grey \0                           ", "灰色の\0                          "), EN_STATIC("of extra healing ", "とてもよく傷を治す"), 0},
 {10, EN_STATIC("brown \0                          ", "茶色の\0                          "), EN_STATIC("of poison ", "毒の"), 0},
{300, EN_STATIC("clear \0                          ", "透明な\0                          "), EN_STATIC("of raise level ", "レベルが上がる"), 0},
 {10, EN_STATIC("pink \0                           ", "ピンクの\0                        "), EN_STATIC("of blindness ", "目が見えなくなる"), 0},
 {25, EN_STATIC("white \0                          ", "白い\0                            "), EN_STATIC("of hallucination ", "幻覚をおこす"), 0},
{100, EN_STATIC("purple \0                         ", "紫の\0                            "), EN_STATIC("of detect monster ", "怪物探知の"), 0},
{100, EN_STATIC("black \0                          ", "黒い\0                            "), EN_STATIC("of detect things ", "物品探知の"), 0},
 {10, EN_STATIC("yellow \0                         ", "黄色い\0                          "), EN_STATIC("of confusion ", "頭が混乱する"), 0},
 {80, EN_STATIC("plaid \0                          ", "チェック柄の\0                    "), EN_STATIC("of levitation ", "空中に浮きあがる"), 0},
{150, EN_STATIC("burgundy \0                       ", "赤紫の\0                          "), EN_STATIC("of haste self ", "素早くなる"), 0},
{145, EN_STATIC("beige \0                          ", "ベージュ色の\0                    "), EN_STATIC("of see invisible ", "見えないものが見える"), 0}
};

struct id id_scrolls[SCROLS] = {
{505, "                                   ", EN_STATIC("of protect armor ", "よろいを守る"), 0},
{200, "                                   ", EN_STATIC("of hold monster ", "怪物を封じこめる"), 0},
{235, "                                   ", EN_STATIC("of enchant weapon ", "武器に魔法をかける"), 0},
{235, "                                   ", EN_STATIC("of enchant armor ", "よろいに魔法をかける"), 0},
{175, "                                   ", EN_STATIC("of identify ", "持ちものの種類がわかる"), 0},
{190, "                                   ", EN_STATIC("of teleportation ", "テレポートする"), 0},
 {25, "                                   ", EN_STATIC("of sleep ", "眠りにおちる"), 0},
{610, "                                   ", EN_STATIC("of scare monster ", "怪物を怖がらせる"), 0},
{210, "                                   ", EN_STATIC("of remove curse ", "のろいを解く"), 0},
 {80, "                                   ", EN_STATIC("of create monster ", "怪物を作りだす"),0},
 {25, "                                   ", EN_STATIC("of aggravate monster ", "怪物を怒らせる"),0},
{180, "                                   ", EN_STATIC("of magic mapping ", "魔法の地図の"), 0},
 {90, "                                   ", EN_STATIC("of confuse monster ", "怪物を混乱させる"), 0}
};

struct id id_weapons[WEAPONS] = {
	{150, EN_STATIC("short bow ", "短弓"), "", 0},
	  {8, EN_STATIC("darts ", "投げ矢"), "", 0},
	 {15, EN_STATIC("arrows ", "矢"), "", 0},
	 {27, EN_STATIC("daggers ", "短剣"), "", 0},
	 {35, EN_STATIC("shurikens ", "手裏剣"), "", 0},
	{360, EN_STATIC("mace ", "つちほこ"), "", 0},
	{470, EN_STATIC("long sword ", "長い剣"), "", 0},
	{580, EN_STATIC("two-handed sword ", "両手持ちの剣"), "", 0}
};

struct id id_armors[ARMORS] = {
	{300, EN_STATIC("leather armor ", "革のよろい"), "", (UNIDENTIFIED)},
	{300, EN_STATIC("ring mail ", "鉄環のよろい"), "", (UNIDENTIFIED)},
	{400, EN_STATIC("scale mail ", "鱗のよろい"), "", (UNIDENTIFIED)},
	{500, EN_STATIC("chain mail ", "鎖のよろい"), "", (UNIDENTIFIED)},
	{600, EN_STATIC("banded mail ", "鉄片のよろい"), "", (UNIDENTIFIED)},
	{600, EN_STATIC("splint mail ", "帯金のよろい"), "", (UNIDENTIFIED)},
	{700, EN_STATIC("plate mail ", "全身のよろい"), "", (UNIDENTIFIED)}
};

struct id id_wands[WANDS] = {
	 {25, "                                 ", EN_STATIC("of teleport away ", "怪物を遠くに飛ばす"),0},
	 {50, "                                 ", EN_STATIC("of slow monster ", "怪物を遅くする"), 0},
	  {8, "                                 ", EN_STATIC("of invisibility ", "怪物を見えなくする"),0},
	 {55, "                                 ", EN_STATIC("of polymorph ", "変身させる"),0},
	  {2, "                                 ", EN_STATIC("of haste monster ", "怪物を素早くする"),0},
	 {20, "                                 ", EN_STATIC("of magic missile ", "魔法のミサイルの"),0},
	 {20, "                                 ", EN_STATIC("of cancellation ", "魔力を封じる"),0},
	  {0, "                                 ", EN_STATIC("of do nothing ", "役に立たない"),0},
	 {35, "                                 ", EN_STATIC("of drain life ", "命を吸い取る"),0},
	 {20, "                                 ", EN_STATIC("of cold ", "冷気の"),0},
	 {20, "                                 ", EN_STATIC("of fire ", "炎の"),0}
};

struct id id_rings[RINGS] = {
	 {250, "                                 ", EN_STATIC("of stealth ", "身を隠す"),0},
	 {100, "                                 ", EN_STATIC("of teleportation ", "テレポートする"), 0},
	 {255, "                                 ", EN_STATIC("of regeneration ", "再生の"),0},
	 {295, "                                 ", EN_STATIC("of slow digestion ", "消化が遅くなる"),0},
	 {200, "                                 ", EN_STATIC("of add strength ", "強さが増す"),0},
	 {250, "                                 ", EN_STATIC("of sustain strength ", "強さが減らない"),0},
	 {250, "                                 ", EN_STATIC("of dexterity ", "機敏になる"),0},
	  {25, "                                 ", EN_STATIC("of adornment ", "飾りものの"),0},
	 {300, "                                 ", EN_STATIC("of see invisible ", "見えないものが見える"),0},
	 {290, "                                 ", EN_STATIC("of maintain armor ", "よろいが丈夫になる"),0},
	 {270, "                                 ", EN_STATIC("of searching ", "探索の"),0},
};

extern short cur_level, max_level;
extern short party_room;
extern char *error_file;
extern boolean is_wood[];

void put_objects(void)
{
	short i, n;
	object *obj;

	if (cur_level < max_level) {
		return;
	}
	n = coin_toss() ? get_rand(2, 4) : get_rand(3, 5);
	while (rand_percent(33)) {
		n++;
	}
	if (party_room != NO_ROOM) {
		make_party();
	}
	for (i = 0; i < n; i++) {
		obj = gr_object();
		rand_place(obj);
	}
	put_gold();
}

void put_gold(void)
{
	short i, j;
	short row,col;
	boolean is_maze, is_room;

	for (i = 0; i < MAXROOMS; i++) {
		is_maze = (rooms[i].is_room & R_MAZE) ? 1 : 0;
		is_room = (rooms[i].is_room & R_ROOM) ? 1 : 0;

		if (!(is_room || is_maze)) {
			continue;
		}
		if (is_maze || rand_percent(GOLD_PERCENT)) {
			for (j = 0; j < 50; j++) {
				row = get_rand(rooms[i].top_row+1,
				rooms[i].bottom_row-1);
				col = get_rand(rooms[i].left_col+1,
				rooms[i].right_col-1);
				if ((dungeon[row][col] == FLOOR) ||
					(dungeon[row][col] == TUNNEL)) {
					plant_gold(row, col, is_maze);
					break;
				}
			}
		}
	}
}

void plant_gold(short row, short col, boolean is_maze)
{
	object *obj;

	if (!ROGUE_IN_DUNGEON(row, col)) {
		return;
	}
	obj = alloc_object();
	obj->row = row; obj->col = col;
	obj->what_is = GOLD;
	obj->quantity = get_rand((2 * cur_level), (16 * cur_level));
	if (is_maze) {
		obj->quantity += obj->quantity / 2;
	}
	dungeon[row][col] |= OBJECT;
	(void) add_to_pack(obj, &level_objects, 0);
}

void place_at(object *obj, int row, int col)
{
	if (!obj || !ROGUE_IN_DUNGEON(row, col)) {
		return;
	}
	obj->row = row;
	obj->col = col;
	dungeon[row][col] |= OBJECT;
	(void) add_to_pack(obj, &level_objects, 0);
}

object * object_at(register object *pack, short row, short col)
{
	object *obj = (object *) 0;

	if (!ROGUE_IN_DUNGEON(row, col)) {
		return((object *) 0);
	}
	if (dungeon[row][col] & (MONSTER | OBJECT)) {
		obj = pack->next_object;

		while (obj && ((obj->row != row) || (obj->col != col))) {
			obj = obj->next_object;
		}
		if (!obj) {
			message(EN("object_at(): inconsistent", "object_at(): 矛盾があります。"), 1);
		}
	}
	return(obj);
}

object * get_letter_object(int ch)
{
	object *obj;

	obj = rogue.pack.next_object;

	while (obj && (obj->ichar != ch)) {
		obj = obj->next_object;
	}
	return(obj);
}

void free_stuff(object *objlist)
{
	object *obj;

	while (objlist->next_object) {
		obj = objlist->next_object;
		objlist->next_object =
			objlist->next_object->next_object;
		free_object(obj);
	}
}

char * name_of(object *obj)
{
	char *retstring;

	switch(obj->what_is) {
	case SCROL:
		retstring = EN((obj->quantity > 1 ? "scrolls " : "scroll "), "巻き物");
		break;
	case POTION:
		retstring = EN((obj->quantity > 1 ? "potions " : "potion "), "水薬");
		break;
	case FOOD:
		if (obj->which_kind == RATION) {
			retstring = EN("food ", "食糧");
		} else {
			retstring = fruit;
		}
		break;
	case WAND:
		retstring = EN((is_wood[obj->which_kind] ? "staff " : "wand "),
                       (is_wood[obj->which_kind] ? "つえ" : "魔法の棒"));
		break;
	case WEAPON:
		switch(obj->which_kind) {
		case DART:
			retstring=EN((obj->quantity > 1 ? "darts " : "dart "), "投げ矢");
			break;
		case ARROW:
			retstring=EN((obj->quantity > 1 ? "arrows " : "arrow "), "矢");
			break;
		case DAGGER:
			retstring=EN((obj->quantity > 1 ? "daggers " : "dagger "), "短剣");
			break;
		case SHURIKEN:
			retstring=EN((obj->quantity > 1?"shurikens ":"shuriken "), "手裏剣");
			break;
		default:
			retstring = id_weapons[obj->which_kind].title;
		}
		break;
	case ARMOR:
		retstring = EN("armor ", "よろい");
		break;
	case RING:
			retstring = EN("ring ", "指輪");
		break;
	case AMULET:
		retstring = EN("amulet ", "魔除け");
		break;
	default:
		retstring = EN("unknown ", "？？？");
		break;
	}
	return(retstring);
}

object * gr_object(void)
{
	object *obj;

	obj = alloc_object();

	if (foods < (cur_level / 3)) {
		obj->what_is = FOOD;
		foods++;
	} else {
		obj->what_is = gr_what_is();
	}
	switch(obj->what_is) {
	case SCROL:
		gr_scroll(obj);
		break;
	case POTION:
		gr_potion(obj);
		break;
	case WEAPON:
		gr_weapon(obj, 1);
		break;
	case ARMOR:
		gr_armor(obj);
		break;
	case WAND:
		gr_wand(obj);
		break;
	case FOOD:
		get_food(obj, 0);
		break;
	case RING:
		gr_ring(obj, 1);
		break;
	}
	return(obj);
}

unsigned short gr_what_is(void)
{
	short percent;
	unsigned short what_is;

	percent = get_rand(1, 91);

	if (percent <= 30) {
		what_is = SCROL;
	} else if (percent <= 60) {
		what_is = POTION;
	} else if (percent <= 64) {
		what_is = WAND;
	} else if (percent <= 74) {
		what_is = WEAPON;
	} else if (percent <= 83) {
		what_is = ARMOR;
	} else if (percent <= 88) {
		what_is = FOOD;
	} else {
		what_is = RING;
	}
	return(what_is);
}

void gr_scroll(object *obj)
{
	short percent;

	percent = get_rand(0, 91);

	obj->what_is = SCROL;

	if (percent <= 5) {
		obj->which_kind = PROTECT_ARMOR;
	} else if (percent <= 10) {
		obj->which_kind = HOLD_MONSTER;
	} else if (percent <= 20) {
		obj->which_kind = CREATE_MONSTER;
	} else if (percent <= 35) {
		obj->which_kind = IDENTIFY;
	} else if (percent <= 43) {
		obj->which_kind = TELEPORT;
	} else if (percent <= 50) {
		obj->which_kind = SLEEP;
	} else if (percent <= 55) {
		obj->which_kind = SCARE_MONSTER;
	} else if (percent <= 64) {
		obj->which_kind = REMOVE_CURSE;
	} else if (percent <= 69) {
		obj->which_kind = ENCH_ARMOR;
	} else if (percent <= 74) {
		obj->which_kind = ENCH_WEAPON;
	} else if (percent <= 80) {
		obj->which_kind = AGGRAVATE_MONSTER;
	} else if (percent <= 86) {
		obj->which_kind = CON_MON;
	} else {
		obj->which_kind = MAGIC_MAPPING;
	}
}

void gr_potion(object *obj)
{
	short percent;

	percent = get_rand(1, 118);

	obj->what_is = POTION;

	if (percent <= 5) {
		obj->which_kind = RAISE_LEVEL;
	} else if (percent <= 15) {
		obj->which_kind = DETECT_OBJECTS;
	} else if (percent <= 25) {
		obj->which_kind = DETECT_MONSTER;
	} else if (percent <= 35) {
		obj->which_kind = INCREASE_STRENGTH;
	} else if (percent <= 45) {
		obj->which_kind = RESTORE_STRENGTH;
	} else if (percent <= 55) {
		obj->which_kind = HEALING;
	} else if (percent <= 65) {
		obj->which_kind = EXTRA_HEALING;
	} else if (percent <= 75) {
		obj->which_kind = BLINDNESS;
	} else if (percent <= 85) {
		obj->which_kind = HALLUCINATION;
	} else if (percent <= 95) {
		obj->which_kind = CONFUSION;
	} else if (percent <= 105) {
		obj->which_kind = POISON;
	} else if (percent <= 110) {
		obj->which_kind = LEVITATION;
	} else if (percent <= 114) {
		obj->which_kind = HASTE_SELF;
	} else {
		obj->which_kind = SEE_INVISIBLE;
	}
}

void gr_weapon(object *obj, int assign_wk)
{
	short percent;
	short i;
	short blessing, increment;

	obj->what_is = WEAPON;
	if (assign_wk) {
		obj->which_kind = get_rand(0, (WEAPONS - 1));
	}
	if ((obj->which_kind == ARROW) || (obj->which_kind == DAGGER) ||
		(obj->which_kind == SHURIKEN) | (obj->which_kind == DART)) {
		obj->quantity = get_rand(3, 15);
		obj->quiver = get_rand(0, 126);
	} else {
		obj->quantity = 1;
	}
	obj->hit_enchant = obj->d_enchant = 0;

	percent = get_rand(1, 96);
	blessing = get_rand(1, 3);

	if (percent <= 16) {
		increment = 1;
	} else if (percent <= 32) {
		increment = -1;
		obj->is_cursed = 1;
	}
	if (percent <= 32) {
		for (i = 0; i < blessing; i++) {
			if (coin_toss()) {
				obj->hit_enchant += increment;
			} else {
				obj->d_enchant += increment;
			}
		}
	}
	switch(obj->which_kind) {
	case BOW:
	case DART:
		obj->damage = "1d1";
		break;
	case ARROW:
		obj->damage = "1d2";
		break;
	case DAGGER:
		obj->damage = "1d3";
		break;
	case SHURIKEN:
		obj->damage = "1d4";
		break;
	case MACE:
		obj->damage = "2d3";
		break;
	case LONG_SWORD:
		obj->damage = "3d4";
		break;
	case TWO_HANDED_SWORD:
		obj->damage = "4d5";
		break;
	}
}

static short base_armor_class(unsigned short kind)
{
	short armor_class = kind + 2;

	if ((kind == PLATE) || (kind == SPLINT)) {
		armor_class--;
	}
	return armor_class;
}

void gr_armor(object *obj)
{
	short percent;
	short blessing;

	obj->what_is = ARMOR;
	obj->which_kind = get_rand(0, (ARMORS - 1));
	obj->class = base_armor_class(obj->which_kind);
	obj->is_protected = 0;
	obj->d_enchant = 0;

	percent = get_rand(1, 100);
	blessing = get_rand(1, 3);

	if (percent <= 16) {
		obj->is_cursed = 1;
		obj->d_enchant -= blessing;
	} else if (percent <= 33) {
		obj->d_enchant += blessing;
	}
}

void gr_wand(object *obj)
{
	obj->what_is = WAND;
	obj->which_kind = get_rand(0, (WANDS - 1));
	obj->class = get_rand(3, 7);
}

void get_food(object *obj, boolean force_ration)
{
	obj->what_is = FOOD;

	if (force_ration || rand_percent(80)) {
		obj->which_kind = RATION;
	} else {
		obj->which_kind = FRUIT;
	}
}

void put_stairs(void)
{
	short row, col;

	if (!gr_row_col(&row, &col, (FLOOR | TUNNEL))) {
		clean_up(EN("no position for stairs", "階段を配置できません。"));
	}
	dungeon[row][col] |= STAIRS;
}

int get_armor_class(object *obj)
{
	if (obj) {
		return(obj->class + obj->d_enchant);
	}
	return(0);
}

object * alloc_object(void)
{
	object *obj;

	if (free_list) {
		obj = free_list;
		free_list = free_list->next_object;
	} else if (!(obj = (object *) md_malloc(sizeof(object)))) {
			message(EN("cannot allocate object, saving game",
				"物を確保できません。ゲームを保存します。"), 0);
			save_into_file(error_file);
	}
	obj->quantity = 1;
	obj->ichar = 'L';
	obj->picked_up = obj->is_cursed = 0;
	obj->in_use_flags = NOT_USED;
	obj->identified = UNIDENTIFIED;
	obj->damage = "1d1";
	return(obj);
}

void free_object(object *obj)
{
	obj->next_object = free_list;
	free_list = obj;
}

void make_party(void)
{
	short n;

	party_room = gr_room();

	n = rand_percent(99) ? party_objects(party_room) : 11;
	if (rand_percent(99)) {
		party_monsters(party_room, n);
	}
}

void show_objects(void)
{
	object *obj;
	short mc, rc, row, col;
	object *monster;

	obj = level_objects.next_object;

	while (obj) {
		row = obj->row;
		col = obj->col;

		rc = get_mask_char(obj->what_is);

		if (dungeon[row][col] & MONSTER) {
			if ((monster = object_at(&level_monsters, row, col))) {
				monster->trail_char = rc;
			}
		}
		mc = ROGUE_MVINCH_SHORT(row, col);
		if (((mc < 'A') || (mc > 'Z')) &&
			((row != rogue.row) || (col != rogue.col))) {
			mvaddch(row, col, rc);
		}
		obj = obj->next_object;
	}

	monster = level_monsters.next_object;

	while (monster) {
		if (monster->m_flags & IMITATES) {
			mvaddch(monster->row, monster->col, (int) monster->disguise);
		}
		monster = monster->next_monster;
	}
}

void put_amulet(void)
{
	object *obj;

	obj = alloc_object();
	obj->what_is = AMULET;
	rand_place(obj);
}

void rand_place(object *obj)
{
	short row, col;

	if (gr_row_col(&row, &col, (FLOOR | TUNNEL))) {
		place_at(obj, row, col);
	} else {
		free_object(obj);
	}
}

void c_object_for_wizard(void)
{
	short ch, max = 0, wk;
	object *obj;
	char buf[ROGUE_DESC_LEN];

	if (pack_count((object *) 0) >= MAX_PACK_COUNT) {
		message(EN("pack full", "これ以上は物を持てない。"), 0);
		return;
	}
	message(EN("type of object?", "どんなものですか？"), 0);
	rogue_show_cursor(1);

	while (r_index("!?:)]=/,\033", (ch = rgetchar()), 0) == -1) {
		sound_bell();
	}
	check_message();
	rogue_show_cursor(show_cursor);

	if (ch == '\033') {
		return;
	}
	obj = alloc_object();

	switch(ch) {
	case '!':
		obj->what_is = POTION;
		max = POTIONS - 1;
		break;
	case '?':
		obj->what_is = SCROL;
		max = SCROLS - 1;
		break;
	case ',':
		obj->what_is = AMULET;
		break;
	case ':':
		get_food(obj, 0);
		break;
	case ')':
		obj->what_is = WEAPON;
		max = WEAPONS - 1;
		break;
	case ']':
		gr_armor(obj);
		max = ARMORS - 1;
		break;
	case '/':
		gr_wand(obj);
		max = WANDS - 1;
		break;
	case '=':
		max = RINGS - 1;
		obj->what_is = RING;
		break;
	}
	if ((ch != ',') && (ch != ':')) {
GIL:
		if (get_input_line(EN("which kind?", "種類は？"), "", buf, "", 0, 1)) {
			wk = get_number(buf);
			if ((wk >= 0) && (wk <= max)) {
				obj->which_kind = (unsigned short) wk;
				if (obj->what_is == RING) {
					gr_ring(obj, 0);
				} else if (obj->what_is == WEAPON) {
					gr_weapon(obj, 0);
				} else if (obj->what_is == ARMOR) {
					obj->class = base_armor_class(obj->which_kind);
				}
			} else {
				sound_bell();
				goto GIL;
			}
		} else {
			free_object(obj);
			return;
		}
	}
	get_desc(obj, buf);
	message(buf, 0);
	(void) add_to_pack(obj, &rogue.pack, 1);
}
