// block bored game
#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
extern void pstr(SDL_Renderer* renderer, int x, int y, const std::string& str); // from sdltxt.cpp


class Missle{
    constexpr static double MISSLE_SPEED = 0.01;
    bool dead = false;
    double fx, fy, angle;
public:
    Missle(double X, double Y, double angleRad): fx(X), fy(Y), angle(angleRad) { }
    bool isDead(){ return dead; }
    void draw(SDL_Renderer* rend, SDL_DisplayMode& dm){
	fx += MISSLE_SPEED * cos(angle);
        fy += MISSLE_SPEED * sin(angle);
        if(fx < -1.0 || fx > 1.0 || fy < -1.0 || fy > 1.0){ dead = true; }

        int px = fx*dm.w/2+dm.w/2;
        int py = fy*dm.h/2+dm.h/2;
        SDL_RenderDrawPoint(rend, px, py);
    }
};


class Block: public SDL_Rect{
    double fx, fy, angle; // SPEED
    unsigned char r,g,b;
public:
    Block(double X, double Y): fx(X), fy(Y) {
        r = rand()%200+50;
        g = rand()%200+50;
        b = rand()%200+50;
        w = 10;
        h = 10;
    }

    void draw(SDL_Renderer* rend, SDL_DisplayMode& dm){ 
        SDL_SetRenderDrawColor(rend, r, g, b, SDL_ALPHA_OPAQUE);
        x = fx*dm.w/2+dm.w/2;
        y = fy*dm.h/2+dm.h/2;
        SDL_RenderFillRect(rend, this);
    }
};


class Game {
    double angle = 0.0;
    bool left  = false;
    bool right = false;
    bool fire  = false;

    constexpr static double SHIP_SPEED = 0.003;
    double x = 0.0;
    double y = 0.0;
    std::vector<Block> blocks;
    std::vector<Missle> missles;
public:
    void init();
    Game(){ init(); }
    void shoot(){ fire = true; }
    void move(double angleRad, bool leftBtn, bool rightBtn){ angle = angleRad; left = leftBtn; right = rightBtn; }
    void draw(SDL_Renderer* rend, SDL_DisplayMode& dm);
};


void Game::init(){
    for(int i =0; i < 50; ++i){
        blocks.emplace_back( (rand()%1000-500)/500.0, (rand()%1000-500)/500.0 );
    }
}


void Game::draw(SDL_Renderer* rend, SDL_DisplayMode& dm){
    // draw blocks:
    for(Block& b: blocks){
        b.draw(rend, dm);
    }

    // draw missles:
    missles.erase(std::remove_if(missles.begin(),missles.end(), [](Missle& m){ return m.isDead(); }), missles.end() );

    if(fire){
        fire = false;
        missles.emplace_back(x, y, angle); // current x,y,angle of the ship
    }

    SDL_SetRenderDrawColor(rend, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE); // missles are all same color
    for(Missle& m: missles){
        m.draw(rend, dm);
    }

    // draw the ship:
    SDL_SetRenderDrawColor(rend, 0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE);
    double s = left  ?  SHIP_SPEED : 0;
           s = right ? -SHIP_SPEED : s;

    x += s * cos(angle);
    y += s * sin(angle);

    int px = x*dm.w/2+dm.w/2;
    int py = y*dm.h/2+dm.h/2;
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

    Game game;
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
	            case SDLK_SPACE:  game.shoot();     break;
	        }
            }
	    else if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP){
                int x,y;
	        Uint32 state = SDL_GetMouseState(&x, &y);
		double angle = atan2(dm.h/2-y, dm.w/2-x); // translate (x,y) vector to polar coordinates' angle [-Pi,+Pi]
		bool left = state & SDL_BUTTON(SDL_BUTTON_LEFT);
		bool right = state & SDL_BUTTON(SDL_BUTTON_RIGHT);
		game.move(angle, left, right);
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
