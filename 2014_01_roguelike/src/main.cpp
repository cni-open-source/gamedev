#include <string>
#include <vector>
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


bool plansza[80][25];

int x = 1;
int y = 3;
int hp = 100;
int hp_max = 100;
int kills = 0;
int attack = 15;
std::string mesg;

void clearMsg()
{
    mesg = "";
}

void addMsg(const char * str)
{
    mesg += str;
    mesg += " ";
}

class Monster
{
private:
    int x, y;
    int hp;
    int attack;
    char c;
public:
    int GetX() { return x; }
    int GetY() { return y; }
    void Set(int x, int y, int hp, int attack, char c)
    {
        this->x = x;
        this->y = y;
        this->hp = hp;
        this->attack = attack;
        this->c = c;
    }
    void Draw()
    {
        if(hp <= 0)
            mvprintw(y, x, "%%");
        else
            mvprintw(y, x, "%c", c);
    }
    void Attack(int dmg)
    {
        if(hp <= 0) return;
        char buffer[256];
        sprintf(buffer, "You hit a monster for %d dmg!", dmg);
        addMsg(buffer);

        hp -= dmg;
        if(hp <= 0)
            addMsg("The monster is dead!");
    }
};

std::vector<Monster> monsters;

void generateLineH(int x1, int x2, int y)
{
    for(int i = x1; i <= x2; i++)
    {
        plansza[i][y] = false;
    }
}

void generateLineW(int y1, int y2, int x)
{
    for(int j = y1; j <= y2; j++)
    {
        plansza[x][j] = false;
    }
}

void generateRoom(int x1, int y1, int x2, int y2)
{
    if(x2 - x1 < 9) return;
    if(y2 - y1 < 9) return;

    if(rand()%2 == 0)
    {
        int x = rand()%((x2-x1)/2) + (x2 - x1)/4 + x1;
        generateLineW(y1, y2, x);

        generateRoom(x1, y1, x, y2);
        generateRoom(x, y1, x2, y2);

        int y = rand()%((y2-y1)/2) + (y2 - y1)/4 + y1;
        plansza[x][y] = true;
    }
    else
    {
        int y = rand()%((y2-y1)/2) + (y2 - y1)/4 + y1;
        generateLineH(x1, x2, y);

        generateRoom(x1, y1, x2, y);
        generateRoom(x1, y, x2, y2);

        int x = rand()%((x2-x1)/2) + (x2 - x1)/4 + x1;
        plansza[x][y] = true;
    }
}

void generateMonsters()
{
    for(int i = 0; i < 20; i++)
    {
        Monster m;

        int x = 0;
        int y = 0;
        do
        {
            x = rand()%80;
            y = (rand()%22)+2;
        } while(!plansza[x][y]);

        m.Set(x, y, 15, 3, 'Z');

        monsters.push_back(m);
    }
}

void generateLevel()
{
    for(int i = 0; i < 80; i++)
    {
        for(int j = 0; j < 25; j++)
        {
            plansza[i][j] = true;
        }
    }
    generateLineH(0, 79, 2);
    generateLineH(0, 79, 23);
    generateLineW(2, 23, 0);
    generateLineW(2, 23, 79);

    generateRoom(0, 2, 79, 23);

    generateMonsters();
}


void draw()
{
    clear();

    attron(COLOR_PAIR(2));
    for(int i = 0; i < 80; i++)
    {
        for(int j = 2; j < 24; j++)
        {
            if(plansza[i][j])
                mvprintw(j, i, ".");
            else
                mvprintw(j, i, "#");
        }
    }
    attroff(COLOR_PAIR(2));

    for(int i = 0; i < monsters.size(); i++)
    {
        monsters[i].Draw();
    }

    attron(COLOR_PAIR(1));
    mvprintw(y, x, "@");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(1));
    mvprintw(24, 1, "HP: %d/%d (KILLS: %d)", hp, hp_max, kills);
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(3));
    mvprintw(0, 0, "%s", mesg.substr(0, 80).c_str());
    if(mesg.length() >= 80)
        mvprintw(1, 0, "%s", mesg.substr(80, 160).c_str());
    attroff(COLOR_PAIR(3));

    refresh();
}

void update()
{
    clearMsg();
    int ch = getch();
    int xprev = x;
    int yprev = y;
    switch(ch)
    {
        case KEY_A1: if(plansza[x-1][y-1]) { --x; --y; } break;
        case KEY_A3: if(plansza[x+1][y-1]) { ++x; --y; } break;
        case KEY_C1: if(plansza[x-1][y+1]) { --x; ++y; } break;
        case KEY_C3: if(plansza[x+1][y+1]) { ++x; ++y; } break;
        case KEY_LEFT: case KEY_B1: if(plansza[x-1][y]) --x; break;
        case KEY_RIGHT: case KEY_B3: if(plansza[x+1][y]) ++x; break;
        case KEY_UP: case KEY_A2: if(plansza[x][y-1]) --y; break;
        case KEY_DOWN: case KEY_C2: if(plansza[x][y+1]) ++y; break;
    }
    for(int i = 0; i < monsters.size(); i++)
    {
        if(monsters[i].GetX() == x && monsters[i].GetY() == y)
        {
            monsters[i].Attack(attack);
            x = xprev;
            y = yprev;
        }
    }
}

int main()
{
    initscr();
    start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    srand(time(NULL));

    generateLevel();

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);

    for(;;)
    {
        draw();
        update();
    }


    getch();
    endwin();
    return 0;
}
