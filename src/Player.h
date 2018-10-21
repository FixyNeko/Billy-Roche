#ifndef DEF_PLAYER
#define DEF_PLAYER

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <math.h>
#include "sdlglutils.h"

#ifndef PI
    #define PI 3.14159265359
#endif

class Player{
private:
    float m_x;
    float m_y;
    float m_angle;
    
    GLuint m_textureID;
    GLuint m_vboID;
    GLuint m_vaoID;

public:
    Player();
    Player(Player& playerSource);
    Player(float x, float y, float angle, const char * textureFile);

    float getX();
    float getY();
    float getAngle();
    void move(float x, float y);
    void setAngle(float x, float y);
    void tp(float x, float y);
    void turn(float angle);

    void draw(/*glm::mat4 projection, glm::mat4 modelview*/);

};


#endif