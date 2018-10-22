#ifndef PATTERN_DEF
#define PATTERN_DEF

#include <string>
#include <vector>
#include "Wall.h"

class Pattern {

private:
    int m_size;
    int m_maxSize;
    int** m_wallsID;
    int m_rotation;

public:
    Pattern(int size, int maxSize, std::vector<int> str, int rotation);
    ~Pattern();

    int getType(int x, int y);
    int getRotation();

};

#endif
