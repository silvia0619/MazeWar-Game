#include "maze.h"
#include "csapp.h"
#include "debug.h"

#include <string.h>

OBJECT **the_maze;
int the_row;
int the_col;
sem_t mutex;

void maze_init(char **template) {
	debug("maze_init");
	if(template != NULL) {
		the_col = strlen(template[0]);
		the_row = sizeof(template);

		the_maze = (OBJECT**)malloc(the_row * sizeof(OBJECT*));
	    for (int i = 0; i < the_row; i++) {
	        the_maze[i] = (OBJECT*)malloc(the_col * sizeof(OBJECT));
	    }

		for(int i = 0; i < the_row; i++) {
			for(int j = 0; j < the_col; j++) {
				if(IS_EMPTY(template[i][j])) {
					the_maze[i][j] = EMPTY;
				}
				else {
					the_maze[i][j] = template[i][j];
				}
			}
		}
	}
	Sem_init(&mutex, 0, 1);
}

void maze_fini() {
	debug("maze_fini\n");
	for (int i = 0; i < maze_get_rows(); i++) {
        free(the_maze[i]);
	}
	free(the_maze);
}

int maze_get_rows() {
	return the_row;
}

int maze_get_cols() {
	return the_col;
}

int maze_set_player(OBJECT avatar, int row, int col) {
	debug("maze_set_player\n");
	if(the_maze[row][col] == EMPTY) {
		the_maze[row][col] = avatar;
		debug("SET %c in (%d, %d)\n", avatar, row, col);
		return 0;
	}
	return -1;
}

int maze_set_player_random(OBJECT avatar, int *rowp, int *colp) {
	debug("maze_set_player_random\n");
	// long int idx = (long int)void_ptr;
    unsigned int seedp = (unsigned int)time(0);
	int i = 0;
	int is_set = -1;
	int x, y;
	// srand(time(0));
	while(is_set != 0 && i < 20) {
		// x = rand() % maze_get_rows();
		// y = rand() % maze_get_cols();
		x = rand_r(&seedp) % maze_get_rows();
		y = rand_r(&seedp) % maze_get_cols();

		is_set = maze_set_player(avatar, x, y);
		debug("tried to SET %c in (%d, %d)\n", avatar, x, y);
	}
	if(is_set == 0) {
		*rowp = x;
		*colp = y;
		return 0;
	}
	else {
		return -1;
	}
}

void maze_remove_player(OBJECT avatar, int row, int col) {
	debug("maze_remove_player\n");
	P(&mutex);
	if(the_maze[row][col] != EMPTY) {
		the_maze[row][col] = EMPTY;
	}
	V(&mutex);
}

int maze_move(int row, int col, int dir) {
	debug("maze_move\n");
	if(the_maze[row][col] != EMPTY) {
		if(dir == NORTH) {
			if(the_maze[row - 1][col] == EMPTY) {
				// debug("???????");
				the_maze[row - 1][col] = the_maze[row][col];
				the_maze[row][col] = EMPTY;
				debug("DONE maze_move NORTH");
				return 0;
			}
		}
		else if(dir == WEST) {
			if(the_maze[row][col - 1] == EMPTY) {
				the_maze[row][col - 1] = the_maze[row][col];
				// debug("???????");
				the_maze[row][col] = EMPTY;
				debug("DONE maze_move WEST");
				return 0;
			}
		}
		else if(dir == SOUTH) {
			if(the_maze[row + 1][col] == EMPTY) {
				// debug("???????");
				the_maze[row + 1][col] = the_maze[row][col];
				the_maze[row][col] = EMPTY;
				debug("DONE maze_move SOUTH");
				return 0;
			}
		}
		else if(dir == EAST) {
			if(the_maze[row][col + 1] == EMPTY) {
				// debug("???????");
				the_maze[row][col + 1] = the_maze[row][col];
				the_maze[row][col] = EMPTY;
				debug("DONE maze_move EAST");
				return 0;
			}
		}
	}
	return -1;
}

