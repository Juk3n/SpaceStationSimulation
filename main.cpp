#include <iostream> 
#include <thread>
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
#include "Screen.hpp"
#include "GraphicRepresentation.hpp"
#include "Resources.hpp"
#include "Map.hpp"
#include "ProcessExit.hpp"

   struct Flags {
      std::string picksUp[5];
   };

   Flags flags{ "0", "0", "0", "0", "0" };


// Gather Limonium from Mines using LaserPickaxe
class Gatherer {
   std::mt19937 mersenne{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };

   int id;
   std::string graphicRepresentation{"G"};
   int speed;
   Position position;
   std::thread lifeThread;
   
   Map* map;

   Destiny destiny;

   Item* firstItem{ nullptr };
   Item* secondItem{ nullptr };

   



   void findGoal(GoalType type) {
      switch(type) {
         Goal* nearestGoal;
         case GoalType::LaserPickaxe:
            nearestGoal = findTheNearestLaserPickaxe();
            destiny.setGoal(nearestGoal);            
            destiny.setItem(&(*(std::next(map->laserPickaxes.begin(), nearestGoal->getID()))));
            break;
         case GoalType::Mine:
            nearestGoal = findTheNearestMine();
            destiny.setGoal(findTheNearestMine()); 
            break;
      }
   }

   Goal* findTheNearestLaserPickaxe() {
      for(auto& pickaxe : map->laserPickaxes) {
         if(!pickaxe.isUsed)
            return &pickaxe;
      }
   }

   Goal* findTheNearestMine() {
      for(auto& mine : map->mines) {
         if(!mine.isUsed)
            return &mine;
      }
   }

   bool isAtGoal() {
      return (destiny.getGoal()->getPositionGoal().x == position.x && 
         destiny.getGoal()->getPositionGoal().y == position.y);
   }

   void goToGoal() {
      if(destiny.getGoal()->getPositionGoal().x > position.x) 
         position.move(Position::Direction::Right);
      else if(destiny.getGoal()->getPositionGoal().x < position.x)
         position.move(Position::Direction::Left);

      if(destiny.getGoal()->getPositionGoal().y > position.y)
         position.move(Position::Direction::Down);
      else if(destiny.getGoal()->getPositionGoal().y < position.y)
         position.move(Position::Direction::Up);

      if(firstItem) 
         *(firstItem->getPositionItem()) = {position.x - 1, position.y};
      
      if(secondItem)         
         secondItem->position = {position.x, position.y + 1};
   }

   void pick(Item* item) { 
      item->mutex.lock();  
      setAttachment(item, true);  
   }

   void putDown(Item* item) {
      setAttachment(item, false);
      item->mutex.unlock();
   }

   void setAttachment(Item* item, bool isUsed) {
      switch(destiny.getGoalType()) {
         case GoalType::LaserPickaxe:
            flags.picksUp[id] = "5";
            firstItem = item;
            firstItem->setUsing(isUsed);
            break;
         case GoalType::Mine:
            secondItem = item;
            secondItem->setUsing(isUsed);
            break;
      }
   }

   void setRandomSpeed() {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      static thread_local std::uniform_int_distribution<> range(1, 10);
      speed = 100 * range(mersenne);
   }

public:
   Gatherer(int id, Map* map) :
      id(id), map(map), lifeThread(&Gatherer::live, this)
   {
      findGoal(GoalType::LaserPickaxe);

      position = { 30, 20 };

      graphicRepresentation += std::to_string(id);

      setRandomSpeed();      
   }

   ~Gatherer() {
      lifeThread.join();
   }

   void live() {
      while(!map->ready) {};

      while(map->ready) {
         /*
            if not at destination
               go to destination
            else
               choice := process destination
               if choice is pickable
                  pick up
               else if choice is mine
                  use pickaxe and get limonium
               else if having limonium
                  put pickaxe on the ground
               else 
                  put limonium on the ground
         */

         /*
            if(!isAtGoal())
               moveToGoal();
            else
               if(goal == LaserPickaxe)
                  pickUp(LaserPickaxe);
               else if(goal == Mine)
                  mine(Mine);
               else if(goal == LaserPickaxeArea)
                  drop(LaserPickaxe);
               else
                  drop(Limonium);
         */
         
         std::this_thread::sleep_for(std::chrono::milliseconds(speed));
         if(!isAtGoal()) {
            findGoal(destiny.getGoalType()); //refresh goal becouse actual goal can be used 
            goToGoal();
         }
         else {
            if(destiny.getGoalType() == GoalType::LaserPickaxe) {
               pick(destiny.getItem());
               destiny.setGoalType(GoalType::Mine);
               findGoal(destiny.getGoalType());
            }
         }   
         
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
   int id;
   Map const& map;

public:
   Worker(Map const& map, int id) :
      map(map), id(id), lifeThread(&Worker::live, this)
   {
      
   }

   ~Worker() {
      lifeThread.join();
   }

   void live() {
      /*
         if not at destination
            go to destination
         else
            choice := process destination
            if choice is pickable
               pick up limonium
            else
               put metal/wire on the ground
      */

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
      /*
         if not at destination
            go to destination
         else
            choice := process destination
            if choice is pickable
               pick up wire/metal
            else
               put metal/wire to the spaceship
      */
          
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
      Worker{ map, 1 },
      Worker{ map, 2 },
      Worker{ map, 3 },
      Worker{ map, 4 },
      Worker{ map, 5 }
   };
   std::array<Gatherer, 5> gatherers {
      Gatherer{ 0, &map },
      Gatherer{ 1, &map },
      Gatherer{ 2, &map },
      Gatherer{ 3, &map },
      Gatherer{ 4, &map }
   };
   
   // the simulation begins 
   map.ready = true;
   
   ProcessExit processExit{ map };

   // na głównym wątku odbywa się odświeżanie ekranu i wypisywanie stanu wątków i zasobów
   Screen screen;

   while(map.ready) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
      screen.printRecElement(map.spaceship.graphic, map.spaceship.position, map.spaceship.size);
      
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

      int u{};
      for(auto& gatherer : gatherers) {
         screen.printLine("Gatherer #" + std::to_string(u) + " : " + std::to_string(gatherer.getPosition().x) + " " + std::to_string(gatherer.getPosition().y), 23 + u++);
      }
      
      for(int x = 0; x < 5; x++) {
         screen.printElement(flags.picksUp[x], x, 30);
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