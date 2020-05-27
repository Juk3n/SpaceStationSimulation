#pragma once

#include <atomic>
#include <vector>
#include <string>
#include <array>

#include "Resources.hpp"
#include "MapFile.hpp"

class Map {
public:
   std::atomic<bool> ready;

   int width;
   int height;

   std::vector<std::string> mapGraphics;

   std::array<Mine, 3> mines;
   std::array<LaserPickaxe, 10> laserPickaxes;
   std::array<Limonium, 1000> limoniums;
   int limoniumCounter{};

   std::array<Metal, 1000> metals;
   int metalCounter{};

   std::array<Wire, 1000> wires;
   int wireCounter{};

   Spaceship spaceship;
   LaserPickaxeArea laserPickaxeArea;
   LimoniumArea limoniumArea;
   MetalWireArea metalWireArea;
   TransformingArea transformingArea;

   Map() {  
      ready = false; 

      for (size_t i = 0; i < 3; i++) {
         mines[i].position = { 5, static_cast<int>((i + 1) * 4) };
      }

      laserPickaxeArea.position = { 20, 1 };
      limoniumArea.position = { 30, 1 };
      transformingArea.position = { 45, 1 };
      metalWireArea.position = {  55, 1 };
      

      for (size_t i = 0; i < 1000; i++) {
         limoniums[i].setID(i);
         wires[i].setID(i);
         metals[i].setID(i);
      }
      

      for (size_t i = 0; i < 10; i++) {
         laserPickaxes[i].position = { 15, static_cast<int>((i + 1) * 2) };
         laserPickaxes[i].setID(i);
      } 

      spaceship.position = Position{ 85, 11 }; 
   }

   void readMapFromFile(std::string fileName) {
      myMapFile file(fileName);
      width = file.readMapWidth();
      height = file.readMapHeight();
      for(int i = 0; i < height; i++)
         mapGraphics.push_back(file.readNextLine());
   }

   Item* createLimonium() {
      return &(*(std::next(limoniums.begin(), limoniumCounter)));
   }
};