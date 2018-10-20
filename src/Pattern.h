#ifndef PATTERN_DEF
#define PATTERN_DEF

#include <string>
#include "Wall.h"

class Pattern {

private:
    int m_size;
    int m_maxSize;
    unsigned char* m_wallsID;

public:
    Pattern(int size, int maxSize, std::string str);
    ~Pattern();

    unsigned char getType(int x, int y);

};

#endif
