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

#define F_FOOD 0
#define F_LIVE 1
#define F_NOWALL 2

#define S_NORMAL 0
#define S_NOWALL 1

int dir[4][2] =
            {{-1, 0}, //left
             {1, 0},  //right
             {0, -1}, //up
             {0, 1}}; //down
             
char foodsymbol[] = ".+*";

int snake[1000];
bool smap[250000], lmap[250000];
int foodpos, snakelen, sdir = D_RIGHT,
    foodtype, snakestate,
    level = 1, lives = 2, 
    delay = 1000, lastnow,
    snakelim = 15, h, w;
bool action = false, 
     gameover = false, 
     paused = false;
     
char t[80], f[80];

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
        sprintf(t, "%d", i);
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
    snakestate = S_NORMAL;
}

void on_key(int code) {
    if (!action) {
        if (gameover) {
            endwin();
            exit(0);
        }
    } else {
        if (code == 'Q' || code == 'q') {
            endwin();
            exit(0);
        }
        if (!paused) {
            if (code == 'P' || code == 'p') {
                paused = true;
                message("Pause");
                return;
            }
            int newdir;
            switch (code) {
                case KEY_LEFT: newdir = D_LEFT;
                    break;
                case KEY_RIGHT: newdir = D_RIGHT;
                    break;
                case KEY_UP: newdir = D_UP;
                    break;
                case KEY_DOWN: newdir = D_DOWN;
                    break;
                default:
                    return;
            }
            if (newdir / 2 != sdir / 2) {
                sdir = newdir;
            }
            lastnow = mstime();
            smove();
        } else if (code == 'P' || code == 'p') {
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
        if (snakestate != S_NOWALL) {
            end();
            return;
        }
        hx = (hx + w) % w;
        hy = (hy + h) % h;
    }
    int newhead = hy * w + hx;
    smap[snake[0]] = false;
    if (smap[newhead] || lmap[newhead]) {
        end();
        return;
    }
    if (newhead != foodpos || foodtype != F_FOOD) {
        int i;
        for (i = 0; i < snakelen - 1; i++)
            snake[i] = snake[i + 1];
        snake[snakelen - 1] = newhead;
        smap[newhead] = true;
    }
    if (newhead == foodpos) {
        switch (foodtype) {
            case F_FOOD:
                snake[snakelen++] = newhead;
                smap[newhead] = true;
                snakestate = S_NORMAL;
                break;
            case F_LIVE:
                lives++;
                snakestate = S_NORMAL;
                break;
            case F_NOWALL:
                snakestate = S_NOWALL;
                break;
        }
        gen_food();
    }
    show();
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
    move(foodpos / w, foodpos % w);
    addch(foodsymbol[foodtype]);
    int k = 0, i, j;
    for (i = 0; i < h; i++)
        for (j = 0; j < w; j++, k++) {
            move(i, j);
            if (smap[k] || lmap[k]) {
                if (smap[k])
                    addch('O');
                else
                    addch('X');
                refresh();
            } else if (k != foodpos)
                addch(' ');
        }
    attrset(A_REVERSE);
    for (i = 0; i < w; i++) {
        move(h, i);
        addch(' ');
    }
    move(h, 0);
    addstr("Level: ");
    sprintf(t, "%d", level);
    addstr(t);
    addstr("\tLives: ");
    sprintf(t, "%d", lives);
    addstr(t);
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
    int extrafood = (rand() % 31 == 0);
    foodtype = 0;
    if (extrafood)
        foodtype = rand() % 2 + 1;
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
