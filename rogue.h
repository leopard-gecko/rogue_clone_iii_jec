/*
 * rogue.h
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  This notice shall not be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 *	@(#)rogue.h	5.2 (Berkeley) 11/25/87
 */

#ifndef ROGUE_CLONE_III_H
#define ROGUE_CLONE_III_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "jec/rogue_i18n.h"
#include "jec/unicode_text.h"
#define boolean char

#define NOTHING		((unsigned short)     0)
#define OBJECT		((unsigned short)    01)
#define MONSTER		((unsigned short)    02)
#define STAIRS		((unsigned short)    04)
#define HORWALL		((unsigned short)   010)
#define VERTWALL	((unsigned short)   020)
#define DOOR		((unsigned short)   040)
#define FLOOR		((unsigned short)  0100)
#define TUNNEL		((unsigned short)  0200)
#define TRAP		((unsigned short)  0400)
#define HIDDEN		((unsigned short) 01000)

#define ARMOR		((unsigned short)   01)
#define WEAPON		((unsigned short)   02)
#define SCROL		((unsigned short)   04)
#define POTION		((unsigned short)  010)
#define GOLD		((unsigned short)  020)
#define FOOD		((unsigned short)  040)
#define WAND		((unsigned short) 0100)
#define RING		((unsigned short) 0200)
#define AMULET		((unsigned short) 0400)
#define ALL_OBJECTS	((unsigned short) 0777)

#define LEATHER 0
#define RINGMAIL 1
#define SCALE 2
#define CHAIN 3
#define BANDED 4
#define SPLINT 5
#define PLATE 6
#define ARMORS 7

#define BOW 0
#define DART 1
#define ARROW 2
#define DAGGER 3
#define SHURIKEN 4
#define MACE 5
#define LONG_SWORD 6
#define TWO_HANDED_SWORD 7
#define WEAPONS 8

#define MAX_PACK_COUNT 24

#define PROTECT_ARMOR 0
#define HOLD_MONSTER 1
#define ENCH_WEAPON 2
#define ENCH_ARMOR 3
#define IDENTIFY 4
#define TELEPORT 5
#define SLEEP 6
#define SCARE_MONSTER 7
#define REMOVE_CURSE 8
#define CREATE_MONSTER 9
#define AGGRAVATE_MONSTER 10
#define MAGIC_MAPPING 11
#define CON_MON 12
#define SCROLS 13

#define INCREASE_STRENGTH 0
#define RESTORE_STRENGTH 1
#define HEALING 2
#define EXTRA_HEALING 3
#define POISON 4
#define RAISE_LEVEL 5
#define BLINDNESS 6
#define HALLUCINATION 7
#define DETECT_MONSTER 8
#define DETECT_OBJECTS 9
#define CONFUSION 10
#define LEVITATION 11
#define HASTE_SELF 12
#define SEE_INVISIBLE 13
#define POTIONS 14

#define TELE_AWAY 0
#define SLOW_MONSTER 1
#define INVISIBILITY 2
#define POLYMORPH 3
#define HASTE_MONSTER 4
#define MAGIC_MISSILE 5
#define CANCELLATION 6
#define DO_NOTHING 7
#define DRAIN_LIFE 8
#define COLD 9
#define FIRE 10
#define WANDS 11

#define STEALTH 0
#define R_TELEPORT 1
#define REGENERATION 2
#define SLOW_DIGEST 3
#define ADD_STRENGTH 4
#define SUSTAIN_STRENGTH 5
#define DEXTERITY 6
#define ADORNMENT 7
#define R_SEE_INVISIBLE 8
#define MAINTAIN_ARMOR 9
#define SEARCHING 10
#define RINGS 11

#define RATION 0
#define FRUIT 1

#define NOT_USED		((unsigned short)   0)
#define BEING_WIELDED	((unsigned short)  01)
#define BEING_WORN		((unsigned short)  02)
#define ON_LEFT_HAND	((unsigned short)  04)
#define ON_RIGHT_HAND	((unsigned short) 010)
#define ON_EITHER_HAND	((unsigned short) 014)
#define BEING_USED		((unsigned short) 017)

#define NO_TRAP -1
#define TRAP_DOOR 0
#define BEAR_TRAP 1
#define TELE_TRAP 2
#define DART_TRAP 3
#define SLEEPING_GAS_TRAP 4
#define RUST_TRAP 5
#define TRAPS 6

#define STEALTH_FACTOR 3
#define R_TELE_PERCENT 8

