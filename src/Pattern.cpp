#include "Pattern.h"

Pattern::Pattern(int size, int maxSize, std::vector<int> str, int rotation):
    m_size(size), m_maxSize(maxSize), m_rotation(rotation) {

    m_wallsID = (int**) calloc(m_size, sizeof(int*));
    for(int i = 0; i < m_size; i++) {
        m_wallsID[i] = (int*) calloc(m_size, sizeof(int));
    }

    for(int i = 0; i < m_size; i++) {
        for(int j = 0; j < m_size; j++) {
            m_wallsID[i][j] = str[j*maxSize + i];
        }
    }

    for(int k = 0; k < m_rotation; k++) {
        int tmp;
        for (int i = 0; i < m_size / 2; i++)
        {
            for (int j = i; j < m_size - i - 1; j++)
            {
                tmp                                 = m_wallsID[i][j];
                m_wallsID[i][j]                     = m_wallsID[j][m_size-i-1];
                m_wallsID[j][m_size-i-1]            = m_wallsID[m_size-i-1][m_size-j-1];
                m_wallsID[m_size-i-1][m_size-j-1]   = m_wallsID[m_size-j-1][i];
                m_wallsID[m_size-j-1][i]            = tmp;
            }
        }
    }
}

Pattern::~Pattern() {
    for(int i = 0; i < m_size; i++) {
        free(m_wallsID[i]);
    }
    free(m_wallsID);
}

int Pattern::getType(int x, int y) {
    return m_wallsID[x][y];
}

int Pattern::getRotation() {
    return m_rotation;
}
