/*
 * inventory.c
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
static char sccsid[] __attribute__((unused)) = "@(#)inventory.c	5.2 (Berkeley) 1/2/88";
#endif /* not lint */

#include "rogue.h"

boolean is_wood[WANDS];
short potion_title_id[POTIONS];
short wand_material_id[WANDS];
short ring_gem_id[RINGS];
short scroll_title_syllable_count[SCROLS];
short scroll_title_syllables[SCROLS][MAX_SCROLL_TITLE_SYLLABLES];
char *press_space = EN_STATIC(" --press space to continue--", " --スペースで続く-- ");


static void copy_id_title(char dest[ID_TITLE_SIZE], const char *src)
{
	if (!src) {
		dest[0] = '\0';
		return;
	}
	(void) snprintf(dest, ID_TITLE_SIZE, "%s", src);
}

static void cat_id_title(char dest[ID_TITLE_SIZE], const char *src)
{
	size_t len;

	if (!src) {
		return;
	}
	len = strlen(dest);
	if (len >= ID_TITLE_SIZE - 1) {
		return;
	}
	(void) snprintf(dest + len, ID_TITLE_SIZE - len, "%s", src);
}


char *wand_materials[WAND_MATERIALS] = {
	EN_STATIC("steel ", "鋼鉄"),
	EN_STATIC("bronze ", "青銅"),
	EN_STATIC("gold ", "金"),
	EN_STATIC("silver ", "銀"),
	EN_STATIC("copper ", "銅"),
	EN_STATIC("nickel ", "ニッケル"),
	EN_STATIC("cobalt ", "コバルト"),
	EN_STATIC("tin ", "すず"),
	EN_STATIC("iron ", "鉄"),
	EN_STATIC("magnesium ", "マグネシウム"),
	EN_STATIC("chrome ", "クロム"),
	EN_STATIC("carbon ", "石炭"),
	EN_STATIC("platinum ", "プラチナ"),
	EN_STATIC("silicon ", "シリコン"),
	EN_STATIC("titanium ", "チタン"),

	EN_STATIC("teak ", "チーク"),
	EN_STATIC("oak ", "樫"),
	EN_STATIC("cherry ", "桜"),
	EN_STATIC("birch ", "白樺"),
	EN_STATIC("pine ", "松"),
	EN_STATIC("cedar ", "杉"),
	EN_STATIC("redwood ", "セコイア"),
	EN_STATIC("balsa ", "バルサ"),
	EN_STATIC("ivory ", "象牙"),
	EN_STATIC("walnut ", "くるみ"),
	EN_STATIC("maple ", "かえで"),
	EN_STATIC("mahogany ", "マホガニー"),
	EN_STATIC("elm ", "にれ"),
	EN_STATIC("palm ", "やし"),
	EN_STATIC("wooden ", "木")
};

char *gems[GEMS] = {
	EN_STATIC("diamond ", "ダイヤ "),
	EN_STATIC("stibotantalite ", "安タンタル石 "),
	EN_STATIC("lapis-lazuli ", "るり "),
	EN_STATIC("ruby ", "ルビー "),
	EN_STATIC("emerald ", "エメラルド "),
	EN_STATIC("sapphire ", "サファイア "),
	EN_STATIC("amethyst ", "アメジスト "),
	EN_STATIC("quartz ", "水晶 "),
	EN_STATIC("tiger-eye ", "虎目石 "),
	EN_STATIC("opal ", "オパール "),
	EN_STATIC("agate ", "めのう "),
	EN_STATIC("turquoise ", "トルコ石 "),
	EN_STATIC("pearl ", "真珠 "),
	EN_STATIC("garnet ", "ざくろ石 ")
};

char *syllables_en[MAXSYLLABLES] = {
	"blech ", "foo ", "barf ", "rech ", "bar ", "blech ",
	"quo ", "bloto ", "oh ", "caca ", "blorp ", "erp ",
	"festr ", "rot ", "slie ", "snorf ", "iky ", "yuky ",
	"ooze ", "ah ", "bahl ", "zep ", "druhl ", "flem ",
	"behil ", "arek ", "mep ", "zihr ", "grit ", "kona ",
	"kini ", "ichi ", "tims ", "ogr ", "oo ", "ighr ",
	"coph ", "swerr ", "mihln ", "poxi "
};

char *syllables_ja[MAXSYLLABLES] = {
	"ぶれく ", "ふぅ ", "ばーふ ", "れく ", "ばぁ ", "ぶれく ",
	"くお ", "ぶろと ", "うぉ ", "かか ", "ぶろるぷ ", "えるぷ ",
	"ふぇすとる ", "ろと ", "すりぃ ", "すのるふ ", "いきぃ ", "ゆきぃ ",
	"うぅず ", "あぁ ", "ばぁる ", "ぜぷ ", "どるぅる ", "ふれむ ",
	"べひる ", "あれく ", "めぷ ", "じぃあ ", "ぐりと ", "こな ",
	"きに ", "いち ", "てぃむす ", "おぐる ", "うぅ ", "いぐる ",
	"こふ ", "すうぇる ", "みるん ", "ぽきし "
};

