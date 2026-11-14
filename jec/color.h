/* Rogue Clone III JEC shared color and terminal drawing interface.
 * See LICENSE.txt for copyright and redistribution conditions.
 * Included by rogue.h after <curses.h> has defined WINDOW.
 */
#ifndef ROGUE_CLONE_JEC_COLOR_H
#define ROGUE_CLONE_JEC_COLOR_H

#define ROGUE_EFFECT_NONE 0
#define ROGUE_EFFECT_COLD 1
#define ROGUE_EFFECT_FIRE 2

/* JEC option: enable game colors. */
extern boolean use_color;

int rogue_addch(int ch);
int rogue_clear(void);
int rogue_mvaddch(short row, short col, int ch);
int rogue_mvaddch_skull_dark(short row, short col, int ch);
void rogue_init_colors(void);
void rogue_apply_color_mode(void);
void rogue_recolor_screen(void);
void rogue_restore_terminal_colors(void);
void rogue_set_effect_color(short effect);
void rogue_set_score_title_color(void);
void rogue_set_score_header_color(void);
void rogue_set_score_reverse_color(void);
void rogue_reset_score_color(void);
void rogue_end_score_reverse_color(void);

#ifndef _WIN32
void rogue_prepare_input_attr(WINDOW *wn);
#endif

#endif /* ROGUE_CLONE_JEC_COLOR_H */
