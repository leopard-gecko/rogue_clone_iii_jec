/* Runtime language support for Rogue Clone III. */
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif
#endif
#include "rogue.h"

/* Use UTF-8 for text processing on Windows, just like Rogue 5.4.5 JEC.
 * The startup game language is chosen separately from Windows UI language. */
void rogue_init_locale(void)
{
#ifdef _WIN32
    if (setlocale(LC_ALL, ".UTF8") == NULL)
        (void)setlocale(LC_ALL, "");
#else
    (void)setlocale(LC_ALL, "");
#endif
}

int rogue_lang_is_japanese = DEFAULT_JAPANESE;
char *language_option = (char *) 0;

extern char *you_can_move_again;
extern char *curse_message;
extern char *left_or_right;
extern char *no_ring;
extern char *strange_feeling;
extern char *unknown_command;
extern char *fruit;
extern char hunger_str[];

#ifndef _WIN32
static int lang_is(char *s, char a, char b)
{
	if (!s || !*s) return(0);
	return(((s[0] == a) || (s[0] == a - 32)) &&
		((s[1] == b) || (s[1] == b - 32)));
}
#endif

/* An exact, case-insensitive match: do not accept "japanese" or "english". */
static int lang_code_is(const char *code, const char *expected)
{
	unsigned char a, b;

	if (!code || !expected) {
		return(0);
	}
	while (*code && *expected) {
		a = (unsigned char) *code++;
		b = (unsigned char) *expected++;
		if ((a >= 'A') && (a <= 'Z')) {
			a += 'a' - 'A';
		}
		if (a != b) {
			return(0);
		}
	}
	return((*code == '\0') && (*expected == '\0'));
}

char *rogue_str(char *en, char *ja)
{
	return(rogue_lang_is_japanese ? ja : en);
}

char *rogue_language_code(void)
{
	return(rogue_lang_is_japanese ? "ja" : "en");
}

/* Only Japanese and English may be selected during a game. */
void rogue_set_language_code(char *code)
{
	if (lang_code_is(code, "ja")) {
		rogue_lang_is_japanese = 1;
	} else if (lang_code_is(code, "en")) {
		rogue_lang_is_japanese = 0;
	}
	if (language_option) {
		(void) strcpy(language_option, rogue_language_code());
	}
}

/* Automatic selection is for initial startup only, never the options UI. */
static void rogue_detect_os_language(void)
{
#ifdef _WIN32
	/* Match the system UI language, as in Rogue 5.4.5's Windows build. */
	rogue_lang_is_japanese =
		(PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_JAPANESE);
#else
	char *lang;

	lang = md_getenv("LC_ALL");
	if (!lang || !*lang) {
		lang = md_getenv("LC_MESSAGES");
	}
	if (!lang || !*lang) {
		lang = md_getenv("LANG");
	}
	/* All non-Japanese locales, including C and POSIX, default to English. */
	rogue_lang_is_japanese = lang_is(lang, 'j', 'a');
#endif
}

void rogue_detect_language(void)
{
#if DEFAULT_LANGUAGE == 1
	rogue_lang_is_japanese = 1;
#elif DEFAULT_LANGUAGE == 2
	rogue_lang_is_japanese = 0;
#else
	rogue_detect_os_language();
#endif
}

/* ROGUEOPTS at startup overrides the build default.  "auto" explicitly
 * requests OS detection, even when the binary's default is ja or en.
 * Return false for malformed values so the caller can reject them.
 */
int rogue_set_initial_language_code(const char *code)
{
	if (lang_code_is(code, "auto")) {
		rogue_detect_os_language();
	} else if (lang_code_is(code, "ja")) {
		rogue_lang_is_japanese = 1;
	} else if (lang_code_is(code, "en")) {
		rogue_lang_is_japanese = 0;
	} else {
		return(0);
	}
	if (language_option) {
		(void) strcpy(language_option, rogue_language_code());
	}
	return(1);
}


static int rogue_is_default_fruit_name(char *s, char *name)
{
	int len;

	if (!s || !name) {
		return(0);
	}
	len = strlen(name);
	if (strncmp(s, name, len)) {
		return(0);
	}
	while (s[len] == ' ') {
		len++;
	}
	return(s[len] == '\0');
}

