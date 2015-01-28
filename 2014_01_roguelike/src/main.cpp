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

const int levelX = 0; // Pocz¹tek planszy
const int levelY = 2; // (przesuniêcie na potrzeby wiadomoœci)
const int levelW = 80; // Wymiary planszy. Standardowo konsola
const int levelH = 22; // ma 80 kolumn i 25 wierszy

const int monsterCount = 20; // liczba potworów na mapie


bool level[levelW][levelH]; // Zazwyczaj pola planszy s¹ obiektem jakiejœ struktury
                    // ale poniewa¿ potrzebujemy tylko jednej informacji (œciana/wolne)
                    // mo¿emy siê "wymigaæ" z tablic¹ booli

int playerX = 1;   // Pocz¹tkowe koordynaty gracza to lewy górny róg planszy
int playerY = 1;

int playerHp = 100; // Statystyki gracza
int playerHpMax = 100;
int playerKills = 0;
int playerAttack = 15;
std::string mesg; // Komunikaty

void generateLevel(); // Deklaracje funkcji (definicje poni¿ej main() )
void draw();
void update();
void generateLineH(int x1, int x2, int y);
void generateLineW(int y1, int y2, int x);
void generateRoom(int x1, int y1, int x2, int y2);
void clearMsg();
void addMsg(const char * str);
void generateMonsters();

class Monster // Pierwsze liŸniêcie obiektowoœci, klasa dla potworka!
{
private:
    int x, y; // koordynaty
    int hp; // pozosta³e punkty ¿ycia
    int attack; // zdolnoœæ ataku
    char ch; // wyœwietlany znak
    int color; // kolor tego znaku
public:
    int GetX() { return x; } // Standardowe akcesory
    int GetY() { return y; }
    bool IsDead() { return hp <= 0; } // Czy potworek jest martwy?
    // Ustawianie pocz¹tkowych informacji o potworku:
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
        if(IsDead()) return;
        static char buffer[256];
        sprintf(buffer, "You hit a monster for %d dmg!", dmg);
        addMsg(buffer);

        hp -= dmg;
        if(hp <= 0)
        {
            ++playerKills; // Zabiliœmy potworka!
            addMsg("The monster is dead!");
        }
    }
    // Tutaj siedzi ca³e A.I. potworka. Definicja poni¿ej w pliku
    void Update();
};

std::vector<Monster> monsters;

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
            level[i][j] = true; // Wszystkie tile nie zawieraj¹ œciany
        }
    }

    // Tworzymy prostok¹t ze œcian dooko³a planszy (¿eby plansza by³a zamkniêta)
    generateLineH(0, levelW - 1, 0);
    generateLineH(0, levelW - 1, levelH - 1);
    generateLineW(0, levelH - 1, 0);
    generateLineW(0, levelH - 1, levelW - 1);

    // rekurencyjnie tworzymy poziom z³o¿ony z pokoi
    generateRoom(0, 0, levelW - 1, levelH - 1);

    generateMonsters(); // Ustawiamy potworki na mapie
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
                mvprintw(j + levelY, i + levelX, "."); // pusta przestrzeñ. UWAGA mvprintw bierze najpierw y, potem x!
            else
                mvprintw(j + levelY, i + levelX, "#"); // œciana. UWAGA mvprintw bierze najpierw y, potem x!
        }
    }
    attroff(COLOR_PAIR(2)); // Wy³¹czamy kolor 2

    for(int i = 0; i < monsters.size(); i++)
    {
        monsters[i].Draw();
    }

    attron(COLOR_PAIR(1));
    if(playerHp > 0)
        mvprintw(playerY + levelY, playerX + levelX, "@"); // Rysujemy postaæ gracza kolorem 1. UWAGA mvprintw bierze najpierw y, potem x!
    else
        mvprintw(playerY + levelY, playerX + levelX, "%%"); // Jesteœmy trupem...
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(1)); // Drukujemy statystyki na ekran
    mvprintw(24, 1, "HP: %d/%d (KILLS: %d)", playerHp, playerHpMax, playerKills);
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(3)); // Drukujemy komunikaty na ekran
    mvprintw(0, 0, "%s", mesg.substr(0, 80).c_str());
    if(mesg.length() >= 80)
        mvprintw(1, 0, "%s", mesg.substr(80, 160).c_str());
    attroff(COLOR_PAIR(3));

    refresh(); // odœwie¿amy ekran, to ostatni krok w pêtli renderuj¹cej
}

void update()
{
    clearMsg(); // na pocz¹tku ka¿dego update'a czyœcimy bufor wiadomoœci (w ten sposób
                // wiadomoœci s¹ przechowywane tylko do nastêpnego wciœniêtego klawisza)
    int ch = getch(); // Pobieramy znak z klawiatury

    if(playerHp <= 0)
    {
        addMsg("You're dead!");
        return; // W momencie œmierci dalsza aktualizacja gry nie ma sensu...
    }

    int xprev = playerX; // Zapamiêtujemy poprzedni¹ pozycjê gracza, bo mo¿emy
    int yprev = playerY; // wpaœæ na potworka!
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
        case KEY_B2: break; // Czekamy w miejscu
        default: return; // Nie idziemy dalej je¿eli wcisêliœmy inny klawisz
    }

    for(int i = 0; i < monsters.size(); i++) // Wykonajmy AI dla wszystkich potworków
    {
        monsters[i].Update(); // A.I. potworków (ruch i atak)
    }

    for(int i = 0; i < monsters.size(); i++) // Sprawdzamy, czy wpadliœmy na potworka
    {
        if(monsters[i].GetX() == playerX && monsters[i].GetY() == playerY && !monsters[i].IsDead())
        {
            monsters[i].Attack(playerAttack); // Tak! To go atakujemy
            playerX = xprev; // I cofamy zmianê pozycji (nie mo¿na "wejœæ" na potworka)
            playerY = yprev;
        }
    }
}

