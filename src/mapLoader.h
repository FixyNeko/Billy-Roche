#ifndef MAPLOADER_DEF
#define MAPLOADER_DEF

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <GL/glew.h>
#include "Map.h"
#include "Pattern.h"
#include "TMXParser.h"
#include "TSXParser.h"
#include "sdlglutils.h"

struct mapStruct{
    int id;
    bool up;
    bool down;
    bool left;
    bool right;
};

class mapLoader {
private:
    std::string m_filePath;
    std::string m_basicName;
    std::string m_extension;
    int m_currentLevel;

public:
    static TMX::Parser* tmx;
    static TSX::Parser* tsx;

    mapLoader(std::string filePath, std::string basicName, std::string extension);
    ~mapLoader();
    Map * nextLevel();
    static Map * readFromFile(std::string filePath, std::string fileName, int mapWidth, int mapHeight);
    static bool isAvailable(int x, int y, int width, int height, int size, mapStruct* tiles);
    static bool isConnecting(int x, int y, int width, int height, int size, mapStruct* tiles, bool* isWall, Pattern* pattern);
};

#endif
