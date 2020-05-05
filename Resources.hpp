#pragma once

#include <atomic>
#include <mutex>
#include <vector>

#include "Position.hpp"
#include "GraphicRepresentation.hpp"



class Goal {
protected:
   Position position;
   int ID;
public:
   virtual int setID(int) = 0;
   virtual int getID() = 0;

   virtual Position getPositionGoal() {
      return {-1, -1};
   }
};

class Item {
public:
   std::atomic<bool> isUsed{ false };
   Position position;
   std::mutex mutex;

   virtual Position* getPositionItem() {
      return nullptr;
   }

   virtual void setUsing(bool isUsed) = 0;
};

class Resource {

};

enum class GoalType {
   LaserPickaxe,
   Mine,
   LaserPickaxeArea,
   LimoniumArea
};

enum class ItemType {
   LaserPickaxe,
   Limonium
};

class Mine : public Goal {
public:
   std::mutex mutex;
   Position position;
   GraphicRepresentation graphic{"m"};

   std::atomic<bool> isUsed{ false };

   int setID(int id) override {
      ID = id;
   }

   int getID() override {
      return ID;
   }

   Position getPositionGoal() override {
      return position;
   }
};

class LaserPickaxe : public Goal, public Item {   
public:
   Position position;
   std::atomic<bool> isUsed{ false };
   GraphicRepresentation graphic{"p"};

   int setID(int id) override {
      ID = id;
   }

   int getID() override {
      return ID;
   }

   Position getPositionGoal() override {
      return position;
   }

   Position* getPositionItem() override {
      return &position;
   }

   void setUsing(bool isUs) override {
      isUsed = isUs;
   }
};

class Limonium : public Item {
public:
   Position position;
   GraphicRepresentation graphic{"l"};

   void setUsing(bool isUs) override {
      isUsed = isUs;
   }

   Position* getPositionItem() override {
      return &position;
   }
};

class Metal : public Item {
public:
   std::mutex mutex;
   Position position;
   GraphicRepresentation graphic{"m"};

   void setUsing(bool isUs) override {
      isUsed = isUs;
   }

   Position* getPositionItem() override {
      return &position;
   }
};

class Wire : public Item {
public:
   std::mutex mutex;
   Position position;
   GraphicRepresentation graphic{"w"};

   void setUsing(bool isUs) override {
      isUsed = isUs;
   }

   Position* getPositionItem() override {
      return &position;
   }
};

class Spaceship : public Goal {
public:
   std::vector<std::mutex> workplaces;
   Position position;
   int size{3};
   GraphicRepresentation graphic{"s"};

   int setID(int id) override {
      ID = id;
   }

   int getID() override {
      return ID;
   }

   Position getPositionGoal() override {
      return position;
   }
};

class Destiny {
   Goal* goal;
   Item* item;

   GoalType goalType;
   ItemType itemType;
public:
   Goal* getGoal() {
      return goal;
   }

   void setGoal(Goal* g) {
      goal = g;
   }

   Item* getItem() {
      switch(itemType) {
         case ItemType::LaserPickaxe:
            return item;
            break;
         case ItemType::Limonium:
            //return Map::createLimonium();
            break;
      }
   }

   void setItem(Item *i) {
      item = i;
   }

   GoalType getGoalType() {
      return goalType;
   }

   void setGoalType(GoalType g) {
      goalType = g;
   }

   ItemType getItemType() {
      return itemType;
   }

   void setItemType(ItemType t) {
      itemType = t;
   }
};


