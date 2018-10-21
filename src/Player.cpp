#include "Player.h"

#ifndef BUFFER_OFFSET

    #define BUFFER_OFFSET(offset) ((char*)NULL + (offset))

#endif

Player::Player(){}

Player::Player(Player& playerSource){}

Player::Player(float x, float y, float angle, const char * textureFile):
    m_x(x), m_y(y), m_angle(angle) {

    m_textureID = loadTexture(textureFile);
    
}

float Player::getX() {
    return m_x;
}

float Player::getY() {
    return m_y;
}

void Player::move(float x, float y) {
    m_x += x;
    m_y += y;
}

void Player::setAngle(float dx, float dy) {
    if(dy == 0 && dx < 0){
        m_angle = 90;
    }
    else if(dy == 0 && dx > 0){
        m_angle = -90;
    }
    else if(dy < 0){
        m_angle = std::atan(dx/dy)*180/PI;
    }
    else{
        m_angle = std::atan(dx/dy)*180/PI + 180;
    }
}

void Player::tp(float x, float y) {
    m_x = x;
    m_y = y;
}

void Player::draw(){

    glPushMatrix();


        gluOrtho2D(-16./9, 16./9, -1, 1);
        glRotatef(m_angle , 0, 0, 1);

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        glBegin(GL_QUADS); /* character drawing */
            glTexCoord2f(0. ,0. );glVertex2f(-0.05, -0.05);
            glTexCoord2f(1. ,0. );glVertex2f(0.05, -0.05);
            glTexCoord2f(1. ,1. );glVertex2f(0.05, 0.05);
            glTexCoord2f(0. ,1. );glVertex2f(-0.05, 0.05);
        glEnd();
        
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
    
    glPopMatrix();
}