#define COMS 48

struct id_com_s {
	short com_char;
	char *com_desc;
};

struct id_com_s com_id_tab[COMS] = {
	{'?',	EN_STATIC("?       prints help", "?       ヘルプを表示する")},
	{'r',	EN_STATIC("r       read scroll", "r       巻き物を読む")},
	{'/',	EN_STATIC("/       identify object", "/       画面の文字を調べる")},
	{'e',	EN_STATIC("e       eat food", "e       食べ物を食べる")},
	{'h',	EN_STATIC("h       left ", "h       左")},
	{'w',	EN_STATIC("w       wield a weapon", "w       武器を手に持つ")},
	{'j',	EN_STATIC("j       down", "j       下")},
	{'W',	EN_STATIC("W       wear armor", "W       よろいを着る")},
	{'k',	EN_STATIC("k       up", "k       上")},
	{'T',	EN_STATIC("T       take armor off", "T       よろいを脱ぐ")},
	{'l',	EN_STATIC("l       right", "l       右")},
	{'P',	EN_STATIC("P       put on ring", "P       指輪をはめる")},
	{'y',	EN_STATIC("y       up & left", "y       左上")},
	{'R',	EN_STATIC("R       remove ring", "R       指輪をはずす")},
	{'u',	EN_STATIC("u       up & right", "u       右上")},
	{'d',	EN_STATIC("d       drop object", "d       持ちものを落とす")},
	{'b',	EN_STATIC("b       down & left", "b       左下")},
	{'c',	EN_STATIC("c       call object", "c       呼び名をつける")},
	{'n',	EN_STATIC("n       down & right", "n       右下")},
	{0,	EN_STATIC("<SHIFT><dir>: run that way", "<SHIFT><方角>: 進み続ける")},
	{')',	EN_STATIC(")       print current weapon", ")       現在の武器を見る")},
	{0,	EN_STATIC("<CTRL><dir>: run till adjacent", "<CTRL><方角>: 見つけるまで進む")},
	{']',	EN_STATIC("]       print current armor", "]       現在のよろいを見る")},
	{'f',	EN_STATIC("f<dir>  fight till death or near death", "f<方角>  危なくなるまで戦う")},
	{'=',	EN_STATIC("=       print current rings", "=       現在の指輪を見る")},
	{'t',	EN_STATIC("t<dir>  throw something", "t<方角>  物を投げる")},
	{'\001',	EN_STATIC("^A      print Hp-raise average", "^A      体力上昇率を見る")},
	{'m',	EN_STATIC("m<dir>  move onto without picking up", "m<方角>  拾わず上に乗る")},
	{'z',	EN_STATIC("z<dir>  zap a wand in a direction", "z<方角>  魔法の棒を振る")},
	{'o',	EN_STATIC("o       examine/set options", "o       オプション表示・設定")},
	{'^',	EN_STATIC("^<dir>  identify trap type", "^<方角>  わなの種類を調べる")},
	{'\022',	EN_STATIC("^R      redraw screen", "^R      画面を描き直す")},
	{'&',	EN_STATIC("&       save screen into 'rogue.screen'", "&       画面をファイルへ保存")},
	{'s',	EN_STATIC("s       search for trap/secret door", "s       わなや隠し扉を探す")},
	{'\020',	EN_STATIC("^P      repeat last message", "^P      最後のメッセージを再表示")},
	{'>',	EN_STATIC(">       go down a staircase", ">       階段を降りる")},
	{'\033',	EN_STATIC("^[      cancel command", "^[      コマンドを中止")},
	{'<',	EN_STATIC("<       go up a staircase", "<       階段を登る")},
	{'S',	EN_STATIC("S       save game", "S       セーブして終了")},
	{'.',	EN_STATIC(".       rest for a turn", ".       その場で休む")},
	{'Q',	EN_STATIC("Q       quit", "Q       ゲームを中止")},
	{',',	EN_STATIC(",       pick something up", ",       足元のものを拾う")},
	{'!',	EN_STATIC("!       shell escape", "!       シェルを実行")},
	{'i',	EN_STATIC("i       inventory", "i       持ちものを見る")},
	{'F',	EN_STATIC("F<dir>  fight till either of you dies", "F<方角>  どちらか倒れるまで戦う")},
	{'I',	EN_STATIC("I       inventory single item", "I       1つの持ちものを見る")},
	{'v',	EN_STATIC("v       print version number", "v       バージョン情報")},
	{'q',	EN_STATIC("q       quaff potion", "q       水薬を飲む")}
};

