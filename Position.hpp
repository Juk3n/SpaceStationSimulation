#ifndef Position_H
#define Position_H

struct Position {
   int x = 1;
   int y = 1;

   enum class Direction {
      Left,
      Right,
      Up,
      Down
   };

   // direction are suited to ncurses
   void move(Direction dir) {
      switch(dir) {
         case Direction::Up:
            y--;
            break;
         case Direction::Down:
            y++;
            break;
         case Direction::Left:
            x--;
            break;
         case Direction::Right:
            x++;
            break;
      }
   }
};

#endif Position_H