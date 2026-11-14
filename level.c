/*
 * level.c
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
static char sccsid[] __attribute__((unused)) = "@(#)level.c	5.1 (Berkeley) 11/25/87";
#endif /* not lint */

#include "rogue.h"

#define swap(x,y) {t = x; x = y; y = t;}
#define LEVEL_GENERATION_ATTEMPTS 100
#define LEVEL_ARRAY_CELL(r,c) ((r) >= 0 && (r) < DROWS && \
	(c) >= 0 && (c) < DCOLS)

short cur_level = 0;
short max_level = 1;
short cur_room;
char *new_level_message = 0;
short party_room = NO_ROOM;
short r_de;

long level_points[MAX_EXP_LEVEL] = {
		  10L,
		  20L,
		  40L,
		  80L,
		 160L,
		 320L,
		 640L,
		1300L,
		2600L,
		5200L,
	   10000L,
	   20000L,
	   40000L,
	   80000L,
	  160000L,
	  320000L,
	 1000000L,
	 3333333L,
	 6666666L,
	  MAX_EXP,
	99900000L
};

short random_rooms[MAXROOMS] = {3, 7, 5, 2, 0, 6, 1, 4, 8};

extern boolean being_held, wizard, detect_monster;
extern boolean see_invisible;
extern short bear_trap, levitate, extra_hp, less_hp, cur_room;

static int level_room_index_ok(short rn)
{
	return((rn >= 0) && (rn < MAXROOMS));
}

static void level_set_cell(short row, short col, unsigned short value)
{
	if (ROGUE_IN_DUNGEON(row, col)) {
		dungeon[row][col] = value;
	}
}

static void generate_level_layout(void)
{
	short i, j;
	short must_1, must_2 = 1, must_3 = 2;
	boolean big_room;

	must_1 = get_rand(0, 5);

	switch(must_1) {
	case 0:
		must_1 = 0;
		must_2 = 1;
		must_3 = 2;
		break;
	case 1:
		must_1 = 3;
		must_2 = 4;
		must_3 = 5;
		break;
	case 2:
		must_1 = 6;
		must_2 = 7;
		must_3 = 8;
		break;
	case 3:
		must_1 = 0;
		must_2 = 3;
		must_3 = 6;
		break;
	case 4:
		must_1 = 1;
		must_2 = 4;
		must_3 = 7;
		break;
	case 5:
		must_1 = 2;
		must_2 = 5;
		must_3 = 8;
		break;
	default:
		/* Defensive fallback if the RNG ever returns an invalid value. */
		must_1 = 0;
		must_2 = 1;
		must_3 = 2;
		break;
	}
	if (rand_percent(8)) {
		party_room = 0;
	}
	big_room = ((party_room != NO_ROOM) && rand_percent(1));
	if (big_room) {
		make_room(BIG_ROOM, 0, 0, 0);
	} else {
		for (i = 0; i < MAXROOMS; i++) {
			make_room(i, must_1, must_2, must_3);
		}
	}
	if (!big_room) {
		add_mazes();

		mix_random_rooms();

		for (j = 0; j < MAXROOMS; j++) {

			i = random_rooms[j];

			if (i < (MAXROOMS-1)) {
				(void) connect_rooms(i, i+1);
			}
			if (i < (MAXROOMS-3)) {
				(void) connect_rooms(i, i+3);
			}
			if (i < (MAXROOMS-2)) {
				if (rooms[i+1].is_room & R_NOTHING) {
					if (connect_rooms(i, i+2)) {
						rooms[i+1].is_room = R_CROSS;
					}
				}
			}
			if (i < (MAXROOMS-6)) {
				if (rooms[i+3].is_room & R_NOTHING) {
					if (connect_rooms(i, i+6)) {
						rooms[i+3].is_room = R_CROSS;
					}
				}
			}
			if (is_all_connected()) {
				break;
			}
		}
		fill_out_level();
	}
}