static void rogue_apply_fruit_language(void)
{
	if (!fruit) {
		return;
	}
	if (rogue_is_default_fruit_name(fruit, "slime-mold") ||
	    rogue_is_default_fruit_name(fruit, "ネバネバかび")) {
		(void) strcpy(fruit, rogue_lang_is_japanese ? "ネバネバかび" : "slime-mold ");
	}
}

static void rogue_apply_hunger_language(void)
{
	if (!hunger_str[0]) {
		return;
	}
	if (!strcmp(hunger_str, "hungry") || !strcmp(hunger_str, "空腹")) {
		(void) strcpy(hunger_str, EN("hungry", "空腹"));
	} else if (!strcmp(hunger_str, "weak") || !strcmp(hunger_str, "飢餓")) {
		(void) strcpy(hunger_str, EN("weak", "飢餓"));
	} else if (!strcmp(hunger_str, "faint") || !strcmp(hunger_str, "瀕死")) {
		(void) strcpy(hunger_str, EN("faint", "瀕死"));
	}
}

static void rogue_apply_object_language(void);

void rogue_apply_language(void)
{
	more = ROGUE_MORE;
	byebye_string = ROGUE_BYEBYE;
	press_space = EN(" --press space to continue--", " --スペースで続く-- ");
	you_can_move_again = EN("you can move again", "また動けるようになった。");
	curse_message = EN("you can't, it appears to be cursed",
		"それはできない。のろわれているようだ。");
	left_or_right = EN("left or right hand?", "左手(L)、それとも右手(R)？");
	no_ring = EN("there's no ring on that hand", "そちらの手には指輪をしていない。");
	strange_feeling = EN("you have a strange feeling for a moment, then it passes",
		"不思議な気分におそわれたが、すぐに消えていった。");
	unknown_command = ROGUE_UNKNOWN_COMMAND;
	rogue_apply_fruit_language();
	rogue_apply_hunger_language();
	if (language_option) {
		(void) strcpy(language_option, rogue_language_code());
	}
	rogue_apply_object_language();
	rogue_apply_command_language();
}

