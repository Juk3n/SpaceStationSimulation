#pragma once

#include <fstream>
#include <iostream>

class myMapFile
{
	std::ifstream file;
public:
	myMapFile(std::string fileName) {
      file.open(fileName);
      
      if (!file)
        //myOutput::displayOnScreen("Nie udalo sie odczytac pliku");
        std::cout << "Nie udalo sie odczytac pliku";
   }

	~myMapFile() {
      file.close();
   }

   int readMapWidth() {
      std::string value;    
      std::getline(file,value);
      return std::stoi(value);
   }

   int readMapHeight() {
      std::string value;      
      std::getline(file,value);
      return std::stoi(value);
   }

	std::string readNextLine() {
      std::string line;
      getline(file,line);
      return line;
   }
};
