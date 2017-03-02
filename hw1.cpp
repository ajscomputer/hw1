//modified by Adam Curtis
//date:2-27-17
//purpose:Redo hw1 and learn making gif 
//
//
//cmps3350 Spring 2017 HW1-Redo
//author: Gordon Greisel
//date: 2014 to present
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"
#include <string>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 30000
#define MAX_STARS_PARTICLES 30000
#define MAX_MANA_PARTICLES 50000
#define NUM_BOXES 5
#define TRIANGLES 155
#define GRAVITY 0.3
#define STARS 0.0015
#define rnd() (float)rand() / (float)RAND_MAX
#define CONSOLE_OUT 0

/*extern "C" {
#include "fonts.h"
}*/

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
	//Shape box;
	Shape box[5], circle;
	Shape semicircle[4];
	Particle particle[MAX_PARTICLES];
	Particle stars[MAX_STARS_PARTICLES];
	Particle mana[MAX_MANA_PARTICLES];
	int n, ns;
	int bubbler;
	int mouse[2];
	//set bubbler to 1 for byzanz capture but reset to 0 
	Game() { n=0; ns=0; bubbler=1;}
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
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;

	//declare a circle adapted from
	//codeproject.com/Questions/64657/how-to-draw-a-filled-circle-in-opengl
	//used in render, this is just the declaration
	game.circle.radius = 150;
	game.circle.center.x = 710;
	game.circle.center.y = 0;

	//declare a box shape
	for (int i = 0; i < NUM_BOXES; i++){
		game.box[i].width = 125;
		game.box[i].height = 15;
		game.box[i].center.x =  250 + 5*65 -(i*75);
		game.box[i].center.y = (60*i)+ 500 - 5*60;
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
	cleanup_fonts();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "CMPS3350 HW1-REDO by Adam Curtis");
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
	//from bump for fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void makeParticle(Game *game, int x, int y)
{
	if (game->n >= MAX_PARTICLES)
		return;
	if (CONSOLE_OUT)
		std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	/*p->velocity.y =  rnd() * 0.5 - 0.25;
	  p->velocity.x =  rnd() * 0.5 - 0.25;*/
	p->velocity.y =  rnd() * 0.9 /*- 0.05*/;
	p->velocity.x =  rnd() * 0.9 /*- 0.05*/;
	game->n++;
}