static char *com_desc_en[COMS] = {
	"?       prints help",
	"r       read scroll",
	"/       identify object",
	"e       eat food",
	"h       left ",
	"w       wield a weapon",
	"j       down",
	"W       wear armor",
	"k       up",
	"T       take armor off",
	"l       right",
	"P       put on ring",
	"y       up & left",
	"R       remove ring",
	"u       up & right",
	"d       drop object",
	"b       down & left",
	"c       call object",
	"n       down & right",
	"<SHIFT><dir>: run that way",
	")       print current weapon",
	"<CTRL><dir>: run till adjacent",
	"]       print current armor",
	"f<dir>  fight till death or near death",
	"=       print current rings",
	"t<dir>  throw something",
	"^A      print Hp-raise average",
	"m<dir>  move onto without picking up",
	"z<dir>  zap a wand in a direction",
	"o       examine/set options",
	"^<dir>  identify trap type",
	"^R      redraw screen",
	"&       save screen into 'rogue.screen'",
	"s       search for trap/secret door",
	"^P      repeat last message",
	">       go down a staircase",
	"^[      cancel command",
	"<       go up a staircase",
	"S       save game",
	".       rest for a turn",
	"Q       quit",
	",       pick something up",
	"!       shell escape",
	"i       inventory",
	"F<dir>  fight till either of you dies",
	"I       inventory single item",
	"v       print version number",
	"q       quaff potion"
};

static char *com_desc_ja[COMS] = {
	"?       ヘルプを表示する",
	"r       巻き物を読む",
	"/       画面の文字を調べる",
	"e       食べ物を食べる",
	"h       左",
	"w       武器を手に持つ",
	"j       下",
	"W       よろいを着る",
	"k       上",
	"T       よろいを脱ぐ",
	"l       右",
	"P       指輪をはめる",
	"y       左上",
	"R       指輪をはずす",
	"u       右上",
	"d       持ちものを落とす",
	"b       左下",
	"c       呼び名をつける",
	"n       右下",
	"<SHIFT><方角>: 進み続ける",
	")       現在の武器を見る",
	"<CTRL><方角>: 見つけるまで進む",
	"]       現在のよろいを見る",
	"f<方角>  危なくなるまで戦う",
	"=       現在の指輪を見る",
	"t<方角>  物を投げる",
	"^A      体力上昇率を見る",
	"m<方角>  拾わず上に乗る",
	"z<方角>  魔法の棒を振る",
	"o       オプション表示・設定",
	"^<方角>  わなの種類を調べる",
	"^R      画面を描き直す",
	"&       画面をファイルへ保存",
	"s       わなや隠し扉を探す",
	"^P      最後のメッセージを再表示",
	">       階段を降りる",
	"^[      コマンドを中止",
	"<       階段を登る",
	"S       セーブして終了",
	".       その場で休む",
	"Q       ゲームを中止",
	",       足元のものを拾う",
	"!       シェルを実行",
	"i       持ちものを見る",
	"F<方角>  どちらか倒れるまで戦う",
	"I       1つの持ちものを見る",
	"v       バージョン情報",
	"q       水薬を飲む"
};

void rogue_apply_command_language(void)
{
	short i;

	for (i = 0; i < COMS; i++) {
		com_id_tab[i].com_desc = rogue_lang_is_japanese ? com_desc_ja[i] : com_desc_en[i];
	}
}


extern boolean wizard;
extern char *m_names[], *more;

void inventory(object *pack, unsigned short mask)
{
	object *obj;
	short i = 0, j, maxlen = 0, n;
	char descs[MAX_PACK_COUNT+1][ROGUE_DESC_LEN];
	char save[DROWS][DCOLS];
	short row, col, rows_shown;

	obj = pack->next_object;

	if (!obj) {
		message(EN("your pack is empty", "持ちものは、１つも持っていない。"), 0);
		return;
	}
	while (obj) {
		if (obj->what_is & mask) {
			descs[i][0] = ' ';
			descs[i][1] = obj->ichar;
			descs[i][2] = ((obj->what_is & ARMOR) && obj->is_protected)
				? '}' : ')';
			descs[i][3] = ' ';
			get_desc_into(obj, descs[i]+4, ROGUE_DESC_LEN-4);
			if ((n = jrogue_text_width(descs[i])) > maxlen) {
				maxlen = n;
			}
		i++;
		}
		obj = obj->next_object;
	}
	(void) strcpy(descs[i++], press_space);
	if (maxlen < 27) maxlen = 27;
	if (maxlen > (DCOLS - 2)) maxlen = DCOLS - 2;
	col = DCOLS - (maxlen + 2);
	rows_shown = (i < DROWS) ? i : DROWS;

	for (row = 0; row < rows_shown; row++) {
		for (j = col; j < DCOLS; j++) {
			save[row][j] = ROGUE_MVINCH_CHAR(row, j);
		}
	}

	for (row = 0; row < rows_shown; row++) {
		move(row, col);
		rogue_addstr_fit(descs[row], DCOLS - col);
		clrtoeol();
	}
	refresh();
	wait_for_ack();

	for (row = 0; row < rows_shown; row++) {
		move(row, col);
		for (j = col; j < DCOLS; j++) {
			addch(save[row][j]);
		}
	}
	/*
	 * When the pack is full, the inventory window reaches the
	 * status line.  Restoring that line character-by-character is
	 * not safe for UTF-8 text, so redraw the complete status line
	 * after closing the inventory display.
	 */
	print_stats(STAT_ALL);
	move(rogue.row, rogue.col);
	refresh();
}

