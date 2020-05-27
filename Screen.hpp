#pragma once

#include <ncurses.h>

#include "GraphicRepresentation.hpp"
#include "Position.hpp"

#define PLAYER_PAIR 4

class Screen {
   WINDOW* win;
public:
   Screen() {
      initscr();

      start_color();
      init_pair(0, COLOR_WHITE, COLOR_BLACK);	
      init_pair(1, COLOR_RED, COLOR_BLACK);	
      init_pair(2, COLOR_GREEN, COLOR_BLACK);	
      init_pair(3, COLOR_BLUE, COLOR_BLACK);	
      init_pair(4, COLOR_YELLOW, COLOR_BLACK);
      init_pair(5, COLOR_CYAN, COLOR_BLACK);
      init_pair(6, COLOR_MAGENTA, COLOR_BLACK);


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

   void printElement(GraphicRepresentation& graphic, Position& position, int colorNum = 0) {
      wmove(win, position.y, position.x);
      wattron(win,COLOR_PAIR(colorNum));
      waddstr(win, graphic.graphic.c_str());
      wattroff(win,COLOR_PAIR(colorNum));        
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

   void printElement(std::string const & text, int xPos, int yPos, int colorNum = 0) {     
      wmove(win, yPos, xPos);
      wattron(win,COLOR_PAIR(colorNum));
      waddstr(win, text.c_str());
      wattroff(win,COLOR_PAIR(colorNum));        
      wrefresh(win);	
   }

   void printLine(std::string const & text,  int yPos) {
      wmove(win, yPos, 0);
      waddstr(win, text.c_str());        
      wrefresh(win);	
   }
};