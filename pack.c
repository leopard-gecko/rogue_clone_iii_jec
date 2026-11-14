/*
 * pack.c
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
static char sccsid[] __attribute__((unused)) = "@(#)pack.c	5.1 (Berkeley) 11/25/87";
#endif /* not lint */

#include "rogue.h"

char *curse_message = EN_STATIC("you can't, it appears to be cursed",
	"呪われているので、それはできない。");

extern short levitate;

object * add_to_pack(object *obj, object *pack, int condense)
{
	object *op;

	if (condense) {
		if ((op = check_duplicate(obj, pack))) {
			free_object(obj);
			return(op);
		} else {
			obj->ichar = next_avail_ichar();
		}
	}
	if (pack->next_object == 0) {
		pack->next_object = obj;
	} else {
		op = pack->next_object;

		while (op->next_object) {
			op = op->next_object;
		}
		op->next_object = obj;
	}
	obj->next_object = 0;
	return(obj);
}

void take_from_pack(object *obj, object *pack)
{
	if (!obj || !pack) {
		return;
	}
	while (pack->next_object && (pack->next_object != obj)) {
		pack = pack->next_object;
	}
	if (pack->next_object == obj) {
		pack->next_object = pack->next_object->next_object;
	}
}

/* Note: *status is set to 0 if the rogue attempts to pick up a scroll
 * of scare-monster and it turns to dust.  *status is otherwise set to 1.
 */

object * pick_up(int row, int col, short *status)
{
	object *obj;

	*status = 1;

	if (levitate) {
		message(EN("you're floating in the air!", "宙に浮いている！"), 0);
		return((object *) 0);
	}
	obj = object_at(&level_objects, row, col);
	if (!obj) {
		if (ROGUE_IN_DUNGEON(row, col)) {
			dungeon[row][col] &= (~OBJECT);
		}
		message("pick_up(): inconsistent", 1);
		*status = 0;
		return((object *) 0);
	}
	if (	(obj->what_is == SCROL) &&
			(obj->which_kind == SCARE_MONSTER) &&
			obj->picked_up) {
		message(EN("the scroll turns to dust as you pick it up",
			"拾いあげたとたん、巻き物はちりになってしまった。"), 0);
		dungeon[row][col] &= (~OBJECT);
		vanish(obj, 0, &level_objects);
		*status = 0;
		if (id_scrolls[SCARE_MONSTER].id_status == UNIDENTIFIED) {
			id_scrolls[SCARE_MONSTER].id_status = IDENTIFIED;
		}
		return((object *) 0);
	}
	if (obj->what_is == GOLD) {
		rogue.gold += obj->quantity;
		dungeon[row][col] &= ~(OBJECT);
		take_from_pack(obj, &level_objects);
		print_stats(STAT_GOLD);
		return(obj);	/* obj will be free_object()ed in caller */
	}
	if (pack_count(obj) >= MAX_PACK_COUNT) {
		message(EN("pack too full", "もうこれ以上、物は持てない。"), 1);
		return((object *) 0);
	}
	dungeon[row][col] &= ~(OBJECT);
	take_from_pack(obj, &level_objects);
	obj = add_to_pack(obj, &rogue.pack, 1);
	obj->picked_up = 1;
	return(obj);
}

void drop(void)
{
	object *obj, *new;
	short ch;
	char desc[ROGUE_DESC_LEN];

	if (dungeon[rogue.row][rogue.col] & (OBJECT | STAIRS | TRAP)) {
		message(EN("there's already something there", "そこには、すでに何かが置かれている。"), 0);
		return;
	}
	if (!rogue.pack.next_object) {
		message(EN("you have nothing to drop", "落とす物を持っていない。"), 0);
		return;
	}
	if ((ch = pack_letter(EN("drop what?", "落とす物は？"), ALL_OBJECTS)) == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(EN("no such item.", "それは持っていない。"), 0);
		return;
	}
	if (obj->in_use_flags & BEING_WIELDED) {
		if (obj->is_cursed) {
			message(curse_message, 0);
			return;
		}
		unwield(rogue.weapon);
	} else if (obj->in_use_flags & BEING_WORN) {
		if (obj->is_cursed) {
			message(curse_message, 0);
			return;
		}
		mv_aquatars();
		unwear(rogue.armor);
		print_stats(STAT_ARMOR);
	} else if (obj->in_use_flags & ON_EITHER_HAND) {
		if (obj->is_cursed) {
			message(curse_message, 0);
			return;
		}
		un_put_on(obj);
	}
	obj->row = rogue.row;
	obj->col = rogue.col;

	if ((obj->quantity > 1) && (obj->what_is != WEAPON)) {
		obj->quantity--;
		new = alloc_object();
		*new = *obj;
		new->quantity = 1;
		obj = new;
	} else {
		obj->ichar = 'L';
		take_from_pack(obj, &rogue.pack);
	}
	place_at(obj, rogue.row, rogue.col);
if (rogue_lang_is_japanese) {
		get_desc(obj, desc);
		(void) strcat(desc, "を落とした。");
	} else {
		(void) strcpy(desc, "dropped ");
		get_desc_into(obj, desc+8, ROGUE_DESC_LEN-8);
	}
	message(desc, 0);
	(void) reg_move();
}

