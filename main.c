#include <stdlib.h>
#include <SDL2/SDL.h>
#include "node.c"
#include "input.c"
#include "settings.c"

#define DELTA_TIME_MILIS ((float)(SDL_GetTicks() - start))

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define RANGE(a, b) (ABS(a) <= b)
#define SIGN(x) (x > 0) - (x < 0)
#define ABS(x) ((signed int)x)*(sign((signed int)x))

typedef struct face {
    float x;
    float y;
} vec2_float;
typedef char face; // (0,0)=0: negative x negative y, (0,1)=1: negative x positive y, (1,0)=2....

typedef struct snake_segment
{
   vec2_float location;
   face facing;
} snake_segment;

bool not_lost = true;
unsigned int start;
node * snake_head; // snake is linked list of snake_segment's

SDL_Event window_event;
SDL_Window *win;
SDL_Renderer * renderer;



void update_game(float dt){
    
}

void delete_end(){
    snake_segment * segment = (snake_segment (*))remove_last(snake_head);
    free(segment);
}


void draw()
{
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_Rect rect = {
        .h = HEIGHT,
        .w = WIDTH,
        .x = 0,
        .y = 0
    };
    SDL_RenderFillRect(renderer, &(rect));
    // // for (int i = 0; i < main_window.width; i++){
    // //     for (int j = 0; j < main_window.height; j++){
    // //         SDL_RenderDrawPoint(renderer, i, j);
    // //     }
    // // }
    // // SDL_RenderPresent(renderer);
    // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    // SDL_Rect b = {
    //     .h = BALL_WIDTH,
    //     .w = BALL_WIDTH,
    //     .x = (int)main_game.b.x + (int)((main_window.width - BALL_WIDTH)*0.5),
    //     .y = (int)main_game.b.y + (int)((main_window.height - BALL_WIDTH)*0.5)
    // };

    // SDL_RenderFillRect(renderer, &b);
    
    // // for (int i = main_game.b.y + ((main_window.height - BALL_WIDTH)*0.5) ;
    // //              i < main_game.b.y + ((main_window.height + BALL_WIDTH)*0.5); i++){
    // //     for (int j = main_game.b.x + ((main_window.width - BALL_WIDTH)*0.5);
    // //              j < main_game.b.x + ((main_window.width + BALL_WIDTH)*0.5); j++){
    // //         // if (range(i - (main_game.b.y + (main_window.height / 2)), BALL_WIDTH) && range(j -(main_game.b.x + (main_window.width / 2)), BALL_WIDTH)){
    // //         //     ch = FULL;
    // //         SDL_RenderDrawPoint(renderer, j, i);
    // //         // }
    // //         // main_window.field[i * main_window.width + j] = ch; // render
    // //     }
    // // }
    // for (int i = (int)main_game.p1.location + (main_window.height*0.5);
    //      i < main_game.p1.location + (main_window.height*0.5) + main_game.p1.length; i++){
    //     for (int j = 0; j < PADDLE_WIDTH; j++){
    //         // main_window.field[(i + main_game.p1.location + (main_window.height / 2)) * main_window.width + main_window.width - 1 ] = FULL;
    //         SDL_RenderDrawPoint(renderer, j, i);
    //     }
    // }
    // for (int i = (int)main_game.p2.location + (main_window.height*0.5);
    //      i < main_game.p2.location + (main_window.height*0.5) + main_game.p2.length; i++){
    //     for (int j = 0; j < PADDLE_WIDTH; j++){
    //         // main_window.field[(i + main_game.p2.location + (main_window.height / 2)) * main_window.width] = FULL;
    //         SDL_RenderDrawPoint(renderer, main_window.width - j, i);
    //     }
    // }
    SDL_RenderPresent(renderer);
}


void init() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &win, &renderer);

    SDL_RenderSetScale(renderer, 1, 1);
    
    snake_head = malloc(sizeof(node));
    

    start = SDL_GetTicks();

    
}
void update(float dt)
{
    if (SDL_PollEvent(&window_event))
    {
        if (window_event.type == SDL_QUIT){
            not_lost = false;
        }
    }
    start = SDL_GetTicks();
    update_game(dt);
    draw();
}

// int SDLMAIN_DECLSPEC SDL_main(int argc, char *argv[])
int main(int argc, char *argv[])
{
    
    char title[] = "snake";
    init();
    free(&snake_head);
    SDL_SetWindowTitle(win, (char (*))&title);
    while (not_lost)
    {
        
        check_press(&window_event);
        float dtime = DELTA_TIME_MILIS;
        if (dtime >= (1000.0/FRAME_RATE)){
            update(dtime);

            sprintf((char (*))&title, "snake    fps: %f\n", (1000.0/dtime));
            SDL_SetWindowTitle(win, (char (*))&title);
        }
        
    }
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