#define UNIDENTIFIED ((unsigned short) 00)	/* MUST BE ZERO! */
#define IDENTIFIED ((unsigned short) 01)
#define CALLED ((unsigned short) 02)

#define DROWS 24
#define DCOLS 80
#define ROGUE_DESC_LEN 256
#define NMESSAGES 5
#define SCORE_LINE_LEN ROGUE_DESC_LEN
#define SCORE_NAME_LEN 30
#define MAX_TITLE_LENGTH 80
#define ID_TITLE_SIZE 80
#define MAXSYLLABLES 40
#define MAX_SCROLL_TITLE_SYLLABLES 5
#define MAX_METAL 14
#define WAND_MATERIALS 30
#define GEMS 14

#define GOLD_PERCENT 46

#ifndef ROGUE_PROGRAM_NAME
#define ROGUE_PROGRAM_NAME "rogue"
#endif
#ifndef SCORE_FILE
#define SCORE_FILE ROGUE_PROGRAM_NAME ".scores"
#endif
#ifndef DEFAULT_SAVE_FILE
#define DEFAULT_SAVE_FILE ROGUE_PROGRAM_NAME ".save"
#endif
#ifndef LOCK_FILE
#define LOCK_FILE ROGUE_PROGRAM_NAME ".lock"
#endif

#ifndef DEFAULT_JUMP
#define DEFAULT_JUMP 1
#endif
#ifndef DEFAULT_PASSGO
#define DEFAULT_PASSGO 0
#endif
#ifndef DEFAULT_COLOR
#define DEFAULT_COLOR 0
#endif
#ifndef DEFAULT_CURSOR
#define DEFAULT_CURSOR 1
#endif

#define MAX_OPT_LEN 80

struct id {
	short value;
	char title[ID_TITLE_SIZE];
	char *real;
	unsigned short id_status;
};

/* The following #defines provide more meaningful names for some of the
 * struct object fields that are used for monsters.  This, since each monster
 * and object (scrolls, potions, etc) are represented by a struct object.
 * Ideally, this should be handled by some kind of union structure.
 */

#define m_damage damage
#define hp_to_kill quantity
#define m_char ichar
#define first_level is_protected
#define last_level is_cursed
#define m_hit_chance class
#define stationary_damage identified
#define drop_percent which_kind
#define trail_char d_enchant
#define slowed_toggle quiver
#define moves_confused hit_enchant
#define nap_length picked_up
#define disguise what_is
#define next_monster next_object

struct obj {				/* comment is monster meaning */
	unsigned long m_flags;	/* monster flags */
	char *damage;			/* damage it does */
	short quantity;			/* hit points to kill */
	short ichar;			/* 'A' is for aquatar */
	short kill_exp;			/* exp for killing it */
	short is_protected;		/* level starts */
	short is_cursed;		/* level ends */
	short class;			/* chance of hitting you */
	short identified;		/* 'F' damage, 1,2,3... */
	unsigned short which_kind; /* item carry/drop % */
	short o_row, o_col, o;	/* o is how many times stuck at o_row, o_col */
	short row, col;			/* current row, col */
	short d_enchant;		/* room char when detect_monster */
	short quiver;			/* monster slowed toggle */
	short trow, tcol;		/* target row, col */
	short hit_enchant;		/* how many moves is confused */
	unsigned short what_is;	/* imitator's charactor (?!%: */
	short picked_up;		/* sleep from wand of sleep */
	unsigned short in_use_flags;
	struct obj *next_object;	/* next monster */
};

typedef struct obj object;

#define INIT_AW (object*)0,(object*)0
#define INIT_RINGS (object*)0,(object*)0
#define INIT_HP 12,12
#define INIT_HP_CURRENT 12
#define INIT_STR 16,16
#define INIT_EXP 1,0
#define INIT_PACK {0}
#define INIT_GOLD 0
#define INIT_CHAR '@'
#define INIT_MOVES 1250

struct fightr {
	object *armor;
	object *weapon;
	object *left_ring, *right_ring;
	short hp_current;
	short hp_max;
	short str_current;
	short str_max;
	object pack;
	long gold;
	short exp;
	long exp_points;
	short row, col;
	short fchar;
	short moves_left;
};

typedef struct fightr fighter;

struct dr {
	short oth_room;
	short oth_row,
	      oth_col;
	short door_row,
		  door_col;
};

typedef struct dr door;

struct rm {
	short bottom_row, right_col, left_col, top_row;
	door doors[4];
	unsigned short is_room;
};

typedef struct rm room;

