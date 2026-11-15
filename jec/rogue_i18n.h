/*
 * rogue_i18n.h - runtime language strings.
 *
 * The program is built once and can display Japanese or English at runtime.
 * Startup selection order:
 *   1. ROGUEOPTS=language=ja|en|auto (if set)
 *   2. DEFAULT_LANGUAGE: 0=auto, 1=Japanese, 2=English
 *   3. OS locale when auto is selected (LC_ALL, LC_MESSAGES, LANG on Unix).
 * In-game switching is ja/en only.
 */

#ifndef ROGUE_I18N_H
#define ROGUE_I18N_H

#ifndef DEFAULT_LANGUAGE
#define DEFAULT_LANGUAGE 0   /* 0 = locale auto, 1 = Japanese, 2 = English */
#endif
#ifndef DEFAULT_JAPANESE
#if DEFAULT_LANGUAGE == 1
#define DEFAULT_JAPANESE 1
#else
#define DEFAULT_JAPANESE 0
#endif
#endif

extern int rogue_lang_is_japanese;
extern char *more;
extern char *byebye_string;
extern char *press_space;
extern char *language_option;

void rogue_init_locale(void);
void rogue_detect_language(void);
void rogue_set_language_code(char *code);
int rogue_set_initial_language_code(const char *code);
void rogue_apply_language(void);
void rogue_apply_command_language(void);
char * rogue_potion_title_for_id(int i);
char * rogue_wand_material_for_id(int i);
char * rogue_ring_gem_for_id(int i);
void rogue_refresh_generated_titles(void);
char *rogue_language_code(void);
char *rogue_str(char *en, char *ja);

#define EN(en, ja) rogue_str((char *)(en), (char *)(ja))
#if DEFAULT_JAPANESE
#define EN_STATIC(en, ja) ja
#else
#define EN_STATIC(en, ja) en
#endif
#define JP(s) (rogue_lang_is_japanese ? (s) : "")

#define ROGUE_MORE EN("-more-", " --続く-- ")
#define ROGUE_BYEBYE EN("Okay, bye bye!", "了解、またね！")
#define ROGUE_FRUIT EN("slime-mold", "ネバネバかび")
#define ROGUE_UNKNOWN_COMMAND EN("unknown command", "コマンドが違います。")
#define ROGUE_SOMETHING EN("something", "何者か")
#define ROGUE_DIRECTION EN("direction?", "方角は？")
#define ROGUE_TRUE EN("True", "有効(T)")
#define ROGUE_FALSE EN("False", "無効(F)")

#define ROGUE_STAT_LEVEL EN("Level: ", "階: ")
#define ROGUE_STAT_GOLD EN("Gold: ", "金塊: ")
#define ROGUE_STAT_HP EN("Hp: ", "体力: ")
#define ROGUE_STAT_STRENGTH EN("Str: ", "強さ: ")
#define ROGUE_STAT_ARMOR EN("Arm: ", "守備: ")
#define ROGUE_STAT_EXP EN("Exp: ", "経験: ")
#define ROGUE_STAT_LEVEL_LABEL_COL 0
#define ROGUE_STAT_LEVEL_VALUE_COL (rogue_lang_is_japanese ? 4 : 7)
#define ROGUE_STAT_GOLD_LABEL_COL (rogue_lang_is_japanese ? 7 : 10)
#define ROGUE_STAT_GOLD_VALUE_COL (rogue_lang_is_japanese ? 13 : 16)
#define ROGUE_STAT_HP_LABEL_COL (rogue_lang_is_japanese ? 19 : 23)
#define ROGUE_STAT_HP_VALUE_COL (rogue_lang_is_japanese ? 25 : 27)
#define ROGUE_STAT_STRENGTH_LABEL_COL (rogue_lang_is_japanese ? 33 : 36)
#define ROGUE_STAT_STRENGTH_VALUE_COL (rogue_lang_is_japanese ? 39 : 41)
#define ROGUE_STAT_ARMOR_LABEL_COL (rogue_lang_is_japanese ? 46 : 48)
#define ROGUE_STAT_ARMOR_VALUE_COL (rogue_lang_is_japanese ? 52 : 53)
#define ROGUE_STAT_EXP_LABEL_COL (rogue_lang_is_japanese ? 55 : 56)
#define ROGUE_STAT_EXP_VALUE_COL 61

#endif /* ROGUE_I18N_H */
