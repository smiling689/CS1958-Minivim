#include <cstdlib>
#include <ncurses.h>
#include <stdlib.h>
#include<string>
#include<iostream>
#include<cstdlib>
#include <stdio.h>
#include <string.h>
#include<vector>
#include<cmath>
#include<cstring>

#include <fstream>

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cin;
using ll = long long;


#define REG_COLOR_NUM 1
#define CUS_COLOR_NUM 2
#define ABC_COLOR_RED 3//test
#define ABC_COLOR_GREEN 4
#define ABC_COLOR_PINK 5
#define MAX_LENGTH 1000

enum mod { Normal_mod, Insert_mod, Command_mod };


mod mod_now = Normal_mod;

void copyFilesToVisible(WINDOW *File, WINDOW *Visible_File, int start_line);

void copyLinesToVisible(WINDOW *Line, WINDOW *Visible_Line, int start_line);

vector<string> txt;
string line;
string open_mod;
int start_line;
int win_line;
std::vector<size_t> y_max;
std::vector<size_t> line_max;
std::vector<size_t> line_break;
std::vector<size_t> y_to_line;
std::vector<size_t> line_to_y_first;
std::vector<size_t> line_to_y_last;
vector<string> command_history;
ll command_str;
bool only_read = false;


void initial();

int main(int argc, char *argv[]) {
    initscr();
    raw();
    noecho();
    keypad(stdscr, true); //stdscr is the regular window
    // init color
    start_color();
    init_pair(REG_COLOR_NUM, COLOR_WHITE, COLOR_BLACK);
    init_pair(CUS_COLOR_NUM, COLOR_YELLOW, COLOR_CYAN);
    init_pair(ABC_COLOR_RED, COLOR_RED, COLOR_CYAN);
    init_pair(ABC_COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE);
    init_pair(ABC_COLOR_PINK, COLOR_WHITE, COLOR_BLUE);
    wbkgd(stdscr, COLOR_PAIR(REG_COLOR_NUM)); // background
    wrefresh(stdscr);
    //some settings
    if (LINES < 7) {
        endwin();
        printf("window line size is small than 7\n");
        return 0;
    }
    if (COLS < 20) {
        endwin();
        printf("window column size is small than 20\n");
        return 0;
    }
    win_line = LINES - 5;
    WINDOW *Visible_File;
    Visible_File = newwin(win_line, COLS - 6, 0, 6);
    wmove(Visible_File, 0, 0);
    wbkgd(Visible_File, COLOR_PAIR(REG_COLOR_NUM));
    keypad(Visible_File, true);
    wrefresh(Visible_File);

    WINDOW *Visible_Line = newwin(win_line, 6, 0, 0);
    wmove(Visible_Line, 0, 0);
    wbkgd(Visible_Line, COLOR_PAIR(REG_COLOR_NUM));
    keypad(Visible_Line, true);
    wrefresh(Visible_Line);

    WINDOW *File = newwin(200, COLS - 6, 20000, 200);
    wmove(File, 0, 0);
    wbkgd(File, COLOR_PAIR(REG_COLOR_NUM));
    keypad(File, true);
    wrefresh(File);

    WINDOW *Line = newwin(200, 6, 20000, 200);
    wmove(Line, 0, 0);
    wbkgd(Line, COLOR_PAIR(REG_COLOR_NUM));
    keypad(Line, true);
    wrefresh(Line);


    for (int i = 0; i < win_line; i++) {
        wmove(Visible_Line, i, 5);
        wprintw(Visible_Line, "|");
    }
    if (argc != 2 && argc != 3) {
        printf("WRONG:./minivim <filename>\n");
        return 1;
    }
    int file = 1;
    if (argc == 3) {
        file = 2;
        open_mod = argv[1];
    }
    bool isCRLF = false;
    bool new_file = false;
    FILE *fp = freopen(argv[file], "r", stdin);
    if (fp == NULL) {
        std::ofstream outfile(argv[file]);
        if(!outfile.is_open()) {
            printf("WRONG:./minivim <filename> , cannot make out this new file!\n");
        }
        new_file = true;
        line_max.push_back(0);
        y_max.push_back(1);
        line_to_y_first.push_back(0);
        line_to_y_last.push_back(0);
        wmove(Line, 0, 0);
        wprintw(Line, "%5d", 1);
    }
    else {
        fseek(fp, 0, SEEK_END);
        long int size = ftell(fp);
        if (size == 0) {
            line_max.push_back(0);
            y_max.push_back(1);
            line_to_y_first.push_back(0);
            line_to_y_last.push_back(0);
            wmove(Line, 0, 0);
            wprintw(Line, "%5d", 1);
        }
        else {

            fseek(fp, 0, SEEK_SET);
            int i = 0;
            while (std::getline(cin, line)) {
                txt.push_back(line);
                ll num_of_lines = (txt[i].size() - 1) / (COLS - 6);
                if (line == "") {
                    line_max.push_back(0);
                } else {
                    line_max.push_back(txt[i].size() - 1);
                    if (txt[i].size() >= 2 && txt[i][txt[i].size() - 2] == '\r' && txt[i][txt[i].size() - 1] == '\n') {
                        isCRLF = true;
                    } else {
                        isCRLF = false;
                    }
                }
                line_break.push_back((txt[i].size() - 1) / (COLS - 6));
                for (int i = 0; i < num_of_lines; i++) {
                    y_to_line.push_back(i);
                }
                i++;
            }
            ll line_num = 0, y_num = 0;
            for (auto it = txt.begin(); it != txt.end(); it++, line_num++) {
                line_to_y_first.push_back(y_num);
                ll len = it->size();
                ll line_num_now = len / (COLS - 6) + 1;
                wmove(Line, y_num, 0);
                wprintw(Line, "%5d", line_num + 1);
                for (int x = 1; x < line_num_now; x++) {
                    char a[COLS];
                    memset(a, 0, COLS);
                    for (ll i = (x - 1) * (COLS - 6); i < x * (COLS - 6); i++) {
                        a[i - (x - 1) * (COLS - 6)] = (*it)[i];
                    }
                    wmove(File, y_num, 0);
                    wprintw(File, "%s", a);
                    y_max.push_back((COLS - 6));
                    y_num++;
                }
                if (len % (COLS - 6) != 0) {
                    char a[len + 5];
                    memset(a, 0, len + 5);
                    for (ll i = 0; i < len % (COLS - 6); i++) {
                        a[i] = (*it)[i + (len / (COLS - 6)) * (COLS - 6)];
                    }
                    y_max.push_back(len % (COLS - 6));
                    wmove(File, y_num, 0);
                    wprintw(File, "%s", a);
                    y_num++;
                }
                line_to_y_last.push_back(y_num - 1);
            }
        }

    }



    wrefresh(File);
    wrefresh(Line);
    //set the File window

    fflush(fp);
    freopen("/dev/tty", "r", stdin);

    if (argc == 3) {
        if (strcmp(argv[1], "-t") == 0) {
            wclear(File);
        } else if (strcmp(argv[1], "-R") == 0) {
            only_read = true;
        } else {
            endwin();
            printf("WRONG:./minivim [options] <filename>\n");
            printf("your option:%s.\n", argv[1]);
            return 1;
        }
    }

    WINDOW *Information = newwin(2, COLS, win_line, 0);
    wbkgd(Information, COLOR_PAIR(ABC_COLOR_RED));
    wclear(Information);
    wprintw(Information, "Mod : Normal  ");
    int y, x;
    getmaxyx(Information, y, x);
    wmove(Information, y, x);
    wprintw(Information, " Filename : %s", argv[file]);
    wmove(Information, 1, 0);
    wprintw(Information, "Row :%3d , Col :%3d", 1, 1);
    wrefresh(Information);
    //set the information window
    WINDOW *Command = newwin(3, COLS, win_line + 2, 0);
    wbkgd(Command, COLOR_PAIR(ABC_COLOR_GREEN));
    wprintw(Command, "This is the command window ");
    wrefresh(Command);
    //set the command window
    start_line = 0;
    copyFilesToVisible(File, Visible_File, start_line);
    copyLinesToVisible(Line, Visible_Line, start_line);
    wrefresh(Visible_File);
    wrefresh(Visible_Line);

    //settings
    //------------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------------//
    //------------------------------------------------------------------------------------//
    ll y_up_max = 0;
    ll x_up_max = 0;
    ll x_command = 1;
    bool do_some = false;
    ll ch = 0;
    string command_line;
    ll fast_this = 0, fast_last = 0;
    //ch is the char that get in every time ,command line is the command we need to get
    wmove(File, 0, 0);
    wrefresh(Command);
    wrefresh(File);
    vector<int> ch_current;
    ll x_now = 0, y_now = 0, line_now = 0, y_now_visible = 0, y_last = 0;
    while (ch = getch()) {
        if (ch == 27) {
            if (mod_now == Insert_mod && x_now == line_max[y_now] && x_now > 0) {
                x_now--;
                wmove(File, y_now, x_now);
                wrefresh(File);
            }
            wmove(Information, 0, 0);
            wprintw(Information, "Mod : Normal  ");
            wrefresh(Information);
            mod_now = Normal_mod;
            wmove(File, y_now, x_now);
            wrefresh(File);
        }
        //if enter 'esc' and now it is not normal mod ,change the mod to normal
        else if (mod_now == Insert_mod) {
            switch (ch) {
                case KEY_UP: {
                    if (line_now > 0) {
                        ll l1 = y_now - line_to_y_first[line_now];
                        if (l1 >= y_up_max) {
                            y_up_max = l1;
                            if (x_now > x_up_max) {
                                x_up_max = x_now;
                            }
                        }
                        ll l2 = line_to_y_last[line_now - 1] - line_to_y_first[line_now - 1];
                        if (y_up_max < l2 || (y_up_max == l2 && x_up_max < y_max[line_to_y_last[line_now - 1]])) {
                            line_now--;
                            y_now = line_to_y_first[line_now] + y_up_max;
                            x_now = x_up_max;
                        } else {
                            line_now--;
                            y_now = line_to_y_last[line_now];
                            x_now = y_max[y_now];
                        }
                    }
                    break;
                }
                case KEY_DOWN: {
                    if (line_now < line_max.size() - 1) {
                        ll l1 = y_now - line_to_y_first[line_now];
                        if (l1 >= y_up_max) {
                            y_up_max = l1;
                            if (x_now > x_up_max) {
                                x_up_max = x_now;
                            }
                        }
                        ll l2 = line_to_y_last[line_now + 1] - line_to_y_first[line_now + 1];
                        if (y_up_max < l2 || (y_up_max == l2 && x_up_max < y_max[line_to_y_last[line_now + 1]])) {
                            line_now++;
                            y_now = line_to_y_first[line_now] + y_up_max;
                            x_now = x_up_max;
                        } else {
                            line_now++;
                            y_now = line_to_y_last[line_now];
                            x_now = y_max[y_now];
                        }
                    }
                    break;
                }
                case KEY_LEFT: {
                    if (x_now >= 1) {
                        x_now--;
                    } else if (x_now == 0 && line_to_y_first[line_now] != y_now) {
                        y_now--;
                        x_now = (COLS - 6) - 1;
                    } else if (x_now == 0 && y_now >= 1 && line_to_y_first[line_now] == y_now) {
                        y_now--;
                        line_now--;
                        x_now = y_max[y_now];
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                }
                case KEY_RIGHT: {
                    if (x_now < y_max[y_now] && x_now < (COLS - 6) - 1) {
                        x_now++;
                    } else if (x_now == (COLS - 6) - 1 && y_now < y_max.size() - 1 && y_now != line_to_y_last[
                                   line_now]) {
                        y_now++;
                        x_now = 0;
                    } else if (x_now == y_max[y_now] && y_now < y_max.size() - 1 && y_now == line_to_y_last[line_now]) {
                        y_now++;
                        line_now++;
                        x_now = 0;
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                }
                //move
                case 10: {
                    do_some = true;
                    if (x_now == y_max[y_now] && y_now == line_to_y_last[line_now]) {
                        int y_current = y_max.size() - 1;
                        while (y_current > y_now) {
                            for (int x_current = 0; x_current < y_max[y_current]; x_current++) {
                                int ch_line = mvwinch(File, y_current, x_current);
                                mvwprintw(File, y_current + 1, x_current, "%c", ch_line);
                            }
                            wmove(File, y_current, 0);
                            wclrtoeol(File);
                            y_current--;
                        }
                        y_max.push_back(y_max[y_max.size() - 1]);
                        for (int l = y_max.size() - 2; l > line_to_y_last[line_now]; l--) {
                            y_max[l] = y_max[l - 1];
                        }
                        for (int l = y_now + 1; l <= line_to_y_last[line_now]; l++) {
                            wmove(File, l, 0);
                            wclrtoeol(File);
                        }
                        y_now++;
                        x_now = 0;
                        line_now++;
                        line_max.insert(line_max.begin() + line_now, 0);
                        line_to_y_first.insert(line_to_y_first.begin() + line_now, y_now);
                        line_to_y_last.insert(line_to_y_last.begin() + line_now, line_to_y_last[line_now - 1] + 1);
                        line_to_y_last[line_now - 1] = y_now - 1;
                        for (int l = line_to_y_last.size() - 1; l > line_now; l--) {
                            line_to_y_last[l]++;
                        }
                        for (int l = line_to_y_first.size() - 1; l > line_now; l--) {
                            line_to_y_first[l]++;
                        }
                        for (int l = line_now + 1; l < line_max.size(); l++) {
                            wmove(Line, line_to_y_first[l] - 1, 0);
                            wprintw(Line, "     ");
                        }
                        for (int l = line_now; l < line_max.size(); l++) {
                            wmove(Line, line_to_y_first[l], 0);
                            wprintw(Line, "%5d", l + 1);
                        }
                        y_max[y_now] = 0;
                        wrefresh(Line);
                        break;
                    }
                    vector<int>().swap(ch_current);
                    for (int x_current = x_now; x_current < y_max[y_now]; x_current++) {
                        int ch_line = mvwinch(File, y_now, x_current);
                        ch_current.push_back(ch_line);
                        line_max[line_now]--;
                    }
                    y_max[y_now] = x_now;
                    for (int l = y_now + 1; l <= line_to_y_last[line_now]; l++) {
                        for (int x_current = 0; x_current < y_max[l]; x_current++) {
                            int ch_line = mvwinch(File, l, x_current);
                            ch_current.push_back(ch_line);
                            line_max[line_now]--;
                        }
                    }
                    if (y_max[line_to_y_last[line_now]] < x_now) {
                        for (int l = y_now + 1; l <= line_to_y_last[line_now]; l++) {
                            wmove(File, l, 0);
                            wclrtoeol(File);
                        }
                        wmove(File, y_now, x_now);
                        wclrtoeol(File);
                        int len = ch_current.size();
                        int line_num_now = len / (COLS - 6) + 1;
                        for (int x = 1; x < line_num_now; x++) {
                            char a[COLS];
                            memset(a, 0, COLS);
                            for (int i = (x - 1) * (COLS - 6); i < x * (COLS - 6); i++) {
                                a[i - (x - 1) * (COLS - 6)] = ch_current[i];
                            }
                            wmove(File, y_now + x, 0);
                            wprintw(File, "%s", a);
                            y_max[y_now + x] = (COLS - 6);
                        }
                        if (len % (COLS - 6) != 0) {
                            char a[len + 5];
                            memset(a, 0, len + 5);
                            for (int i = 0; i < len % (COLS - 6); i++) {
                                a[i] = ch_current[i + (len / (COLS - 6)) * (COLS - 6)];
                            }
                            y_max[y_now + line_num_now] = (len % (COLS - 6));
                            wmove(File, y_now + line_num_now, 0);
                            wprintw(File, "%s", a);
                        }
                        y_now++;
                        x_now = 0;
                        line_now++;
                        line_max.insert(line_max.begin() + line_now, ch_current.size());
                        line_to_y_first.insert(line_to_y_first.begin() + line_now, y_now);
                        line_to_y_last.insert(line_to_y_last.begin() + line_now, line_to_y_last[line_now - 1]);
                        line_to_y_last[line_now - 1] = y_now - 1;
                        for (int l = line_now; l < line_max.size(); l++) {
                            wmove(Line, line_to_y_first[l], 0);
                            wprintw(Line, "%5d", l + 1);
                        }
                    } else {
                        int y_current = y_max.size() - 1;
                        while (y_current > line_to_y_last[line_now]) {
                            for (int x_current = 0; x_current < y_max[y_current]; x_current++) {
                                int ch_line = mvwinch(File, y_current, x_current);
                                mvwprintw(File, y_current + 1, x_current, "%c", ch_line);
                            }
                            wmove(File, y_current, 0);
                            wclrtoeol(File);
                            y_current--;
                        }
                        y_max.push_back(y_max[y_max.size() - 1]);
                        for (int l = y_max.size() - 2; l > line_to_y_last[line_now]; l--) {
                            y_max[l] = y_max[l - 1];
                        }
                        for (int l = y_now + 1; l <= line_to_y_last[line_now]; l++) {
                            wmove(File, l, 0);
                            wclrtoeol(File);
                        }
                        wmove(File, y_now, x_now);
                        wclrtoeol(File);
                        int len = ch_current.size();
                        int line_num_now = len / (COLS - 6) + 1;
                        for (int x = 1; x < line_num_now; x++) {
                            char a[COLS];
                            memset(a, 0, COLS);
                            for (int i = (x - 1) * (COLS - 6); i < x * (COLS - 6); i++) {
                                a[i - (x - 1) * (COLS - 6)] = ch_current[i];
                            }
                            wmove(File, y_now + x, 0);
                            wprintw(File, "%s", a);
                            y_max[y_now + x] = (COLS - 6);
                        }
                        if (len % (COLS - 6) != 0) {
                            char a[len + 5];
                            memset(a, 0, len + 5);
                            for (int i = 0; i < len % (COLS - 6); i++) {
                                a[i] = ch_current[i + (len / (COLS - 6)) * (COLS - 6)];
                            }
                            y_max[y_now + line_num_now] = (len % (COLS - 6));
                            wmove(File, y_now + line_num_now, 0);
                            wprintw(File, "%s", a);
                        }
                        y_now++;
                        x_now = 0;
                        line_now++;
                        line_max.insert(line_max.begin() + line_now, ch_current.size());
                        line_to_y_first.insert(line_to_y_first.begin() + line_now, y_now);
                        line_to_y_last.insert(line_to_y_last.begin() + line_now, line_to_y_last[line_now - 1] + 1);
                        line_to_y_last[line_now - 1] = y_now - 1;
                        for (int l = line_to_y_last.size() - 1; l > line_now; l--) {
                            line_to_y_last[l]++;
                        }
                        for (int l = line_to_y_first.size() - 1; l > line_now; l--) {
                            line_to_y_first[l]++;
                        }
                        for (int l = line_now + 1; l < line_max.size(); l++) {
                            wmove(Line, line_to_y_first[l] - 1, 0);
                            wprintw(Line, "     ");
                        }
                        for (int l = line_now; l < line_max.size(); l++) {
                            wmove(Line, line_to_y_first[l], 0);
                            wprintw(Line, "%5d", l + 1);
                        }
                    }
                    wrefresh(Line);
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                } //enter
                case KEY_BACKSPACE: {
                    do_some = true;
                    if (x_now != 0) {
                        if (y_max[line_to_y_last[line_now]] == 1) {
                            x_now -= 1;
                            mvwdelch(File, y_now, x_now);
                            for (int l = y_now + 1; l <= line_to_y_last[line_now]; l++) {
                                int ch_line = mvwinch(File, l, 0);
                                mvwprintw(File, l - 1, COLS - 7, "%c", ch_line);
                                mvwdelch(File, l, 0);
                            }
                            y_max[line_to_y_last[line_now]]--;
                            line_max[line_now]--;
                            line_to_y_last[line_now]--;
                            int y_current = line_to_y_last[line_now] + 2;
                            while (y_current <= y_max.size() - 1) {
                                for (int x_current = 0; x_current < y_max[y_current]; x_current++) {
                                    int ch_line = mvwinch(File, y_current, x_current);
                                    mvwprintw(File, y_current - 1, x_current, "%c", ch_line);
                                }
                                wmove(File, y_current, 0);
                                wclrtoeol(File);
                                y_current++;
                            }
                            for (int l = line_now + 1; l < line_max.size(); l++) {
                                line_to_y_last[l]--;
                                line_to_y_first[l]--;
                            }
                            for (int l = line_now + 1; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l] + 1, 0);
                                wprintw(Line, "     ");
                            }
                            for (int l = line_now; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l], 0);
                                wprintw(Line, "%5d", l + 1);
                            }
                            y_max.erase(y_max.begin() + line_to_y_last[line_now] + 1);
                            wrefresh(Line);
                        } else {
                            x_now -= 1;
                            mvwdelch(File, y_now, x_now);
                            for (int l = y_now + 1; l <= line_to_y_last[line_now]; l++) {
                                int ch_line = mvwinch(File, l, 0);
                                mvwprintw(File, l - 1, COLS - 7, "%c", ch_line);
                                mvwdelch(File, l, 0);
                            }
                            y_max[line_to_y_last[line_now]]--;
                            line_max[line_now]--;
                        }
                    } else if (x_now == 0 && y_now != line_to_y_first[line_now]) {
                        if (y_max[line_to_y_last[line_now]] == 1) {
                            x_now = COLS - 7;
                            y_now--;
                            mvwdelch(File, y_now, x_now);
                            for (int l = y_now + 1; l <= line_to_y_last[line_now]; l++) {
                                int ch_line = mvwinch(File, l, 0);
                                mvwprintw(File, l - 1, COLS - 7, "%c", ch_line);
                                mvwdelch(File, l, 0);
                            }
                            y_max[line_to_y_last[line_now]]--;
                            line_max[line_now]--;
                            line_to_y_last[line_now]--;
                            int y_current = line_to_y_last[line_now] + 2;
                            while (y_current <= y_max.size() - 1) {
                                for (int x_current = 0; x_current < y_max[y_current]; x_current++) {
                                    int ch_line = mvwinch(File, y_current, x_current);
                                    mvwprintw(File, y_current - 1, x_current, "%c", ch_line);
                                }
                                wmove(File, y_current, 0);
                                wclrtoeol(File);
                                y_current++;
                            }
                            for (int l = line_now + 1; l < line_max.size(); l++) {
                                line_to_y_last[l]--;
                                line_to_y_first[l]--;
                            }
                            for (int l = line_now + 1; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l] + 1, 0);
                                wprintw(Line, "     ");
                            }
                            for (int l = line_now; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l], 0);
                                wprintw(Line, "%5d", l + 1);
                            }
                            y_max.erase(y_max.begin() + line_to_y_last[line_now] + 1);
                            wrefresh(Line);
                        } else {
                            x_now = COLS - 7;
                            y_now--;
                            mvwdelch(File, y_now, x_now);
                            for (int l = y_now + 1; l <= line_to_y_last[line_now]; l++) {
                                int ch_line = mvwinch(File, l, 0);
                                mvwprintw(File, l - 1, COLS - 7, "%c", ch_line);
                                mvwdelch(File, l, 0);
                            }
                            y_max[line_to_y_last[line_now]]--;
                            line_max[line_now]--;
                        }
                    } else if (x_now == 0 && y_now == line_to_y_first[line_now] && line_now != 0) {
                        if (y_max[y_now] == 0 && line_to_y_last[line_now] == line_to_y_first[line_now]) {
                            y_now--;
                            line_now--;
                            x_now = y_max[y_now];
                            int y_current = y_now + 1;
                            while (y_current <= y_max.size() - 1) {
                                for (int x_current = 0; x_current < y_max[y_current]; x_current++) {
                                    int ch_line = mvwinch(File, y_current, x_current);
                                    mvwprintw(File, y_current - 1, x_current, "%c", ch_line);
                                }
                                wmove(File, y_current, 0);
                                wclrtoeol(File);
                                y_current++;
                            }
                            for (int l = line_now + 1; l < line_max.size(); l++) {
                                line_to_y_last[l]--;
                                line_to_y_first[l]--;
                            }
                            y_max.erase(y_max.begin() + y_now + 1);
                            line_max.erase(line_max.begin() + line_now + 1);
                            line_to_y_first.erase(line_to_y_first.begin() + line_now + 1);
                            line_to_y_last.erase(line_to_y_last.begin() + line_now + 1);
                            for (int l = line_now + 1; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l] + 1, 0);
                                wprintw(Line, "     ");
                            }
                            for (int l = line_now; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l], 0);
                                wprintw(Line, "%5d", l + 1);
                            }
                            wrefresh(Line);
                            copyLinesToVisible(File, Visible_File, start_line);
                            break;
                        }
                        vector<int>().swap(ch_current);
                        y_now--;
                        x_now = y_max[y_now];
                        for (int x_current = 0; x_current < y_max[y_now]; x_current++) {
                            int ch_line = mvwinch(File, y_now, x_current);
                            ch_current.push_back(ch_line);
                        }
                        for (int l = y_now + 1; l <= line_to_y_last[line_now]; l++) {
                            for (int x_current = 0; x_current < y_max[l]; x_current++) {
                                int ch_line = mvwinch(File, l, x_current);
                                ch_current.push_back(ch_line);
                            }
                        }
                        if (y_max[line_to_y_last[line_now - 1]] + y_max[line_to_y_last[line_now]] <= COLS - 4) {
                            for (int l = y_now; l <= line_to_y_last[line_now]; l++) {
                                wmove(File, l, 0);
                                wclrtoeol(File);
                            }
                            line_max[line_now - 1] += line_max[line_now];
                            int len = ch_current.size();
                            int line_num_now = len / (COLS - 6) + 1;
                            for (int x = 1; x < line_num_now; x++) {
                                char a[COLS];
                                memset(a, 0, COLS);
                                for (int i = (x - 1) * (COLS - 6); i < x * (COLS - 6); i++) {
                                    a[i - (x - 1) * (COLS - 6)] = ch_current[i];
                                }
                                wmove(File, y_now + x - 1, 0);
                                wprintw(File, "%s", a);
                                y_max[y_now + x - 1] = (COLS - 6);
                            }
                            if (len % (COLS - 6) != 0) {
                                char a[len + 5];
                                memset(a, 0, len + 5);
                                for (int i = 0; i < len % (COLS - 6); i++) {
                                    a[i] = ch_current[i + (len / (COLS -6)) * (COLS - 6)];
                                }
                                y_max[y_now + line_num_now - 1] = (len % (COLS - 6));
                                wmove(File, y_now + line_num_now - 1, 0);
                                wprintw(File, "%s", a);
                            }
                            int y_current = line_to_y_last[line_now] + 1;
                            while (y_current <= y_max.size() - 1) {
                                for (int x_current = 0; x_current < y_max[y_current]; x_current++) {
                                    int ch_line = mvwinch(File, y_current, x_current);
                                    mvwprintw(File, y_current - 1, x_current, "%c", ch_line);
                                }
                                wmove(File, y_current, 0);
                                wclrtoeol(File);
                                y_current++;
                            }
                            line_now--;
                            line_max.erase(line_max.begin() + line_now + 1);
                            line_to_y_first.erase(line_to_y_first.begin() + line_now + 1);
                            line_to_y_last.erase(line_to_y_last.begin() + line_now);
                            line_to_y_last[line_now]--;
                            for (int l = line_now + 1; l < line_max.size(); l++) {
                                line_to_y_last[l]--;
                                line_to_y_first[l]--;
                            }
                            for (int l = line_now; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l] + 1, 0);
                                wprintw(Line, "     ");
                            }
                            for (int l = line_now; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l], 0);
                                wprintw(Line, "%5d", l + 1);
                            }
                            y_max.erase(y_max.begin() + line_to_y_last[line_now] + 1);
                            wrefresh(Line);
                        } else {
                            for (int l = y_now; l <= line_to_y_last[line_now]; l++) {
                                wmove(File, l, 0);
                                wclrtoeol(File);
                            }
                            line_max[line_now - 1] += line_max[line_now] + 1;
                            int len = ch_current.size();
                            int line_num_now = len / (COLS - 6);
                            for (int x = 0; x < line_num_now; x++) {
                                char a[COLS];
                                memset(a, 0, COLS);
                                for (int i = x * (COLS - 6); i < (x + 1) * (COLS - 6); i++) {
                                    a[i - x * (COLS - 6)] = ch_current[i];
                                }
                                wmove(File, y_now + x, 0);
                                wprintw(File, "%s", a);
                                y_max[y_now + x] = (COLS - 6);
                            }
                            if (len % (COLS - 6) != 0) {
                                char a[len + 5];
                                memset(a, 0, len + 5);
                                for (int i = 0; i < len % (COLS - 6); i++) {
                                    a[i] = ch_current[i + (len / (COLS -6)) * (COLS - 6)];
                                }
                                y_max[y_now + line_num_now] = (len % (COLS - 6));
                                wmove(File, y_now + line_num_now, 0);
                                wprintw(File, "%s", a);
                            }
                            line_now--;
                            line_max.erase(line_max.begin() + line_now + 1);
                            line_to_y_first.erase(line_to_y_first.begin() + line_now + 1);
                            line_to_y_last.erase(line_to_y_last.begin() + line_now);
                            for (int l = line_now; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l] + 1, 0);
                                wprintw(Line, "     ");
                            }
                            for (int l = line_now; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l], 0);
                                wprintw(Line, "%5d", l + 1);
                            }
                            wrefresh(Line);
                        }
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                } //delete
                case '\t': {
                    for (int i = 0; i <= 3; i++) {
                        do_some = true;
                        if (y_max[line_to_y_last[line_now]] != COLS - 6) {
                            for (int l = line_to_y_last[line_now] - 1; l >= y_now; l--) {
                                int ch_line = mvwinch(File, l, COLS - 7);
                                mvwinsch(File, l + 1, 0, ch_line);
                            }
                            mvwinsch(File, y_now, x_now, ' ');
                            y_max[line_to_y_last[line_now]]++;
                            line_max[line_now]++;
                        } else {
                            int y_current = y_max.size() - 1;
                            while (y_current > line_to_y_last[line_now]) {
                                for (int x_current = 0; x_current < y_max[y_current]; x_current++) {
                                    int ch_line = mvwinch(File, y_current, x_current);
                                    mvwprintw(File, y_current + 1, x_current, "%c", ch_line);
                                }
                                wmove(File, y_current, 0);
                                wclrtoeol(File);
                                y_current--;
                            }
                            y_max.push_back(y_max[y_max.size() - 1]);
                            for (int l = y_max.size() - 2; l > line_to_y_last[line_now]; l--) {
                                y_max[l] = y_max[l - 1];
                            }
                            for (int l = line_to_y_last.size() - 1; l >= line_now; l--) {
                                line_to_y_last[l]++;
                            }
                            for (int l = line_to_y_first.size() - 1; l > line_now; l--) {
                                line_to_y_first[l]++;
                            }
                            for (int l = line_now + 1; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l] - 1, 0);
                                wprintw(Line, "     ");
                            }
                            for (int l = line_now; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l], 0);
                                wprintw(Line, "%5d", l + 1);
                            }
                            for (int l = line_to_y_last[line_now] - 1; l >= y_now; l--) {
                                int ch_line = mvwinch(File, l, COLS - 7);
                                mvwinsch(File, l + 1, 0, ch_line);
                            }
                            mvwinsch(File, y_now, x_now, ' ');
                            y_max[line_to_y_last[line_now]] = 1;
                            line_max[line_now]++;
                            wrefresh(Line);
                        }
                        if (x_now == COLS - 7) {
                            x_now = 0;
                            y_now++;
                        } else {
                            x_now++;
                        }
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                }//tab
                default: {
                    do_some = true;
                    if (y_max[line_to_y_last[line_now]] != COLS - 6) {
                        for (int l = line_to_y_last[line_now] - 1; l >= y_now; l--) {
                            int ch_line = mvwinch(File, l, COLS - 7);
                            mvwinsch(File, l + 1, 0, ch_line);
                        }
                        mvwinsch(File, y_now, x_now, ch);
                        y_max[line_to_y_last[line_now]]++;
                        line_max[line_now]++;
                    } else {
                        int y_current = y_max.size() - 1;
                        while (y_current > line_to_y_last[line_now]) {
                            for (int x_current = 0; x_current < y_max[y_current]; x_current++) {
                                int ch_line = mvwinch(File, y_current, x_current);
                                mvwprintw(File, y_current + 1, x_current, "%c", ch_line);
                            }
                            wmove(File, y_current, 0);
                            wclrtoeol(File);
                            y_current--;
                        }
                        y_max.push_back(y_max[y_max.size() - 1]);
                        for (int l = y_max.size() - 2; l > line_to_y_last[line_now]; l--) {
                            y_max[l] = y_max[l - 1];
                        }
                        for (int l = line_to_y_last.size() - 1; l >= line_now; l--) {
                            line_to_y_last[l]++;
                        }
                        for (int l = line_to_y_first.size() - 1; l > line_now; l--) {
                            line_to_y_first[l]++;
                        }
                        for (int l = line_now + 1; l < line_max.size(); l++) {
                            wmove(Line, line_to_y_first[l] - 1, 0);
                            wprintw(Line, "     ");
                        }
                        for (int l = line_now; l < line_max.size(); l++) {
                            wmove(Line, line_to_y_first[l], 0);
                            wprintw(Line, "%5d", l + 1);
                        }
                        for (int l = line_to_y_last[line_now] - 1; l >= y_now; l--) {
                            int ch_line = mvwinch(File, l, COLS - 7);
                            mvwinsch(File, l + 1, 0, ch_line);
                        }
                        mvwinsch(File, y_now, x_now, ch);
                        y_max[line_to_y_last[line_now]] = 1;
                        line_max[line_now]++;
                        wrefresh(Line);
                    }
                    if (x_now == COLS - 7) {
                        x_now = 0;
                        y_now++;
                    } else {
                        x_now++;
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                } //insert
            }
        }
        //if the mod is insert , we can load in words
        else if (mod_now == Command_mod) {
            y_up_max = 0;
            x_up_max = 0;
            if (ch == KEY_BACKSPACE) {
                if(x_command>1) {
                    command_str = command_history.size() - 1;
                    x_command--;
                    command_line.erase(command_line.begin() + x_command - 1);
                    mvwdelch(Command, 1, x_command);
                }
            }
            //delete
            else if(ch == KEY_UP) {
                if(command_str >= 1) {
                    command_str --;
                    command_line = command_history[command_str];
                    wmove(Command,1,1);
                    wclrtoeol(Command);
                    wmove(Command, 1,0);
                    wprintw(Command,":");
                    for(ll i = 0 ; i < command_line.size();i++) {
                        wmove(Command, 1, i+ 1);
                        wprintw(Command, "%c", command_line[i]);
                    }
                    x_command = command_line.size();
                    if(command_line[command_line.size() - 1] == '\n') {
                        command_line.pop_back();
                    }
                }
            }
            else if(ch == KEY_DOWN) {
                if(command_str < command_history.size() - 1) {
                    command_str ++;
                    command_line = command_history[command_str];
                    wmove(Command,1,1);
                    wclrtoeol(Command);
                    wmove(Command, 1,0);
                    wprintw(Command,":");
                    for(ll i = 0 ; i < command_line.size();i++) {
                        wmove(Command, 1, i + 1);
                        wprintw(Command, "%c", command_line[i]);
                    }
                    x_command = command_line.size();
                    if(command_str == command_history.size() - 1) {
                        x_command++;
                    }
                    if(command_line[command_line.size() - 1] == '\n') {
                        command_line.pop_back();
                    }
                }
            }
            //command_history
            else if(ch == KEY_LEFT) {
                if(x_command > 1) {
                    x_command --;
                }
            }
            else if(ch == KEY_RIGHT) {
                if(x_command <= command_line.size()) {
                    x_command++;
                }
            }
            //move the mouse
            else if(ch == '\t') {
                command_str = command_history.size() - 1;
                command_history[command_history.size() - 1] = command_line;
                command_line.insert(command_line.begin() + x_command - 1,' ');
                command_line.insert(command_line.begin() + x_command - 1,' ');
                command_line.insert(command_line.begin() + x_command - 1,' ');
                command_line.insert(command_line.begin() + x_command - 1,' ');
                x_command+=4;
                command_line+="    ";
            }
            //tab
            else {
                command_str = command_history.size() - 1;
                wmove(Command,2,0);
                wclrtoeol(Command);
                wmove(Command, 1, x_command);
                wprintw(Command, "%c", ch);
                wrefresh(Command);
                command_line.insert(command_line.begin() + x_command - 1,char(ch));
                for(ll i = 0 ; i < command_line.size();i++) {
                    mvwprintw(File,2,i,"%c",command_line[i]);
                }
                x_command++;
                command_history[command_str] = command_line;
                bool wrong_command = false;
                if (ch == 10) {
                    if (command_line == "w\n") {
                        FILE *file_out = freopen(argv[file], "w", stdout);
                        for (ll i = 0; i < line_max.size(); i++) {
                            for (ll x = line_to_y_first[i]; x <= line_to_y_last[i]; x++) {
                                char buffer[y_max[x] + 1];
                                mvwinnstr(File, x, 0, buffer, y_max[x]);
                                fputs(buffer, file_out);
                            }
                            if (isCRLF) {
                                fputs("\r\n", file_out);
                            } else {
                                fputs("\n", file_out);
                            }
                        }
                        fclose(file_out);
                        fflush(file_out);
                        freopen("/dev/tty", "w", stdout);
                        do_some = false;
                        if(new_file) {
                            new_file = false;
                        }
                    }
                    // use 'w' to save the text
                    else if (command_line == "wq\n") {
                        FILE *file_out = freopen(argv[file], "w", stdout);
                        for (ll i = 0; i < line_max.size(); i++) {
                            for (ll x = line_to_y_first[i]; x <= line_to_y_last[i]; x++) {
                                char buffer[y_max[x] + 1];
                                mvwinnstr(File, x, 0, buffer, y_max[x]);
                                fputs(buffer, file_out);
                            }
                            if (isCRLF) {
                                fputs("\r\n", file_out);
                            } else {
                                fputs("\n", file_out);
                            }
                        }
                        fclose(file_out);
                        fflush(file_out);
                        freopen("/dev/tty", "w", stdout);
                        wmove(File, y_now, x_now);
                        wrefresh(File);
                        endwin();
                        return 0;
                    }
                    //use 'wq' to save and then quit
                    else if (command_line == "q\n") {
                        if (do_some) {
                            mvwprintw(Command, 2, 0,"You have do some operations and not save yet , do you want to quit now ?");
                            wmove(Command, 1, 0);
                            wclrtoeol(Command);
                            wrefresh(Command);
                            int ch1 = getch();
                            ll pos = 0;
                            string quit;
                            while (ch1 != '\n') {
                                quit += char(ch1);
                                wmove(Command, 1, pos);
                                wprintw(Command, "%c", ch1);
                                wrefresh(Command);
                                pos++;
                                ch1 = getch();
                            }
                            if (quit.size() > 0 && (quit[0] == 'y' || quit[0] == 'Y')) {
                                endwin();
                                return 0;
                            }
                            wmove(File, y_now, x_now);
                            wrefresh(File);
                        } else {
                            if(new_file) {
                               remove(argv[file]);
                            }
                            endwin();
                            return 0;
                        }
                    }
                    //use 'q' to quit
                    else if (command_line == "q!\n") {
                        if(new_file) {
                            remove(argv[file]);
                        }
                        endwin();
                        return 0;
                    }
                    //use 'q!' to quit
                    else if (command_line[0] == 'j' && command_line[1] == 'm' && command_line[2] == 'p') {
                        command_line.erase(command_line.begin(),command_line.begin()+3);
                        if (command_line[3] == ' ') {
                            command_line.erase(command_line.begin());
                        }
                        ll l = std::stoll(command_line);
                        if(l <= line_max.size() && l >= 1 ) {
                            y_now = line_to_y_first[l - 1];
                            x_now = 0;
                        }
                    }
                    else {
                        wrong_command = true;
                    }
                    wclear(Command);
                    wmove(Command, 0, 0);
                    wprintw(Command, "This is the command window ");
                    if(wrong_command) {
                        mvwprintw(Command,2,0,"Wrong command!");
                    }
                    wrefresh(Command);
                    wmove(Information, 0, 0);
                    wprintw(Information, "Mod : Normal  ");
                    wmove(File, 0, 0);
                    mod_now = Normal_mod;
                    wrefresh(Information);
                    wmove(File, y_now, x_now);
                    wrefresh(File);
                    //after the command had done , we need to quit command mod and return to normal mod
                }
            }
            //if enter 'enter' , do the command that load in
        }
        //if the mod is command , we can cin words in the command window --done!
        else if (mod_now == Normal_mod) {
            fast_last = fast_this;
            fast_this = ch;
            switch (ch) {
                case KEY_UP: {
                    if (line_now > 0) {
                        ll l1 = y_now - line_to_y_first[line_now];
                        if (l1 >= y_up_max) {
                            y_up_max = l1;
                            if (x_now > x_up_max) {
                                x_up_max = x_now;
                            }
                        }
                        ll l2 = line_to_y_last[line_now - 1] - line_to_y_first[line_now - 1];
                        if (y_up_max < l2 || (y_up_max == l2 && x_up_max < y_max[line_to_y_last[line_now - 1]])) {
                            line_now--;
                            y_now = line_to_y_first[line_now] + y_up_max;
                            x_now = x_up_max;
                        } else {
                            line_now--;
                            y_now = line_to_y_last[line_now];
                            x_now = y_max[y_now];
                        }
                    }
                    break;
                }
                case KEY_DOWN: {
                    if (line_now < line_max.size() - 1) {
                        ll l1 = y_now - line_to_y_first[line_now];
                        if (l1 >= y_up_max) {
                            y_up_max = l1;
                            if (x_now > x_up_max) {
                                x_up_max = x_now;
                            }
                        }
                        ll l2 = line_to_y_last[line_now + 1] - line_to_y_first[line_now + 1];
                        if (y_up_max < l2 || (y_up_max == l2 && x_up_max < y_max[line_to_y_last[line_now + 1]])) {
                            line_now++;
                            y_now = line_to_y_first[line_now] + y_up_max;
                            x_now = x_up_max;
                        } else {
                            line_now++;
                            y_now = line_to_y_last[line_now];
                            x_now = y_max[y_now];
                        }
                    }
                    break;
                }
                case KEY_LEFT: {
                    if (x_now >= 1) {
                        x_now--;
                    } else if (x_now == 0 && line_to_y_first[line_now] != y_now) {
                        y_now--;
                        x_now = (COLS - 6) - 1;
                    } else if (x_now == 0 && y_now >= 1 && line_to_y_first[line_now] == y_now) {
                        y_now--;
                        line_now--;
                        x_now = y_max[y_now];
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                }
                case KEY_RIGHT: {
                    if (x_now < y_max[y_now] && x_now < (COLS - 6) - 1) {
                        x_now++;
                    } else if (x_now == (COLS - 6) - 1 && y_now < y_max.size() - 1 && y_now != line_to_y_last[
                                   line_now]) {
                        y_now++;
                        x_now = 0;
                    } else if (x_now == y_max[y_now] && y_now < y_max.size() - 1 && y_now == line_to_y_last[line_now]) {
                        y_now++;
                        line_now++;
                        x_now = 0;
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                }
                //move the mouse
                case 105: {
                    if (!only_read) {
                        wmove(Information, 0, 0);
                        wprintw(Information, "Mod : Insert  ");
                        wmove(File, 0, 0);
                        wrefresh(Information);
                        mod_now = Insert_mod;
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                } //if enter 'i' in normal mod , change the mod to insert --done!
                case 58: {
                    wclear(Command);
                    wprintw(Command, "This is the command window ");
                    wmove(Information, 0, 0);
                    wprintw(Information, "Mod : Command ");
                    wrefresh(Information);
                    wmove(Command, 1, 0);
                    wprintw(Command, ":");
                    wrefresh(Command);
                    wmove(Command, 1, 1);
                    x_command = 1;
                    mod_now = Command_mod;
                    command_line = "";
                    y_up_max = 0;
                    x_up_max = 0;
                    command_str = command_history.size() -1;
                    command_history.push_back("");
                    break;
                } //if enter ':' int normal mod , change the mod to command --done!
                case 100: {
                    do_some = true;
                    if (fast_last == 100) {
                        if (line_max.size() >= 2) {
                            ll up = line_to_y_last[line_now] - line_to_y_first[line_now] + 1;
                            ll y_current = line_to_y_last[line_now] + 1;
                            for (ll l = line_to_y_first[line_now]; l <= line_to_y_last[line_now]; l++) {
                                wmove(File, l, 0);
                                wclrtoeol(File);
                            }
                            while (y_current < y_max.size()) {
                                for (ll x_current = 0; x_current < y_max[y_current]; x_current++) {
                                    ll ch_line = mvwinch(File, y_current, x_current);
                                    mvwprintw(File, y_current - up, x_current, "%c", ch_line);
                                }
                                wmove(File, y_current, 0);
                                wclrtoeol(File);
                                y_current++;
                            }
                            ll times = up;
                            while (times--) {
                                y_max.erase(y_max.begin() + line_to_y_first[line_now]);
                            }
                            line_max.erase(line_max.begin() + line_now);
                            line_to_y_first.erase(line_to_y_first.begin() + line_now);
                            line_to_y_last.erase(line_to_y_last.begin() + line_now);
                            for (ll l = line_now; l < line_max.size(); l++) {
                                line_to_y_first[l] -= up;
                                line_to_y_last[l] -= up;
                            }
                            for (ll l = 0; l < win_line; l++) {
                                wmove(Line, l, 0);
                                wprintw(Line, "     ");
                            }
                            for (ll l = 0; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l], 0);
                                wprintw(Line, "%5d", l + 1);
                            }
                            if (line_now < line_max.size() - 1) {
                                y_now = line_to_y_last[line_now];
                                x_now = y_max[y_now];
                            } else if (line_max.size() >= 2) {
                                y_now = line_to_y_last[line_now - 1];
                                x_now = y_max[y_now];
                                line_now--;
                            } else {
                                y_now = 0;
                                x_now = 0;
                                line_now = 0;
                            }
                        } else if (line_max.size() == 1) {
                            wclear(File);
                            line_to_y_first[line_now] = 0;
                            line_to_y_last[line_now] = 0;
                            y_now = 0;
                            x_now = 0;
                            line_now = 0;
                            for (ll l = 0; l < 20; l++) {
                                wmove(Line, l, 0);
                                wprintw(Line, "     ");
                            }
                            for (ll l = 0; l < line_max.size(); l++) {
                                wmove(Line, line_to_y_first[l], 0);
                                wprintw(Line, "%5d", l + 1);
                            }
                        }
                        wrefresh(Line);
                        fast_last = 0;
                        fast_this = 0;
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                } //if enter 'dd' ,delete the whole line --done!
                case 48: {
                    x_now = 0;
                    y_now = line_to_y_first[line_now];
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                } //if enter '0' ,move the arrow to the first of this line --done!
                case 36: {
                    y_now = line_to_y_last[line_now];
                    x_now = y_max[y_now];
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                } //if enter '$' ,move the arrow to the first of this line --done!
                case 119: {
                    if (x_now < y_max[y_now] || y_now < line_to_y_last[line_now]) {
                        bool if_find = false;
                        ll x_current, y_current, x_current_next, y_current_next;
                        if (x_now < y_max[y_now] - 1) {
                            x_current = x_now + 1;
                            y_current = y_now;
                        } else if (y_now < line_to_y_last[line_now]) {
                            x_current = 0;
                            y_current = y_now + 1;
                        } else {
                            x_now = 0;
                            y_now++;
                            line_now++;
                            break;
                        }
                        while (x_current < y_max[line_to_y_last[line_now]] || y_current < line_to_y_last[line_now]) {
                            if (x_current < y_max[y_current] - 1) {
                                x_current_next = x_current + 1;
                                y_current_next = y_current;
                            } else {
                                x_current_next = 0;
                                y_current_next = y_current + 1;
                            }
                            chtype char_pos = mvwinch(File, y_current, x_current);
                            chtype char_pos_next = mvwinch(File, y_current_next, x_current_next);
                            if ((char_pos & A_CHARTEXT) == ' ' && (char_pos_next & A_CHARTEXT) != ' ') {
                                if (y_current > line_to_y_last[line_now] && line_now < line_max.size() - 1) {
                                    x_now = 0;
                                    y_now = y_current_next;
                                } else if (y_current > line_to_y_last[line_now] && line_now == line_max.size() - 1) {
                                } else {
                                    x_now = x_current_next;
                                    y_now = y_current_next;
                                }
                                if (y_now > line_to_y_last[line_now]) {
                                    line_now++;
                                    x_now = 0;
                                }
                                if_find = true;
                                break;
                            }
                            x_current = x_current_next;
                            y_current = y_current_next;
                        }
                        if (!if_find && line_now < line_max.size() - 1) {
                            line_now++;
                            y_now = line_to_y_first[line_now];
                            x_now = 0;
                        }
                    } else {
                        if (line_now < line_max.size() - 1) {
                            line_now++;
                            y_now = line_to_y_first[line_now];
                            x_now = 0;
                        }
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                }//if enter 'w',move to the next word
                case 98: {
                    if (y_now == line_to_y_first[line_now] && x_now == 0 && line_now > 0) {
                        line_now--;
                        y_now = line_to_y_last[line_now];
                        x_now = y_max[y_now];
                        break;
                    }
                    if (y_max[y_now] == 1 && y_now == line_to_y_first[line_now] && x_now > 0) {
                        x_now--;
                        break;
                    }
                    if (x_now >= 2 || y_now > line_to_y_first[line_now]) {
                        bool if_find = false;
                        ll x_current, y_current, x_current_next, y_current_next;
                        if (x_now >= 2) {
                            x_current = x_now - 2;
                            y_current = y_now;
                        } else if (x_now == 1 && y_now > line_to_y_first[line_now]) {
                            y_current = y_now - 1;
                            x_current = y_max[y_now] - 1;
                        } else if (x_now == 0 && y_now > line_to_y_first[line_now]) {
                            y_current = y_now - 1;
                            x_current = y_max[y_now] - 2;
                        } else if (line_now >= 1) {
                            y_now--;
                            line_now--;
                            x_now = y_max[y_now];
                            break;
                        }
                        while (x_current > 0 || y_current > line_to_y_first[line_now]) {
                            if (x_current < y_max[y_current] - 1) {
                                x_current_next = x_current + 1;
                                y_current_next = y_current;
                            } else {
                                x_current_next = 0;
                                y_current_next = y_current + 1;
                            }
                            chtype char_pos = mvwinch(File, y_current, x_current);
                            chtype char_pos_next = mvwinch(File, y_current_next, x_current_next);
                            if ((char_pos & A_CHARTEXT) == ' ' && (char_pos_next & A_CHARTEXT) != ' ') {
                                x_now = x_current_next;
                                y_now = y_current_next;
                                if_find = true;
                                break;
                            }
                            if (x_current == 0 && y_current > line_to_y_first[line_now]) {
                                y_current--;
                                x_current = y_max[y_current] - 1;
                            } else if (x_current > 0) {
                                x_current--;
                            }
                        }
                        if (!if_find) {
                            y_now = line_to_y_first[line_now];
                            x_now = 0;
                        }
                    }
                    y_up_max = 0;
                    x_up_max = 0;
                    break;
                }//if enter 'b',move to the last word
            }
            // not only the mod change , but some special input that can try some special operation
        }
        wmove(Information, 1, 0);
        wprintw(Information, "Row :%3d , Col :%3d", line_now + 1,
                (y_now - line_to_y_first[line_now]) * (COLS - 6) + x_now + 1);
        wrefresh(Information);
        if (y_now > start_line + win_line - 1) {
            start_line = y_now - win_line + 1;
            y_now_visible = win_line - 1;
        } else if (y_now < start_line) {
            start_line = y_now;
            y_now_visible = 0;
        } else {
            y_now_visible += y_now - y_last;
        }
        y_last = y_now;
        copyFilesToVisible(File, Visible_File, start_line);
        copyLinesToVisible(Line, Visible_Line, start_line);
        //move the window , if need
        wrefresh(Visible_Line);
        if (mod_now != Command_mod) {
            wmove(Visible_File, y_now_visible, x_now);
            wrefresh(Visible_File);
        } else {
            wmove(Command, 1, x_command);
            wrefresh(Command);
        }
    }

    endwin(); // End curses mode
    return 0;
}

void copyFilesToVisible(WINDOW *File, WINDOW *Visible_File, int start_line) {
    int end_line = start_line +win_line;
    for (int i = start_line; i < end_line && i < 20000; i++) {
        char line[COLS - 6];
        mvwinnstr(File, i, 0, line, COLS - 6);
        mvwprintw(Visible_File, i - start_line, 0, line);
    }
    wrefresh(Visible_File);
}

void copyLinesToVisible(WINDOW *Line, WINDOW *Visible_Line, int start_line) {
    int end_line = start_line + win_line;
    for (int i = start_line; i < end_line && i < 20000; i++) {
        char line_content[5];
        mvwinnstr(Line, i, 0, line_content, 5);
        mvwprintw(Visible_Line, i - start_line, 0, line_content);
    }
    wrefresh(Visible_Line);
}