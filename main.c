/*
 * main.c
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
static char sccsid[] __attribute__((unused)) = "@(#)main.c	5.1 (Berkeley) 11/25/87";
#endif /* not lint */

#include <locale.h>
#include "rogue.h"

extern short party_room;

int main(int argc, char **argv)
{
#ifdef _WIN32
	rogue_win_resize_direct_launch();
	rogue_win_utf8_argv(&argc, &argv);
#endif
	setlocale(LC_ALL, ROGUE_LOCALE);

	if (init(argc, argv)) {		/* restored game */
		goto PL;
	}

	for (;;) {
		clear_level();
		make_level();
		put_objects();
		put_stairs();
		add_traps();
		put_mons();
		put_player(party_room);
		print_stats(STAT_ALL);
PL:		
		play_level();
		free_stuff(&level_objects);
		free_stuff(&level_monsters);
	}
}
