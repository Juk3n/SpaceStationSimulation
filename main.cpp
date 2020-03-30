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
   static void print(std::string const & text, int xPos, int id) {
         move(xPos, 0);       
         clrtoeol();      
         mvprintw(xPos, 0, "%s", std::to_string(id).c_str());
         mvprintw(xPos, 3, "%s", text.c_str());        
         refresh();			
   }

   static void print(std::string const & text, int xPos) {
         move(xPos, 0);       
         clrtoeol();               
         mvprintw(xPos, 0, "%s", text.c_str());        
         refresh();			
   }
};

struct Fork {
   std::mutex mutex;
   bool isUsed{false};
};

struct Table {
   std::atomic<bool> ready{false}; // zmienna atomowa, poniewaz korzystaja z nie wszystkie watki filozofow w tym samym momencie
   std::array<Fork, numberOfPhilosophers> forks;
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
   Table table;
   
   std::array<Philosopher, numberOfPhilosophers> philosophers {
      {
         { 1, table, table.forks[0], table.forks[1] },
         { 2, table, table.forks[1], table.forks[2] },
         { 3, table, table.forks[2], table.forks[3] },
         { 4, table, table.forks[3], table.forks[4] },
         { 5, table, table.forks[4], table.forks[0] },
      }
   };
   
   // w tym momencie filozofowie rozpoczną myślenie/jedzenie
   table.ready = true;
   
   ProcessExit processExit{table};

   // na głównym wątku odbywa się odświeżanie ekranu i wypisywanie stanu wątków i zasobów
   while(table.ready == true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      ScreenManager::print("PHILOSOPHERS", 0);
      for(auto& philosopher : philosophers) {    
         ScreenManager::print(philosopher.getState(), philosopher.getID(), philosopher.getID());
      }   
      ScreenManager::print("FORKS 0 - available 1 - used", 7);
      for (int i = 0; i < numberOfPhilosophers; i++)
      {
         ScreenManager::print(std::to_string(table.forks[i].isUsed), i + 8, i + 1);
      }

      ScreenManager::print("Press q for exit", 18);   
   } 
   ScreenManager::print("Waiting for all threads to end...", 18); 
   ScreenManager::print("Screen is not refreshing now.", 19);
}
 
int main() 
{ 	
   
   try {
      initscr();		
      beginSimulation();                  		
	   endwin();
   } catch(...) {
      std::cout << "xD";
   }
 					
	return 0; 
} 