void id_com(void)
{
	int ch = 0;
	short i, k;

	while (ch != CANCEL) {
		check_message();
		message(EN("Character you want help for (* for all):",
			"どの文字のヘルプ？（*ですべて）:"), 0);

		refresh();
#ifndef _WIN32
		/* This help prompt reads directly, bypassing rgetchar(). */
		rogue_prepare_input_attr(stdscr);
#endif
		ch = getchar();

		switch(ch) {
		case LIST:
			{
				WINDOW *saved_screen;
				int saved_y, saved_x;
				short rows = (((COMS / 2) + (COMS % 2)) + 1);
				boolean need_two_screens;

				/* Preserve the entire curses screen, including Unicode and
				 * color attributes.  Saving individual char cells loses both. */
				getyx(stdscr, saved_y, saved_x);
				saved_screen = dupwin(stdscr);

				if (rows > LINES) {
					need_two_screens = 1;
					rows = LINES;
				}
				k = 0;
MORE:
				for (i = 0; i < rows; i++) {
					move(i, 0);
					clrtoeol();
				}
				for (i = 0; i < (rows-1); i++) {
					if (i < (LINES-1)) {
						if (((i + i) < COMS) && ((i+i+k) < COMS)) {
							mvaddstr(i, 0, com_id_tab[i+i+k].com_desc);
						}
						if (((i + i + 1) < COMS) && ((i+i+k+1) < COMS)) {
							mvaddstr(i, (DCOLS/2),
										com_id_tab[i+i+k+1].com_desc);
						}
					}
				}
				mvaddstr(rows - 1, 0, need_two_screens ? more : press_space);
				refresh();
				wait_for_ack();

				if (need_two_screens) {
					k += ((rows-1) * 2);
					need_two_screens = 0;
					goto MORE;
				}
				if (saved_screen) {
					if (overwrite(saved_screen, stdscr) != ERR) {
						/* Redraw the saved cells rather than regenerating the
						 * visible terrain from the player's current position. */
						touchwin(stdscr);
						move(saved_y, saved_x);
						refresh();
					} else {
						rogue_redraw_screen();
					}
					delwin(saved_screen);
				} else {
					/* Allocation failure: at least restore the local view. */
					rogue_redraw_screen();
				}
			}
			break;
		default:
			if (!pr_com_id(ch)) {
				if (!pr_motion_char(ch)) {
					check_message();
					message(EN("unknown character", "知らない文字です。"), 0);
				}
			}
			ch = CANCEL;
			break;
		}
	}
}

int pr_com_id(int ch)
{
	int i;

	if (!get_com_id(&i, ch)) {
		return(0);
	}
	check_message();
	message(com_id_tab[i].com_desc, 0);
	return(1);
}

int get_com_id(int *index, short ch)
{
	short i;

	for (i = 0; i < COMS; i++) {
		if (com_id_tab[i].com_char == ch) {
			*index = i;
			return(1);
		}
	}
	return(0);
}

int pr_motion_char(int ch)
{
	if (	(ch == 'J') ||
			(ch == 'K') ||
			(ch == 'L') ||
			(ch == 'H') ||
			(ch == 'Y') ||
			(ch == 'U') ||
			(ch == 'N') ||
			(ch == 'B') ||
			(ch == '\012') ||
			(ch == '\013') ||
			(ch == '\010') ||
			(ch == '\014') ||
			(ch == '\025') ||
			(ch == '\031') ||
			(ch == '\016') ||
			(ch == '\002')) {
		char until[18], buf[DCOLS];
		int n;
		boolean until_adjacent = (ch <= '\031');

		if (until_adjacent) {
			ch += 96;
			(void) strcpy(until, "until adjacent");
		} else {
			ch += 32;
			until[0] = '\0';
		}
		if (!get_com_id(&n, ch)) {
			return(0);
		}
		if (rogue_lang_is_japanese) {
			(void) snprintf(buf, sizeof(buf),
				until_adjacent ? "%sへ障害物の手前まで走る" : "%sへ走る",
				com_id_tab[n].com_desc + 8);
		} else {
			/* Preserve the original English wording and spacing. */
			(void) snprintf(buf, sizeof(buf), "run %s %s",
				com_id_tab[n].com_desc + 8, until);
		}
		check_message();
		message(buf, 0);
		return(1);
	} else {
		return(0);
	}
}