static char *potion_title_en[POTIONS] = {
	"blue \0                           ", "red \0                            ",
	"green \0                          ", "grey \0                           ",
	"brown \0                          ", "clear \0                          ",
	"pink \0                           ", "white \0                          ",
	"purple \0                         ", "black \0                          ",
	"yellow \0                         ", "plaid \0                          ",
	"burgundy \0                       ", "beige \0                          "
};
static char *potion_title_ja[POTIONS] = {
	"青い\0                            ", "赤い\0                            ",
	"緑の\0                            ", "灰色の\0                          ",
	"茶色の\0                          ", "透明な\0                          ",
	"ピンクの\0                        ", "白い\0                            ",
	"紫の\0                            ", "黒い\0                            ",
	"黄色い\0                          ", "チェック柄の\0                    ",
	"赤紫の\0                          ", "ベージュ色の\0                    "
};
static char *potion_real_en[POTIONS] = {
	"of increase strength ", "of restore strength ", "of healing ",
	"of extra healing ", "of poison ", "of raise level ",
	"of blindness ", "of hallucination ", "of detect monster ",
	"of detect things ", "of confusion ", "of levitation ",
	"of haste self ", "of see invisible "
};
static char *potion_real_ja[POTIONS] = {
	"強さが増す", "強さが元にもどる", "傷を治す",
	"とてもよく傷を治す", "毒の", "レベルが上がる",
	"目が見えなくなる", "幻覚をおこす", "怪物探知の",
	"物品探知の", "頭が混乱する", "空中に浮きあがる",
	"素早くなる", "見えないものが見える"
};
static char *scroll_real_en[SCROLS] = {
	"of protect armor ", "of hold monster ", "of enchant weapon ",
	"of enchant armor ", "of identify ", "of teleportation ",
	"of sleep ", "of scare monster ", "of remove curse ",
	"of create monster ", "of aggravate monster ", "of magic mapping ",
	"of confuse monster "
};
static char *scroll_real_ja[SCROLS] = {
	"よろいを守る", "怪物を封じこめる", "武器に魔法をかける",
	"よろいに魔法をかける", "持ちものの種類がわかる", "テレポートする",
	"眠りにおちる", "怪物を怖がらせる", "のろいを解く",
	"怪物を作りだす", "怪物を怒らせる", "魔法の地図の",
	"怪物を混乱させる"
};
static char *weapon_title_en[WEAPONS] = {
	"short bow ", "darts ", "arrows ", "daggers ", "shurikens ",
	"mace ", "long sword ", "two-handed sword "
};
static char *weapon_title_ja[WEAPONS] = {
	"短弓", "投げ矢", "矢", "短剣", "手裏剣",
	"つちほこ", "長い剣", "両手持ちの剣"
};
static char *armor_title_en[ARMORS] = {
	"leather armor ", "ring mail ", "scale mail ", "chain mail ",
	"banded mail ", "splint mail ", "plate mail "
};
static char *armor_title_ja[ARMORS] = {
	"革のよろい", "鉄環のよろい", "鱗のよろい", "鎖のよろい",
	"鉄片のよろい", "帯金のよろい", "全身のよろい"
};
static char *wand_real_en[WANDS] = {
	"of teleport away ", "of slow monster ", "of invisibility ",
	"of polymorph ", "of haste monster ", "of magic missile ",
	"of cancellation ", "of do nothing ", "of drain life ",
	"of cold ", "of fire "
};
static char *wand_real_ja[WANDS] = {
	"怪物を遠くに飛ばす", "怪物を遅くする", "怪物を見えなくする",
	"変身させる", "怪物を素早くする", "魔法のミサイルの",
	"魔力を封じる", "役に立たない", "命を吸い取る",
	"冷気の", "炎の"
};
static char *ring_real_en[RINGS] = {
	"of stealth ", "of teleportation ", "of regeneration ",
	"of slow digestion ", "of add strength ", "of sustain strength ",
	"of dexterity ", "of adornment ", "of see invisible ",
	"of maintain armor ", "of searching "
};
static char *ring_real_ja[RINGS] = {
	"身を隠す", "テレポートする", "再生の",
	"消化が遅くなる", "強さが増す", "強さが減らない",
	"機敏になる", "飾りものの", "見えないものが見える",
	"よろいが丈夫になる", "探索の"
};
static char *monster_en[MONSTERS] = {
	"aquator", "bat", "centaur", "dragon", "emu", "venus fly-trap",
	"griffin", "hobgoblin", "ice monster", "jabberwock", "kestrel",
	"leprechaun", "medusa", "nymph", "orc", "phantom", "quagga",
	"rattlesnake", "snake", "troll", "black unicorn", "vampire",
	"wraith", "xeroc", "yeti", "zombie"
};
static char *monster_ja[MONSTERS] = {
	"水の怪物", "こうもり", "ケンタウロス", "ドラゴン", "エミュー", "ハエトリグサ",
	"グリフィン", "ホブゴブリン", "氷の怪物", "ジャバウォック", "はやぶさ",
	"レプラコーン", "メデューサ", "ニンフ", "オーク", "幽霊", "半シマウマ",
	"がらがらへび", "へび", "トロル", "黒ユニコーン", "ヴァンパイア",
	"死霊", "ゼロック", "イエティ", "ゾンビ"
};
static char *wand_material_en[WAND_MATERIALS] = {
	"steel ", "bronze ", "gold ", "silver ", "copper ", "nickel ",
	"cobalt ", "tin ", "iron ", "magnesium ", "chrome ", "carbon ",
	"platinum ", "silicon ", "titanium ", "teak ", "oak ", "cherry ",
	"birch ", "pine ", "cedar ", "redwood ", "balsa ", "ivory ",
	"walnut ", "maple ", "mahogany ", "elm ", "palm ", "wooden "
};
static char *wand_material_ja[WAND_MATERIALS] = {
	"鋼鉄", "青銅", "金", "銀", "銅", "ニッケル",
	"コバルト", "すず", "鉄", "マグネシウム", "クロム", "石炭",
	"プラチナ", "シリコン", "チタン", "チーク", "樫", "桜",
	"白樺", "松", "杉", "セコイア", "バルサ", "象牙",
	"くるみ", "かえで", "マホガニー", "にれ", "やし", "木"
};
static char *gem_en[GEMS] = {
	"diamond ", "stibotantalite ", "lapis-lazuli ", "ruby ", "emerald ",
	"sapphire ", "amethyst ", "quartz ", "tiger-eye ", "opal ",
	"agate ", "turquoise ", "pearl ", "garnet "
};
static char *gem_ja[GEMS] = {
	"ダイヤ ", "安タンタル石 ", "るり ", "ルビー ", "エメラルド ",
	"サファイア ", "アメジスト ", "水晶 ", "虎目石 ", "オパール ",
	"めのう ", "トルコ石 ", "真珠 ", "ざくろ石 "
};