/* Count actual reachable cells, not only room-door metadata.  These movement
 * rules match can_move(): diagonals cannot involve doors and must not cross
 * a completely empty corner.  No objects or monsters are placed yet. */
static int level_reachable(void)
{
	unsigned char visited[DROWS][DCOLS] = {{0}};
	short qr[DROWS * DCOLS], qc[DROWS * DCOLS];
	short r, c, nr, nc, dr, dc;
	int total = 0, head = 0, tail = 0;

	for (r = MIN_ROW; r <= DROWS-2; r++) {
		for (c = 0; c < DCOLS; c++) {
			if (!(dungeon[r][c] & (FLOOR | TUNNEL | DOOR | STAIRS | TRAP))) {
				continue;
			}
			total++;
			if (!tail) {
				qr[tail] = r;
				qc[tail++] = c;
				visited[r][c] = 1;
			}
		}
	}
	if (!total) {
		return(0);
	}
	while (head < tail) {
		r = qr[head];
		c = qc[head++];
		for (dr = -1; dr <= 1; dr++) {
			for (dc = -1; dc <= 1; dc++) {
				if (!dr && !dc) {
					continue;
				}
				nr = r + dr;
				nc = c + dc;
				if (!ROGUE_IN_DUNGEON(nr, nc) || visited[nr][nc] ||
					!(dungeon[nr][nc] & (FLOOR | TUNNEL | DOOR | STAIRS | TRAP))) {
					continue;
				}
				if (dr && dc && ((dungeon[r][c] & DOOR) ||
					(dungeon[nr][nc] & DOOR) || !dungeon[r][nc] ||
					!dungeon[nr][c])) {
					continue;
				}
				visited[nr][nc] = 1;
				qr[tail] = nr;
				qc[tail++] = nc;
			}
		}
	}
	return(tail == total);
}

void make_level(void)
{
	int attempt;

	if (cur_level < LAST_DUNGEON) {
		cur_level++;
	}
	if (cur_level > max_level) {
		max_level = cur_level;
	}
	for (attempt = 0; attempt < LEVEL_GENERATION_ATTEMPTS; attempt++) {
		if (attempt) {
			/* Only the layout has been generated; no items or monsters yet. */
			clear_level();
		}
		generate_level_layout();
		if (is_all_connected() && level_reachable()) {
			if (!has_amulet() && (cur_level >= AMULET_LEVEL)) {
				put_amulet();
			}
			return;
		}
	}
	clean_up(EN("cannot generate connected dungeon", "接続された迷宮を生成できません。"));
}

