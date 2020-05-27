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
   std::atomic<bool> used{ false };
   std::atomic<bool> onGround{ false };
   Position position;
   std::mutex mutex;

   virtual Position* getPositionItem() {
      return nullptr;
   }

   virtual bool isOnGround() = 0;
   virtual void setOnGround(bool isGround) = 0;
   virtual bool isUsed() = 0;
   virtual void setUsing(bool isUsed) = 0;
};

class Resource {

};

enum class GoalType {
   LaserPickaxe,
   Mine,
   LaserPickaxeArea,
   LimoniumArea,
   MetalWireArea,
   Limonium,
   Transforming,
   TransformingArea,
   Spaceship,
   Metal,
   Wire
};

enum class ItemType {
   LaserPickaxe,
   Limonium,
   Metal,
   Wire
};

class Rectangle {
public:
   int width;
   int height;
   Position position;
};

class Area : public Rectangle, public Goal {
public: 
   
};

class LaserPickaxeArea : public Area {
public:
   Position position;

   LaserPickaxeArea() {
      width = 3;
      height = 20;
   }

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

class LimoniumArea : public Area {  
public:
   Position position;

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

class MetalWireArea : public Area {
public:
   Position position;

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

class TransformingArea: public Area {
public:
   Position position;

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



class Mine : public Goal {
public:
   std::mutex mutex;
   Position position;
   GraphicRepresentation graphic{"m"};

   std::atomic<int> isUsed{ 0 };

   int setID(int id) override {
      ID = id;
   }

   int getID() override {
      return ID;
   }

   Position getPositionGoal() override {
      return position;
   }

   bool used() {
      return isUsed;
   }
};

class LaserPickaxe : public Goal, public Item {   
public:
   Position position;
   std::atomic<bool> used{ false };
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

   bool isUsed() override {
      return used;
   }

   void setUsing(bool isUs) override {
      used = isUs;
   }

   bool isOnGround() override {
      return onGround;
   }

   void setOnGround(bool isGround) override {
      onGround = isGround; 
   }
};

class Limonium : public Goal, public Item {
public:
   Position position;
   std::atomic<bool> used{ false };
   GraphicRepresentation graphic{"l"};

   int setID(int id) override {
      ID = id;
   }

   int getID() override {
      return ID;
   }

   Position getPositionGoal() override {
      return position;
   }

   bool isUsed() override {
      return used;
   }

   void setUsing(bool isUs) override {
      used = isUs;
   }

   bool isOnGround() override {
      return onGround;
   }

   void setOnGround(bool isGround) override {
      onGround = isGround; 
   }

   Position* getPositionItem() override {
      return &position;
   }
};

class Metal : public Goal, public Item {
public:
   std::mutex mutex;
   Position position;
   GraphicRepresentation graphic{"m"};

   int setID(int id) override {
      ID = id;
   }

   int getID() override {
      return ID;
   }

   Position getPositionGoal() override {
      return position;
   }

   bool isUsed() override {
      return used;
   }

   void setUsing(bool isUs) override {
      used = isUs;
   }

   bool isOnGround() override {
      return onGround;
   }

   void setOnGround(bool isGround) override {
      onGround = isGround; 
   }

   Position* getPositionItem() override {
      return &position;
   }
};

class Wire : public Goal, public Item {
public:
   std::mutex mutex;
   Position position;
   GraphicRepresentation graphic{"w"};
   std::atomic<bool> used{ false };

   int setID(int id) override {
      ID = id;
   }

   int getID() override {
      return ID;
   }

   Position getPositionGoal() override {
      return position;
   }

   bool isUsed() override {
      return used;
   }

   void setUsing(bool isUs) override {
      used = isUs;
   }

   bool isOnGround() override {
      return onGround;
   }

   void setOnGround(bool isGround) override {
      onGround = isGround; 
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
      return item;
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


