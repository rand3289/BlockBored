// block bored game
#include <SDL2/SDL.h> // Simple Directmedia Layer lib has to be installed
#include <cmath>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
extern void pstr(SDL_Renderer* renderer, int x, int y, const std::string& str); // from sdltxt.cpp


class Game {
    double angle = 0.0;
    bool left  = false;
    bool right = false;
    bool fire  = false;
//    std::vector<Block> blocks;
//    std::vector<Missle> missles;
public:
    void shoot(){ fire = true; }
    void move(double angleRad, bool leftBtn, bool rightBtn){ angle = angleRad; left = leftBtn; right = rightBtn; }
    void draw(SDL_Renderer* rend, SDL_DisplayMode& dm);
};

void Game::draw(SDL_Renderer* rend, SDL_DisplayMode& dm){
    SDL_SetRenderDrawColor(rend, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE);
    int x = 100.0 * cos(angle);
    int y = 100.0 * sin(angle);
    SDL_RenderDrawLine(rend, dm.w/2, dm.h/2, x+dm.w/2, y+dm.h/2);

    std::stringstream ss;
    ss << "a" << angle << " x" << x << " y" << y; // << " f" << fire? "1": "0";
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
//    SDL_ShowCursor(SDL_DISABLE);

    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);

    Game game;
    SDL_Event e;
    bool run = true;
    while(run){
        while( SDL_PollEvent( &e ) ){
	    if(e.type == SDL_QUIT){ run=false; }
	    else if(e.type == SDL_KEYUP){
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