void make_room(short rn, short r1, short r2, short r3)
{
	short left_col, right_col, top_row, bottom_row;
	short width, height;
	short row_offset, col_offset;
	short i, j, ch;

	switch(rn) {
	case 0:
		left_col = 0;
		right_col = COL1-1;
		top_row = MIN_ROW;
		bottom_row = ROW1-1;
		break;
	case 1:
		left_col = COL1+1;
		right_col = COL2-1;
		top_row = MIN_ROW;
		bottom_row = ROW1-1;
		break;
	case 2:
		left_col = COL2+1;
		right_col = DCOLS-1;
		top_row = MIN_ROW;
		bottom_row = ROW1-1;
		break;
	case 3:
		left_col = 0;
		right_col = COL1-1;
		top_row = ROW1+1;
		bottom_row = ROW2-1;
		break;
	case 4:
		left_col = COL1+1;
		right_col = COL2-1;
		top_row = ROW1+1;
		bottom_row = ROW2-1;
		break;
	case 5:
		left_col = COL2+1;
		right_col = DCOLS-1;
		top_row = ROW1+1;
		bottom_row = ROW2-1;
		break;
	case 6:
		left_col = 0;
		right_col = COL1-1;
		top_row = ROW2+1;
		bottom_row = DROWS - 2;
		break;
	case 7:
		left_col = COL1+1;
		right_col = COL2-1;
		top_row = ROW2+1;
		bottom_row = DROWS - 2;
		break;
	case 8:
		left_col = COL2+1;
		right_col = DCOLS-1;
		top_row = ROW2+1;
		bottom_row = DROWS - 2;
		break;
	case BIG_ROOM:
		top_row = get_rand(MIN_ROW, MIN_ROW+5);
		bottom_row = get_rand(DROWS-7, DROWS-2);
		left_col = get_rand(0, 10);;
		right_col = get_rand(DCOLS-11, DCOLS-1);
		rn = 0;
		goto B;
	default:
		/* Never compute room dimensions from undefined bounds. */
		return;
	}
	height = get_rand(4, (bottom_row - top_row + 1));
	width = get_rand(7, (right_col - left_col - 2));

	row_offset = get_rand(0, ((bottom_row - top_row) - height + 1));
	col_offset = get_rand(0, ((right_col - left_col) - width + 1));

	top_row += row_offset;
	bottom_row = top_row + height - 1;

	left_col += col_offset;
	right_col = left_col + width - 1;

	if ((rn != r1) && (rn != r2) && (rn != r3) && rand_percent(40)) {
		goto END;
	}
B:
	rooms[rn].is_room = R_ROOM;

	for (i = top_row; i <= bottom_row; i++) {
		for (j = left_col; j <= right_col; j++) {
			if ((i == top_row) || (i == bottom_row)) {
				ch = HORWALL;
			} else if (	((i != top_row) && (i != bottom_row)) &&
						((j == left_col) || (j == right_col))) {
				ch = VERTWALL;
			} else {
				ch = FLOOR;
			}
			level_set_cell(i, j, ch);
		}
	}
END:
	rooms[rn].top_row = top_row;
	rooms[rn].bottom_row = bottom_row;
	rooms[rn].left_col = left_col;
	rooms[rn].right_col = right_col;
}

/* Find an existing wall/maze tunnel before modifying any door.  The
 * bounded random search preserves the old selection in normal cases; the
 * deterministic pass avoids a false failure when a candidate exists. */
static int find_door(room *rm, short dir, short *row, short *col)
{
	short low, high, fixed, v, tries;
	unsigned short wanted;
	int horizontal = (dir == UPWARD || dir == DOWN);
	short wall_width = (rm->is_room & R_MAZE) ? 0 : 1;

	if (horizontal) {
		fixed = (dir == UPWARD) ? rm->top_row : rm->bottom_row;
		low = rm->left_col + wall_width;
		high = rm->right_col - wall_width;
		wanted = HORWALL | TUNNEL;
	} else {
		fixed = (dir == LEFT) ? rm->left_col : rm->right_col;
		low = rm->top_row + wall_width;
		high = rm->bottom_row - wall_width;
		wanted = VERTWALL | TUNNEL;
	}
	if (low > high) {
		return(0);
	}
	for (tries = 0; tries < 200; tries++) {
		v = get_rand(low, high);
		*row = horizontal ? fixed : v;
		*col = horizontal ? v : fixed;
		if (ROGUE_IN_DUNGEON(*row, *col) &&
			(dungeon[*row][*col] & wanted)) {
			return(1);
		}
	}
	for (v = low; v <= high; v++) {
		*row = horizontal ? fixed : v;
		*col = horizontal ? v : fixed;
		if (ROGUE_IN_DUNGEON(*row, *col) &&
			(dungeon[*row][*col] & wanted)) {
			return(1);
		}
	}
	return(0);
}

static void finish_door(room *rm, short dir, short row, short col)
{
	if (rm->is_room & R_ROOM) {
		dungeon[row][col] = DOOR;
	}
	if ((cur_level > 2) && rand_percent(HIDE_PERCENT)) {
		dungeon[row][col] |= HIDDEN;
	}
	rm->doors[dir/2].door_row = row;
	rm->doors[dir/2].door_col = col;
}

