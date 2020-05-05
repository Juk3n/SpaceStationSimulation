#pragma once

#include <thread>

#include "Map.hpp"

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