object * check_duplicate(object *obj, object *pack)
{
	object *op;

	if (!(obj->what_is & (WEAPON | FOOD | SCROL | POTION))) {
		return(0);
	}
	if ((obj->what_is == FOOD) && (obj->which_kind == FRUIT)) {
		return(0);
	}
	op = pack->next_object;

	while (op) {
		if ((op->what_is == obj->what_is) && 
			(op->which_kind == obj->which_kind)) {

			if ((obj->what_is != WEAPON) ||
			((obj->what_is == WEAPON) &&
			((obj->which_kind == ARROW) ||
			(obj->which_kind == DAGGER) ||
			(obj->which_kind == DART) ||
			(obj->which_kind == SHURIKEN)) &&
			(obj->quiver == op->quiver))) {
				op->quantity += obj->quantity;
				return(op);
			}
		}
		op = op->next_object;
	}
	return(0);
}

int next_avail_ichar(void)
{
	register object *obj;
	register int i;
	boolean ichars[26];

	for (i = 0; i < 26; i++) {
		ichars[i] = 0;
	}
	obj = rogue.pack.next_object;
	while (obj) {
		ichars[(obj->ichar - 'a')] = 1;
		obj = obj->next_object;
	}
	for (i = 0; i < 26; i++) {
		if (!ichars[i]) {
			return(i + 'a');
		}
	}
	return('?');
}

void wait_for_ack(void)
{
	rogue_show_cursor(1);
	while (rgetchar() != ' ') ;
	rogue_show_cursor(show_cursor);
}

int pack_letter(char *prompt, unsigned short mask)
{
	short ch;
	unsigned short tmask = mask;

	if (!mask_pack(&rogue.pack, mask)) {
		message(EN("nothing appropriate", "それは１つも持っていない。"), 0);
		return(CANCEL);
	}
	for (;;) {

		message(prompt, 0);
		rogue_show_cursor(1);

		for (;;) {
			ch = rgetchar();
			if (!is_pack_letter(&ch, &mask)) {
				sound_bell();
			} else {
				break;
			}
		}

		if (ch == LIST) {
			check_message();
			inventory(&rogue.pack, mask);
		} else {
			break;
		}
		mask = tmask;
	}
	check_message();
	rogue_show_cursor(show_cursor);
	return(ch);
}

void take_off(void)
{
	char desc[ROGUE_DESC_LEN];
	object *obj;

	if (rogue.armor) {
		if (rogue.armor->is_cursed) {
			message(curse_message, 0);
		} else {
			mv_aquatars();
			obj = rogue.armor;
			unwear(rogue.armor);
if (rogue_lang_is_japanese) {
				get_desc(obj, desc);
				(void) strcat(desc, "を脱いだ。");
			} else {
				(void) strcpy(desc, "was wearing ");
				get_desc_into(obj, desc+12, ROGUE_DESC_LEN-12);
			}
			message(desc, 0);
			print_stats(STAT_ARMOR);
			(void) reg_move();
		}
	} else {
		message(EN("not wearing any", "よろいを着ていない。"), 0);
	}
}

void wear(void)
{
	short ch;
	register object *obj;
	char desc[ROGUE_DESC_LEN];

	if (rogue.armor) {
		message(EN("you're already wearing some", "他のよろいを着ている。"), 0);
		return;
	}
	ch = pack_letter(EN("wear what?", "どのよろいを？"), ARMOR);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(EN("no such item.", "それは持っていない。"), 0);
		return;
	}
	if (obj->what_is != ARMOR) {
		message(EN("you can't wear that", "それはよろいではない。"), 0);
		return;
	}
	obj->identified = 1;
if (rogue_lang_is_japanese) {
		get_desc(obj, desc);
		(void) strcat(desc, "を身につけた。");
	} else {
		(void) strcpy(desc, "wearing ");
		get_desc_into(obj, desc + 8, ROGUE_DESC_LEN-8);
	}
	message(desc, 0);
	do_wear(obj);
	print_stats(STAT_ARMOR);
	(void) reg_move();
}

void unwear(object *obj)
{
	if (obj) {
		obj->in_use_flags &= (~BEING_WORN);
	}
	rogue.armor = (object *) 0;
}

void do_wear(object *obj)
{
	rogue.armor = obj;
	obj->in_use_flags |= BEING_WORN;
	obj->identified = 1;
}