int connect_rooms(short room1, short room2)
{
	short row1, col1, row2, col2, dir, dir2;

	if ((!(rooms[room1].is_room & (R_ROOM | R_MAZE))) ||
		(!(rooms[room2].is_room & (R_ROOM | R_MAZE)))) {
		return(0);
	}
	if (same_row(room1, room2) &&
		(rooms[room1].left_col > rooms[room2].right_col)) {
		dir = LEFT;
	} else if (same_row(room1, room2) &&
		(rooms[room2].left_col > rooms[room1].right_col)) {
		dir = RIGHT;
	} else if (same_col(room1, room2) &&
		(rooms[room1].top_row > rooms[room2].bottom_row)) {
		dir = UPWARD;
	} else if (same_col(room1, room2) &&
		(rooms[room2].top_row > rooms[room1].bottom_row)) {
		dir = DOWN;
	} else {
		return(0);
	}
	dir2 = (dir + 4) % DIRS;
	if (!find_door(&rooms[room1], dir, &row1, &col1) ||
		!find_door(&rooms[room2], dir2, &row2, &col2)) {
		return(0);
	}
	finish_door(&rooms[room1], dir, row1, col1);
	finish_door(&rooms[room2], dir2, row2, col2);

	do {
		draw_simple_passage(row1, col1, row2, col2, dir);
	} while (rand_percent(4));

	rooms[room1].doors[dir/2].oth_room = room2;
	rooms[room1].doors[dir/2].oth_row = row2;
	rooms[room1].doors[dir/2].oth_col = col2;

	rooms[room2].doors[(((dir+4)%DIRS)/2)].oth_room = room1;
	rooms[room2].doors[(((dir+4)%DIRS)/2)].oth_row = row1;
	rooms[room2].doors[(((dir+4)%DIRS)/2)].oth_col = col1;
	return(1);
}

void clear_level(void)
{
	short i, j;

	for (i = 0; i < MAXROOMS; i++) {
		rooms[i].is_room = R_NOTHING;
		for (j = 0; j < 4; j++) {
			rooms[i].doors[j].oth_room = NO_ROOM;
		}
	}

	for (i = 0; i < MAX_TRAPS; i++) {
		traps[i].trap_type = NO_TRAP;
	}
	for (i = 0; i < DROWS; i++) {
		for (j = 0; j < DCOLS; j++) {
			dungeon[i][j] = NOTHING;
		}
	}
	detect_monster = see_invisible = 0;
	being_held = bear_trap = 0;
	party_room = NO_ROOM;
	rogue.row = rogue.col = -1;
	clear();
}

int put_door(room *rm, short dir, short *row, short *col)
{
	if (!find_door(rm, dir, row, col)) {
		return(0);
	}
	finish_door(rm, dir, *row, *col);
	return(1);
}

void draw_simple_passage(short row1, short col1, short row2, short col2, short dir)
{
	short i, middle, t;

	if ((dir == LEFT) || (dir == RIGHT)) {
		if (col1 > col2) {
			swap(row1, row2);
			swap(col1, col2);
		}
		if ((col2 - col1) < 2) {
			/* Adjacent doorways need no passage cells; keep their doors. */
			return;
		}
		middle = get_rand(col1+1, col2-1);
		for (i = col1+1; i != middle; i++) {
			level_set_cell(row1, i, TUNNEL);
		}
		for (i = row1; i != row2; i += (row1 > row2) ? -1 : 1) {
			level_set_cell(i, middle, TUNNEL);
		}
		for (i = middle; i != col2; i++) {
			level_set_cell(row2, i, TUNNEL);
		}
	} else {
		if (row1 > row2) {
			swap(row1, row2);
			swap(col1, col2);
		}
		if ((row2 - row1) < 2) {
			/* Adjacent doorways need no passage cells; keep their doors. */
			return;
		}
		middle = get_rand(row1+1, row2-1);
		for (i = row1+1; i != middle; i++) {
			level_set_cell(i, col1, TUNNEL);
		}
		for (i = col1; i != col2; i += (col1 > col2) ? -1 : 1) {
			level_set_cell(middle, i, TUNNEL);
		}
		for (i = middle; i != row2; i++) {
			level_set_cell(i, col2, TUNNEL);
		}
	}
	if (rand_percent(HIDE_PERCENT)) {
		hide_boxed_passage(row1, col1, row2, col2, 1);
	}
}

