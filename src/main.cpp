#include "main.h"

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 900

bool noclip = false;

bool msaa = false;
int vsync = 0;
int fps = 0;
unsigned int lastTime, startTime, timePassed;
SDL_Window *screen = NULL;
SDL_GLContext glcontext = NULL;

void draw(SDL_Window *);
void SDLGLSetup();
void glSetup();
void prepareVBO();
void CheckVBO();
bool GetMonitorInfo(int nDeviceIndex, LPSTR lpszMonitorInfo);
GLenum err;

Shader * occlusionShader;
Shader * drawShader;

Player * player;

GLuint occlusionTexture;
GLuint occlusionDepthTexture;
GLuint occlusionFramebuffer;

mapLoader * ml;

float nearr = 0.1f;
float farr = 400.f;

float scl = 50;
Map * map;


int main(int argc, char *argv[])
{
    SDLGLSetup();


    const GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cout << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        exit(1);
    }

    glSetup();

    std::cout << glGetString(GL_VENDOR) << std::endl << glGetString(GL_RENDERER) << std::endl;

    prepareVBO();

    ml = new mapLoader(std::string("resources/maps/"), std::string("mine"), std::string(".tmx"));

    player = new Player(0, 0, 0, "resources/textures/geologue.png");

    try {
        map = ml->nextLevel();
        player->tp(map->getEntryCoordX(), map->getEntryCoordY());
    } catch(const std::exception& e) {
        std::cout << "Whoops! Something got wrong! "<< e.what() << std::endl;
        exit(1);
    }

    occlusionShader = new Shader(std::string("Shaders/occlusion3D.vert"), std::string("Shaders/occlusion3D.frag"));
    drawShader = new Shader(std::string("Shaders/shader2D.vert"), std::string("Shaders/shader2D.frag"));

    occlusionShader->charger();
    drawShader->charger();

    SDL_Event event;
    bool terminate = false;
    bool loadLevel = true;

    lastTime = SDL_GetTicks();

    while(!terminate){

        lastTime = startTime;
        startTime = SDL_GetTicks();
        timePassed = startTime - lastTime;

        SDL_PollEvent(&event);
        
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        const double speed = 0.4;

        double dx = 0, dy = 0;
        double movement = timePassed * speed;

        if (state[SDL_SCANCODE_ESCAPE])
            terminate = true;
        if (state[SDL_SCANCODE_W])
            if(map->getWall(player->getX(), player->getY() - movement)->isFloor() || noclip)
                dy -= 1;
        if (state[SDL_SCANCODE_S])
            if(map->getWall(player->getX(), player->getY() + movement)->isFloor() || noclip)
                dy += 1;
        if (state[SDL_SCANCODE_A])
            if(map->getWall(player->getX() - movement, player->getY())->isFloor() || noclip)
                dx -= 1;
        if (state[SDL_SCANCODE_D])
            if(map->getWall(player->getX() + movement, player->getY())->isFloor() || noclip)
                dx += 1;
        if (state[SDL_SCANCODE_SPACE]) {
            if(loadLevel) {
                map = ml->nextLevel();
                player->tp(map->getEntryCoordX(), map->getEntryCoordY());
                loadLevel = false;
            }
        }
        else loadLevel = true;

        /* player movement*/
        dx = (dx*dx + dy*dy > 1)? dx / sqrt(2.) : dx;
        dy = (dx*dx + dy*dy > 1)? dy / sqrt(2.) : dy;
        dx *= movement;
        dy *= movement;
        if(dx != 0 || dy != 0) {
            player->move(dx, dy);
            player->setAngle(dx, dy);
        }

        if(map->isOnExitTile(player->getX(), player->getY())) {
            map = ml->nextLevel();
            player->tp(map->getEntryCoordX(), map->getEntryCoordY());
        }

        try {
            draw(screen);
        }
        catch (const std::exception& e) {
            std::cout << "Draw Exception: " << e.what() <<std::endl;
        }

        if(fps != 0) {
            int sleep = 1000./fps - (SDL_GetTicks() - startTime);
            if(sleep > 0)
                SDL_Delay(sleep);
        }
    }


    return 0;
}



