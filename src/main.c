#include <ncurses.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define TIMEOUT 100

typedef struct {
    int fruit_x;
    int fruit_y;
    int state;
    int height;
    int width;
    int score;
} Game_field;

typedef enum {
    LEFT,
    RIGHT,
    UP,
    DOWN
} direction;

typedef struct snake_head {
    int location_y;
    int location_x;
    struct snake_head *next;
    direction dir;
} snake_piece;

Game_field Init_game_field();

void Game_input(Game_field board, snake_piece *head);
void Draw(Game_field board, snake_piece head);
void Spawn_fruit(Game_field *board);
void Movement(snake_piece *head);
void Collission(Game_field *board, snake_piece *head);
void Grow(Game_field *board, snake_piece *head, snake_piece **tail);
WINDOW *game;
void Free_snake(snake_piece *head);

int part_count;



int main(void) {
    srand(time(0));
    initscr();
    curs_set(0);
    keypad(stdscr, TRUE);
    cbreak();
    noecho();
    timeout(0);
    Game_field board = Init_game_field();
    board.height = 20;
    board.width = 50;
    board.state = 1;
    int start_y =  (LINES - board.height) / 2;
    int start_x =  (COLS - board.width) / 2;

    snake_piece head;
    snake_piece *tail = NULL;
    head.dir = RIGHT;
    head.location_y = (board.height / 2);
    head.location_x = (board.width / 2);
    part_count = 1;

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);

    refresh();

    game = newwin(board.height, board.width, start_y, start_x);
    box(game, 0, 0);
    wrefresh(game);
    
    wattron(game, COLOR_PAIR(1));
    Spawn_fruit(&board);
    wattroff(game, COLOR_PAIR(1));

    printw("fruity: %d, fruitx: %d", board.fruit_y, board.fruit_x);
    printw("heady: %d, headx: %d", head.location_y, head.location_x);

    while (board.state == 1) {
	Game_input(board, &head);

	Grow(&board, &head, &tail);

	Movement(&head);
	
	Collission(&board, &head);

	Draw(board, head);

	if (board.state == 0) {
	    sleep(1);
	}

    }
    Free_snake(&head);
    delwin(game);
    endwin();
    return 0;
}

Game_field Init_game_field() {
    Game_field game = {0};
    return game;
}

void Game_input(Game_field board, snake_piece *head) {
    int ch;
    int dir_change = 0;

    while ((ch = getch()) != ERR) {
	if (ch == KEY_RESIZE) {
	    endwin();
	    refresh();
	    clear();

	    int start_y = (LINES - board.height) / 2;
	    int start_x = (COLS - board.width) / 2;

	    delwin(game);
	    refresh();
	    game = newwin(board.height, board.width, start_y, start_x);
	    box(game, 0, 0);
	    wrefresh(game);
	}
	if (dir_change) continue;

	if (ch == KEY_UP && head->dir != DOWN) {
	    head->dir = UP;
	    dir_change = 1;
	} else if (ch == KEY_DOWN && head->dir != UP) {
	    head->dir = DOWN;
	    dir_change = 1;
	} else if (ch == KEY_LEFT && head->dir != RIGHT) {
	    head->dir = LEFT;
	    dir_change = 1;
	} else if (ch == KEY_RIGHT && head->dir != LEFT) {
	    head->dir = RIGHT;
	    dir_change = 1;
	}
    }

    napms(TIMEOUT);
}

void Draw(Game_field board, snake_piece head) {
    werase(game);

    box(game, 0, 0);
    // map
    for (int i = 1; i < board.height - 1; ++i) {
	for (int j = 1; j < board.width - 1; ++j) {
	    mvwprintw(game, i, j, ".");
	}

    }

    //score
    attron(COLOR_PAIR(2));
    mvprintw(20, 10, "Score: %d", board.score);
    attroff(COLOR_PAIR(2));
    
    // fruit
    wattron(game, COLOR_PAIR(1));
    mvwprintw(game, board.fruit_y, board.fruit_x, "A");
    wattroff(game, COLOR_PAIR(1));

    // snake
    wattron(game,COLOR_PAIR(2));
    for (snake_piece *tmp = &head; tmp != NULL; tmp = tmp->next) {
	mvwprintw(game, tmp->location_y, tmp->location_x, "#");
    }
    wattroff(game,COLOR_PAIR(2));


    wrefresh(game);
}

void Spawn_fruit(Game_field *board) {
    board->fruit_y = rand() % (board->height - 2) + 1;
    board->fruit_x = rand() % (board->width - 2) + 1;
}

void Movement(snake_piece *head) {
    int next_y;
    int next_x;
    int piece_next_y;
    int piece_next_x;
    int piece_next_dir;

    int previous_dir;
    if (head->dir == UP) {
	next_y = head->location_y;
	next_x = head->location_x;
	head->location_y--;
    }
    if (head->dir == DOWN) {
	next_y = head->location_y;
	next_x = head->location_x;
	head->location_y++;
    }
    if (head->dir == LEFT) {
	next_y = head->location_y;
	next_x = head->location_x;
	head->location_x--;
    }
    if (head->dir == RIGHT) {
	next_y = head->location_y;
	next_x = head->location_x;
	head->location_x++;
    }

    previous_dir = head->dir;
    for (snake_piece *tmp = head->next; tmp != NULL; tmp = tmp->next) {
	piece_next_x = tmp->location_x;
	piece_next_y = tmp->location_y;
	piece_next_dir = tmp->dir;

	tmp->dir = previous_dir;
	tmp->location_y = next_y;
	tmp->location_x = next_x;

	next_y = piece_next_y;
	next_x = piece_next_x;
	previous_dir = piece_next_dir;
    }
}

void Collission(Game_field *board, snake_piece *head) {
    if (head->location_y == 0 || head->location_y == board->height - 1 || head->location_x == 0 || head->location_x == board->width - 1) {
	board->state = 0;
    }

    for (snake_piece *tmp = head->next; tmp != NULL; tmp = tmp->next) {
	if (tmp->location_y == head->location_y && tmp->location_x == head->location_x) {
	    board->state = 0;
	}
    }
}

void Grow(Game_field *board, snake_piece *head, snake_piece **tail) {
    snake_piece *part = NULL;

    if (head->location_y == board->fruit_y && head->location_x == board->fruit_x) {
	part = malloc(sizeof(snake_piece));

	if (part != NULL && part_count == 1) {
	    part->dir = head->dir;
	    part->next = NULL;
	    head->next = part;
	    *tail = part;
	} else if (part != NULL) {
	    part->dir = (*tail)->dir;
	    part->next = NULL;
	    (*tail)->next = part;
	    *tail = part;
	}

	Spawn_fruit(board);
	board->score++;
	part_count++;
    }
}

void Free_snake(snake_piece *head) {
    snake_piece *tmp_head = head->next;
    snake_piece *next = NULL;

    while (tmp_head != NULL) {
	next = tmp_head->next;
	free(tmp_head);
	tmp_head = next;
    }
}
