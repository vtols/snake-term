#include <curses.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define D_LEFT 0
#define D_RIGHT 1
#define D_UP 2
#define D_DOWN 3

int dir[4][2] =
			{{-1, 0}, //left
			 {1, 0},  //right
			 {0, -1}, //up
			 {0, 1}}; //down

int snake[1000];
bool smap[250000], lmap[250000];
int foodpos, snakelen, sdir = D_RIGHT;
bool livefood;
int level = 1, lives = 2, delay = 1000, lastnow;
bool action = false, gameover = false, paused = false;
int snakelim = 15;
int h, w;

void game();
int mstime();
void clear_snake();
void on_key(int);
void smove();
void end();
void show();
void gen_food();
void gen_level();
void message(const char*);
void restart();
void wait(int);

int main() {
	initscr();
	noecho();
	curs_set(false);
	nodelay(stdscr, true);
	keypad(stdscr, true);
	h = LINES - 1;
	w = COLS;
	game();
	endwin();
	return 0;
}

int mstime() {
	return (clock() / (CLOCKS_PER_SEC / 1000));
}

void game() {
	restart();
	while (true) {
		int c = getch();
		if (c != -1) {
			
			move(0, 0);
			refresh();
			
			on_key(c);
		}
		if (action && !paused) {
			int now = mstime();
			if (now - lastnow >= delay) {
				smove();
				lastnow = now;
			}
		}
	}
}

void restart() {
	int waits = 3;
	int i;
	for (i = waits; i >= 0; i--) {
		char t[50];
		char f[] = "%d";
		sprintf(t, f, i);
		message(t);
		if (i > 0)
			wait(1000);
	}
	clear_snake();
	gen_level();
	gen_food();
	sdir = D_RIGHT;
	action = true;
	lastnow = mstime();
	show();
	while (getch() != -1);
}

void clear_snake() {
	snakelen = 3;
	int i;
	for (i = 0; i < snakelen; i++)
		snake[i] = i;
	for (i = 0; i < w * h; i++)
		lmap[i] = smap[i] = false;
	for (i = 0; i < snakelen; i++)
		smap[i] = true;
}

void on_key(int code) {
	if (!action) {
		if (gameover) {
			endwin();
			exit(0);
		}
	} else {
		if (code == 113) {
			endwin();
			exit(0);
		}
		if (!paused) {
			if (code == 112) {
				paused = true;
				message("Pause");
				return;
			}
			int newdir;
			switch (code) {
				case 260: newdir = D_LEFT;
					break;
				case 261: newdir = D_RIGHT;
					break;
				case 259: newdir = D_UP;
					break;
				case 258: newdir = D_DOWN;
					break;
				default:
					return;
			}
			if (newdir / 2 != sdir / 2) {
				sdir = newdir;
			}
			lastnow = mstime();
			smove();
		} else if (code == 112) {
			paused = false;
			lastnow = mstime();
			show();
		}
	}
}

void smove() {
	int hx = snake[snakelen - 1] % w, 
		hy = snake[snakelen - 1] / w;
	hx += dir[sdir][0];
	hy += dir[sdir][1];
	if (hx < 0 || hx >= w ||
		hy < 0 || hy >= h) {
		end();
		return;
	}
	int newhead = hy * w + hx;
	smap[snake[0]] = false;
	if (smap[newhead] || lmap[newhead]) {
		end();
		return;
	}
	if (newhead == foodpos && !livefood) {
		snake[snakelen++] = newhead;
		smap[newhead] = true;
		gen_food();
	} else {
		
		int i;
		for (i = 0; i < snakelen - 1; i++)
			snake[i] = snake[i + 1];
		snake[snakelen - 1] = newhead;
		smap[newhead] = true;
		if (newhead == foodpos && livefood) {
			lives++;
			gen_food();
		}
	}
	show();
	beep();
	if (snakelen >= snakelim) {
		action = false;
		delay /= 1.2;
		level++;
		restart();
	}
}

void message(const char *s) {
	int l = strlen(s);
	int x = w / 2 - l / 2,
		y = h / 2;
		clear();
	move(y, x);
	addstr(s);
	refresh();
}

void show() {
	clear();
	move(foodpos / w, foodpos % w);
	if (livefood)
		addch('+');
	else
		addch('.');
	refresh();
	int k = 0, i, j;
	for (i = 0; i < h; i++)
		for (j = 0; j < w; j++, k++)
			if (smap[k] || lmap[k]) {
				move(i, j);
				if (smap[k])
					addch('O');
				else
					addch('X');
				refresh();
			} 
	move(h, 0);
	char t[20];
	attrset(A_REVERSE);
	addstr("Level: ");
	sprintf(t, "%d", level);
	addstr(t);
	addstr("\tLives: ");
	for (i = 0; i < lives; i++)
		addch('+');
	addstr("\tSnake: ");
	sprintf(t, "%d/%d", snakelen, snakelim);
	addstr(t);
	attrset(0);
}

void gen_level() {
	int walls = (level + 1) * level * 2, i;
	for (i = 0; i < walls; i++) {
		int wpos;
		do {
			wpos = rand() % (w * h);
		} while (smap[wpos] || lmap[wpos]);
		lmap[wpos] = true;
	}
}

void gen_food() {
	srand(mstime());
	while (smap[foodpos] || lmap[foodpos])
		foodpos = rand() % (w * h);
	livefood = (rand() % 31 == 0);
}

void end() {
	if (lives > 0) {
		lives--;
		restart();
		return;
	}
	message("Game over. Press any key.");
	action = false;
	gameover = true;
}

void wait(int ms) {
	int start = mstime();
	while (mstime() - start < ms);
}
