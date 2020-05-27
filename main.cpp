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

   Item* pickaxePocket{ nullptr };
   Item* limoniumPocket{ nullptr };

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
            destiny.setGoal(nearestGoal);
            if(destiny.getItem()->isUsed()) {
               map->limoniumCounter++;
               destiny.setItem(&(*(std::next(map->limoniums.begin(), map->limoniumCounter))));
               // Item* nearestItem;
               // do {
               //    nearestItem = &(*(std::next(map->limoniums.begin(), map->limoniumCounter)));
               // } while(nearestItem->isOnGround());              
               // destiny.setItem(nearestItem);
            }
            break;
         case GoalType::LaserPickaxeArea:
            nearestGoal = findLaserPickaxeArea();
            destiny.setGoal(nearestGoal);
            break;
         case GoalType::LimoniumArea:
            nearestGoal = findLimoniumArea();
            destiny.setGoal(nearestGoal);
            break;
      }
   }

   Goal* findTheNearestLaserPickaxe() {
      for(auto& pickaxe : map->laserPickaxes) {
         if(!pickaxe.isUsed())
            return &pickaxe;
      }
   }

   Goal* findTheNearestMine() {
      for(auto& mine : map->mines) {
         if(mine.used() < 5)
            return &mine;
      }
   }

   Goal* findLaserPickaxeArea() {
      return &(map->laserPickaxeArea);
   }

   Goal* findLimoniumArea() {
      return &(map->limoniumArea);
   }

   bool isAtGoal() {
      if(isGoalAnArea()) {
         return (destiny.getGoal()->getPositionGoal().x == position.x);
      }
      else {
         return (destiny.getGoal()->getPositionGoal().x == position.x && 
            destiny.getGoal()->getPositionGoal().y == position.y);
      }
   }

   void goToGoal() {
      goToGoalHorizontal();
      if(!isGoalAnArea())
         goToGoalVertical();      

      if(pickaxePocket) 
         *(pickaxePocket->getPositionItem()) = {position.x - 1, position.y};
      
      if(limoniumPocket)         
         *(limoniumPocket->getPositionItem()) = {position.x, position.y + 1};
   }

   bool isGoalAnArea() {
      return (destiny.getGoalType() == GoalType::LaserPickaxeArea || destiny.getGoalType() == GoalType::LimoniumArea);
   }

   void goToGoalHorizontal() {
      if(destiny.getGoal()->getPositionGoal().x > position.x) 
         position.move(Position::Direction::Right);
      else if(destiny.getGoal()->getPositionGoal().x < position.x)
         position.move(Position::Direction::Left);
   }

   void goToGoalVertical() {
      if(destiny.getGoal()->getPositionGoal().y > position.y)
         position.move(Position::Direction::Down);
      else if(destiny.getGoal()->getPositionGoal().y < position.y)
         position.move(Position::Direction::Up);
   }

   bool pick(Item* item) { 
      if(item->isUsed()) return false;
      
      item->mutex.lock();  
      setAttachment(item, true); 

      return true;
   }

   void putDown(Item* item) {
      setAttachment(item, false);
      item->mutex.unlock();
   }

   void setAttachment(Item* item, bool isUsed) {
      switch(destiny.getGoalType()) {
         case GoalType::LaserPickaxe:
            flags.picksUp[id] = "4";
            pickaxePocket = item;
            pickaxePocket->setUsing(isUsed);
            break;
         case GoalType::Mine:
            limoniumPocket = item;
            limoniumPocket->setUsing(isUsed);
            break;
         case GoalType::LaserPickaxeArea:
            pickaxePocket->setUsing(isUsed);
            pickaxePocket = nullptr;
            break;
         case GoalType::LimoniumArea:
            limoniumPocket->setUsing(isUsed);
            limoniumPocket->setOnGround(true);
            limoniumPocket = nullptr;
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
         std::this_thread::sleep_for(std::chrono::milliseconds(speed));
         if(!isAtGoal()) {
            findGoal(destiny.getGoalType()); //refresh goal becouse actual goal can be used 
            goToGoal();
         }
         else {
            // gatherer jest przy kilofie
            if(destiny.getGoalType() == GoalType::LaserPickaxe) {
               if(pick(destiny.getItem())) 
                  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                  destiny.setGoalType(GoalType::Mine);
               findGoal(destiny.getGoalType());
            }
            // gatherer jest w kopalni
            else if(destiny.getGoalType() == GoalType::Mine) {
               if(pick(destiny.getItem()))
                  destiny.setGoalType(GoalType::LaserPickaxeArea);
               findGoal(destiny.getGoalType());
            }
            // gatherer jest na terenie kolofow
            else if(destiny.getGoalType() == GoalType::LaserPickaxeArea) {
               putDown(pickaxePocket);
               destiny.setGoalType(GoalType::LimoniumArea);
               findGoal(destiny.getGoalType());
            }
            // gatherer jest na terenie limonium
            else if(destiny.getGoalType() == GoalType::LimoniumArea) {
               putDown(limoniumPocket);
               destiny.setGoalType(GoalType::LaserPickaxe);
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
   int id;
   std::string graphicRepresentation{"W"};
   int speed;
   Position position;
   std::thread lifeThread;

   std::mt19937 mersenne{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
    
   Map* map;

   Destiny destiny;

   Item* pocket{ nullptr };

   void findGoal(GoalType type) {
      switch(type) {
         Goal* nearestGoal;
         case GoalType::Limonium:
            nearestGoal = findTheNearestLimonium();
            destiny.setGoal(nearestGoal);

            if(nearestGoal != nullptr) {
               destiny.setItem(&(*(std::next(map->limoniums.begin(), nearestGoal->getID()))));
            }
            break;
         case GoalType::TransformingArea:
            nearestGoal = findTransformingArea();
            destiny.setGoal(nearestGoal);
            break;
         case GoalType::Transforming:
            nearestGoal = findTransformingArea();
            destiny.setGoal(nearestGoal);
            
            //if(destiny.getItem()->isUsed()) {
               map->wireCounter++;
               destiny.setItem(&(*(std::next(map->wires.begin(), map->wireCounter))));
            //}
            break;
         case GoalType::MetalWireArea:
            nearestGoal = findMetalWireArea();
            destiny.setGoal(nearestGoal);
            break;
         
      }
   }

   Goal* findTheNearestLimonium() {
      for(auto& limonium : map->limoniums) {
         if(limonium.isOnGround() && !limonium.isUsed())
            return &limonium;
      }
      return nullptr;
   }

   Goal* findTransformingArea() {
      return &(map->transformingArea);
   }

   Goal* findMetalWireArea() {
      return &(map->metalWireArea);
   }


   bool isAtGoal() {
      if(destiny.getGoal() == nullptr) return false;

      if(isGoalAnArea()) {
         return (destiny.getGoal()->getPositionGoal().x == position.x);
      }
      else {
         return (destiny.getGoal()->getPositionGoal().x == position.x && 
            destiny.getGoal()->getPositionGoal().y == position.y);
      }
   }

   void goToGoal() {
      goToGoalHorizontal();
      if(!isGoalAnArea())
         goToGoalVertical();      
      
      if(pocket)         
         *(pocket->getPositionItem()) = {position.x, position.y + 1};
   }

   bool isGoalAnArea() {
      return (destiny.getGoalType() == GoalType::MetalWireArea || destiny.getGoalType() == GoalType::TransformingArea || destiny.getGoalType() == GoalType::Transforming);
   }

   void goToGoalHorizontal() {
      if(destiny.getGoal()->getPositionGoal().x > position.x) 
         position.move(Position::Direction::Right);
      else if(destiny.getGoal()->getPositionGoal().x < position.x)
         position.move(Position::Direction::Left);
   }

   void goToGoalVertical() {
      if(destiny.getGoal()->getPositionGoal().y > position.y)
         position.move(Position::Direction::Down);
      else if(destiny.getGoal()->getPositionGoal().y < position.y)
         position.move(Position::Direction::Up);
   }

   bool pick(Item* item) { 
      if(item->isUsed()) return false;
      
      item->mutex.lock();  
      setAttachment(item, true); 

      return true;
   }

   void putDown(Item* item) {
      setAttachment(item, false);
      item->mutex.unlock();
   }

   void setAttachment(Item* item, bool isUsed) {
      switch(destiny.getGoalType()) {
         case GoalType::Limonium:
            pocket = item;
            pocket->setUsing(isUsed);
            break;
         case GoalType::TransformingArea:
            pocket->setUsing(isUsed);
            pocket->setOnGround(isUsed);      
            pocket = nullptr;
            break;
         case GoalType::Transforming:
            pocket = item;
            pocket->setUsing(isUsed);
            break;
         case GoalType::MetalWireArea:
            pocket->setUsing(isUsed);
            pocket->setOnGround(true);
            pocket = nullptr;
            break;
      }
   }

   void setRandomSpeed() {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      static thread_local std::uniform_int_distribution<> range(1, 10);
      speed = 100 * range(mersenne);
   }

public:
   Worker(int id, Map* map) :
      id(id), map(map), lifeThread(&Worker::live, this)
   {
      destiny.setGoalType(GoalType::Limonium);
      findGoal(GoalType::Limonium);

      position = { 40, 20 };

      graphicRepresentation += std::to_string(id);

      setRandomSpeed();      
   }

   ~Worker() {
      lifeThread.join();
   }

   void live() {
      while(!map->ready) {};

      while(map->ready) {
         std::this_thread::sleep_for(std::chrono::milliseconds(speed));
         if(!isAtGoal()) {
            findGoal(destiny.getGoalType()); //refresh goal becouse actual goal can be used 
            if(destiny.getGoal() != nullptr) goToGoal();
         }
         else {
            // worker jest na terenie limonium
            if(destiny.getGoalType() == GoalType::Limonium) {
               if(pick(destiny.getItem())) {
                  destiny.setGoalType(GoalType::TransformingArea);
               }
               findGoal(destiny.getGoalType());
            }
            // worker jest na terenie przerabiania
            else if(destiny.getGoalType() == GoalType::TransformingArea) {
               putDown(destiny.getItem());
               destiny.setGoalType(GoalType::Transforming);
               findGoal(destiny.getGoalType());
            }
            //tutaj sie dzieje przerabianie
            else if(destiny.getGoalType() == GoalType::Transforming) {
               if(pick(destiny.getItem())) {
                  destiny.setGoalType(GoalType::MetalWireArea);
               }
               findGoal(destiny.getGoalType());
            }
            // worker jest na terenie MetalWire
            else if(destiny.getGoalType() == GoalType::MetalWireArea) {
               putDown(destiny.getItem());
               destiny.setGoalType(GoalType::Limonium);
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

// Gets Metal or Wire and carries it to the Spaceship
class Builder {
   int id;
   std::string graphicRepresentation{"B"};
   int speed;
   Position position;
   std::thread lifeThread;

   std::mt19937 mersenne{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
    
   Map* map;

   Destiny destiny;

   Item* pocket{ nullptr };

   void findGoal(GoalType type) {
      switch(type) {
         Goal* nearestGoal;
         case GoalType::Wire:
            nearestGoal = findTheNearestWire();
            destiny.setGoal(nearestGoal);

            if(nearestGoal != nullptr) {
               destiny.setItem(&(*(std::next(map->wires.begin(), nearestGoal->getID()))));
            }
            break;
         case GoalType::Metal:
            nearestGoal = findTheNearestMetal();
            destiny.setGoal(nearestGoal);

            if(nearestGoal != nullptr) {
               destiny.setItem(&(*(std::next(map->metals.begin(), nearestGoal->getID()))));
            }
            break;
         case GoalType::Spaceship:
            nearestGoal = findSpaceship();
            destiny.setGoal(nearestGoal);
            break;
      }
   }

   Goal* findTheNearestMetal() {
      for(auto& metal : map->metals) {
         if(metal.isOnGround() && !metal.isUsed())
            return &metal;
      }
      return nullptr;
   }

   Goal* findTheNearestWire() {
      for(auto& wire : map->wires) {
         if(wire.isOnGround() && !wire.isUsed())
            return &wire;
      }
      return nullptr;
   }

   Goal* findSpaceship() {
      return &(map->spaceship);
   }



   bool isAtGoal() {
      if(destiny.getGoal() == nullptr) return false;

      return (destiny.getGoal()->getPositionGoal().x == position.x && 
            destiny.getGoal()->getPositionGoal().y == position.y);
   }

   void goToGoal() {
      goToGoalHorizontal();
      goToGoalVertical();      
      
      if(pocket)         
         *(pocket->getPositionItem()) = {position.x, position.y + 1};
   }

   void goToGoalHorizontal() {
      if(destiny.getGoal()->getPositionGoal().x > position.x) 
         position.move(Position::Direction::Right);
      else if(destiny.getGoal()->getPositionGoal().x < position.x)
         position.move(Position::Direction::Left);
   }

   void goToGoalVertical() {
      if(destiny.getGoal()->getPositionGoal().y > position.y)
         position.move(Position::Direction::Down);
      else if(destiny.getGoal()->getPositionGoal().y < position.y)
         position.move(Position::Direction::Up);
   }

   bool pick(Item* item) { 
      if(item->isUsed()) return false;
      
      item->mutex.lock();  
      setAttachment(item, true); 

      return true;
   }

   void putDown(Item* item) {
      setAttachment(item, false);
      item->mutex.unlock();
   }

   void setAttachment(Item* item, bool isUsed) {
      switch(destiny.getGoalType()) {
         case GoalType::Metal:
            pocket = item;
            pocket->setUsing(isUsed);
            break;
         case GoalType::Wire:
            pocket = item;
            pocket->setUsing(isUsed);      
            break;
         case GoalType::Spaceship:
            pocket->setUsing(isUsed);
            pocket->setOnGround(isUsed);      
            pocket = nullptr;
            break;
      }
   }

   void setRandomSpeed() {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      static thread_local std::uniform_int_distribution<> range(1, 10);
      speed = 100 * range(mersenne);
   }

public:
   Builder(int id, Map* map) :
      id(id), map(map), lifeThread(&Builder::live, this)
   {
      destiny.setGoalType(GoalType::Wire);
      findGoal(GoalType::Wire);

      position = { 40, 20 };

      graphicRepresentation += std::to_string(id);

      setRandomSpeed();      
   }

   ~Builder() {
      lifeThread.join();
   }

   void live() {  
      while(!map->ready) {};

      while(map->ready) {
         std::this_thread::sleep_for(std::chrono::milliseconds(speed));
         if(!isAtGoal()) {
            findGoal(destiny.getGoalType()); //refresh goal becouse actual goal can be used 
            if(destiny.getGoal() != nullptr) goToGoal();
         }
         else {
            // builder jest na terenie metalwire
            if(destiny.getGoalType() == GoalType::Metal || destiny.getGoalType() == GoalType::Wire) {
               if(pick(destiny.getItem())) {
                  destiny.setGoalType(GoalType::Spaceship);
               }
               findGoal(destiny.getGoalType());
            }
            // builder jest przy rakiecie
            else if(destiny.getGoalType() == GoalType::Spaceship) {
               putDown(destiny.getItem());
               //tutaj niech losowo bierze
               destiny.setGoalType(GoalType::Wire);
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

void beginSimulation() {
   Map map{};
   map.readMapFromFile("/home/adrian/Documents/SpaceStationSimulation/map.txt");
   
   std::array<Builder, 15> builders {
      Builder{ 0, &map },
      Builder{ 1, &map },
      Builder{ 2, &map },
      Builder{ 3, &map },
      Builder{ 4, &map },
      Builder{ 5, &map },
      Builder{ 6, &map },
      Builder{ 7, &map },
      Builder{ 8, &map },
      Builder{ 9, &map },
      Builder{ 10, &map },
      Builder{ 11, &map },
      Builder{ 12, &map },
      Builder{ 13, &map },
      Builder{ 14, &map }
   };
   std::array<Worker, 15> workers {
      Worker{ 0, &map },
      Worker{ 1, &map },
      Worker{ 2, &map },
      Worker{ 3, &map },
      Worker{ 4, &map },
      Worker{ 5, &map },
      Worker{ 6, &map },
      Worker{ 7, &map },
      Worker{ 8, &map },
      Worker{ 9, &map },
      Worker{ 10, &map },
      Worker{ 11, &map },
      Worker{ 12, &map },
      Worker{ 13, &map },
      Worker{ 14, &map } 
   };
   std::array<Gatherer, 15> gatherers {
      Gatherer{ 0, &map },
      Gatherer{ 1, &map },
      Gatherer{ 2, &map },
      Gatherer{ 3, &map },
      Gatherer{ 4, &map },
      Gatherer{ 5, &map },
      Gatherer{ 6, &map },
      Gatherer{ 7, &map },
      Gatherer{ 8, &map },
      Gatherer{ 9, &map },
      Gatherer{ 10, &map },
      Gatherer{ 11, &map },
      Gatherer{ 12, &map },
      Gatherer{ 13, &map },
      Gatherer{ 14, &map }
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
         screen.printLine("Gatherer #" + std::to_string(u - 1) + " : " + std::to_string(gatherer.getPosition().x) + " " + std::to_string(gatherer.getPosition().y), 23 + u++);
      }
      
      for(int x = 0; x < 5; x++) {
         screen.printElement(flags.picksUp[x], x, 30);
      }
      screen.printElement(std::to_string(map.wireCounter), 6, 30);
   
   } 

   screen.printLine("Waiting for all threads to end...", 41); 
   screen.printLine("Screen is not refreshing now.", 42);
}
 
int main() 
{ 	      	
   beginSimulation(); 
				
	return 0; 
}