//modified by: Adam Curtis

//cs3350 Spring 2017 Homework-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "fonts.h"
#include <string>
#include <GL/glx.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define MAX_BOXES 5
#define MAX_FLOATING_STARS 5000
#define MAX_PARTICLES 10000
#define GRAVITY 0.15
#define rnd() (float)rand() / (float)RAND_MAX
#define TANGLES 180

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

struct Game {
    Shape box[5], circle;
    Particle particle[MAX_PARTICLES];
	Particle stars[MAX_FLOATING_STARS];
    int n, rs;
    int bubbler;
    int mouse[2];
    Game() { n=0; bubbler =0; mouse[2]=0; }
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;
    game.n=0;

	//circle
	game.circle.center.x = 510;
	game.circle.center.y = 0;
	game.circle.radius = 170;

    //declare a box shape
	for (int i = 0; i<MAX_BOXES; i++)
	{
		game.box[i].width = 95;
		game.box[i].height = 19;
		game.box[i].center.x = 120 + i * 65;
		game.box[i].center.y = 500 - i * 60;
	}

    //start animation
    while (!done) {
	while (XPending(dpy)) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    check_mouse(&e, &game);
	    done = check_keys(&e, &game);
	}
	movement(&game);
	render(&game);
	glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "3350 HW1 particles by Adam Curtis");
}

void cleanupXWindows(void)
{
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void)
{
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
	std::cout << "\n\tcannot connect to X server\n" << std::endl;
	exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if (vi == NULL) {
	std::cout << "\n\tno appropriate visual found\n" << std::endl;
	exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
	ButtonPress | ButtonReleaseMask | PointerMotionMask |
	StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
	    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
}

void makeParticle(Game *game, int x, int y)
{
    if (game->n >= MAX_PARTICLES)
	return;
    //std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.x = rnd() * 0.5 - 0.25;
	p->velocity.y = rnd() * 0.5 - 0.25;
	game->n++;
    //std::cout << "Particle amount inc " << game->n << std::endl;
}
void riseStar(Game *game, int x, int y)
{
	if (game->rs >= MAX_FLOATING_STARS)
		return;
	//position of rising stars
	Particle *s = &game->stars[game->rs];
	s->s.center.x = x;
	s->s.center.y = y;
	s->velocity.x = rnd() * 0.2 /*- 0.05*/;
	s->velocity.y = rnd() * 0.03 /*- 0.05*/;
	game->rs++;
}
void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    //static int n = 0;

    if (e->type == ButtonRelease) {
	return;
    }
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    //Left button was pressed
	    int y = WINDOW_HEIGHT - e->xbutton.y;
	    for (int i=0; i<10; i++) {
		makeParticle(game, e->xbutton.x, y);
	    }
	    return;
	}
	if (e->xbutton.button==3) {
	    //Right button was pressed
	    return;
	}
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
	savex = e->xbutton.x;
	savey = e->xbutton.y;
	int y = WINDOW_HEIGHT - e->xbutton.y;
	if(game->bubbler == 0){
	    game->mouse[0] = savex;
	    game->mouse[1] = y;
	}
	for (int i=0; i<5; i++) {
	    makeParticle(game, e->xbutton.x, y);
	}
	//if (++n < 10)
	//	return;
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
	int key = XLookupKeysym(&e->xkey, 0);
	if (key == XK_b) {
	    game->bubbler ^= 1;

	}
	if (key == XK_Escape) {
	    return 1;
	}
	//You may check other keys here.



    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;

    if (game->n <= 0)
	return;
	if (game->bubbler != 0){
		//the bubbler is on
		makeParticle(game, game->mouse[0], game->mouse[1]);
	}
	for (int i = 0; i < game->n; i++) {
		p = &game->particle[i];
		p->velocity.y -= GRAVITY;
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
	}
	//check for collision with shapes...
	for (int b = 0; b < MAX_BOXES; b++){
		Shape *s;
	s = &game->box[b];
		if (p->s.center.y < s->center.y + s->height && 
			p->s.center.x >= s->center.x - s->width &&
			p->s.center.x <= s->center.x + s->width) {	   
			p->s.center.y = s->center.y + s->height;
			p->velocity.y = -p->velocity.y * 0.8f;
			p->velocity.x += 0.05f;
		}
		//check for off screen
		if (p->s.center.y < 0.0) {
			//std::cout << "off screen" << std::endl;
			p[b] = p[--game->n];
			//--game->n;
			//std::cout << "Particle amount dec " << game->n << std::endl;
			// game->n = 0;
		}
    }

}
void render(Game *game)
{
	Rect f;//short for fonts; It is a font rectangle!
	float w, h, xh, yd, rad, x, y, stardust;
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...

	//Draw circle
	Shape *s;
	glColor3ub(90, 140, 90);
	s = &game->circle;
	rad = s->radius;
	xh = s->center.x;
	yd = s->center.y;
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	glBegin(GL_TRIANGLE_FAN);
	for (int t = 0; t < TANGLES; t++){
		x = rad * cos(t) - xh;
		y = rad * sin(t) + yd;
		glVertex3f(x + xh, y - yd, 0);

		x = rad * cos(t + 0.1) - xh;
		y = rad * sin(t + 0.1) + yd;
		glVertex3f(x + xh, y - yd, 0);
	}
	glEnd();
	glPopMatrix();

    //draw box
	for (int db = 0; db < MAX_BOXES; db++){
		Shape *s;
		glColor3ub(5, 10, 15);
		s = &game->box[db];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
		glVertex2i(-w, -h);
		glVertex2i(-w, h);
		glVertex2i(w, h);
		glVertex2i(w, -h);
		glEnd();
		glPopMatrix();
	}
	//draw fonts
	f.bot = WINDOW_HEIGHT - 100;
	f.center = 0;
	f.left = 420;
	ggprint17(&f, 0, 0x00ffaaff, "Water falls,");
	f.bot = WINDOW_HEIGHT - 150;
	f.center = 0;
	f.left = 420;
	ggprint17(&f, 0, 0x00ffaaff, "Stars Rise!");
	char phrase[5][32] = { "MAINTENANCE", "TESTING", "CODING", "DESIGN", "REQUIREMENTS" };
	for (int ph = 0; ph < 5; ph++){
		f.left = game->box[ph].center.x - 50;
		f.bot = game->box[ph].center.y - 25;
		ggprint17(&f, 0, 0x00ffff00, phrase[ph]);
	}


    //draw all particles here
    for (int gp=0; gp < game->n; gp++) {
	glPushMatrix();
	glColor3ub(150,160,220);
	Vec *c = &game->particle[gp].s.center;
	w = 2;
	h = 2;
	glBegin(GL_QUADS);
	glVertex2i(c->x-w, c->y-h);
	glVertex2i(c->x-w, c->y+h);
	glVertex2i(c->x+w, c->y+h);
	glVertex2i(c->x+w, c->y-h);
	glEnd();
	glPopMatrix();
    }
	//draw all stars here
	for (int as = 0; as < game->rs; as++){
		glPushMatrix();
		stardust = rand() % 170 + 69;
		glColor3ub(stardust, stardust, stardust);
		Vec *c = &game->stars[as].s.center;
		w = 1;
		h = 2;
		glBegin(GL_QUADS);
		glVertex2i(c->x + w, c->y + h);
		glVertex2i(c->x + w, c->y - h);
		glVertex2i(c->x - w, c->y - h);
		glVertex2i(c->x - w, c->y + h);
		glEnd();
		glPopMatrix();
	}
}