static char *trap_en[TRAPS * 2] = {
	"trap door", "you fell down a trap",
	"bear trap", "you are caught in a bear trap",
	"teleport trap", "teleport",
	"poison dart trap", "a small dart just hit you in the shoulder",
	"sleeping gas trap", "a strange white mist envelops you and you fall asleep",
	"rust trap", "a gush of water hits you on the head"
};
static char *trap_ja[TRAPS * 2] = {
	"落とし戸のわな", "わなの中に落ちた",
	"とらばさみのわな", "とらばさみのわなにつかまった",
	"テレポートのわな", "テレポートしてしまった",
	"毒投げ矢のわな", "小さな投げ矢が飛んできて、肩にささった",
	"眠りガスのわな", "不思議な白いもやにつつまれ、眠り込んでしまった",
	"さびつきのわな", "ほとばしる水が頭から浴びせられた。"
};

static void copy_id(char dest[ID_TITLE_SIZE], char *src)
{
	(void) snprintf(dest, ID_TITLE_SIZE, "%s", src);
}

char * rogue_potion_title_for_id(int i)
{
	if ((i < 0) || (i >= POTIONS)) {
		i = 0;
	}
	return(rogue_lang_is_japanese ? potion_title_ja[i] : potion_title_en[i]);
}

char * rogue_wand_material_for_id(int i)
{
	if ((i < 0) || (i >= WAND_MATERIALS)) {
		i = 0;
	}
	return(rogue_lang_is_japanese ? wand_material_ja[i] : wand_material_en[i]);
}

char * rogue_ring_gem_for_id(int i)
{
	if ((i < 0) || (i >= GEMS)) {
		i = 0;
	}
	return(rogue_lang_is_japanese ? gem_ja[i] : gem_en[i]);
}

static void rogue_apply_object_language(void)
{
	int i;
	extern char *wand_materials[];
	extern char *gems[];
	extern char *m_names[];
	extern char *trap_strings[];

	for (i = 0; i < POTIONS; i++) {
		id_potions[i].real = rogue_lang_is_japanese ? potion_real_ja[i] : potion_real_en[i];
	}
	for (i = 0; i < SCROLS; i++) {
		id_scrolls[i].real = rogue_lang_is_japanese ? scroll_real_ja[i] : scroll_real_en[i];
	}
	for (i = 0; i < WEAPONS; i++) {
		copy_id(id_weapons[i].title, rogue_lang_is_japanese ? weapon_title_ja[i] : weapon_title_en[i]);
	}
	for (i = 0; i < ARMORS; i++) {
		copy_id(id_armors[i].title, rogue_lang_is_japanese ? armor_title_ja[i] : armor_title_en[i]);
	}
	for (i = 0; i < WANDS; i++) {
		id_wands[i].real = rogue_lang_is_japanese ? wand_real_ja[i] : wand_real_en[i];
	}
	for (i = 0; i < RINGS; i++) {
		id_rings[i].real = rogue_lang_is_japanese ? ring_real_ja[i] : ring_real_en[i];
	}
	for (i = 0; i < WAND_MATERIALS; i++) {
		wand_materials[i] = rogue_lang_is_japanese ? wand_material_ja[i] : wand_material_en[i];
	}
	for (i = 0; i < GEMS; i++) {
		gems[i] = rogue_lang_is_japanese ? gem_ja[i] : gem_en[i];
	}
	for (i = 0; i < MONSTERS; i++) {
		m_names[i] = rogue_lang_is_japanese ? monster_ja[i] : monster_en[i];
	}
	for (i = 0; i < TRAPS * 2; i++) {
		trap_strings[i] = rogue_lang_is_japanese ? trap_ja[i] : trap_en[i];
	}
	rogue_refresh_generated_titles();
}