void rogue_refresh_generated_titles(void)
{
	short i, j, n;

	for (i = 0; i < POTIONS; i++) {
		if (id_potions[i].id_status != CALLED) {
			copy_id_title(id_potions[i].title,
				rogue_potion_title_for_id(potion_title_id[i]));
		}
	}
	for (i = 0; i < WANDS; i++) {
		if (id_wands[i].id_status != CALLED) {
			copy_id_title(id_wands[i].title,
				rogue_wand_material_for_id(wand_material_id[i]));
		}
	}
	for (i = 0; i < RINGS; i++) {
		if (id_rings[i].id_status != CALLED) {
			copy_id_title(id_rings[i].title,
				rogue_ring_gem_for_id(ring_gem_id[i]));
		}
	}
	for (i = 0; i < SCROLS; i++) {
		if (id_scrolls[i].id_status == CALLED) {
			continue;
		}
		copy_id_title(id_scrolls[i].title, "'");
		for (j = 0; j < scroll_title_syllable_count[i]; j++) {
			n = scroll_title_syllables[i][j];
			if ((n < 0) || (n >= MAXSYLLABLES)) {
				n = 0;
			}
			cat_id_title(id_scrolls[i].title,
				rogue_lang_is_japanese ? syllables_ja[n] : syllables_en[n]);
		}
		n = strlen(id_scrolls[i].title);
		if (n > 0) {
			(void) snprintf(id_scrolls[i].title + (n - 1),
				ID_TITLE_SIZE - (n - 1), "' ");
		}
	}
}

void mix_colors(void)
{
	short i, j, k, t;

	for (i = 0; i < POTIONS; i++) {
		potion_title_id[i] = i;
	}
	for (i = 0; i <= 32; i++) {
		j = get_rand(0, (POTIONS - 1));
		k = get_rand(0, (POTIONS - 1));
		t = potion_title_id[j];
		potion_title_id[j] = potion_title_id[k];
		potion_title_id[k] = t;
	}
	rogue_refresh_generated_titles();
}

void make_scroll_titles(void)
{
	short i, j;

	for (i = 0; i < SCROLS; i++) {
		scroll_title_syllable_count[i] = get_rand(2, MAX_SCROLL_TITLE_SYLLABLES);
		for (j = 0; j < scroll_title_syllable_count[i]; j++) {
			scroll_title_syllables[i][j] = get_rand(1, (MAXSYLLABLES-1));
		}
		for (; j < MAX_SCROLL_TITLE_SYLLABLES; j++) {
			scroll_title_syllables[i][j] = 0;
		}
	}
	rogue_refresh_generated_titles();
}

void get_desc(object *obj, char *desc)
{
	get_desc_into(obj, desc, ROGUE_DESC_LEN);
}

static void safe_cat(char *dest, int len, char *src)
{
	size_t used = strlen(dest);

	if ((int) used >= len - 1) {
		return;
	}
	(void) snprintf(dest + used, len - used, "%s", src);
}

static char *jp_scroll_title(char *title)
{
	static char buf[ID_TITLE_SIZE];
	size_t start = 0, end;

	(void) snprintf(buf, sizeof(buf), "%s", title);
	end = strlen(buf);
	while ((start < end) && ((buf[start] == '\'') || (buf[start] == ' '))) {
		start++;
	}
	while ((end > start) && ((buf[end-1] == '\'') || (buf[end-1] == ' '))) {
		buf[--end] = 0;
	}
	return(buf + start);
}

static char *jp_trim_title(char *title)
{
	static char buf[ID_TITLE_SIZE];
	size_t end;

	(void) snprintf(buf, sizeof(buf), "%s", title ? title : "");
	end = strlen(buf);
	while ((end > 0) && (buf[end-1] == ' ')) {
		buf[--end] = 0;
	}
	return(buf);
}

static char *jp_counter(object *obj)
{
	if (obj->what_is == SCROL) {
		return("本の");
	}
	if ((obj->what_is == WEAPON) &&
		((obj->which_kind == DART) || (obj->which_kind == ARROW) ||
		(obj->which_kind == DAGGER))) {
		return("本の");
	}
	if ((obj->what_is == WEAPON) && (obj->which_kind == SHURIKEN)) {
		return("枚の");
	}
	if (obj->what_is == FOOD) {
		return("袋の");
	}
	return("個の");
}

static void jp_cat_modifier(char *desc, int len, char *kind, char *item_name)
{
	safe_cat(desc, len, jp_trim_title(kind));
	safe_cat(desc, len, item_name);
}