int same_row(int room1, int room2)
{
	return((room1 / 3) == (room2 / 3));
}

int same_col(int room1, int room2)
{
	return((room1 % 3) == (room2 % 3));
}

void add_mazes(void)
{
	short i, j;
	short start;
	short maze_percent;

	if (cur_level > 1) {
		start = get_rand(0, (MAXROOMS-1));
		maze_percent = (cur_level * 5) / 4;

		if (cur_level > 15) {
			maze_percent += cur_level;
		}
		for (i = 0; i < MAXROOMS; i++) {
			j = ((start + i) % MAXROOMS);
			if (rooms[j].is_room & R_NOTHING) {
				if (rand_percent(maze_percent) &&
					(rooms[j].top_row+1 <= rooms[j].bottom_row-1) &&
					(rooms[j].left_col+1 <= rooms[j].right_col-1)) {
				rooms[j].is_room = R_MAZE;
				make_maze(get_rand(rooms[j].top_row+1, rooms[j].bottom_row-1),
					get_rand(rooms[j].left_col+1, rooms[j].right_col-1),
					rooms[j].top_row, rooms[j].bottom_row,
					rooms[j].left_col, rooms[j].right_col);
				hide_boxed_passage(rooms[j].top_row, rooms[j].left_col,
					rooms[j].bottom_row, rooms[j].right_col,
					get_rand(0, 2));
				}
			}
		}
	}
}

void fill_out_level(void)
{
	short i, rn;

	mix_random_rooms();

	r_de = NO_ROOM;

	for (i = 0; i < MAXROOMS; i++) {
		rn = random_rooms[i];
		if ((rooms[rn].is_room & R_NOTHING) ||
			((rooms[rn].is_room & R_CROSS) && coin_toss())) {
			fill_it(rn, 1);
		}
	}
	if (r_de != NO_ROOM) {
		fill_it(r_de, 0);
	}
}

void fill_it(int rn, boolean do_rec_de)
{
	short i, tunnel_dir, door_dir, drow, dcol;
	short target_room, rooms_found = 0;
	short srow, scol, t;
	static short offsets[4] = {-1, 1, 3, -3};
	boolean did_this = 0;

	for (i = 0; i < 10; i++) {
		srow = get_rand(0, 3);
		scol = get_rand(0, 3);
		t = offsets[srow];
		offsets[srow] = offsets[scol];
		offsets[scol] = t;
	}
	for (i = 0; i < 4; i++) {

		target_room = rn + offsets[i];

		if (((target_room < 0) || (target_room >= MAXROOMS)) ||
			(!(same_row(rn,target_room) || same_col(rn,target_room))) ||
			(!(rooms[target_room].is_room & (R_ROOM | R_MAZE)))) {
			continue;
		}
		if (same_row(rn, target_room)) {
			tunnel_dir = (rooms[rn].left_col < rooms[target_room].left_col) ?
				RIGHT : LEFT;
		} else {
			tunnel_dir = (rooms[rn].top_row < rooms[target_room].top_row) ?
				DOWN : UPWARD;
		}
		door_dir = ((tunnel_dir + 4) % DIRS);
		if (rooms[target_room].doors[door_dir/2].oth_room != NO_ROOM) {
			continue;
		}
		if (((!do_rec_de) || did_this) ||
			(!mask_room(rn, &srow, &scol, TUNNEL))) {
			srow = (rooms[rn].top_row + rooms[rn].bottom_row) / 2;
			scol = (rooms[rn].left_col + rooms[rn].right_col) / 2;
		}
		if (!put_door(&rooms[target_room], door_dir, &drow, &dcol)) {
			continue;
		}
		rooms_found++;
		draw_simple_passage(srow, scol, drow, dcol, tunnel_dir);
		rooms[rn].is_room = R_DEADEND;
		level_set_cell(srow, scol, TUNNEL);

		if ((i < 3) && (!did_this)) {
			did_this = 1;
			if (coin_toss()) {
				continue;
			}
		}
		if ((rooms_found < 2) && do_rec_de) {
			recursive_deadend(rn, offsets, srow, scol);
		}
		break;
	}
}