OBJECT maze_find_target(int row, int col, DIRECTION dir) {
	debug("maze_find_target\n");
	P(&mutex);
	if(dir == NORTH && row - 1 >= 0) {
		for(int i = row - 1; i >= 0; i--) {
			if(the_maze[i][col] != EMPTY) {
				if(IS_AVATAR(the_maze[i][col])) {
					V(&mutex);
					debug("TTTTTTTTTTTtarget: %c", the_maze[i][col]);
					return the_maze[i][col];
				}
				V(&mutex);
				return EMPTY;
			}
		}
	}
	else if(dir == WEST && col - 1 >= 0) {
		for(int i = col - 1; i >= 0; i--) {
			if(the_maze[row][i] != EMPTY) {
				if(IS_AVATAR(the_maze[row][i])) {
					V(&mutex);
					debug("TTTTTTTTTTTTtarget: %c", the_maze[row][i]);
					return the_maze[row][i];
				}
				V(&mutex);
				return EMPTY;
			}
		}
	}
	else if(dir == SOUTH && row + 1 <= maze_get_rows()) {
		for(int i = row + 1; i < maze_get_rows(); i++) {
			if(the_maze[i][col] != EMPTY) {
				if(IS_AVATAR(the_maze[i][col])) {
					V(&mutex);
					debug("TTTTTTTTTTTTTTTtarget: %c", the_maze[i][col]);
					return the_maze[i][col];
				}
				V(&mutex);
				return EMPTY;
			}
		}
	}
	else if(dir == EAST && col + 1 <= maze_get_cols()) {
		for(int i = col + 1; i < maze_get_cols(); i++) {
			if(the_maze[row][i] != EMPTY) {
				if(IS_AVATAR(the_maze[row][i])) {
					V(&mutex);
					debug("TTTTTTTTTTTTTtarget: %c", the_maze[row][i]);
					return the_maze[row][i];
				}
				V(&mutex);
				return EMPTY;
			}
		}
	}
	V(&mutex);
	return EMPTY;
}

int maze_get_view(VIEW *view, int row, int col, DIRECTION gaze, int depth) {
	debug("maze_get_view depth: %d\n", depth);
	if(depth >= VIEW_DEPTH) {
		depth = VIEW_DEPTH;
	}
	char (*the_view)[depth][VIEW_WIDTH] = view;
	int end = depth;
	if(gaze == NORTH) {
		if(row - depth < 0){
			debug("N wall before depth\n");
			end = row + 1;
		}
		for(int i = 0; i < end; i++) {
			(*the_view)[i][LEFT_WALL] = the_maze[row - i][col - 1];
			(*the_view)[i][CORRIDOR] = the_maze[row - i][col];
			(*the_view)[i][RIGHT_WALL] = the_maze[row - i][col + 1];
			debug("%s", *the_view[i]);
		}
		debug("DONE maze_get_view NORTH\n");
		return end;
	}
	else if(gaze == WEST) {
		if(col - depth < 0){
			debug("W wall before depth\n");
			end = col + 1;
		}
		for(int i = 0; i < end; i++) {
			(*the_view)[i][LEFT_WALL] = the_maze[row + 1][col - i];
			(*the_view)[i][CORRIDOR] = the_maze[row][col - i];
			(*the_view)[i][RIGHT_WALL] = the_maze[row - 1][col - i];
			debug("%s", *the_view[i]);
		}
		debug("DONE maze_get_view WEST\n");
		return end;
	}
	else if(gaze == SOUTH) {
		if(row + depth > maze_get_rows()){
			debug("S wall after depth\n");
			end = maze_get_rows() - row;
		}
		for(int i = 0; i < end; i++) {
			(*the_view)[i][LEFT_WALL] = the_maze[row + i][col + 1];
			(*the_view)[i][CORRIDOR] = the_maze[row + i][col];
			(*the_view)[i][RIGHT_WALL] = the_maze[row + i][col - 1];
			debug("%s", *the_view[i]);
		}
		debug("DONE maze_get_view SOUTH\n");
		return end;
	}
	else if(gaze == EAST) {
		if(col + depth > maze_get_cols()){
			debug("E wall after depth\n");
			end = maze_get_cols() - col;
		}
		for(int i = 0; i < end; i++) {
			(*the_view)[i][LEFT_WALL] = the_maze[row - 1][col + i];
			(*the_view)[i][CORRIDOR] = the_maze[row][col + i];
			(*the_view)[i][RIGHT_WALL] = the_maze[row + 1][col + i];
		}
		debug("DONE maze_get_view EAST\n");
		return end;
	}
	return -1;
}

void show_view(VIEW *view, int depth) {
	debug("show_view\n");
	char (*the_view)[depth][VIEW_WIDTH] = view;
	fprintf(stderr, "%s\n", *the_view[LEFT_WALL]);
	fprintf(stderr, "%s\n", *the_view[CORRIDOR]);
	fprintf(stderr, "%s\n", *the_view[RIGHT_WALL]);
}

void show_maze() {
	debug("show_maze\n");
	for(int i = 0; i < maze_get_rows(); i++) {
		fprintf(stderr, "%s\n", the_maze[i]);
	}
}