void get_desc_into(object *obj, char *desc, int len)
{
	char *item_name;
	struct id *id_table;
	char more_info[32];
	short i;

if (rogue_lang_is_japanese) {
		item_name = name_of(obj);
	id_table = get_id_table(obj);

	if (obj->what_is == AMULET) {
		(void) snprintf(desc, len, "イェンダーの魔除け");
		goto JP_FLAGS;
	}
	if (obj->what_is == GOLD) {
		(void) snprintf(desc, len, "%d個の金塊", obj->quantity);
		return;
	}
	if (obj->what_is == FOOD) {
		if (obj->which_kind == RATION) {
			(void) snprintf(desc, len, "%d袋の食糧", obj->quantity);
		} else if (obj->quantity > 1) {
			(void) snprintf(desc, len, "%d個の%s", obj->quantity, item_name);
		} else {
			(void) snprintf(desc, len, "%s", item_name);
		}
		goto JP_FLAGS;
	}
	desc[0] = 0;
	if ((obj->what_is != ARMOR) && (obj->quantity > 1)) {
		(void) snprintf(desc, len, "%d%s", obj->quantity, jp_counter(obj));
	}
	if (wizard) {
		goto JP_ID;
	}
	if (obj->what_is & (WEAPON | ARMOR | WAND | RING)) {
		goto JP_CHECK;
	}
	switch(id_table[obj->which_kind].id_status) {
	case UNIDENTIFIED:
JP_CHECK:
		switch(obj->what_is) {
		case SCROL:
			(void) snprintf(desc+strlen(desc), len-strlen(desc),
				"「%s」と書かれた%s", jp_scroll_title(id_table[obj->which_kind].title),
				item_name);
			break;
		case POTION:
			(void) snprintf(desc+strlen(desc), len-strlen(desc),
				"%s%s", jp_trim_title(id_table[obj->which_kind].title),
				item_name);
			break;
		case WAND:
		case RING:
			if (obj->identified ||
			(id_table[obj->which_kind].id_status == IDENTIFIED)) {
				goto JP_ID;
			}
			if (id_table[obj->which_kind].id_status == CALLED) {
				goto JP_CALL;
			}
			(void) snprintf(desc+strlen(desc), len-strlen(desc),
				"%sの%s", jp_trim_title(id_table[obj->which_kind].title),
				item_name);
			break;
		case ARMOR:
			if (obj->identified) {
				goto JP_ID;
			}
			safe_cat(desc, len, id_table[obj->which_kind].title);
			break;
		case WEAPON:
			if (obj->identified) {
				goto JP_ID;
			}
			safe_cat(desc, len, item_name);
			break;
		}
		break;
	case CALLED:
JP_CALL:
		(void) snprintf(desc+strlen(desc), len-strlen(desc),
			"%s「%s」と呼ぶ", item_name,
			id_table[obj->which_kind].title);
		break;
	case IDENTIFIED:
JP_ID:
		switch(obj->what_is) {
		case SCROL:
			(void) snprintf(desc+strlen(desc), len-strlen(desc),
				"%s%s", jp_trim_title(id_table[obj->which_kind].real), item_name);
			break;
		case POTION:
			(void) snprintf(desc+strlen(desc), len-strlen(desc),
				"%s%s", jp_trim_title(id_table[obj->which_kind].real),
				item_name);
			break;
		case RING:
			if (wizard || obj->identified) {
				if ((obj->which_kind == DEXTERITY) ||
					(obj->which_kind == ADD_STRENGTH)) {
					sprintf(more_info, "%s%d ", ((obj->class > 0) ? "+" : ""),
						obj->class);
					safe_cat(desc, len, more_info);
				}
			}
			jp_cat_modifier(desc, len, id_table[obj->which_kind].real, item_name);
			break;
		case WAND:
			jp_cat_modifier(desc, len, id_table[obj->which_kind].real, item_name);
			if (wizard || obj->identified) {
				sprintf(more_info, "[%d]", obj->class);
				safe_cat(desc, len, more_info);
			}
			break;
		case ARMOR:
			(void) snprintf(desc, len, "(%s%d) %s", ((obj->d_enchant >= 0) ? "+" : ""),
				obj->d_enchant, id_table[obj->which_kind].title);
			sprintf(more_info, "[守備 %d]", get_armor_class(obj));
			safe_cat(desc, len, more_info);
			break;
		case WEAPON:
			(void) snprintf(more_info, sizeof(more_info), "%s", desc);
			(void) snprintf(desc, len, "(%s%d,%s%d) %s%s",
				((obj->hit_enchant >= 0) ? "+" : ""), obj->hit_enchant,
				((obj->d_enchant >= 0) ? "+" : ""), obj->d_enchant,
				more_info, item_name);
			break;
		}
		break;
	}
JP_FLAGS:
	if (obj->in_use_flags & BEING_WIELDED) {
		safe_cat(desc, len, "（使っている）");
	} else if (obj->in_use_flags & BEING_WORN) {
		safe_cat(desc, len, "（着ている）");
	} else if (obj->in_use_flags & ON_LEFT_HAND) {
		safe_cat(desc, len, "（左手）");
	} else if (obj->in_use_flags & ON_RIGHT_HAND) {
		safe_cat(desc, len, "（右手）");
	}
		return;
	}

	if (obj->what_is == AMULET) {
		(void) snprintf(desc, len, "the amulet of Yendor ");
		return;
	}
	item_name = name_of(obj);

	if (obj->what_is == GOLD) {
		(void) snprintf(desc, len, "%d pieces of gold", obj->quantity);
		return;
	}

	desc[0] = 0;
	if (obj->what_is != ARMOR) {
		if (obj->quantity == 1) {
			(void) snprintf(desc, len, "a ");
		} else {
			(void) snprintf(desc, len, "%d ", obj->quantity);
		}
	}
	if (obj->what_is == FOOD) {
		if (obj->which_kind == RATION) {
			if (obj->quantity > 1) {
				(void) snprintf(desc, len, "%d rations of ", obj->quantity);
			} else {
				(void) snprintf(desc, len, "some ");
			}
		} else {
			(void) snprintf(desc, len, "a ");
		}
		safe_cat(desc, len, item_name);
		goto ANA;
	}
	id_table = get_id_table(obj);

	if (wizard) {
		goto ID;
	}
	if (obj->what_is & (WEAPON | ARMOR | WAND | RING)) {
		goto CHECK;
	}

	switch(id_table[obj->which_kind].id_status) {
	case UNIDENTIFIED:
	CHECK:
		switch(obj->what_is) {
		case SCROL:
			safe_cat(desc, len, item_name);
			safe_cat(desc, len, "entitled: ");
			safe_cat(desc, len, id_table[obj->which_kind].title);
			break;
		case POTION:
			safe_cat(desc, len, id_table[obj->which_kind].title);
			safe_cat(desc, len, item_name);
			break;
		case WAND:
		case RING:
			if (obj->identified ||
			(id_table[obj->which_kind].id_status == IDENTIFIED)) {
				goto ID;
			}
			if (id_table[obj->which_kind].id_status == CALLED) {
				goto CALL;
			}
			safe_cat(desc, len, id_table[obj->which_kind].title);
			safe_cat(desc, len, item_name);
			break;
		case ARMOR:
			if (obj->identified) {
				goto ID;
			}
			(void) snprintf(desc, len, "%s", id_table[obj->which_kind].title);
			break;
		case WEAPON:
			if (obj->identified) {
				goto ID;
			}
			safe_cat(desc, len, name_of(obj));
			break;
		}
		break;
	case CALLED:
CALL:	switch(obj->what_is) {
		case SCROL:
		case POTION:
		case WAND:
		case RING:
			safe_cat(desc, len, item_name);
			safe_cat(desc, len, "called ");
			safe_cat(desc, len, id_table[obj->which_kind].title);
			break;
		}
		break;
	case IDENTIFIED:
ID:		switch(obj->what_is) {
		case SCROL:
		case POTION:
			safe_cat(desc, len, item_name);
			safe_cat(desc, len, id_table[obj->which_kind].real);
			break;
		case RING:
			if (wizard || obj->identified) {
				if ((obj->which_kind == DEXTERITY) ||
					(obj->which_kind == ADD_STRENGTH)) {
					(void) snprintf(more_info, sizeof(more_info),
						"%s%d ", ((obj->class > 0) ? "+" : ""),
						obj->class);
					safe_cat(desc, len, more_info);
				}
			}
			safe_cat(desc, len, item_name);
			safe_cat(desc, len, id_table[obj->which_kind].real);
			break;
		case WAND:
			safe_cat(desc, len, item_name);
			safe_cat(desc, len, id_table[obj->which_kind].real);
			if (wizard || obj->identified) {
				(void) snprintf(more_info, sizeof(more_info), "[%d]", obj->class);
				safe_cat(desc, len, more_info);
			}
			break;
		case ARMOR:
			(void) snprintf(desc, len, "%s%d ", ((obj->d_enchant >= 0) ? "+" : ""),
			obj->d_enchant);
			safe_cat(desc, len, id_table[obj->which_kind].title);
			(void) snprintf(more_info, sizeof(more_info), "[%d] ", get_armor_class(obj));
			safe_cat(desc, len, more_info);
			break;
		case WEAPON:
			(void) snprintf(desc+strlen(desc), len-strlen(desc), "%s%d,%s%d ",
			((obj->hit_enchant >= 0) ? "+" : ""), obj->hit_enchant,
			((obj->d_enchant >= 0) ? "+" : ""), obj->d_enchant);
			safe_cat(desc, len, name_of(obj));
			break;
		}
		break;
	}
	ANA:
	if (!strncmp(desc, "a ", 2)) {
		if (is_vowel(desc[2])) {
			if ((int) strlen(desc) + 1 < len) {
				for (i = strlen(desc) + 1; i > 1; i--) {
					desc[i] = desc[i-1];
				}
				desc[1] = 'n';
			}
		}
	}
	if (obj->in_use_flags & BEING_WIELDED) {
		safe_cat(desc, len, "in hand");
	} else if (obj->in_use_flags & BEING_WORN) {
		safe_cat(desc, len, "being worn");
	} else if (obj->in_use_flags & ON_LEFT_HAND) {
		safe_cat(desc, len, "on left hand");
	} else if (obj->in_use_flags & ON_RIGHT_HAND) {
		safe_cat(desc, len, "on right hand");
	}
}