void draw(SDL_Window * screen) {
    float fovx = glm::radians(90.f);
    float fovy = 2*atan(((float)1/WINDOW_WIDTH) * tan(fovx/2));
    glm::mat4 projection = glm::perspectiveFov(fovy, (float)WINDOW_WIDTH, (float)1, nearr, farr);
    glm::mat4 modelview = glm::mat4(1.);
    
    //Generate occlusion map in occlusionTexture
    glUseProgram(occlusionShader->getProgramID());

        glBindFramebuffer(GL_FRAMEBUFFER, occlusionFramebuffer);
        

            glClearColor(1,1,1,1);
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            

            for(int i = 0; i < 4; i++){ // draw from a cube camera
                glViewport(WINDOW_WIDTH * i, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

                
                modelview = glm::lookAt(glm::vec3(  player->getX(),
                                                    player->getY(),
                                                    0), 
                                        glm::vec3(  player->getX() + cos((i)*glm::two_pi<float>() / 4),
                                                    player->getY() - sin((i)*glm::two_pi<float>() / 4),
                                                    0.),
                                        glm::vec3(0., 0., 1.));


                glUniformMatrix4fv(glGetUniformLocation(occlusionShader->getProgramID(), "projection"), 1, GL_FALSE, &projection[0][0]);
                glUniformMatrix4fv(glGetUniformLocation(occlusionShader->getProgramID(), "modelview"), 1, GL_FALSE, &modelview[0][0]);

                    glUniform1f(glGetUniformLocation(occlusionShader->getProgramID(), "near"), nearr);
                    glUniform1f(glGetUniformLocation(occlusionShader->getProgramID(), "far"), farr);
    
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
    
                map->drawOcc(player->getX(), player->getY());
    
                glCullFace(GL_FRONT);
                glDisable(GL_CULL_FACE);
    
                //draw scene with only walls
            }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(0);

    projection = glm::ortho(-WINDOW_WIDTH / 2.f, WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f, -WINDOW_HEIGHT / 2.f);
    modelview = glm::mat4(1.0f);
    modelview = glm::translate(modelview, glm::vec3(-player->getX(), -player->getY(), 0));
    
    glUseProgram(drawShader->getProgramID());

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        glClearColor(0,0,0,1);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        glUniformMatrix4fv(glGetUniformLocation(drawShader->getProgramID(), "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(drawShader->getProgramID(), "modelview"), 1, GL_FALSE, &modelview[0][0]);
        /* one time needed */
            glUniform1f(glGetUniformLocation(drawShader->getProgramID(), "scl"), scl);
            glUniform1f(glGetUniformLocation(drawShader->getProgramID(), "near"), nearr);
            glUniform1f(glGetUniformLocation(drawShader->getProgramID(), "far"), farr);
            glUniform1i(glGetUniformLocation(drawShader->getProgramID(), "width"), WINDOW_WIDTH);
            glUniform1i(glGetUniformLocation(drawShader->getProgramID(), "height"), WINDOW_HEIGHT);
        /* one time needed */

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, occlusionTexture); // bind 1

            map->draw();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0); // unbind 1
/*
        while((err = glGetError()) != GL_NO_ERROR)
            std::cout << "Error drawing: " << gluErrorString(err) << std::endl;
*/
    glUseProgram(0);

    glDisable(GL_DEPTH_TEST);

    player->draw();
    
    glEnable(GL_DEPTH_TEST);

    SDL_GL_SwapWindow(screen);
}

void SDLGLSetup(){
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    GLint * samples = (GLint *) calloc(1, sizeof(GLint));
    glGetIntegerv(GL_MAX_SAMPLES, samples);

    if(msaa){
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, *samples);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

    screen = SDL_CreateWindow("SDL GL Application",
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                WINDOW_WIDTH, WINDOW_HEIGHT,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    glcontext = SDL_GL_CreateContext(screen);

    if(!glcontext)
        throw std::string("Error creating opengl context");

}

void glSetup() {

    SDL_GL_SetSwapInterval(vsync);

    if(msaa)
        glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void prepareVBO(){
    //occlusion texture/framebuffer generation
    glGenFramebuffers(1, &occlusionFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, occlusionFramebuffer);
    
        glGenTextures(1, &occlusionTexture);

        glBindTexture(GL_TEXTURE_2D, occlusionTexture); /* color texture init */

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WINDOW_WIDTH * 4, 1, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, 0);
                
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            
        glBindTexture(GL_TEXTURE_2D, 0);
            
        glGenTextures(1, &occlusionDepthTexture);

        glBindTexture(GL_TEXTURE_2D, occlusionDepthTexture); /* depth texture init */

            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, WINDOW_WIDTH * 4, 1, 0,
                GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, occlusionTexture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, occlusionDepthTexture, 0);
        GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, DrawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while((err = glGetError()) != GL_NO_ERROR){
        std::cout << "prepare error: " << std::hex << "0x" << err << std::endl;
    }
}

bool GetMonitorInfo(int nDeviceIndex, LPSTR lpszMonitorInfo) {
    bool bResult = TRUE;
    EDDType EnumDisplayDevices;
    HINSTANCE  hInstUserLib;
    DISPLAY_DEVICE DispDev;
    char szDeviceName[32];

    hInstUserLib = LoadLibrary("User32.DLL");

    EnumDisplayDevices = (EDDType) GetProcAddress(hInstUserLib,
                                                 "EnumDisplayDevicesA");
    if(!EnumDisplayDevices) {
        FreeLibrary(hInstUserLib);
        return FALSE;
    }

    ZeroMemory(&DispDev, sizeof(DISPLAY_DEVICE));
    DispDev.cb = sizeof(DISPLAY_DEVICE);

    // After first call to EnumDisplayDevices DispDev.DeviceString 
    //contains graphic card name
    if(EnumDisplayDevices(NULL, nDeviceIndex, &DispDev, 0)) {
        lstrcpy(szDeviceName, DispDev.DeviceName);

        // after second call DispDev.DeviceString contains monitor's name 
        EnumDisplayDevices(szDeviceName, 0, &DispDev, 0);

        lstrcpy(lpszMonitorInfo, DispDev.DeviceString);
    }
    else {
        bResult = FALSE;
    }

    FreeLibrary(hInstUserLib);

    return bResult;
}