#define MAXROOMS 9
#define BIG_ROOM 10

#define NO_ROOM -1

#define PASSAGE -3		/* cur_room value */

#define AMULET_LEVEL 26

#define R_NOTHING	((unsigned short) 01)
#define R_ROOM		((unsigned short) 02)
#define R_MAZE		((unsigned short) 04)
#define R_DEADEND	((unsigned short) 010)
#define R_CROSS		((unsigned short) 020)

#define MAX_EXP_LEVEL 21
#define MAX_EXP 10000001L
#define MAX_GOLD 999999
#define MAX_ARMOR 99
#define MAX_HP 999
#define MAX_STRENGTH 99
#define LAST_DUNGEON 99

#define STAT_LEVEL 01
#define STAT_GOLD 02
#define STAT_HP 04
#define STAT_STRENGTH 010
#define STAT_ARMOR 020
#define STAT_EXP 040
#define STAT_HUNGER 0100
#define STAT_LABEL 0200
#define STAT_ALL 0377

#define PARTY_TIME 10	/* one party somewhere in each 10 level span */

#define MAX_TRAPS 10	/* maximum traps per level */

#define HIDE_PERCENT 12

struct tr {
	short trap_type;
	short trap_row, trap_col;
};

typedef struct tr trap;

extern fighter rogue;
/* JEC option: cursor visibility during play. */
extern boolean show_cursor;
extern room rooms[];
extern trap traps[];
extern unsigned short dungeon[DROWS][DCOLS];
extern object level_objects;

extern struct id id_scrolls[];
extern struct id id_potions[];
extern struct id id_wands[];
extern struct id id_rings[];
extern struct id id_weapons[];
extern struct id id_armors[];
extern short potion_title_id[];
extern short wand_material_id[];
extern short ring_gem_id[];
extern short scroll_title_syllable_count[];
extern short scroll_title_syllables[][MAX_SCROLL_TITLE_SYLLABLES];

extern object mon_tab[];
extern object level_monsters;

#define MONSTERS 26

#define HASTED					01L
#define SLOWED					02L
#define INVISIBLE				04L
#define ASLEEP				   010L
#define WAKENS				   020L
#define WANDERS				   040L
#define FLIES				  0100L
#define FLITS				  0200L
#define CAN_FLIT			  0400L		/* can, but usually doesn't, flit */
#define CONFUSED	 		 01000L
#define RUSTS				 02000L
#define HOLDS				 04000L
#define FREEZES				010000L
#define STEALS_GOLD			020000L
#define STEALS_ITEM			040000L
#define STINGS			   0100000L
#define DRAINS_LIFE		   0200000L
#define DROPS_LEVEL		   0400000L
#define SEEKS_GOLD		  01000000L
#define FREEZING_ROGUE	  02000000L
#define RUST_VANISHED	  04000000L
#define CONFUSES		 010000000L
#define IMITATES		 020000000L
#define FLAMES			 040000000L
#define STATIONARY		0100000000L		/* damage will be 1,2,3,... */
#define NAPPING			0200000000L		/* can't wake up for a while */
#define ALREADY_MOVED	0400000000L

#define SPECIAL_HIT		(RUSTS|HOLDS|FREEZES|STEALS_GOLD|STEALS_ITEM|STINGS|DRAINS_LIFE|DROPS_LEVEL)

#define WAKE_PERCENT 45
#define FLIT_PERCENT 40
#define PARTY_WAKE_PERCENT 75

#define HYPOTHERMIA 1
#define STARVATION 2
#define POISON_DART 3
#define QUIT 4
#define WIN 5
#define KFIRE 6

#define UPWARD 0
#define UPRIGHT 1
#define RIGHT 2
#define DOWNRIGHT 3
#define DOWN 4
#define DOWNLEFT 5
#define LEFT 6
#define UPLEFT 7
#define DIRS 8

#define ROW1 7
#define ROW2 15

#define COL1 26
#define COL2 52

#define MOVED 0
#define MOVE_FAILED -1
#define STOPPED_ON_SOMETHING -2
#define CANCEL '\033'
#define LIST '*'

#define HUNGRY 300
#define WEAK 150
#define FAINT 20
#define STARVE 0

#define MIN_ROW 1
#define ROGUE_IN_DUNGEON(row, col) \
	(((row) >= MIN_ROW) && ((row) <= (DROWS - 2)) && \
	 ((col) >= 0) && ((col) <= (DCOLS - 1)))