void get_wand_and_ring_materials(void)
{
	short i, j;
	boolean used[WAND_MATERIALS];

	for (i = 0; i < WAND_MATERIALS; i++) {
		used[i] = 0;
	}
	for (i = 0; i < WANDS; i++) {
		do {
			j = get_rand(0, WAND_MATERIALS-1);
		} while (used[j]);
		used[j] = 1;
		wand_material_id[i] = j;
		is_wood[i] = (j > MAX_METAL);
	}
	for (i = 0; i < GEMS; i++) {
		used[i] = 0;
	}
	for (i = 0; i < RINGS; i++) {
		do {
			j = get_rand(0, GEMS-1);
		} while (used[j]);
		used[j] = 1;
		ring_gem_id[i] = j;
	}
	rogue_refresh_generated_titles();
}

void single_inv(short ichar)
{
	short ch;
	char desc[ROGUE_DESC_LEN];
	object *obj;

	ch = ichar ? ichar : pack_letter(EN("inventory what?", "調べる持ちものの種類は？"), ALL_OBJECTS);

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message(EN("no such item.", "それは持っていない。"), 0);
		return;
	}
	desc[0] = ch;
	desc[1] = ((obj->what_is & ARMOR) && obj->is_protected) ? '}' : ')';
	desc[2] = ' ';
	desc[3] = 0;
	get_desc_into(obj, desc+3, ROGUE_DESC_LEN-3);
	message(desc, 0);
}

