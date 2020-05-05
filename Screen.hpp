#pragma once

#include <ncurses.h>

#include "GraphicRepresentation.hpp"
#include "Position.hpp"

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

   void printRecElement(GraphicRepresentation& graphic, Position& position, int size) {
      for(int yLocal = 0; yLocal < size; yLocal++) {
         for(int xLocal = 0; xLocal < size; xLocal++) {
            wmove(win, position.y + yLocal, position.x + xLocal);
            waddstr(win, graphic.graphic.c_str()); 
         }
      }
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