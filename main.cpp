#include <iostream> 
#include <thread>
#include <ncurses.h>
#include <array>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>
#include <string>
#include <random>
#include <ctime>
#include <memory>

const int numberOfPhilosophers{5};

// Klasa obslugująca wypisywanie tekstu na ekranie 
class ScreenManager {
public:
   static void printElement(std::string const & text, int xPos, int yPos) {     
      //mvprintw(xPos, yPos + 1, "'\b'");
      mvprintw(yPos, xPos, "%s", text.c_str());        
      refresh();	
   }

   static void print(std::string const & text, int xPos, int id) {
         move(xPos, 0);       
         clrtoeol();      
         mvprintw(xPos, 0, "%s", std::to_string(id).c_str());
         mvprintw(xPos, 3, "%s", text.c_str());        
         refresh();			
   }

   static void print(std::string const & text, int yPos) {
         move(yPos, 0);       
         clrtoeol();               
         mvprintw(yPos, 0, "%s", text.c_str());        
         refresh();			
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



struct Position {
   int x;
   int y;

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

// Gather Limonium from Mines using LaserPickaxe
class Gatherer {
   std::thread lifeThread;
   Position position;

public:
   Gatherer() :
      lifeThread(&Gatherer::live, this)
   {
      
   }

   void live() {

   }
};

// Converts Limonium into Metal or Wire
class Worker {
   std::thread lifeThread;
   std::mt19937 mersenne{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
   Position position;

public:
   Worker() :
      lifeThread(&Worker::live, this)
   {
      
   }

   

   void live() {

   }
};

// Gets Metal or Wire and carries it to the Spaceship
class Builder {
   std::thread lifeThread;
   Position position;

public:
   Builder() :
      lifeThread(&Builder::live, this)
   {
      position = Position{0, 0};
   }

   void live() {
      while(true) {
         std::this_thread::sleep_for(std::chrono::milliseconds(500));
         position.move(Position::Direction::RIGHT);
      }
   }  

   Position getPosition() {
      return position;
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
   
   
   Builder builder;
   
   // w tym momencie ludzie rozpoczną myślenie/jedzenie
   //map.ready = true;
   
   //ProcessExit processExit{table};

   // na głównym wątku odbywa się odświeżanie ekranu i wypisywanie stanu wątków i zasobów
   while(true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(300));
      ScreenManager::print(std::to_string(builder.getPosition().x), 0);
   } 
   ScreenManager::print("Waiting for all threads to end...", 18); 
   ScreenManager::print("Screen is not refreshing now.", 19);
}
 
int main() 
{ 	    
   initscr();		
   beginSimulation();                  		
	endwin();
					
	return 0; 
}