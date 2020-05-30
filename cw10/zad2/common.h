#define PLAYERS_MAX 20
#define BACKLOG_MAX 10
#define MESSAGE_LENGTH_MAX 128

typedef enum { EMPTY, X, O } field;

typedef struct {
    int x_on_move;
    field fields[9];
} board_t;

board_t init_new_board() {
    board_t board;
    board.x_on_move = 0;
    for(int i=0; i<9; i++){
        board.fields[i] = EMPTY;
    }
    return board;
}


field check_if_winner(board_t* board) {
    if (board->fields[0] == board->fields[4]
    && board->fields[0] == board->fields[8]
    && board->fields[0] != EMPTY)
        return board->fields[0];

    if (board->fields[2] == board->fields[4]
        && board->fields[2] == board->fields[6]
        && board->fields[2] != EMPTY)
        return board->fields[2];

    for (int x = 0; x < 3; x++) {
        if (board->fields[x] == board->fields[x+3]
            && board->fields[x] == board->fields[x+6]
            && board->fields[x] != EMPTY)
            return board->fields[x];
    }

    for (int y = 0; y < 3; y++) {
        if (board->fields[3*y] == board->fields[3*y+1]
            && board->fields[3*y] == board->fields[3*y+2]
            && board->fields[3*y] != EMPTY)
            return board->fields[3*y];
    }

    return EMPTY;
}

field get_winner(board_t* board) {
    field winner = check_if_winner(board);
    return winner;
}

int make_move(board_t* board, int number_od_field) {
    if (number_od_field < 0 || number_od_field > 9 || board->fields[number_od_field] != EMPTY)
        return 0;
    board->fields[number_od_field] = board->x_on_move ? X : O;
    board->x_on_move = !board->x_on_move;
    return 1;
}