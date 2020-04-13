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
#include "Position.hpp"

const int numberOfPhilosophers{5};

// not included to classes, I will add if need to extend graphic features
struct GraphicRepresentation {
   std::string graphic;

   GraphicRepresentation(std::string g) : graphic(g) {

   }
};


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

   void clearScreen() {
      wclear(win);
   }

   void clearElement(int xPos, int yPos) {
      wmove(win, yPos, xPos);
      waddch(win, ' ');
   }

   void printElement(GraphicRepresentation& graphic, Position& position) {
      wmove(win, position.y, position.x);
      waddstr(win, graphic.graphic.c_str());        
      wrefresh(win);	
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




struct Mine {
   std::mutex mutex;
   Position position;
   GraphicRepresentation graphic{"m"};
};

struct LaserPickaxe {
   std::mutex leftHandMutex;
   std::mutex rightHandMutex;
   Position position;
   GraphicRepresentation graphic{"p"};
};

struct Limonium {
   std::mutex mutex;
   Position position;
   GraphicRepresentation graphic{"l"};
};

struct Metal {
   std::mutex mutex;
   Position position;
   GraphicRepresentation graphic{"m"};
};

struct Wire {
   std::mutex mutex;
   Position position;
   GraphicRepresentation graphic{"w"};
};

struct Spaceship {
   std::vector<std::mutex> workplaces;
   Position position;
   GraphicRepresentation graphic{"s"};
};

class Map {
public:
   std::atomic<bool> ready;

   int width;
   int height;

   std::vector<std::string> mapGraphics;

   std::array<Mine, 3> mines;
   std::array<LaserPickaxe, 10> laserPickaxes;
   std::vector<Limonium> limoniums;
   std::vector<Metal> metals;
   std::vector<Wire> wires;
   Spaceship spaceship;


   Map() {  
      ready = false; 

      for (size_t i = 0; i < 3; i++) {
         mines[i].position = { 5, (i + 1) * 10 };
      }

      for (size_t i = 0; i < 10; i++) {
         laserPickaxes[i].position = { 15, (i + 1) * 3 };
      } 

      spaceship.position = Position{ 90, 20 }; 
   }

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
   Map const& map;

public:
   Gatherer(Map const& map) :
      map(map), lifeThread(&Gatherer::live, this)
   {
      
   }

   ~Gatherer() {
      lifeThread.join();
   }

   void live() {
      while(!map.ready) {};

      while(map.ready) {
         std::this_thread::sleep_for(std::chrono::milliseconds(600));
         position.move(Position::Direction::Right);
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
   Map const& map;

public:
   Worker(Map const& map) :
      map(map), lifeThread(&Worker::live, this)
   {
      
   }

   ~Worker() {
      lifeThread.join();
   }

   void live() {
      while(!map.ready) {};

      while(map.ready) {
         std::this_thread::sleep_for(std::chrono::milliseconds(700));
         position.move(Position::Direction::Right);
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
   Map const& map;

public:
   Builder(Map const& map) :
      map(map), lifeThread(&Builder::live, this)
   {
      position = Position{1, 1};
   }

   ~Builder() {
      lifeThread.join();
   }

   void live() {
      while(!map.ready) {};

      while(map.ready) {
         std::this_thread::sleep_for(std::chrono::milliseconds(500));       
         position.move(Position::Direction::Right);
         position.move(Position::Direction::Down);
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
   Map& map;
   std::thread lifeThread;

public:
   ProcessExit(Map& map) :
      map(map), lifeThread(&ProcessExit::checkPlayerInputForExit, this)
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
            map.ready = false;
            return;
         }
      }
   }
};

void beginSimulation() {
   Map map{};
   map.readMapFromFile("/home/adrian/Documents/SpaceStationSimulation/map.txt");
   
   std::array<Builder, 5> builders {
      Builder{ map },
      Builder{ map },
      Builder{ map },
      Builder{ map },
      Builder{ map }
   };
   std::array<Worker, 5> workers {
      Worker{ map },
      Worker{ map },
      Worker{ map },
      Worker{ map },
      Worker{ map }
   };
   std::array<Gatherer, 5> gatherers {
      Gatherer{ map },
      Gatherer{ map },
      Gatherer{ map },
      Gatherer{ map },
      Gatherer{ map }
   };
   
   // the simulation begins 
   map.ready = true;
   
   ProcessExit processExit{ map };

   // na głównym wątku odbywa się odświeżanie ekranu i wypisywanie stanu wątków i zasobów
   Screen screen;

   while(map.ready) {
      std::this_thread::sleep_for(std::chrono::milliseconds(300));
      screen.clearScreen();

      //printing map
      int i{};
      for(const auto& mapLine : map.mapGraphics) 
         screen.printLine(mapLine, i++);

      //printing threads
      for(auto& builder : builders) {
         screen.printElement(builder.getGraphicRepresentation(), builder.getPosition().x, builder.getPosition().y);
      }      

      for(auto& worker : workers) {
         screen.printElement(worker.getGraphicRepresentation(), worker.getPosition().x, worker.getPosition().y);
      }     

      for(auto& gatherer : gatherers) {
         screen.printElement(gatherer.getGraphicRepresentation(), gatherer.getPosition().x, gatherer.getPosition().y);
      }

      //printing resources
      screen.printElement(map.spaceship.graphic, map.spaceship.position);
      
      for(auto& wire : map.wires) {
         screen.printElement(wire.graphic, wire.position);
      }  

      for(auto& metal : map.metals) {
         screen.printElement(metal.graphic, metal.position);
      } 

      for(auto& limonium : map.limoniums) {
         screen.printElement(limonium.graphic, limonium.position);
      }

      for(auto& limonium : map.limoniums) {
         screen.printElement(limonium.graphic, limonium.position);
      }

      for(auto& lasePickaxe : map.laserPickaxes) {
         screen.printElement(lasePickaxe.graphic, lasePickaxe.position);
      }

      for(auto& mine : map.mines) {
         screen.printElement(mine.graphic, mine.position);
      }     
   } 
   screen.printLine("Waiting for all threads to end...", 41); 
   screen.printLine("Screen is not refreshing now.", 42);
}
 
int main() 
{ 	      	
   beginSimulation(); 
				
	return 0; 
}