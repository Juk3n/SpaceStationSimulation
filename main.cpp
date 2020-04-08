#include <iostream> 
#include <thread>
#include <ncurses.h>
#include <array>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>
#include <random>
#include <ctime>
#include <memory>

#include "MapFile.hpp"

const int numberOfPhilosophers{5};

// Klasa obslugująca wypisywanie tekstu na ekranie 
class Screen {
   WINDOW* win;
public:
   Screen() {
      initscr();	
      win = newwin(43, 132, 0, 0);
   }

   ~Screen() {
      delwin(win);
      endwin();
   }

   void xd() {
      waddch(win, 'a'); 
      wrefresh(win); 
      for (size_t i = 0; i < 20; i++)
      {
         //y, x
         wmove(win, i, 40);
         waddstr(win, "aaaaa");
      } 
      wrefresh(win);
   }

   void clearScreen() {
      wclear(win);
   }

   void clearElement(int xPos, int yPos) {
      wmove(win, yPos, xPos);
      waddch(win, ' ');
   }

   void printElement(std::string const & text, int xPos, int yPos) {     
      wmove(win, yPos, xPos);
      waddstr(win, text.c_str());        
      wrefresh(win);	
   }

   void printLine(std::string const & text,  int yPos) {
      wmove(win, yPos, 0);
      waddstr(win, text.c_str());        
      wrefresh(win);	
   }
};

struct Fork {
   std::mutex mutex;
   bool isUsed{ false };
};

struct Table {
   std::atomic<bool> ready{false}; // zmienna atomowa, poniewaz korzystaja z nie wszystkie watki filozofow w tym samym momencie
   
   std::array<Fork, numberOfPhilosophers> forks;
};




// not included to classes, I will add if need to extend graphic features
struct GraphicRepresentation {
   std::string graphic;
};

struct Position {
   int x = 1;
   int y = 1;

   enum class Direction {
      LEFT,
      RIGHT,
      UP,
      DOWN
   };

   // direction are suited to ncurses
   void move(Direction dir) {
      switch(dir) {
         case Direction::UP:
            y--;
            break;
         case Direction::DOWN:
            y++;
            break;
         case Direction::LEFT:
            x--;
            break;
         case Direction::RIGHT:
            x++;
            break;
      }
   }
};

struct Mine {
   std::mutex mutex;
   Position position;
};

struct LaserPickaxe {
   std::mutex leftHandMutex;
   std::mutex rightHandMutex;
   Position position;
};

struct Limonium {
   std::mutex mutex;
   Position position;
};

struct Metal {
   std::mutex mutex;
   Position position;
};

struct Wire {
   std::mutex mutex;
   Position position;
};

struct Spaceship {
   std::vector<std::mutex> workplaces;
   Position position;
};

class Map {
public:
   std::atomic<bool> ready{ false };

   int width;
   int height;

   std::vector<std::string> mapGraphics;

   std::array<Mine, 3> mines;
   std::array<LaserPickaxe, 10> laserPickaxes;
   std::vector<Limonium> limoniums;
   std::vector<Metal> metals;
   std::vector<Wire> wires;
   Spaceship spaceship;

   void readMapFromFile(std::string fileName) {
      myMapFile file(fileName);
      width = file.readMapWidth();
      height = file.readMapHeight();
      for(int i = 0; i < height; i++)
         mapGraphics.push_back(file.readNextLine());
   }
};

// Gather Limonium from Mines using LaserPickaxe
class Gatherer {
   std::thread lifeThread;
   Position position;
   std::string graphicRepresentation{"G"};

public:
   Gatherer() :
      lifeThread(&Gatherer::live, this)
   {
      
   }

   void live() {
      while(true) {
         std::this_thread::sleep_for(std::chrono::milliseconds(600));
         position.move(Position::Direction::DOWN);
      }
   }

   Position getPosition() {
      return position;
   }

   std::string getGraphicRepresentation() {
      return graphicRepresentation;
   }
};

// Converts Limonium into Metal or Wire
class Worker {
   std::thread lifeThread;
   std::mt19937 mersenne{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
   Position position;
   std::string graphicRepresentation{"W"};

public:
   Worker() :
      lifeThread(&Worker::live, this)
   {
      
   }

   void live() {
      while(true) {
         std::this_thread::sleep_for(std::chrono::milliseconds(700));
         position.move(Position::Direction::RIGHT);
      }
   }

   Position getPosition() {
      return position;
   }

   std::string getGraphicRepresentation() {
      return graphicRepresentation;
   }
};

// Gets Metal or Wire and carries it to the Spaceship
class Builder {
   std::thread lifeThread;
   Position position;
   std::string graphicRepresentation{"B"};

public:
   Builder() :
      lifeThread(&Builder::live, this)
   {
      position = Position{1, 1};
   }

