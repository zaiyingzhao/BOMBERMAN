#include <stdio.h>
#include <stdlib.h>
#define BOARD_SIZE 11
#define PLAYER_NUM 2


typedef struct point {
    int x;
    int y;
} Point;

typedef struct bomb {
    int x;
    int y;
    int flag;
} Bomb;

typedef struct cond {
    int n;
    int bomb_num;
    int bomb_count_max;
} Cond;


void print_board(const char board[][BOARD_SIZE + 1]) {
    int c;
    for(int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            c = board[i][j];
            if (c == 'B' || c == 'F') {
                printf("\x1b[31m%c\x1b[m", c);
            } else if (c == '$') {
                printf("\x1b[32m%c\x1b[m", c);
            } else {
                printf("%c", c);
            }
        }
        printf("\r\n");
    }
}

int judge_in_area_player(int y, int x, const char board[][BOARD_SIZE + 1]) { //移動可能なら1,不可能なら0を返す
    if (x >= 0 && x < BOARD_SIZE) {
        if (y >= 0 && y < BOARD_SIZE) {
            if (board[y][x] != '#' && board[y][x] != 'B' && board[y][x] != '$') {
                return 1;
            }
        }
    }
    return 0;
}

int judge_in_area_fire(int y, int x, const char board[][BOARD_SIZE + 1]) { //燃えるなら1,誘爆するなら2，壁か範囲外なら0を返す
    if (x >= 0 && x < BOARD_SIZE) {
        if (y >= 0 && y < BOARD_SIZE) {
            if (board[y][x] == '-' || board[y][x] == '$') {
                return 1;
            } else if (board[y][x] == 'B') {
                return 2;
            }
        }
    }
    return 0;
}

void make_fire(const Cond cond, int y, int x, char board[][BOARD_SIZE + 1], Bomb bomb_point[PLAYER_NUM][cond.bomb_num], int bomb_count[PLAYER_NUM][cond.bomb_num]) {
    board[y][x] = 'F';
    int vecs[4][2] = {{1,0}, {0,1}, {-1,0}, {0,-1}};
    for (int cnt = 0; cnt < 4; ++cnt) {
        for (int i = 1; i < cond.n; ++i) {
            int nx = x + i*vecs[cnt][0];
            int ny = y + i*vecs[cnt][1];
            if (!judge_in_area_fire(ny, nx, board)) {
                break;
            }
            if (judge_in_area_fire(ny, nx, board) == 1) {
                board[ny][nx] = 'F';
            } else if (judge_in_area_fire(ny, nx, board) == 2) {
                for (int j = 0; j < PLAYER_NUM; ++j) {
                    for (int k = 0; k < cond.bomb_num; ++k) {
                        if (bomb_point[j][k].flag == 0 && bomb_point[j][k].y == ny && bomb_point[j][k].x == nx) {
                            bomb_point[j][k].flag = 1;
                            bomb_count[j][k] = cond.bomb_count_max;
                            make_fire(cond, bomb_point[j][k].y, bomb_point[j][k].x, board, bomb_point, bomb_count);
                        }
                    }
                }
            }
        }
    }
}

void clear_fire(const Cond cond, int y, int x, char board[][BOARD_SIZE + 1]) {
    board[y][x] = '-';
    int vecs[4][2] = {{1,0}, {0,1}, {-1,0}, {0,-1}};
    for (int cnt = 0; cnt < 4; ++cnt) {
        for (int i = 1; i < cond.n; ++i) {
            int nx = x + i*vecs[cnt][0];
            int ny = y + i*vecs[cnt][1];
            if (!judge_in_area_player(ny, nx, board)) {
                break;
            }
            if (board[ny][nx] == 'F') {
                board[ny][nx] = '-';
            }
        }
    }
}

