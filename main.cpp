//frontline event handling and render setup happen here
#define SDL_MAIN_HANDLED
#include "graphs.h"
#include "files.h"

//gluUnProject is used currently, other utilities may be later.
#include <GL/GLU.h>

//constants for basic 2d camera movement
#define MOVE_STEP (0.05)
#define ZOOM_STEP (1.1)


//function to initialize the display, returns nonzero iff error
static int initializeDisplay();

//main program tick, returns nonzero while program should continue running
static int mainLoop();

//function to refresh the display
static void updateDisplay();

//check if an event is a click on an object, return nonzero if so
static int checkClicks(SDL_Event);
//process events which resize the display, return nonzero if this event did
static int checkResize(SDL_Event);
//process events which move the camera, return nonzero if this event did
static int checkMotion(SDL_Event);
//determine whether this event should end the program
static int checkQuits(SDL_Event);

//convenience function to translate display pixels to world coordinates
static void screenToWorld(double *x, double *y);

//these functions are all static to limit visibility
//they should not need to be used outside of this file
//as such, confine to this translation unit, just as a default

//begin globals 
SDL_Window    *window;
SDL_GLContext  context;

//current width/height of window
int width = 720;
int height = 720;

//current camera information
double centerX = 0.0;
double centerY = 0.0;
double scaleFactor = 10.0;

//all current drawables 
std::vector<Drawable *> objects;
//end globals

int main(int argc, char **argv) {
    if(initializeDisplay()) { return 1; }

    if(argc > 1) {
        //read in specified file
        objects = loadGraph(argv[1]);

    } else {
        //simple hardcoded graph to test basics
        objects.push_back(new GraphNode(5, 0, "First Node"));
        objects.push_back(new GraphNode(5 * 0.707, 5 * 0.707));
        objects.push_back(new GraphNode(0, 5));
        objects.push_back(new GraphNode(-5 * 0.707, 5 * 0.707));
        objects.push_back(new GraphNode(-5, 0));
        objects.push_back(new GraphNode(-5 * 0.707, -5 * 0.707));
        objects.push_back(new GraphNode(0, -5));
        objects.push_back(new GraphNode(5 * 0.707, -5 * 0.707));

        objects.push_back(new GraphEdge((GraphNode *)objects[0], (GraphNode *)objects[1]));
        objects.push_back(((GraphNode *)objects[1])->link((GraphNode *)objects[5]));
    }

    while(mainLoop()) {}
    
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}

static int mainLoop() {
    static SDL_Event e;
    static int redraw = 1;

    if(redraw) {
        updateDisplay();
        redraw = 0;
    }

    while(SDL_PollEvent(&e)) {

        if(checkClicks(e)) { redraw = 1; }

        if(checkResize(e)) { redraw = 1; }

        if(checkMotion(e)) { redraw = 1; }

        if(checkQuits(e)) { return 0; }
    }

    return 1;
}

static int initializeDisplay() {
    if(SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed. Error: %s", SDL_GetError());
        return 1;
    }
    
    window = SDL_CreateWindow("GraphViewer",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              width, height, 
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if(!window) {
        SDL_Log("Window creation failed. Error: %s", SDL_GetError());
        return 1;
    }

    context = SDL_GL_CreateContext(window);
    if(!context) {
        SDL_Log("Context creation failed. Error: %s", SDL_GetError());
        return 1;
    }
    
    glMatrixMode(GL_PROJECTION);
    glClearColor(0.8, 0.8, 0.8, 1.0);
    
    return 0;
}

static void updateDisplay() {
    static double aspectRatio;
    
    //refresh basic display information
    glViewport(0, 0, width, height);
    aspectRatio = ((double) width) / height;

    //refresh camera transformation
    //this is done here as checkResize or checkMotion may both effect a change 
    glLoadIdentity();
    glOrtho(centerX - (aspectRatio * scaleFactor),
            centerX + (aspectRatio * scaleFactor),
            centerY - (scaleFactor),
            centerY + (scaleFactor),
            -1, 1);
    //much of this context-refreshing is, generally, unnecessary
    //however, it should incur minimal performance cost -- many more operations are in the actual drawing
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    for(int i = 0; i < objects.size(); i++) {
        objects[i]->draw();
    }

    //ensure the drawing is actually made visible.
    glFlush();
    SDL_GL_SwapWindow(window);
}

static int checkClicks(SDL_Event e) {
    static GraphNode *n1 = NULL;
    static GraphNode *n2 = NULL;
    if((e.type == SDL_MOUSEBUTTONDOWN)) {
        double x = e.button.x;
        double y = e.button.y;
        screenToWorld(&x, &y);
        
        int i;
        for(i = 0; i < objects.size(); i++) {
            if(objects[i]->onClick(x, y)) {
                //if clicking on two nodes in a row, link them
                if(n1) {
                    n2 = dynamic_cast<GraphNode *>(objects[i]);
                    if(n2) {
                        objects.push_back(n1->link(n2));
                        n1->resetState();
                        n2->resetState();
                        n1 = NULL;
                        n2 = NULL;
                    }
                } else {
                    n1 = dynamic_cast<GraphNode *>(objects[i]);
                }
                
                if(objects[i]->getState() == ExpiredS) {
                    //order of objects need not be preserved
                    //accelerate removal via pulling the last thing to the should-be-empty spot
                    delete objects[i];
                    objects[i] = objects.back();
                    objects.pop_back();
                }
                return 1;
            }
        }

        //clicked nowhere -- deactivate any clicked node
        if(n1) {
            n1->resetState();
            n1 = NULL;
            return 1;
        } else {
            //create a node, if Ctrl active.
            if(SDL_GetModState() & KMOD_CTRL) {
                objects.push_back(new GraphNode(x, y));
                return 1;
            }

        }

    }


    return 0;
}

static int checkResize(SDL_Event e) {
    if((e.type == SDL_WINDOWEVENT) && (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)) {
        SDL_GL_GetDrawableSize(window, &width, &height);
        return 1;
    }
    return 0;
}

static int checkMotion(SDL_Event e) {
    if(e.type == SDL_KEYDOWN) {
        switch(e.key.keysym.sym) {
            case SDLK_w:
            case SDLK_UP:
                centerY += MOVE_STEP * scaleFactor;
                break;
            case SDLK_a:
            case SDLK_LEFT:
                centerX -= MOVE_STEP * scaleFactor;
                break;
            case SDLK_s:
            case SDLK_DOWN:
                centerY -= MOVE_STEP * scaleFactor;
                break;
            case SDLK_d:
            case SDLK_RIGHT:
                centerX += MOVE_STEP * scaleFactor;
                break;
            case SDLK_q:
            case SDLK_KP_MINUS:
                scaleFactor *= ZOOM_STEP;
                break;
            case SDLK_e:
            case SDLK_KP_PLUS:
                scaleFactor /= ZOOM_STEP;
                break;
            default:
                return 0;
                break;
        }
        return 1;
    }
    return 0;
}

static int checkQuits(SDL_Event e) {
    if(e.type == SDL_QUIT) {
        return 1;
    }
    if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        return 1;
    }
    return 0;
}

static void screenToWorld(double *x, double *y) {
    double z = 0;
    double model[16];
    double proj[16];
    int view[4]; 
    //sdl uses top-down measurements, using bottom-up for opengl
    *y = height - *y;

    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);

    gluUnProject(*x, *y, z, model, proj, view, x, y, &z);
}