void makeStars(Game *game, int x, int y)
{
	if (game->ns >= MAX_STARS_PARTICLES)
		return;
	//position of stars particle
	Particle *st = &game->stars[game->ns];
	st->s.center.x = x;
	st->s.center.y = y;
	st->velocity.y =  rnd() * 0.03 /*- 0.05*/;
	st->velocity.x =  rnd() * 0.2 /*- 0.05*/;
	game->ns++;
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
			//disable for byzanz capture
			int y = WINDOW_HEIGHT - e->xbutton.y;
			for (int i = 0; i < 10 ; i++) {
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
		if (game->bubbler == 0){
			//to use mouse to set
			game->mouse[0] = savex;
			game->mouse[1] = y;
			//set for byzanz capture
			/*game->mouse[0] = 300;
			game->mouse[1] = 500;*/
		}
		for (int i = 0; i < 20 ; i++){
			//disable for byzanz
			makeParticle(game, e->xbutton.x, y);
		}
		//if (++n < 10)
		//    return;
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_b) {
		//disable toggle for byzanz capture
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
	Particle *p, *st;
	float ra, hx, ky, dist, d0, d1;

	if (game->n <= 0)
		return;
	if (game->bubbler != 0){
		for (int i = 0; i < 20 ; i++){
			//disable for byzanz capture
			makeParticle(game, game->mouse[0], game->mouse[1]);
			//makeParticle(game, 300, 500);
		}
	}
		if (game->ns == 0)
			makeStars(game, 300, 0);
		for (int i = 0; i < 20; i++){
			makeStars(game, 300, 0);
		}

		for(int i = 0; i < game->ns; i++){
			st = &game->stars[i];
			st->velocity.y += STARS;
			if((i%2)==0)
				st->s.center.x += st->velocity.x;
			else
				st->s.center.x -= st->velocity.x;
			st->s.center.y += st->velocity.y;
			if (st->s.center.y > 700.0 && game->ns > 0) {
				game->stars[i] = game->stars[--game->ns];
			}
		}
	for (int i = 0 ; i < game->n ; i++ ){
		p = &game->particle[i];
		p->velocity.y -= GRAVITY;
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;

		//check for collision with shapes...
		for(int j = 0; j < NUM_BOXES; j++){
			Shape *s;
			s = &game->box[j];
			if (p->s.center.y < s->center.y + s->height &&
					p->s.center.x >= s->center.x - s->width &&
					p->s.center.x <= s->center.x + s->width &&
					p->s.center.y > s->center.y - s->height){
				p->s.center.y = s->center.y + s->height;
				p->velocity.y = -p->velocity.y * 0.1f;
				p->velocity.x += 0.002f;
			} 

			//check for off-screen
			if (p->s.center.y < 0.0) {
				if(CONSOLE_OUT)
					std::cout << "off screen" << std::endl;
				game->particle[i] = game->particle[--game->n];
			}
		}
		Shape *s;
		s = &game->circle;
		ra = s->radius;
		hx= s->center.x;
		ky = s->center.y;
		d0 = p->s.center.x - hx;
		d1 = p->s.center.y - ky;
		dist = sqrt(d0*d0 + d1*d1);
		if(dist < ra){
			p->velocity.y = (p->velocity.y/2) + (d1/dist);
			p->velocity.x = (p->velocity.x/2) + (d0/dist);
		}

		//check for off-screen
		if (p->s.center.y < 0.0) {
			if(CONSOLE_OUT)
				std::cout << "off screen" << std::endl;
			game->particle[i] = game->particle[--game->n];
		}
	}
}

void render(Game *game)
{
	float w, h, ra, hx, ky, x, y, manaC, waterC;
	Rect r;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...

	//draw all star particles here
	for(int i = 0; i < game->ns;i++){
		glPushMatrix();
		manaC = rand()%200 + 215;
		//std::cout << "random red = " << red << std::endl;
		glColor3ub(manaC,manaC,manaC);
		Vec *c = &game->stars[i].s.center;
		w = 20;
		h = 20;
		glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}

	//Draw circle
	Shape *s;
	glColor3ub(140,140,150);
	s = &game->circle;
	ra = s->radius;
	hx= s->center.x;
	ky = s->center.y;
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	glBegin(GL_TRIANGLE_FAN);
	for(int i = 0; i < TRIANGLES; i++){
		x = ra * cos(i) - hx;
		y = ra * sin(i) + ky;
		glVertex3f(x+hx, y-ky, 0);

		x = ra * cos(i + 0.1) - hx;
		y = ra * sin(i + 0.1) + ky;
		glVertex3f(x+hx, y-ky, 0);
	}
	glEnd();
	glPopMatrix();


	//draw box
	for(int i = 0; i < NUM_BOXES; i++){
		Shape *s;
		glColor3ub(98,98,150);
		s = &game->box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
		glEnd();
		glPopMatrix();
	}

	//draw all particles here
	for(int i = 0; i < game->n;i++){
		glPushMatrix();
		manaC = rand()%100 + 100;
		waterC = rand()%200 + 255;
		if(1)//make true for now
		glColor3ub(150,100,waterC);
		else if(i%5 == 0)
		glColor3ub(255,255,0);
		else if(i%7 == 0 && i%5!= 0)
		glColor3ub(255,153,0);
		else
		glColor3ub(manaC,0,0);
		Vec *c = &game->particle[i].s.center;
		w = 2;
		h = 4;
		glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}

	glColor3ub(179,89,0);


	//draw fonts
	r.bot = WINDOW_HEIGHT - 155;
	r.left = 500;
	r.center = 0;
	ggprint17(&r, 0, 0x00ff1a2b, "Waterfall Model");
	char phases[5][15] = {"Maintenance","Testing","Coding","Design","Requirements"};
	for(int i = 0; i < 5; i++){
		r.bot = game->box[i].center.y-15;
		r.left = game->box[i].center.x -80;
		ggprint17(&r, 0, 0x00ddaaaa, phases[i]);
	}
}
