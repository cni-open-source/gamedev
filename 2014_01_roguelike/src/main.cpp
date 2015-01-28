/*
Projekt do skompilowania wymaga biblioteki PDCurses:

http://pdcurses.sourceforge.net/

W innych systemach (Linux/MacOS X) mo¿na tak¿e u¿yæ nCurses.

Sterowanie: strza³ki albo numpad (klawisz Num-lock MUSI BYÆ WY£¥CZONY!)
*/

#include <string>
#include <vector>
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

const int levelW = 80; // Wymiary planszy. Standardowo konsola
const int levelH = 25; // ma 80 kolumn i 25 wierszy


bool level[levelW][levelH]; // Zazwyczaj pola planszy s¹ obiektem jakiejœ struktury
                    // ale poniewa¿ potrzebujemy tylko jednej informacji (œciana/wolne)
                    // mo¿emy siê "wymigaæ" z tablic¹ booli

int playerX = levelW / 2;   // Pocz¹tkowe koordynaty gracza to po³owa planszy
int playerY = levelH / 2;

void generateLevel(); // Deklaracje funkcji (definicje poni¿ej main() )
void draw();
void update();

int main() // G³ówna funkcja programu
{
    initscr(); // W³¹cza PDCurses
    start_color(); // W³¹cza tryb kolorowania
    raw(); // Wy³¹cza buforowanie
    noecho(); // Wy³¹cza wyœwietlanie wpisywanego znaku na ekran
    keypad(stdscr, TRUE); // W³¹cza klawiaturê numeryczn¹
    srand(time(NULL)); // Na potrzeby rand()'a

    generateLevel(); // Tworzy poziom

    init_pair(1, COLOR_GREEN, COLOR_BLACK); // Tworzy pary kolorów
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);

    for(;;) // Pêtla gry
    {
        draw();
        update();
    }


    getch();
    endwin();
    return 0;
}

void generateLevel() // Generujemy planszê
{
    for(int i = 0; i < levelW; i++)
    {
        for(int j = 0; j < levelH; j++)
        {
            level[i][j] = rand() % 5; // ... w taki sposób, ¿e mamy 1/5 szansy na œcianê (false)
                                      // i 4/5 szansy na pust¹ przestrzeñ
        }
    }
}


void draw() // Pêtla rysowania
{
    clear(); // Czyœcimy ekran

    attron(COLOR_PAIR(2)); // Uruchamiamy kolor 2 (zdefiniowany w main() )
    for(int i = 0; i < levelW; i++)
    {
        for(int j = 0; j < levelH; j++)
        {
            if(level[i][j])
                mvprintw(j, i, "."); // pusta przestrzeñ. UWAGA mvprintw bierze najpierw y, potem x!
            else
                mvprintw(j, i, "#"); // œciana. UWAGA mvprintw bierze najpierw y, potem x!
        }
    }
    attroff(COLOR_PAIR(2)); // Wy³¹czamy kolor 2

    attron(COLOR_PAIR(1));
    mvprintw(playerY, playerX, "@"); // Rysujemy postaæ gracza kolorem 1. UWAGA mvprintw bierze najpierw y, potem x!
    attroff(COLOR_PAIR(1));

    refresh();
}

void update()
{
    int ch = getch(); // Pobieramy znak z klawiatury

    // W zale¿noœci od znaku sprawdzamy czy pole jest puste, a je¿eli tak
    // to przesuwamy tam gracza
    switch(ch)
    {
        // A1/A3/C1/C3 to klawisze z numpada
        case KEY_A1: if(level[playerX-1][playerY-1]) { --playerX; --playerY; } break;
        case KEY_A3: if(level[playerX+1][playerY-1]) { ++playerX; --playerY; } break;
        case KEY_C1: if(level[playerX-1][playerY+1]) { --playerX; ++playerY; } break;
        case KEY_C3: if(level[playerX+1][playerY+1]) { ++playerX; ++playerY; } break;
        // Ta sztuczka pozwala nam obs³u¿yæ klawisze numpada i strza³ki w tym samym miejscu
        case KEY_LEFT:  case KEY_B1:    if(level[playerX-1][playerY]) --playerX; break;
        case KEY_RIGHT: case KEY_B3:    if(level[playerX+1][playerY]) ++playerX; break;
        case KEY_UP:    case KEY_A2:    if(level[playerX][playerY-1]) --playerY; break;
        case KEY_DOWN:  case KEY_C2:    if(level[playerX][playerY+1]) ++playerY; break;
    }
}