void wield(void)
{
	short ch;
	register object *obj;
	char desc[ROGUE_DESC_LEN];

	if (rogue.weapon && rogue.weapon->is_cursed) {
		message(curse_message, 0);
		return;
	}
	ch = pack_letter(EN("wield what?", "どの武器を？"), WEAPON);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(EN("No such item.", "それは持っていない。"), 0);
		return;
	}
	if (obj->what_is & (ARMOR | RING)) {
		sprintf(desc, EN("you can't wield %s", "%sを武器にすることはできない。"),
			((obj->what_is == ARMOR) ? EN("armor", "よろい") : EN("rings", "指輪")));
		message(desc, 0);
		return;
	}
	if (obj->in_use_flags & BEING_WIELDED) {
		message(EN("in use", "その武器はもう使っている。"), 0);
	} else {
		unwield(rogue.weapon);
if (rogue_lang_is_japanese) {
			get_desc(obj, desc);
			(void) strcat(desc, "に持ちかえた。");
		} else {
			(void) strcpy(desc, "wielding ");
			get_desc_into(obj, desc + 9, ROGUE_DESC_LEN-9);
		}
		message(desc, 0);
		do_wield(obj);
		(void) reg_move();
	}
}

void do_wield(object *obj)
{
	rogue.weapon = obj;
	obj->in_use_flags |= BEING_WIELDED;
}

void unwield(object *obj)
{
	if (obj) {
		obj->in_use_flags &= (~BEING_WIELDED);
	}
	rogue.weapon = (object *) 0;
}

void call_it(void)
{
	short ch;
	register object *obj;
	struct id *id_table;
	char buf[MAX_TITLE_LENGTH+2];

	ch = pack_letter(EN("call what?", "呼び名をつけるものは？"), (SCROL | POTION | WAND | RING));

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(EN("no such item.", "それは持っていない。"), 0);
		return;
	}
	if (!(obj->what_is & (SCROL | POTION | WAND | RING))) {
		message(EN("surely you already know what that's called",
			"それには呼び名をつけることができない。"), 0);
		return;
	}
	id_table = get_id_table(obj);

	if (get_input_line(EN("call it:", "呼び名は？"),"",buf,id_table[obj->which_kind].title,1,1)) {
		id_table[obj->which_kind].id_status = CALLED;
		(void) strcpy(id_table[obj->which_kind].title, buf);
	}
}

int pack_count(object *new_obj)
{
	object *obj;
	short count = 0;

	obj = rogue.pack.next_object;

	while (obj) {
		if (obj->what_is != WEAPON) {
			count += obj->quantity;
		} else if (!new_obj) {
			count++;
		} else if ((new_obj->what_is != WEAPON) ||
			((obj->which_kind != ARROW) &&
			(obj->which_kind != DAGGER) &&
			(obj->which_kind != DART) &&
			(obj->which_kind != SHURIKEN)) ||
			(new_obj->which_kind != obj->which_kind) ||
			(obj->quiver != new_obj->quiver)) {
			count++;
		}
		obj = obj->next_object;
	}
	return(count);
}

boolean mask_pack(object *pack, unsigned short mask)
{
	while (pack->next_object) {
		pack = pack->next_object;
		if (pack->what_is & mask) {
			return(1);
		}
	}
	return(0);
}

int is_pack_letter(short *c, unsigned short *mask)
{
	if (((*c == '?') || (*c == '!') || (*c == ':') || (*c == '=') ||
		(*c == ')') || (*c == ']') || (*c == '/') || (*c == ','))) {
		switch(*c) {
		case '?':
			*mask = SCROL;
			break;
		case '!':
			*mask = POTION;
			break;
		case ':':
			*mask = FOOD;
			break;
		case ')':
			*mask = WEAPON;
			break;
		case ']':
			*mask = ARMOR;
			break;
		case '/':
			*mask = WAND;
			break;
		case '=':
			*mask = RING;
			break;
		case ',':
			*mask = AMULET;
			break;
		}
		*c = LIST;
		return(1);
	}
	return(((*c >= 'a') && (*c <= 'z')) || (*c == CANCEL) || (*c == LIST));
}

int has_amulet(void)
{
	return(mask_pack(&rogue.pack, AMULET));
}

void kick_into_pack(void)
{
	object *obj;
	char desc[ROGUE_DESC_LEN];
	short n, stat;

	if (!(dungeon[rogue.row][rogue.col] & OBJECT)) {
		message(EN("nothing here", "ここには、何もない。"), 0);
	} else {
		if ((obj = pick_up(rogue.row, rogue.col, &stat))) {
			get_desc(obj, desc);
			if (obj->what_is == GOLD) {
				message(desc, 0);
				free_object(obj);
			} else {
				n = strlen(desc);
				desc[n] = '(';
				desc[n+1] = obj->ichar;
				desc[n+2] = ')';
				desc[n+3] = 0;
				message(desc, 0);
			}
		}
		if (obj || (!stat)) {
			(void) reg_move();
		}
	}
}