   void live() {
      while(true) {
         std::this_thread::sleep_for(std::chrono::milliseconds(500));       
         position.move(Position::Direction::RIGHT);
         position.move(Position::Direction::DOWN);
      }
   }  

   Position getPosition() {
      return position;
   }

   std::string getGraphicRepresentation() {
      return graphicRepresentation;
   }
};



// Klasa filozowa, ktora po utworzeniu rozpoczyna dzialanie watku jako funkcji dine()
class Philosopher {
   int id;
   std::string state{};
   Table const& table;
   Fork& leftFork;
   Fork& rightFork;
   std::thread lifeThread;
   std::mt19937 mersenne{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };

public:
   Philosopher(int id, Table const & table, Fork & leftFork, Fork & rightFork) :
      id(id), table(table), leftFork(leftFork), rightFork(rightFork), lifeThread(&Philosopher::dine, this)
   {
      
   }

   ~Philosopher()
   {
      lifeThread.join();
   }

   int getID() {
      return id;
   }

   std::string getState() {
      return state;
   }

   void dine() {
      while(!table.ready);

      do {
         think();
         eat();
      } while (table.ready);
   }

   void think() {
      state = "is thinking |     |";
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      state = "is thinking |#    |";
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));   
      state = "is thinking |##   |";
      std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
      state = "is thinking |###  |";
      std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
      state = "is thinking |#### |";
      std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
      state = "is thinking |#####|";
      std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
      state = "waiting for food";     
   }

   // Żeby rozpocząć jedznie, na początek widelce muszę zostać zablokowane przez uzycie lock'a
   void eat() {
      std::lock(leftFork.mutex, rightFork.mutex);

      std::lock_guard<std::mutex> leftLock(leftFork.mutex, std::adopt_lock);
      std::lock_guard<std::mutex> rightLock(rightFork.mutex, std::adopt_lock);

      state = "eating";
      leftFork.isUsed = true;
      rightFork.isUsed = true;

      static thread_local std::uniform_int_distribution<> range(1, 1000);
      std::this_thread::sleep_for(std::chrono::milliseconds(6500 - range(mersenne)));

      state = "finished eating.";
      leftFork.isUsed = false;
      rightFork.isUsed = false;
   }
};

// Klasa do obsługi wyjścia z programu przez uzytkownika
class ProcessExit {
   Table& table;
   std::thread lifeThread;

public:
   ProcessExit(Table& table) :
      table(table), lifeThread(&ProcessExit::checkPlayerInputForExit, this)
   {
   }

   ~ProcessExit()
   {
      lifeThread.join();
   }

   void checkPlayerInputForExit() {
      while(true) {
         std::this_thread::sleep_for(std::chrono::milliseconds(1000));
         char a{ static_cast<char>(getchar()) };
         if(a == 'q') {
            table.ready = false;
            return;
         }
      }
   }
};

void beginSimulation() {
   Map map{};
   map.readMapFromFile("/home/adrian/Documents/SpaceStationSimulation/map.txt");
   
   std::array<Builder, 5> builders;
   std::array<Worker, 5> workers;
   std::array<Gatherer, 5> gatherers;
   
   // w tym momencie ludzie rozpoczną myślenie/jedzenie
   //map.ready = true;
   
   //ProcessExit processExit{table};

   // na głównym wątku odbywa się odświeżanie ekranu i wypisywanie stanu wątków i zasobów
   Screen screen;
   int i{};
   for(const auto& mapLine : map.mapGraphics) 
      screen.printLine(mapLine, i++);
   
   while(true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(300));
      screen.clearScreen();

      for(auto& builder : builders) {
         screen.printElement(builder.getGraphicRepresentation(), builder.getPosition().x, builder.getPosition().y);
      }

      for(auto& worker : workers) {
         screen.printElement(worker.getGraphicRepresentation(), worker.getPosition().x, worker.getPosition().y);
      }

      for(auto& gatherer : gatherers) {
         screen.printElement(gatherer.getGraphicRepresentation(), gatherer.getPosition().x, gatherer.getPosition().y);
      }
   } 
   screen.printLine("Waiting for all threads to end...", 18); 
   screen.printLine("Screen is not refreshing now.", 19);
}
 
int main() 
{ 	      	
   beginSimulation(); 
				
	return 0; 
}