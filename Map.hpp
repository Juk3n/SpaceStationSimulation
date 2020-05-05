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
   std::vector<Limonium> limoniums;
   std::vector<Metal> metals;
   std::vector<Wire> wires;
   Spaceship spaceship;



   Map() {  
      ready = false; 

      for (size_t i = 0; i < 3; i++) {
         mines[i].position = { 5, static_cast<int>((i + 1) * 4) };
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

   // static Pickable* createLimonium() {
   //    return Limonium{};
   // }

};