#include "Pattern.h"

Pattern::Pattern(int size, int maxSize, std::vector<int> str): m_size(size), m_maxSize(maxSize) {
    m_wallsID = (int*) calloc(maxSize * maxSize, sizeof(int));
    for(unsigned int i = 0; i < str.size(); i++) {
        m_wallsID[i] = str[i];
    }
}

Pattern::~Pattern() {
    free(m_wallsID);
}

int Pattern::getType(int x, int y) {
    return m_wallsID[y*m_maxSize + x];
}