void recursive_deadend(short rn, short *offsets, short srow, short scol)
{
	short i, de;
	short drow, dcol, tunnel_dir;

	rooms[rn].is_room = R_DEADEND;
	level_set_cell(srow, scol, TUNNEL);

	for (i = 0; i < 4; i++) {
		de = rn + offsets[i];
		if (((de < 0) || (de >= MAXROOMS)) ||
			(!(same_row(rn, de) || same_col(rn, de)))) {
			continue;
		}
		if (!(rooms[de].is_room & R_NOTHING)) {
			continue;
		}
		drow = (rooms[de].top_row + rooms[de].bottom_row) / 2;
		dcol = (rooms[de].left_col + rooms[de].right_col) / 2;
		if (same_row(rn, de)) {
			tunnel_dir = (rooms[rn].left_col < rooms[de].left_col) ?
				RIGHT : LEFT;
		} else {
			tunnel_dir = (rooms[rn].top_row < rooms[de].top_row) ?
				DOWN : UPWARD;
		}
		draw_simple_passage(srow, scol, drow, dcol, tunnel_dir);
		r_de = de;
		recursive_deadend(de, offsets, drow, dcol);
	}
}

boolean mask_room(short rn, short *row, short *col, unsigned short mask)
{
	short i, j;

	if (!level_room_index_ok(rn)) {
		return(0);
	}
	for (i = rooms[rn].top_row; i <= rooms[rn].bottom_row; i++) {
		for (j = rooms[rn].left_col; j <= rooms[rn].right_col; j++) {
			if (ROGUE_IN_DUNGEON(i, j) && (dungeon[i][j] & mask)) {
				*row = i;
				*col = j;
				return(1);
			}
		}
	}
	return(0);
}

void make_maze(short r, short c, short tr, short br, short lc, short rc)
{
	char dirs[4];
	short i, t;

	if (tr < MIN_ROW) {
		tr = MIN_ROW;
	}
	if (br > (DROWS - 2)) {
		br = DROWS - 2;
	}
	if (lc < 0) {
		lc = 0;
	}
	if (rc > (DCOLS - 1)) {
		rc = DCOLS - 1;
	}
	if (!ROGUE_IN_DUNGEON(r, c) || (tr > br) || (lc > rc)) {
		return;
	}
	dirs[0] = UPWARD;
	dirs[1] = DOWN;
	dirs[2] = LEFT;
	dirs[3] = RIGHT;

	level_set_cell(r, c, TUNNEL);

	if (rand_percent(20)) {
		for (i = 0; i < 10; i++) {
			short t1, t2;

			t1 = get_rand(0, 3);
			t2 = get_rand(0, 3);

			swap(dirs[t1], dirs[t2]);
		}
	}
	for (i = 0; i < 4; i++) {
		switch(dirs[i]) {
		case UPWARD:
			if (((r-1) >= tr) &&
				ROGUE_IN_DUNGEON(r-1, c) &&
				ROGUE_IN_DUNGEON(r-1, c-1) &&
				ROGUE_IN_DUNGEON(r-1, c+1) &&
				LEVEL_ARRAY_CELL(r-2, c) &&
				(dungeon[r-1][c] != TUNNEL) &&
				(dungeon[r-1][c-1] != TUNNEL) &&
				(dungeon[r-1][c+1] != TUNNEL) &&
				(dungeon[r-2][c] != TUNNEL)) {
				make_maze((r-1), c, tr, br, lc, rc);
			}
			break;
		case DOWN:
			if (((r+1) <= br) &&
				ROGUE_IN_DUNGEON(r+1, c) &&
				ROGUE_IN_DUNGEON(r+1, c-1) &&
				ROGUE_IN_DUNGEON(r+1, c+1) &&
				LEVEL_ARRAY_CELL(r+2, c) &&
				(dungeon[r+1][c] != TUNNEL) &&
				(dungeon[r+1][c-1] != TUNNEL) &&
				(dungeon[r+1][c+1] != TUNNEL) &&
				(dungeon[r+2][c] != TUNNEL)) {
				make_maze((r+1), c, tr, br, lc, rc);
			}
			break;
		case LEFT:
			if (((c-1) >= lc) &&
				ROGUE_IN_DUNGEON(r, c-1) &&
				ROGUE_IN_DUNGEON(r-1, c-1) &&
				ROGUE_IN_DUNGEON(r+1, c-1) &&
				LEVEL_ARRAY_CELL(r, c-2) &&
				(dungeon[r][c-1] != TUNNEL) &&
				(dungeon[r-1][c-1] != TUNNEL) &&
				(dungeon[r+1][c-1] != TUNNEL) &&
				(dungeon[r][c-2] != TUNNEL)) {
				make_maze(r, (c-1), tr, br, lc, rc);
			}
			break;
		case RIGHT:
			if (((c+1) <= rc) &&
				ROGUE_IN_DUNGEON(r, c+1) &&
				ROGUE_IN_DUNGEON(r-1, c+1) &&
				ROGUE_IN_DUNGEON(r+1, c+1) &&
				LEVEL_ARRAY_CELL(r, c+2) &&
				(dungeon[r][c+1] != TUNNEL) &&
				(dungeon[r-1][c+1] != TUNNEL) &&
				(dungeon[r+1][c+1] != TUNNEL) &&
				(dungeon[r][c+2] != TUNNEL)) {
				make_maze(r, (c+1), tr, br, lc, rc);
			}
			break;
		}
	}
}