void generateLineH(int x1, int x2, int y) // Generuje pionow¹ liniê œciany na mapie
{
    for(int i = x1; i <= x2; i++)
    {
        level[i][y] = false;
    }
}

void generateLineW(int y1, int y2, int x) // Generuje poziom¹ liniê œciany na mapie
{
    for(int j = y1; j <= y2; j++)
    {
        level[x][j] = false;
    }
}

// Implementacja algorytmu BSP:
// http://www.roguebasin.com/index.php?title=Basic_BSP_Dungeon_generation
void generateRoom(int x1, int y1, int x2, int y2) // Funkcja rekurencyjna tworz¹ca poziom
{
    if(x2 - x1 < 9) return; // Je¿eli pokój jest mniejszy od 9 zakoñcz rekurencjê
    if(y2 - y1 < 9) return; // Bo nie ma sensu dzieliæ go bardziej

    if(rand()%2 == 0) // Dzielimy pokój na dwie czêœci poziome
    {
        int x = rand()%((x2-x1)/2) + (x2 - x1)/4 + x1;
        generateLineW(y1, y2, x); // Linia oddzielaj¹ca dwa mniejsze pokoje

        generateRoom(x1, y1, x, y2); // Rekurencja dla obu mniejszych pokoi
        generateRoom(x, y1, x2, y2);

        int y = rand()%((y2-y1)/2) + (y2 - y1)/4 + y1;
        level[x][y] = true; // Tworzymy "drzwi" miêdzy dwoma pokojami
    }
    else // Dzielimy pokój na dwie czêœci pionowe
    {
        int y = rand()%((y2-y1)/2) + (y2 - y1)/4 + y1;
        generateLineH(x1, x2, y); // Linia oddzielaj¹ca dwa mniejsze pokoje

        generateRoom(x1, y1, x2, y); // Rekurencja dla obu mniejszych pokoi
        generateRoom(x1, y, x2, y2);

        int x = rand()%((x2-x1)/2) + (x2 - x1)/4 + x1;
        level[x][y] = true; // Tworzymy "drzwi" miêdzy dwoma pokojami
    }
}

void clearMsg()
{
    mesg = ""; // Czyœcimy bufor komunikatów
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
        do // dopóki nie znajdziesz wolnego miejsca na tego potworka
        {
            x = (rand()%levelW); // losuj jego koordynaty
            y = (rand()%levelH);
        } while(!level[x][y]);

        m.Set(x, y, 15, 3, 'Z', 3); // znaleŸliœmy! To go stwórzmy

        monsters.push_back(m); // i dodajmy do listy potworków
    }
}

////////////////////////////////////////////////////////////////////////////////////
// NOWE:

void Monster::Update() // Tutaj siedzi ca³e A.I. potworka
{
    if(IsDead()) return; // martwy potworek nic nie robi!

    // SprawdŸmy czy w tile'ach +/- 1 od potworka
    // znajduje sie gracz. Je¿eli tak, to go atakujemy!
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            if(this->x + x == playerX &&
               this->y + y == playerY)
            {
                static char buffer[256];
                sprintf(buffer, "The monster hits you for %d dmg!", attack);
                addMsg(buffer);

                playerHp -= attack;
                if(playerHp <= 0)
                {
                    addMsg("You're dead!");
                }
                return; // Atak to wszystko co robimy
            }
        }
    }
    // Ok, nie ma w pobli¿u gracza :-(
    int moveX = 0;
    int moveY = 0;

    // Ruszmy siê o jedno pole w kierunku gracza!
    if(playerX < this->x) moveX = -1;
    if(playerX > this->x) moveX = 1;
    if(playerY < this->y) moveY = -1;
    if(playerY > this->y) moveY = 1;

    bool isXoccupied = false; // czy w osi X trafiamy na innego potworka
    bool isYoccupied = false; // analogicznie dla osi y
    bool isBothOccupied = false; // analogicznie dla bu soi

    for(int i = 0; i < monsters.size(); i++) // Sprawdzamy czy potworek nie wchodzi w innego potworka
    {
        if(&monsters[i] != this && !monsters[i].IsDead()) // w innego, tzn. nie w siebie i nie w trupa!
        {
            if(monsters[i].GetX() == this->x + moveX && monsters[i].GetY() == this->y)
                isXoccupied = true;
            if(monsters[i].GetX() == this->x && monsters[i].GetY() == this->y + moveY)
                isYoccupied = true;
            if(monsters[i].GetX() == this->x + moveX && monsters[i].GetY() == this->y + moveY)
                isBothOccupied = true;
        }
    }

    // Najpierw próbujemy siê ruszyæ po skosie
    if(level[this->x + moveX][this->y + moveY]) // czy jest to puste pole?
    {
        if(!isBothOccupied) // i to nie zajête przez innego potworka?
        {
            this->x += moveX; // skoro tak, to ruszmy siê!
            this->y += moveY;
        }
    }
    else // Po skosie siê nie da, spróbujmy po osi
    {
        if(!isXoccupied && level[this->x + moveX][this->y]) // mo¿e w poziomie?
            this->x += moveX;
        else if(!isYoccupied && level[this->x][this->y + moveY]) // albo w pionie?
            this->y += moveY;
    }
}
