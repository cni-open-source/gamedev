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

const int levelW = 80; // Wymiary planszy. Standardowo konsola
const int levelH = 25; // ma 80 kolumn i 25 wierszy


bool level[levelW][levelH]; // Zazwyczaj pola planszy s� obiektem jakiej� struktury
                    // ale poniewa� potrzebujemy tylko jednej informacji (�ciana/wolne)
                    // mo�emy si� "wymiga�" z tablic� booli

int playerX = levelW / 2;   // Pocz�tkowe koordynaty gracza to po�owa planszy
int playerY = levelH / 2;

void generateLevel(); // Deklaracje funkcji (definicje poni�ej main() )
void draw();
void update();

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
            level[i][j] = rand() % 5; // ... w taki spos�b, �e mamy 1/5 szansy na �cian� (false)
                                      // i 4/5 szansy na pust� przestrze�
        }
    }
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
                mvprintw(j, i, "."); // pusta przestrze�. UWAGA mvprintw bierze najpierw y, potem x!
            else
                mvprintw(j, i, "#"); // �ciana. UWAGA mvprintw bierze najpierw y, potem x!
        }
    }
    attroff(COLOR_PAIR(2)); // Wy��czamy kolor 2

    attron(COLOR_PAIR(1));
    mvprintw(playerY, playerX, "@"); // Rysujemy posta� gracza kolorem 1. UWAGA mvprintw bierze najpierw y, potem x!
    attroff(COLOR_PAIR(1));

    refresh();
}

void update()
{
    int ch = getch(); // Pobieramy znak z klawiatury

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
}