void hide_boxed_passage(short row1, short col1, short row2, short col2, short n)
{
	short i, j, t;
	short row, col, row_cut, col_cut;
	short h, w;

	if (cur_level > 2) {
		if (row1 > row2) {
			swap(row1, row2);
		}
		if (col1 > col2) {
			swap(col1, col2);
		}
		h = row2 - row1;
		w = col2 - col1;

		if ((w >= 5) || (h >= 5)) {
			row_cut = ((h >= 2) ? 1 : 0);
			col_cut = ((w >= 2) ? 1 : 0);

			for (i = 0; i < n; i++) {
				for (j = 0; j < 10; j++) {
					row = get_rand(row1 + row_cut, row2 - row_cut);
					col = get_rand(col1 + col_cut, col2 - col_cut);
					if (ROGUE_IN_DUNGEON(row, col) &&
						(dungeon[row][col] == TUNNEL)) {
						dungeon[row][col] |= HIDDEN;
						break;
					}
				}
			}
		}
	}
}

void put_player(short nr)
{		/* try not to put in this room */
	short rn = nr, misses;
	short row, col;

	row = col = -1;
	for (misses = 0; ((misses < 2) && (rn == nr)); misses++) {
		if (!gr_row_col(&row, &col, (FLOOR | TUNNEL | OBJECT | STAIRS))) {
			clean_up(EN("no position for player", "プレイヤーを配置できません。"));
		}
		rn = get_room_number(row, col);
	}
	if (!ROGUE_IN_DUNGEON(row, col)) {
		if (!gr_row_col(&row, &col, (FLOOR | TUNNEL))) {
			clean_up(EN("no position for player", "プレイヤーを配置できません。"));
		}
		rn = get_room_number(row, col);
	}
	rogue.row = row;
	rogue.col = col;

	if (ROGUE_IN_DUNGEON(rogue.row, rogue.col) &&
		(dungeon[rogue.row][rogue.col] & TUNNEL)) {
		cur_room = PASSAGE;
	} else if (level_room_index_ok(rn)) {
		cur_room = rn;
	} else {
		cur_room = PASSAGE;
	}
	if (cur_room != PASSAGE) {
		light_up_room(cur_room);
	} else {
		light_passage(rogue.row, rogue.col);
	}
	rn = get_room_number(rogue.row, rogue.col);
	if (level_room_index_ok(rn)) {
		wake_room(rn, 1, rogue.row, rogue.col);
	}
	if (new_level_message) {
		message(new_level_message, 0);
		new_level_message = 0;
	}
	if (ROGUE_IN_DUNGEON(rogue.row, rogue.col)) {
		mvaddch(rogue.row, rogue.col, rogue.fchar);
	}
}