struct rogue_time {
	short year;		/* >= 1987 */
	short month;	/* 1 - 12 */
	short day;		/* 1 - 31 */
	short hour;		/* 0 - 23 */
	short minute;	/* 0 - 59 */
	short second;	/* 0 - 59 */
};


/* external routine declarations. */
void add_exp(int e, boolean promotion);
void add_mazes(void);
object * add_to_pack(object *obj, object *pack, int condense);
void add_traps(void);
void aggravate(void);
void aim_monster(object *monster);
object * alloc_object(void);
void bounce(short ball, short dir, short row, short col, short r);
void byebye(int sig);
void c_object_for_wizard(void);
void call_it(void);
int can_move(int row1, int col1, int row2, int col2);
void center(short row, char *buf);
object * check_duplicate(object *obj, object *pack);
void check_gold_seeker(object *monster);
boolean check_hunger(boolean msg_only);
int check_imitator(object *monster);
void check_message(void);
int check_up(void);
void clean_up(char *estr);
void clear_level(void);
void cnfs(void);
int coin_toss(void);
int connect_rooms(short room1, short room2);
void cough_up(object *monster);
void create_monster(void);
int damage_for_strength(void);
void darken_room(short rn);
void disappear(object *monster);
void do_args(int argc, char **argv);
void do_opts(void);
void do_put_on(object *ring, boolean on_left);
void do_shell(void);
void do_wear(object *obj);
void do_wield(object *obj);
void dr_course(object *monster, boolean entering, short row, short col);
void drain_life(void);
void draw_magic_map(void);
void draw_simple_passage(short row1, short col1, short row2, short col2, short dir);
void drop(void);
int drop_check(void);
void drop_level(void);
void eat(void);
void edit_opts(void);
void env_get_value(char **s, char *e, boolean add_blank);
void error_save(int sig);
void fight(boolean to_the_death);
void fill_it(int rn, boolean do_rec_de);
void fill_out_level(void);
int flame_broil(object *monster);
int flit(object *monster);
void flop_weapon(object *weapon, short row, short col);
void free_object(object *obj);
void free_stuff(object *objlist);
void freeze(object *monster);
int get_armor_class(object *obj);
int get_com_id(int *index, short ch);
int get_damage(char *ds, boolean r);
void get_desc(object *obj, char *desc);
void get_desc_into(object *obj, char *desc, int len);
int get_dir(short srow, short scol, short drow, short dcol);
void get_dir_rc(short dir, short *row, short *col, short allow_off_screen);
int get_dungeon_char(register int row, register int col);
char * get_ench_color(void);
int get_exp_level(long e);
void get_food(object *obj, boolean force_ration);
int get_hit_chance(object *weapon);
struct id * get_id_table(object *obj);
int get_input_line(char *prompt, char *insert, char *buf, char *if_cancelled, boolean add_blank, boolean do_echo);
int rogue_utf8_edit_char(int ch);
int rogue_read_utf8_char(int first, char *out, int out_size);
void rogue_redraw_screen(void);
void rogue_addstr_fit(char *s, int max_cols);
object * get_letter_object(int ch);
int get_mask_char(register unsigned short mask);
int get_number(register char *s);
int get_oth_room(short rn, short *row, short *col);
int get_rand(register int x, register int y);
int get_room_number(register int row, register int col);
object * get_thrown_at_monster(object *obj, short dir, short *row, short *col);
int get_value(object *obj);
int get_w_damage(object *obj);
void get_wand_and_ring_materials(void);
int get_weapon_damage(object *weapon);
object * get_zapped_monster(short dir, short *row, short *col);
int gmc(object *monster);
int gmc_row_col(register int row, register int col);
void go_blind(void);
int gold_at(short row, short col);
void gr_armor(object *obj);
int gr_dir(void);
object * gr_monster(register object *monster, register int mn);
int gr_obj_char(void);
object * gr_object(void);
void gr_potion(object *obj);
void gr_ring(object *ring, boolean assign_wk);
int gr_room(void);
int gr_row_col(short *row, short *col, unsigned short mask);
void gr_scroll(object *obj);
void gr_wand(object *obj);
void gr_weapon(object *obj, int assign_wk);
unsigned short gr_what_is(void);
void hallucinate(void);
int has_amulet(void);
boolean has_been_touched(struct rogue_time *saved_time, struct rogue_time *mod_time);
void heal(void);
void hide_boxed_passage(short row1, short col1, short row2, short col2, short n);
void hold_monster(void);
int hp_raise(void);
void id_all(void);
void id_com(void);
void id_trap(void);
void id_type(void);
void idntfy(void);
int imitating(register short row, register short col);
int init(int argc, char **argv);
void init_str(char **str, char *dflt);
void insert_score(char scores[][SCORE_LINE_LEN+2], char n_names[][SCORE_NAME_LEN+1], char *n_name, short rank, short n, object *monster, int other);
void inv_armor_weapon(boolean is_weapon);
void inv_rings(void);
void inventory(object *pack, unsigned short mask);
int is_all_connected(void);
boolean is_digit(short ch);
boolean is_direction(short c, short *d);
int is_pack_letter(short *c, unsigned short *mask);
int is_passable(register int row, register int col);
int is_vowel(short ch);
void kick_into_pack(void);
void killed_by(object *monster, short other);
long lget_number(char *s);
void light_passage(int row, int col);
void light_up_room(int rn);
int m_confuse(object *monster);
void make_level(void);
void make_maze(short r, short c, short tr, short br, short lc, short rc);
void make_party(void);
void make_room(short rn, short r1, short r2, short r3);
void make_scroll_titles(void);
boolean mask_pack(object *pack, unsigned short mask);
boolean mask_room(short rn, short *row, short *col, unsigned short mask);
void md_save_tty(void);
void md_restore_tty(void);
void md_control_keybord(boolean mode);
boolean md_df(char *fname);
void md_exit(int status);
void md_gct(struct rogue_time *rt_buf);
int md_get_file_id(char *fname);
char * md_getenv(char *name);
void md_gfmt(char *fname, struct rogue_time *rt_buf);
char * md_gln(void);
int md_gseed(void);
void md_heed_signals(void);
void md_ignore_signals(void);
int md_link_count(char *fname);
void md_lock(boolean l);
char * md_malloc(int n);
void md_shell(char *shell);
void md_sleep(int nsecs);
void rogue_delay(void);
void rogue_show_cursor(boolean show);