int main (int argc, char *argv[]) {
    system("/bin/stty raw onlcr");  // enterを押さなくてもキー入力を受け付けるようになる

    Cond cond;
    //爆炎の長さの設定
    system("clear");
    printf("Enter how many spaces will be burnt in explosion. Enter a number 1-9.\r\n");
    cond.n = getchar();
    cond.n = cond.n - '0';    //ASCIIコード
    while (cond.n < 1 || cond.n > 9) {
        printf("You entered %c. Please enter a number 1-9.\r\n", cond.n + '0');
        cond.n = getchar();
        cond.n = cond.n - '0';
    }
    printf("%d spaces will be burnt in explostion.\r\n",cond.n);

    //1人が置ける爆弾の個数の設定
    printf("\r\nEnter how many bombs each player has. Enter a number 1-9.\r\n");
    cond.bomb_num = getchar();
    cond.bomb_num = cond.bomb_num - '0';
    while (cond.bomb_num < 1 || cond.bomb_num > 9) {
        printf("You enterd %c. Please enter a number 1-9.\r\n", cond.bomb_num + '0');
        cond.bomb_num = getchar();
        cond.bomb_num = cond.bomb_num - '0';
    }
    printf("Each player has %d bombs.\r\n", cond.bomb_num);

    //爆発までの時間の設定
    printf("\r\nEnter limit time of the bombs. Enter a number 01-99. (about 12 is recommended)\r\n");
    int tmp_countmax;
    char buf_c;
    int flag = 1;
    while (flag) {
        tmp_countmax = 0;
        for (int i = 0; i < 2; ++i) {
            buf_c = fgetc(stdin);
            printf("%c",buf_c);
            if (buf_c >= '0' && buf_c <= '9') {
                tmp_countmax = tmp_countmax*10 + buf_c - '0';
            } else {
                printf("\r\nEnter a number 1-99.\r\n");
                break;
            }
            if (i == 1 && tmp_countmax != 0) {
                flag = 0;
            }
        }
    }
    cond.bomb_count_max = tmp_countmax;
    printf("\r\nLimit time is %d turns.\r\n", cond.bomb_count_max);

    //ゲームモードの設定
    printf("\r\nSelect game mode. Enter 1 or 2.\r\n1: Turn Battle\r\n2: Free Turn Battle\r\n");
    int mode = getchar();
    mode = mode - '0';
    while (mode != 1 && mode != 2) {
        printf("You enterd %c. Please enter 1 or 2.\r\n", mode + '0');
        mode = getchar();
        mode = mode - '0';
    }
    if (mode == 1) {
        printf("Turn Battle will start.\r\n");
    } else if (mode == 2) {
        printf("Free Turn Battle will start.\r\n");
    }
    
    //ゲーム開始
    printf("\r\nStart Game. Press any key to continue.\r\n");

    char board[BOARD_SIZE][BOARD_SIZE + 1] = {
        "--$$$$$$$--",
        "-#$#$#$#$#-",
        "$$$$$$$$$$$",
        "$#$#$#$#$#$",
        "$$$$$$$$$$$",
        "$#$#$#$#$#$",
        "$$$$$$$$$$$",
        "$#$#$#$#$#$",
        "$$$$$$$$$$$",
        "-#$#$#$#$#-",
        "--$$$$$$$--"
    };

    //変数の宣言（及び初期化）
    Point p = {.x = 0, .y = 0};  // Player1の位置
    Point q = {.x = 10, .y = 10};  //Player2の位置
    int c;
    int bomb_count[PLAYER_NUM][cond.bomb_num];
    Bomb bomb_point[PLAYER_NUM][cond.bomb_num];
    for (int i = 0; i < PLAYER_NUM; ++i) {
        for (int j = 0; j < cond.bomb_num; ++j) { //countを十分大きな値で初期化，flagを1にする
            bomb_count[i][j] = cond.bomb_count_max + 1;
            bomb_point[i][j].flag = 1;
        }
    }
    int end_flag = 0;
    int turn = 1;
    //ここまで

    while((c = getchar()) != '.') {   // '.' を押すと抜ける
        system("clear");
        printf("Press '.' to close\r\n");        
        printf("You pressed '%c'\r\n", c);

        //キー入力に対する動作の定義
        if (mode == 1) {
            if (turn % 2 == 1) {
                if (c != 'd' && c != 'w' && c != 'a' && c != 's' && c != 'e') {
                    printf("Turn %d\r\n", turn);
                    printf("Now is the turn of player 1\r\n");
                    board[p.y][p.x] = '1';
                    board[q.y][q.x] = '2';
                    print_board(board);
                    board[p.y][p.x] = '-';
                    board[q.y][q.x] = '-';
                    continue;
                }
                ++turn;
                printf("Turn %d\r\n", turn);
                printf("Now is the turn of player 2\r\n");
            } else {
                if (c != 'l' && c != 'i' && c != 'j' && c != 'k' && c != 'o') {
                    printf("Turn %d\r\n", turn);
                    printf("Now is the turn of player 2\r\n");
                    board[p.y][p.x] = '1';
                    board[q.y][q.x] = '2';
                    print_board(board);
                    board[p.y][p.x] = '-';
                    board[q.y][q.x] = '-';
                    continue;
                }
                ++turn;
                printf("Turn %d\r\n", turn);
                printf("Now is the turn of player 1\r\n");
            }
        } else if (mode == 2) {
            printf("Turn %d\r\n", turn);
            ++turn;
        }

        if (c == 'd') {
            ++p.x;
            if (!judge_in_area_player(p.y, p.x, board)) {
                --p.x;
            }
        } 
        if (c == 'w') {
            --p.y;
            if (!judge_in_area_player(p.y, p.x, board)) {
                ++p.y;
            }
        }
        if (c == 'a') {
            --p.x;
            if (!judge_in_area_player(p.y, p.x, board)) {
                ++p.x;
            }
        }
        if (c == 's') {
            ++p.y;
            if (!judge_in_area_player(p.y, p.x, board)) {
                --p.y;
            }
        }
        if (c == 'e') {
            for (int i = 0; i < cond.bomb_num; ++i){
                if (bomb_point[0][i].flag) {
                    bomb_point[0][i].x = p.x;
                    bomb_point[0][i].y = p.y;
                    bomb_point[0][i].flag = 0;
                    bomb_count[0][i] = -1;
                    break;
                }
            }
        }
        //ここまで

        //ここからPlayer2の動きの定義、ただしPlayer1とPlayer2の入力が交互になるのは手動なのでそこは要修正です←対応済み
        if (c == 'l') {
            ++q.x;
            if (!judge_in_area_player(q.y, q.x, board)) {
                --q.x;
            }
        } 
        if (c == 'i') {
            --q.y;
            if (!judge_in_area_player(q.y, q.x, board)) {
                ++q.y;
            }
        }
        if (c == 'j') {
            --q.x;
            if (!judge_in_area_player(q.y, q.x, board)) {
                ++q.x;
            }
        }
        if (c == 'k') {
            ++q.y;
            if (!judge_in_area_player(q.y, q.x, board)) {
                --q.y;
            }
        }
        if (c == 'o') {
            for (int i = 0; i < cond.bomb_num; ++i){
                if (bomb_point[1][i].flag) {
                    bomb_point[1][i].x = q.x;
                    bomb_point[1][i].y = q.y;
                    bomb_point[1][i].flag = 0;
                    bomb_count[1][i] = -1;
                    break;
                }
            }
        }
        // ここまで

        for (int i = 0; i < PLAYER_NUM; ++i) {
            for (int j = 0; j < cond.bomb_num; ++j){
                ++bomb_count[i][j];
            }
        }
        for (int i = 0; i < PLAYER_NUM; ++i) {
            for (int j = 0; j < cond.bomb_num; ++j) {
                if (bomb_count[i][j] == cond.bomb_count_max) { //爆発
                    bomb_point[i][j].flag = 1;
                    make_fire(cond, bomb_point[i][j].y, bomb_point[i][j].x, board, bomb_point, bomb_count);
                }
            }
        }

        if (board[p.y][p.x] == 'F') { //Player1のgame overの判定
            end_flag = 1;
        }

        if (board[q.y][q.x] == 'F') { //Player2のgame overの判定
            end_flag = 2;
        }

        if (board[p.y][p.x] == 'F' && board[q.y][q.x] == 'F') { //引き分けの判定
            end_flag = 3;
        }

        board[p.y][p.x] = '1';
        board[q.y][q.x] = '2';
        print_board(board);
        board[p.y][p.x] = '-';
        board[q.y][q.x] = '-';


        if (end_flag == 1) {
            printf("\x1b[31mPlayer2 win!!\x1b[m\r\n");
            break;
        }

        if (end_flag == 2) {
            printf("\x1b[31mPlayer1 win!!\x1b[m\r\n");
            break;
        }

        if (end_flag == 3) {
            printf("\x1b[32mDraw.\x1b[m\r\n");
            break;
        }

        for (int i = 0; i < PLAYER_NUM; ++i) {
            for (int j = 0; j < cond.bomb_num; ++j) {
                if (bomb_point[i][j].flag == 0) {
                    board[bomb_point[i][j].y][bomb_point[i][j].x] = 'B';
                }
                if (bomb_count[i][j] == cond.bomb_count_max) {
                    clear_fire(cond, bomb_point[i][j].y, bomb_point[i][j].x, board);
                }
            }
        }
    }

    system("/bin/stty cooked");  // 後始末

    return 0;
}