int drop_check(void)
{
	if (wizard) {
		return(1);
	}
	if (dungeon[rogue.row][rogue.col] & STAIRS) {
		if (levitate) {
			message(EN("you're floating in the air!", "宙に浮いている！"), 0);
			return(0);
		}
		return(1);
	}
	message(EN("I see no way down", "降りる階段は見あたらない。"), 0);
	return(0);
}

int check_up(void)
{
	if (!wizard) {
		if (!(dungeon[rogue.row][rogue.col] & STAIRS)) {
			message(EN("I see no way up", "登る階段は見あたらない。"), 0);
			return(0);
		}
		if (!has_amulet()) {
			message(EN("your way is magically blocked", "そちらは魔法でふさがれている。"), 0);
			return(0);
		}
	}
	new_level_message = EN("you feel a wrenching sensation in your gut",
		"はらわたをねじ切られるような感じだ。");
	if (cur_level == 1) {
		win();
	} else {
		cur_level -= 2;
		return(1);
	}
	return(0);
}

void add_exp(int e, boolean promotion)
{
	char mbuf[ROGUE_DESC_LEN];
	short new_exp;
	short i, hp;

	rogue.exp_points += e;

	if (rogue.exp_points >= level_points[rogue.exp-1]) {
		new_exp = get_exp_level(rogue.exp_points);
		if (rogue.exp_points > MAX_EXP) {
			rogue.exp_points = MAX_EXP + 1;
		}
		for (i = rogue.exp+1; i <= new_exp; i++) {
			sprintf(mbuf, EN("welcome to level %d", "レベル%dへようこそ。"), i);
			message(mbuf, 0);
			if (promotion) {
				hp = hp_raise();
				rogue.hp_current += hp;
				rogue.hp_max += hp;
			}
			rogue.exp = i;
			print_stats(STAT_HP | STAT_EXP);
		}
	} else {
		print_stats(STAT_EXP);
	}
}

int get_exp_level(long e)
{
	short i;

	for (i = 0; i < (MAX_EXP_LEVEL - 1); i++) {
		if (level_points[i] > e) {
			break;
		}
	}
	return(i+1);
}

int hp_raise(void)
{
	int hp;

	hp = (wizard ? 10 : get_rand(3, 10));
	return(hp);
}

void show_average_hp(void)
{
	char mbuf[ROGUE_DESC_LEN];
	float real_average;
	float effective_average;

	if (rogue.exp == 1) {
		real_average = effective_average = 0.00;
	} else {
		real_average = (float)
			((rogue.hp_max - extra_hp - INIT_HP_CURRENT) + less_hp) / (rogue.exp - 1);
		effective_average = (float) (rogue.hp_max - INIT_HP_CURRENT) / (rogue.exp - 1);

	}
	sprintf(mbuf, EN("R-Hp: %.2f, E-Hp: %.2f (!: %d, V: %d)",
		"基本上昇率: %.2f, 実効上昇率: %.2f (!: %d, V: %d)"), real_average,
		effective_average, extra_hp, less_hp);
	message(mbuf, 0);
}

void mix_random_rooms(void)
{
	short i, t;
	short x, y;

	for (i = 0; i < (3 * MAXROOMS); i++) {
		do {
			x = get_rand(0, (MAXROOMS-1));
			y = get_rand(0, (MAXROOMS-1));
		} while (x == y);
		swap(random_rooms[x], random_rooms[y]);
	}
}