void md_slurp(void);
void message(char *msg, boolean intrpt);
void mix_colors(void);
void mix_random_rooms(void);
int mon_can_go(register object *monster, register short row, register short col);
int mon_damage(object *monster, short damage);
void mon_hit(register object *monster);
char * mon_name(object *monster);
boolean mon_sees(object *monster, int row, int col);
int move_confused(object *monster);
void move_mon_to(register object *monster, register short row, register short col);
void move_onto(void);
int mtry(register object *monster, register short row, register short col);
void multiple_move_rogue(short dirch);
void mv_1_monster(register object *monster, short row, short col);
void mv_aquatars(void);
void mv_mons(void);
int name_cmp(char *s1, char *s2);
char * name_of(object *obj);
int next_avail_ichar(void);
int next_to_something(register int drow, register int dcol);
void nickize(char *buf, char *score, char *n_name);
int no_room_for_monster(int rn);
object * object_at(register object *pack, short row, short col);
int one_move_rogue(short dirch, short pickup);
void onintr(int sig);
void opt_erase(int i);
void opt_go(int i);
void opt_show(int i);
int pack_count(object *new_obj);
int pack_letter(char *prompt, unsigned short mask);
void pad(char *s, short n);
void party_monsters(int rn, int n);
int party_objects(int rn);
object * pick_up(int row, int col, short *status);
void place_at(object *obj, int row, int col);
void plant_gold(short row, short col, boolean is_maze);
void play_level(void);
void player_init(void);
void potion_heal(int extra);
int pr_com_id(int ch);
int pr_motion_char(int ch);
void print_stats(register int stat_mask);
void put_amulet(void);
int put_door(room *rm, short dir, short *row, short *col);
void put_gold(void);
void put_m_at(short row, short col, object *monster);
void put_mons(void);
void put_objects(void);
void put_player(short nr);
void put_on_ring(void);
void put_scores(object *monster, short other);
void put_stairs(void);
void quaff(void);
void quit(boolean from_intrpt);
int r_index(char *str, int ch, boolean last);
void r_read(FILE *fp, char *buf, int n);
void r_write(FILE *fp, char *buf, int n);
void rand_around(short i, short *r, short *c);
int rand_percent(register int percentage);
void rand_place(object *obj);
void read_pack(object *pack, FILE *fp, boolean is_rogue, boolean is_monsters);
void read_scroll(void);
void read_string(char *s, int maxlen, FILE *fp);
void recursive_deadend(short rn, short *offsets, short srow, short scol);
boolean reg_move(void);
void relight(void);
void remessage(short c);
void remove_ring(void);
void rest(int count);
void restore(char *fname);
int rgetchar(void);
void ring_stats(boolean pr);
int rogue_can_see(register int row, register int col);
void rogue_damage(short d, object *monster, short other);
void rogue_hit(register object *monster, boolean force_hit);
int rogue_is_around(register int row, register int col);
long rrandom(void);
void rust(object *monster);
void rw_dungeon(FILE *fp, boolean rw);
void rw_id(struct id *id_table, FILE *fp, int n, boolean wr);
void rw_rooms(FILE *fp, boolean rw);
void s_con_mon(object *monster);
int same_col(int room1, int room2);
int same_row(int room1, int room2);
void save_game(void);
void save_into_file(char *sfile);
void save_screen(void);
void search(short n, boolean is_auto);
int seek_gold(object *monster);
void sell_pack(void);
void sf_error(void);
void show_average_hp(void);
void show_monsters(void);
void show_objects(void);
void show_traps(void);
void single_inv(short ichar);
void sound_bell(void);
void special_hit(object *monster);
void srrandom(int x);
void start_window(void);
void steal_gold(object *monster);
void steal_item(object *monster);
void sting(object *monster);
void stop_window(void);
void take_a_nap(void);
void take_from_pack(object *obj, object *pack);
void take_off(void);
void tele(void);
void tele_away(object *monster);
void throw(void);
int throw_at_monster(object *monster, object *weapon);
int to_hit(object *obj);
int trap_at(register int row, register int col);
void trap_player(short row, short col);
int try_to_cough(short row, short col, object *obj);
void turn_passage(short dir, boolean fast);
void un_put_on(object *ring);
void unblind(void);
void unconfuse(void);
void uncurse_all(void);
void unhallucinate(void);
void unwear(object *obj);
void unwield(object *obj);
void vanish(object *obj, short rm, object *pack);
void visit_rooms(int rn);
void wait_for_ack(void);
void wake_room(short rn, boolean entering, short row, short col);
void wake_up(object *monster);
void wanderer(void);
void wdrain_life(object *monster);
void wear(void);
void wield(void);
void win(void);
void wizardize(void);
void write_pack(object *pack, FILE *fp);
void write_string(char *s, FILE *fp);
long xxx(boolean st);
void xxxx(char *buf, short n);
void zap_monster(object *monster, unsigned short kind);
void zapp(void);

