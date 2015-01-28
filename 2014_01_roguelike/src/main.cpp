/*
Projekt do skompilowania wymaga biblioteki PDCurses:

http://pdcurses.sourceforge.net/

W innych systemach (Linux/MacOS X) mo�na tak�e u�y� nCurses.

Sterowanie: strza�ki albo numpad (klawisz Num-lock MUSI BY� WY��CZONY!)
*/

#include <string>
#include <vector>
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

const int levelX = 0; // Pocz�tek planszy
const int levelY = 2; // (przesuni�cie na potrzeby wiadomo�ci)
const int levelW = 80; // Wymiary planszy. Standardowo konsola
const int levelH = 22; // ma 80 kolumn i 25 wierszy

const int monsterCount = 20; // liczba potwor�w na mapie


bool level[levelW][levelH]; // Zazwyczaj pola planszy s� obiektem jakiej� struktury
                    // ale poniewa� potrzebujemy tylko jednej informacji (�ciana/wolne)
                    // mo�emy si� "wymiga�" z tablic� booli

int playerX = 1;   // Pocz�tkowe koordynaty gracza to lewy g�rny r�g planszy
int playerY = 1;

int playerHp = 100; // Statystyki gracza
int playerHpMax = 100;
int playerKills = 0;
int playerAttack = 15;
std::string mesg; // Komunikaty

void generateLevel(); // Deklaracje funkcji (definicje poni�ej main() )
void draw();
void update();
void generateLineH(int x1, int x2, int y);
void generateLineW(int y1, int y2, int x);
void generateRoom(int x1, int y1, int x2, int y2);
void clearMsg();
void addMsg(const char * str);
void generateMonsters();

class Monster // Pierwsze li�ni�cie obiektowo�ci, klasa dla potworka!
{
private:
    int x, y; // koordynaty
    int hp; // pozosta�e punkty �ycia
    int attack; // zdolno�� ataku
    char ch; // wy�wietlany znak
    int color; // kolor tego znaku
public:
    int GetX() { return x; } // Standardowe akcesory
    int GetY() { return y; }
    // Ustawianie pocz�tkowych informacji o potworku:
    void Set(int x, int y, int hp, int attack, char ch, int color)
    {
        this->x = x;
        this->y = y;
        this->hp = hp;
        this->attack = attack;
        this->ch = ch;
        this->color = color;
    }
    // Rysowanie potworka:
    void Draw()
    {
        attron(COLOR_PAIR(color));
        if(hp <= 0)
            mvprintw(y + levelY, x + levelX, "%%");
        else
            mvprintw(y + levelY, x + levelX, "%c", ch);
        attroff(COLOR_PAIR(color));
    }
    // Zaatakowanie potworka przez gracza
    void Attack(int dmg)
    {
        if(hp <= 0) return;
        static char buffer[256];
        sprintf(buffer, "You hit a monster for %d dmg!", dmg);
        addMsg(buffer);

        hp -= dmg;
        if(hp <= 0)
        {
            ++playerKills; // Zabili�my potworka!
            addMsg("The monster is dead!");
        }
    }
};

std::vector<Monster> monsters;

int main() // G��wna funkcja programu
{
    initscr(); // W��cza PDCurses
    start_color(); // W��cza tryb kolorowania
    raw(); // Wy��cza buforowanie
    noecho(); // Wy��cza wy�wietlanie wpisywanego znaku na ekran
    keypad(stdscr, TRUE); // W��cza klawiatur� numeryczn�
    srand(time(NULL)); // Na potrzeby rand()'a

    generateLevel(); // Tworzy poziom

    init_pair(1, COLOR_GREEN, COLOR_BLACK); // Tworzy pary kolor�w
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);

    for(;;) // P�tla gry
    {
        draw();
        update();
    }


    getch();
    endwin();
    return 0;
}

void generateLevel() // Generujemy plansz�
{
    for(int i = 0; i < levelW; i++)
    {
        for(int j = 0; j < levelH; j++)
        {
            level[i][j] = true; // Wszystkie tile nie zawieraj� �ciany
        }
    }

    // Tworzymy prostok�t ze �cian dooko�a planszy (�eby plansza by�a zamkni�ta)
    generateLineH(0, levelW - 1, 0);
    generateLineH(0, levelW - 1, levelH - 1);
    generateLineW(0, levelH - 1, 0);
    generateLineW(0, levelH - 1, levelW - 1);

    // rekurencyjnie tworzymy poziom z�o�ony z pokoi
    generateRoom(0, 0, levelW - 1, levelH - 1);

    generateMonsters(); // Ustawiamy potworki na mapie
}


void draw() // P�tla rysowania
{
    clear(); // Czy�cimy ekran

    attron(COLOR_PAIR(2)); // Uruchamiamy kolor 2 (zdefiniowany w main() )
    for(int i = 0; i < levelW; i++)
    {
        for(int j = 0; j < levelH; j++)
        {
            if(level[i][j])
                mvprintw(j + levelY, i + levelX, "."); // pusta przestrze�. UWAGA mvprintw bierze najpierw y, potem x!
            else
                mvprintw(j + levelY, i + levelX, "#"); // �ciana. UWAGA mvprintw bierze najpierw y, potem x!
        }
    }
    attroff(COLOR_PAIR(2)); // Wy��czamy kolor 2

    for(int i = 0; i < monsters.size(); i++)
    {
        monsters[i].Draw();
    }

    attron(COLOR_PAIR(1));
    mvprintw(playerY + levelY, playerX + levelX, "@"); // Rysujemy posta� gracza kolorem 1. UWAGA mvprintw bierze najpierw y, potem x!
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(1)); // Drukujemy statystyki na ekran
    mvprintw(24, 1, "HP: %d/%d (KILLS: %d)", playerHp, playerHpMax, playerKills);
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(3)); // Drukujemy komunikaty na ekran
    mvprintw(0, 0, "%s", mesg.substr(0, 80).c_str());
    if(mesg.length() >= 80)
        mvprintw(1, 0, "%s", mesg.substr(80, 160).c_str());
    attroff(COLOR_PAIR(3));

    refresh(); // od�wie�amy ekran, to ostatni krok w p�tli renderuj�cej
}

