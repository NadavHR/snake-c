#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "node.c"
#include "input.c"
#include "settings.c"

#define DELTA_TIME_MILIS ((float)(SDL_GetTicks() - start))

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define RANGE(a, b) (a <= b)//(ABS(a) <= ABS(b)) 
#define SIGN(x) ((x > 0) - (x < 0)) 
#define ABS(x) ((signed int)x)*(sign((signed int)x))
#define MOD(a,b) ((a%b)+b)%b

#define CLEAN_FACE(a) a & 3

typedef struct {
    signed short int x;
    signed short int y;
    
} vec2_int;


typedef union { 
    unsigned char as_char : 2;
    struct {
        bool up : 1;
        bool down : 1;
    }; 
} face; //[up, down]  -  [1,0]: up = 1, [0,1]: down = 2, [1,1]: left = 3, [0,0]: right = 0,  |   right=~left, up=~down 

typedef struct snake_segment
{
   vec2_int location;
   face facing;
} snake_segment;

bool not_lost = true;
unsigned int start;
node * p_snake_head; // snake is linked list of snake_segment's

SDL_Event window_event;
SDL_Window *win;
SDL_Renderer * renderer;


vec2_int facing_to_vec(face facing){
    vec2_int delta = {
        .x = 0,
        .y = 0
    };
    delta.y -= (SEGMENT_SPACING*((facing.up) && (!facing.down))); // up
    delta.y += (SEGMENT_SPACING*((!facing.up) && (facing.down))); // down
    delta.x += (SEGMENT_SPACING*((!facing.up) && (!facing.down))); // right
    delta.x -= (SEGMENT_SPACING*((facing.up) && (facing.down))); // left
    return delta;
}

void delete_end(){
    snake_segment * p_segment = (snake_segment (*))remove_last(p_snake_head);
    free(p_segment);
}

void check_colisions(){
    snake_segment * p_head_segment = (snake_segment *)p_snake_head->p_data;
    // if the snake exited the screen
    if (! (RANGE(p_head_segment->location.y, HEIGHT) && RANGE(p_head_segment->location.x, WIDTH)))
    {
        
        // p_head_segment->location.x = WIDTH/2; // for debug
        // p_head_segment->location.y = HEIGHT/2;// for debug

        not_lost = false; // death
    } else {
        // check self collision
        node * p_cur = p_snake_head->p_next;
        // printf("(%i, %i) -> ", p_head_segment->location.x, p_head_segment->location.y); // TEMP
        while (p_cur != NULL)
        {
            vec2_int cur_location = ((snake_segment *)p_cur->p_data)->location;
            // printf("(%i, %i) -> ", cur_location.x, cur_location.y); // TEMP
            if (!memcmp(&cur_location, &p_head_segment->location, sizeof(vec2_int))){
                not_lost = false; // death
                // printf("death");
                break;
            }
            p_cur = p_cur->p_next;
        }
        // printf("\n\n\n"); // TEMP 
        
    }
}