#include <curses.h>

#include "jec/color.h"

#ifdef _WIN32
/* Windows-only interfaces, kept in their own headers as in Rogue 5.4.5. */
#include "windows/rogue_windows.h"
#include "windows/rogue_windows_console.h"
/* Preserve the original UTF-8 file and key-reading hooks. */
#define fopen rogue_win_fopen
#define getchar rogue_win_getchar
#endif

#if defined(MODERN_UNIX) || defined(_WIN32)
static inline char rogue_mvinch_char(int row, int col) { return (char)(mvinch(row, col) & 0xff); }
static inline short rogue_mvinch_short(int row, int col) { return (short)(mvinch(row, col) & 0xff); }
#define ROGUE_MVINCH_CHAR(row, col) rogue_mvinch_char((row), (col))
#define ROGUE_MVINCH_SHORT(row, col) rogue_mvinch_short((row), (col))
#else
#define ROGUE_MVINCH_CHAR(row, col) mvinch((row), (col))
#define ROGUE_MVINCH_SHORT(row, col) mvinch((row), (col))
#endif

/* PDCurses' narrow string functions depend on how the library was
 * compiled. Use its wide-character API explicitly for UTF-8 game text,
 * including user names, fruit, and score rows. */
#if defined(_WIN32) && !defined(ROGUE_COLOR_INTERNAL)
#ifdef addstr
#undef addstr
#endif
#ifdef mvaddstr
#undef mvaddstr
#endif
#define addstr(s) rogue_win_addstr_utf8((s))
#define mvaddstr(y,x,s) (move((y),(x)) == ERR ? ERR : rogue_win_addstr_utf8((s)))
#endif

#ifndef ROGUE_COLOR_INTERNAL
#ifdef addch
#undef addch
#endif
#ifdef mvaddch
#undef mvaddch
#endif
#ifdef clear
#undef clear
#endif
#define addch(ch) rogue_addch((int) (ch))
#define clear() rogue_clear()
#define mvaddch(row, col, ch) rogue_mvaddch((short) (row), (short) (col), (int) (ch))
#endif

#endif /* ROGUE_CLONE_III_H */