void update()
{
    clearMsg(); // na pocz�tku ka�dego update'a czy�cimy bufor wiadomo�ci (w ten spos�b
                // wiadomo�ci s� przechowywane tylko do nast�pnego wci�ni�tego klawisza)
    int ch = getch(); // Pobieramy znak z klawiatury

    int xprev = playerX; // Zapami�tujemy poprzedni� pozycj� gracza, bo mo�emy
    int yprev = playerY; // wpa�� na potworka!
    // W zale�no�ci od znaku sprawdzamy czy pole jest puste, a je�eli tak
    // to przesuwamy tam gracza
    switch(ch)
    {
        // A1/A3/C1/C3 to klawisze z numpada
        case KEY_A1: if(level[playerX-1][playerY-1]) { --playerX; --playerY; } break;
        case KEY_A3: if(level[playerX+1][playerY-1]) { ++playerX; --playerY; } break;
        case KEY_C1: if(level[playerX-1][playerY+1]) { --playerX; ++playerY; } break;
        case KEY_C3: if(level[playerX+1][playerY+1]) { ++playerX; ++playerY; } break;
        // Ta sztuczka pozwala nam obs�u�y� klawisze numpada i strza�ki w tym samym miejscu
        case KEY_LEFT:  case KEY_B1:    if(level[playerX-1][playerY]) --playerX; break;
        case KEY_RIGHT: case KEY_B3:    if(level[playerX+1][playerY]) ++playerX; break;
        case KEY_UP:    case KEY_A2:    if(level[playerX][playerY-1]) --playerY; break;
        case KEY_DOWN:  case KEY_C2:    if(level[playerX][playerY+1]) ++playerY; break;
    }

    for(int i = 0; i < monsters.size(); i++) // Sprawdzamy, czy wpadli�my na potworka
    {
        if(monsters[i].GetX() == playerX && monsters[i].GetY() == playerY)
        {
            monsters[i].Attack(playerAttack); // Tak! To go atakujemy
            playerX = xprev; // I cofamy zmian� pozycji (nie mo�na "wej��" na potworka)
            playerY = yprev;
        }
    }
}

void generateLineH(int x1, int x2, int y) // Generuje pionow� lini� �ciany na mapie
{
    for(int i = x1; i <= x2; i++)
    {
        level[i][y] = false;
    }
}

void generateLineW(int y1, int y2, int x) // Generuje poziom� lini� �ciany na mapie
{
    for(int j = y1; j <= y2; j++)
    {
        level[x][j] = false;
    }
}

// Implementacja algorytmu BSP:
// http://www.roguebasin.com/index.php?title=Basic_BSP_Dungeon_generation
void generateRoom(int x1, int y1, int x2, int y2) // Funkcja rekurencyjna tworz�ca poziom
{
    if(x2 - x1 < 9) return; // Je�eli pok�j jest mniejszy od 9 zako�cz rekurencj�
    if(y2 - y1 < 9) return; // Bo nie ma sensu dzieli� go bardziej

    if(rand()%2 == 0) // Dzielimy pok�j na dwie cz�ci poziome
    {
        int x = rand()%((x2-x1)/2) + (x2 - x1)/4 + x1;
        generateLineW(y1, y2, x); // Linia oddzielaj�ca dwa mniejsze pokoje

        generateRoom(x1, y1, x, y2); // Rekurencja dla obu mniejszych pokoi
        generateRoom(x, y1, x2, y2);

        int y = rand()%((y2-y1)/2) + (y2 - y1)/4 + y1;
        level[x][y] = true; // Tworzymy "drzwi" mi�dzy dwoma pokojami
    }
    else // Dzielimy pok�j na dwie cz�ci pionowe
    {
        int y = rand()%((y2-y1)/2) + (y2 - y1)/4 + y1;
        generateLineH(x1, x2, y); // Linia oddzielaj�ca dwa mniejsze pokoje

        generateRoom(x1, y1, x2, y); // Rekurencja dla obu mniejszych pokoi
        generateRoom(x1, y, x2, y2);

        int x = rand()%((x2-x1)/2) + (x2 - x1)/4 + x1;
        level[x][y] = true; // Tworzymy "drzwi" mi�dzy dwoma pokojami
    }
}

////////////////////////////////////////////////////////////////////////////////////
// NOWE:

void clearMsg()
{
    mesg = ""; // Czy�cimy bufor komunikat�w
}

void addMsg(const char * str)
{
    mesg += str; // Dodajemy nowy komunikat do bufora
    mesg += " ";
}

void generateMonsters() // Losujemy potworki
{
    for(int i = 0; i < monsterCount; i++)
    {
        Monster m;

        int x = 0;
        int y = 0;
        do // dop�ki nie znajdziesz wolnego miejsca na tego potworka
        {
            x = (rand()%levelW); // losuj jego koordynaty
            y = (rand()%levelH);
        } while(!level[x][y]);

        m.Set(x, y, 15, 3, 'Z', 3); // znale�li�my! To go stw�rzmy

        monsters.push_back(m); // i dodajmy do listy potwork�w
    }
}