void update_game(float dt){
    
    // add a new head 
    node * p_new_head = malloc(sizeof(node));
    p_new_head->p_next = p_snake_head;
    const snake_segment * p_old_segment = (snake_segment *)(p_snake_head->p_data); // old segment is used to check where to put new head
    p_snake_head = p_new_head; 
    snake_segment * p_new_segment = malloc(sizeof(snake_segment));
    // memcpy(p_new_segment, p_old_segment, sizeof(snake_segment));
    p_new_segment->location = p_old_segment->location;
    
    p_new_head->p_data = p_new_segment; // update the new head segment to the correct pointer
    
    // printf(up_input ? "true\n" : "false\n");
    
    
    // if up ^ down --> direction is vertical, else direction is horizontal
    bool last_was_vertical = p_old_segment->facing.up ^ p_old_segment->facing.down;
    // ignore the input in both the direction old_facing and ~old_facing to make it so input in the direction the 
    //  snake is already headed in and input back to where you came from is not counted as meaningfull input and get ignored
    up_input = up_input && !last_was_vertical; // ignore up input if last was vertical
    down_input = down_input && !last_was_vertical; // ignore down input if last was vertical
    right_input = right_input && last_was_vertical; // ignore right input if last was horizontal
    left_input = left_input && last_was_vertical; // ignore left input if last was horizontal

    bool was_meaningful_input = up_input || down_input || right_input || left_input;

    // if (up=down=left=right=0) --> new_facing = old_facing
    p_new_segment->facing.up   = (up_input || left_input) || // if meaningful input in one of them value needs to be true
        ((!was_meaningful_input) && p_old_segment->facing.up); // if there wasnt any meaningful input put whatever direction the snake was going last
    p_new_segment->facing.down = (down_input || (!right_input && left_input)) ||// if meaningful input in down value needs to be true and if in right false
        ((!was_meaningful_input) && p_old_segment->facing.down); // if there wasnt any meaningful input put whatever direction the snake was going last
    // printf(was_meaningful_input ? "true\n" : "false\n");
    // printf("%i\n", CLEAN_FACE(p_new_segment->facing.as_char));

    // move head
    vec2_int delta = facing_to_vec(p_new_segment->facing);
    p_new_segment->location.y += delta.y;
    p_new_segment->location.x += delta.x;
    
    // remove tail
    delete_end();// TODO: make it conditional to if eaten apple
    
    check_colisions();
    
}



void draw()
{
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    
    SDL_RenderFillRect(renderer, NULL);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    node * p_cur = p_snake_head;
    SDL_Rect segment_rect = {
        .h = 2*SEGMENT_RADIUS+1,
        .w = 2*SEGMENT_RADIUS+1,
        .x = 0,
        .y = 0
    };

    SDL_Rect connection_rect = {
        .h = SEGMENT_SPACING,
        .w = SEGMENT_SPACING,
        .x = 0,
        .y = 0
    };
    // draws rest of snake segments and connects them
    while (p_cur->p_next != NULL)
    {   
        snake_segment * p_segment = ((snake_segment *)(p_cur->p_data));

        // draw the segment
        segment_rect.x = p_segment->location.x - SEGMENT_RADIUS; 
        segment_rect.y = p_segment->location.y - SEGMENT_RADIUS;
        SDL_RenderFillRect(renderer, &(segment_rect));

        // draw connections
        face connection_facing = p_segment->facing;
        connection_facing.as_char = ~connection_facing.as_char;
        vec2_int delta = facing_to_vec(connection_facing);
        connection_rect.x = segment_rect.x;
        connection_rect.y = segment_rect.y; 
        connection_rect.h = ((signed short)delta.y) + ((2*SEGMENT_RADIUS+1));
        connection_rect.w = ((signed short)delta.x) + ((2*SEGMENT_RADIUS+1));
        // printf("\n%i\n", SIGN((signed short) delta.x));
        // printf("%i\n", ((signed short)(2*SEGMENT_RADIUS+1))*SIGN((signed short) delta.x));

        SDL_RenderFillRect(renderer, &connection_rect);

        p_cur = p_cur->p_next;
        
    }
    // draw last segment indpendently to not draw its connection
    snake_segment * p_segment = ((snake_segment *)(p_cur->p_data));
    segment_rect.x = p_segment->location.x - SEGMENT_RADIUS; 
    segment_rect.y = p_segment->location.y - SEGMENT_RADIUS;
    SDL_RenderFillRect(renderer, &(segment_rect));

    
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
    
    p_snake_head = malloc(sizeof(node));
    p_snake_head->p_next=NULL;
    snake_segment * p_head_segment = malloc(sizeof(snake_segment));
    p_head_segment->facing = (face){
        .up = 0,
        .down = 0
    };
    p_head_segment->location = (vec2_int){
        .x = WIDTH/2,
        .y = HEIGHT/2};
    p_snake_head->p_data = p_head_segment;

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
    // free(p_snake_head);
    
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
    SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
    SDL_Quit();
    return 0;
}