struct id * get_id_table(object *obj)
{
	switch(obj->what_is) {
	case SCROL:
		return(id_scrolls);
	case POTION:
		return(id_potions);
	case WAND:
		return(id_wands);
	case RING:
		return(id_rings);
	case WEAPON:
		return(id_weapons);
	case ARMOR:
		return(id_armors);
	}
	return((struct id *) 0);
}

void inv_armor_weapon(boolean is_weapon)
{
	if (is_weapon) {
		if (rogue.weapon) {
			single_inv(rogue.weapon->ichar);
		} else {
			message(EN("not wielding anything", "武器は使っていない。"), 0);
		}
	} else {
		if (rogue.armor) {
			single_inv(rogue.armor->ichar);
		} else {
			message(EN("not wearing anything", "よろいは着ていない。"), 0);
		}
	}
}

void id_type(void)
{
	char *id;
	int ch;
	char buf[ROGUE_DESC_LEN];

	message(EN("what do you want identified?", "どんな文字ですか？"), 0);
	rogue_show_cursor(1);

	ch = rgetchar();
	rogue_show_cursor(show_cursor);

	if ((ch >= 'A') && (ch <= 'Z')) {
		id = m_names[ch-'A'];
	} else if (ch < 32) {
		check_message();
		return;
	} else {
		switch(ch) {
		case '@':
			id = EN("you", "プレイヤー");
			break;
		case '%':
			id = EN("staircase", "隣の階への階段");
			break;
		case '^':
			id = EN("trap", "隠されたわな");
			break;
		case '+':
			id = EN("door", "部屋のドア");
			break;
		case '-':
		case '|':
			id = EN("wall of a room", "部屋の壁");
			break;
		case '.':
			id = EN("floor", "部屋の床");
			break;
		case '#':
			id = EN("passage", "部屋と部屋をむすぶ通路");
			break;
		case ' ':
			id = EN("solid rock", "岩");
			break;
		case '=':
			id = EN("ring", "魔法の指輪");
			break;
		case '?':
			id = EN("scroll", "巻き物");
			break;
		case '!':
			id = EN("potion", "水薬");
			break;
		case '/':
			id = EN("wand or staff", "魔法の棒／つえ");
			break;
		case ')':
			id = EN("weapon", "武器");
			break;
		case ']':
			id = EN("armor", "よろい");
			break;
		case '*':
			id = EN("gold", "金塊の山か壺");
			break;
		case ':':
			id = EN("food", "食糧");
			break;
		case ',':
			id = EN("the Amulet of Yendor", "イェンダーの魔除け");
			break;
		default:
			id = EN("unknown character", "知らない文字");
			break;
		}
	}
	check_message();
	(void) snprintf(buf, sizeof(buf), "'%c': %s", ch, id);
	message(buf, 0);
}
