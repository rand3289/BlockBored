// block bored game
#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <cmath> // M_PI, sin, cos
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include "sdlaudio.h"
extern void pstr(SDL_Renderer* renderer, int x, int y, const std::string& str); // from sdltxt.cpp


class Item {
protected:
    double fx, fy, angle, SPEED;
    bool dead = false;
    friend class Game;
public:
    Item(double x, double y, double speed): fx(x), fy(y), SPEED(speed) { angle = (rand()%(2*31415))/1000.0; }
    bool isDead(){ return dead; }
    void move(){
	fx += SPEED * cos(angle);
        fy += SPEED * sin(angle);
    }
};


class Missle: public Item {
public:
    Missle(double X, double Y, double angleRad): Item(X, Y, 0.02) { angle = angleRad; }
    void draw(SDL_Renderer* rend, SDL_DisplayMode& dm){
        move();
        if(fx < -1.0 || fx > 1.0 || fy < -1.0 || fy > 1.0){ dead = true; }
        int px = fx*dm.w/2+dm.w/2;
        int py = fy*dm.h/2+dm.h/2;
        SDL_RenderDrawPoint(rend, px, py);
    }
};


class Block: public SDL_Rect{
    constexpr static double SPEED = 0.001;
    bool dead = false;
    double fx, fy, angle;
    friend class Game;

    unsigned char r,g,b;
    double size = 0.03;
public:
    Block(double X, double Y, SDL_DisplayMode& dm): fx(X), fy(Y) {
        angle = (rand()%(2*31415))/1000.0;
        r = rand()%230+20;
        g = rand()%230+20;
        b = rand()%230+20;
        w = dm.w/2 * size;
        h = dm.h/2 * size;
    }

    bool collision(double otherx, double othery){ return fx<= otherx && fx+size >= otherx && fy<= othery && fy+size >= othery;  }
    bool isDead(){ return dead; }

    void draw(SDL_Renderer* rend, SDL_DisplayMode& dm){
	fx += SPEED * cos(angle); // make em move
        fy += SPEED * sin(angle);

        if(fx < -1.0 || fx > 1.0-size){ angle = M_PI-angle; } // make em bounce off the walls
        if(fy < -1.0 || fy > 1.0-size){ angle = 2.0*M_PI-angle; }

        SDL_SetRenderDrawColor(rend, r, g, b, SDL_ALPHA_OPAQUE);
        x = fx*dm.w/2+dm.w/2;
        y = fy*dm.h/2+dm.h/2;
        SDL_RenderFillRect(rend, this);
    }
};


class Game {
    constexpr static double SPEED = 0.003;
    double fx = 0.0;
    double fy = 0.0;
    double angle = 0.0;

    bool left  = false;
    bool right = false;
    bool fire  = false;
    std::vector<Block> blocks;
    std::vector<Missle> missles;
public:
    void init(SDL_DisplayMode& dm);
    void shoot(){ fire = true; }
    void ctl(double angleRad, bool leftBtn, bool rightBtn){ angle = angleRad; left = leftBtn; right = rightBtn; }
    void draw(SDL_Renderer* rend, SDL_DisplayMode& dm);
};


void Game::init(SDL_DisplayMode& dm){
    for(int i =0; i < 50; ++i){
        blocks.emplace_back( (rand()%1000-500)/500.0, (rand()%1000-500)/500.0, dm );
    }
}


void Game::draw(SDL_Renderer* rend, SDL_DisplayMode& dm){
    // draw blocks:
    for(Block& b: blocks){
        b.draw(rend, dm);
    }

    blocks.erase(std::remove_if(blocks.begin(), blocks.end(), [](Block& b){ return b.isDead(); }), blocks.end() );

    // draw missles:
    missles.erase(std::remove_if(missles.begin(), missles.end(), [](Missle& m){ return m.isDead(); }), missles.end() );

    if(fire){
        fire = false;
        missles.emplace_back(fx, fy, angle); // current x,y,angle of the ship
    }

    SDL_SetRenderDrawColor(rend, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE); // missles are all same color
    for(Missle& m: missles){
        for(Block& b: blocks){ // detect missle collisions hack
	    if(b.collision(m.fx, m.fy)){
                m.dead = true;
                b.dead = true;
	    }
	}
        m.draw(rend, dm);
    }

    // draw the ship:
    SDL_SetRenderDrawColor(rend, 0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE);
    double s = left  ?  SPEED : 0;
           s = right ? -SPEED : s;

    fx += s * cos(angle);
    fy += s * sin(angle);

    int px = fx*dm.w/2+dm.w/2;
    int py = fy*dm.h/2+dm.h/2;
    SDL_RenderDrawPoint(rend, px, py);
    SDL_RenderDrawPoint(rend, px-1, py-1);
    SDL_RenderDrawPoint(rend, px+1, py+1);
    SDL_RenderDrawPoint(rend, px-1, py+1);
    SDL_RenderDrawPoint(rend, px+1, py-1);

    // draw some debugging stuff:
    SDL_SetRenderDrawColor(rend, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    int vx = 30.0 * cos(angle);
    int vy = 30.0 * sin(angle);
    SDL_RenderDrawLine(rend, 50, 50, vx+50, vy+50);

    std::stringstream ss;
    ss << angle;
    pstr(rend, 10, 10, ss.str());
}


/******************************* END GAME OBJECTS DEFINITIONS ******************/


void toggleFS(SDL_Window* win){
    bool fs = SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN;
    SDL_SetWindowFullscreen(win, fs ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void exitSDLerr(){
    std::cerr << "SDL error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    exit(1);
}

int main(int argc, char* argv[]){
    const int SCREEN_WIDTH = 1024;
    const int SCREEN_HEIGHT = 768;
    const int WINPOS = SDL_WINDOWPOS_CENTERED;

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) { exitSDLerr(); }

    SDL_Window* window=SDL_CreateWindow("block bored", WINPOS, WINPOS, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(0==window){ exitSDLerr(); }
    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
    if(0==renderer){ exitSDLerr(); }

    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP); // SDL_WINDOW_FULLSCREEN for different resolution
    SDL_ShowCursor(SDL_DISABLE);

    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);

    Audio audio;
    std::shared_ptr<AudioData> snd1 = std::make_shared<AudioData>("hamwal1.wav");

    Game game;
    game.init(dm);

    SDL_Event e;
    bool run = true;
    while(run){
        while( SDL_PollEvent( &e ) ){
	    if(e.type == SDL_QUIT){ run=false; }
	    else if(e.type == SDL_KEYDOWN){
                switch(e.key.keysym.sym){
		    case SDLK_ESCAPE:
	            case SDLK_q:      run=false;        break;
                    case SDLK_RETURN: toggleFS(window); SDL_GetCurrentDisplayMode(0, &dm); break;
	            case SDLK_SPACE:  game.shoot(); audio.play(snd1); break;
	        }
            }
	    else if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP){
                int x,y;
	        Uint32 state = SDL_GetMouseState(&x, &y);
		double angle = atan2(dm.h/2-y, dm.w/2-x); // translate (x,y) vector to polar coordinates' angle [-Pi,+Pi]
		bool left = state & SDL_BUTTON(SDL_BUTTON_LEFT);
		bool right = state & SDL_BUTTON(SDL_BUTTON_RIGHT);
		game.ctl(angle, left, right);
	    }
	}

        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_ADD);

	game.draw(renderer, dm);

        SDL_RenderPresent(renderer);
        SDL_Delay( 16 ); // less than 60fps
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
