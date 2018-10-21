#ifndef PATTERN_DEF
#define PATTERN_DEF

#include <string>
#include <vector>
#include "Wall.h"

class Pattern {

private:
    int m_size;
    int m_maxSize;
    int* m_wallsID;

public:
    Pattern(int size, int maxSize, std::vector<int> str);
    ~Pattern();

    int getType(int x, int y);

};

#endif
