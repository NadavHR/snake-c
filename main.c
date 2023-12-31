#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "node.c"
#include "input.c"
#include "settings.c"


#define DELTA_TIME_MILIS ((float)(SDL_GetTicks() - start))

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define RANGE(a, b) (a < b) //(ABS(a) <= ABS(b))
#define SIGN(x) ((x > 0) - (x < 0))
#define ABS(x) ((signed int)x) * (sign((signed int)x))
#define MOD(a, b) (((a % b) + b) % b)
#define RAND_RANGE(upper, lower)  (MOD(((unsigned short)rand()), ((unsigned short)upper-(unsigned short)lower)) + (unsigned short)lower) // ((rand() % (upper - lower + 1)) + lower)


#define CLEAN_FACE(a) a & 3

typedef struct
{
    unsigned short x;
    unsigned short y;

} vec2_short;

typedef union
{
    unsigned char as_char : 2;
    struct
    {
        bool up : 1;
        bool down : 1;
    };
} face; //[up, down]  -  [1,0]: up = 1, [0,1]: down = 2, [1,1]: left = 3, [0,0]: right = 0,  |   right=~left, up=~down

typedef struct snake_segment
{
    vec2_short location;
    face facing;
} snake_segment;

bool not_lost = true;
bool won = false;
bool is_grace_frame = false; // grace frame is an extra frame given when you are about to die so you could save yourself
unsigned short score = 0;
unsigned int start;
face input_buffer = {.as_char = 0};
node *p_snake_head; // snake is linked list of snake_segment's
vec2_short apple = {
    .x = 0,
    .y = 0
};

SDL_Event window_event;
SDL_Window *win;
SDL_Renderer *renderer;

vec2_short facing_to_vec(face facing)
{
    vec2_short delta = {
        .x = 0,
        .y = 0};
    delta.y -= (SEGMENT_SPACING * ((facing.up) && (!facing.down)));  // up
    delta.y += (SEGMENT_SPACING * ((!facing.up) && (facing.down)));  // down
    delta.x += (SEGMENT_SPACING * ((!facing.up) && (!facing.down))); // right
    delta.x -= (SEGMENT_SPACING * ((facing.up) && (facing.down)));   // left
    return delta;
}
vec2_short grid_pos_to_screen_pos(vec2_short grid_pos){
    vec2_short screen_pos = {
        .x = grid_pos.x * SEGMENT_SPACING,
        .y = grid_pos.y * SEGMENT_SPACING
    };
    return screen_pos;
}
vec2_short screen_pos_to_grid_pos(vec2_short screen_pos){
    vec2_short grid_pos = {
        .x = (screen_pos.x / SEGMENT_SPACING),
        .y = (screen_pos.y / SEGMENT_SPACING) 
     };
     return grid_pos;
}

void spawn_new_apple() {
    vec2_short * full_grid[GRID_WIDTH][GRID_HEIGHT]; 
    vec2_short * positions_pointers = malloc((score+1)*sizeof(vec2_short)); // score+1 to ensure it always allocates enough
    node * p_cur = p_snake_head;
    unsigned short cur_snake_index = 0;

    for (unsigned int i = 0; i < GRID_HEIGHT*GRID_WIDTH; i++){
        full_grid[(i / GRID_HEIGHT)][(i % GRID_HEIGHT)] = NULL;
    }
    vec2_short grid_pos;
    // putts all taken snake segments in the begining of the grid 
    while (p_cur != NULL)
    {
        snake_segment p_cur_seg = *((snake_segment *)(p_cur->p_data));
        grid_pos = screen_pos_to_grid_pos(p_cur_seg.location);
        *(positions_pointers+cur_snake_index) = (vec2_short){.x = cur_snake_index / GRID_HEIGHT,
                                .y = cur_snake_index % GRID_HEIGHT};
        full_grid[grid_pos.x][grid_pos.y] = (positions_pointers+cur_snake_index);
        cur_snake_index++; 
        p_cur = (node *)(p_cur->p_next);
    }
    short random_in_range = RAND_RANGE(cur_snake_index, GRID_HEIGHT*GRID_WIDTH) - 1;// -1 bc this goes from 1 to (GRID_HEIGHT*GRID_WIDTH) instead of from 0 to (GRID_HEIGHT*GRID_WIDTH - 1)
    // printf("%i\n", (full_grid[(random_in_range % GRID_WIDTH)][(random_in_range / GRID_WIDTH)]));

    vec2_short * p_chosen = (full_grid[(random_in_range / GRID_HEIGHT)][(random_in_range % GRID_HEIGHT)]);
    if (p_chosen == NULL) {
        apple.x = (random_in_range / GRID_HEIGHT) * SEGMENT_SPACING;
        apple.y = (random_in_range % GRID_HEIGHT) * SEGMENT_SPACING;
    }
    else{
        while (full_grid[p_chosen->x][p_chosen->y] != NULL)
        {
            p_chosen = full_grid[p_chosen->x][p_chosen->y];
        }
        
        apple.x = (p_chosen->x) * SEGMENT_SPACING;
        apple.y = (p_chosen->y) * SEGMENT_SPACING;
    }
    free(positions_pointers);
    
}


void delete_end()
{
    snake_segment *p_segment = (snake_segment(*))remove_last(p_snake_head);
    free(p_segment);
}

bool check_self_intersect(vec2_short location){
    // check self collision
    node *p_cur = p_snake_head->p_next;
    // snake_segment *p_head_segment = (snake_segment *)p_snake_head->p_data;

    // printf("(%i, %i) -> ", p_head_segment->location.x, p_head_segment->location.y); // TEMP
    while (p_cur->p_next != NULL) // the last segment isnt checked as its supposed to be removed
    {
        vec2_short cur_location = ((snake_segment *)p_cur->p_data)->location;
        // printf("(%i, %i) -> ", cur_location.x, cur_location.y); // TEMP
        if (!memcmp(&cur_location, &location, sizeof(vec2_short)))
        {
            if (is_grace_frame) {
                #if !INVINCIBILITY
                return false; // death
                #endif
            } else {
                is_grace_frame = true;
            }
            
            // printf("death");
            return true;
        }
        p_cur = p_cur->p_next;
    }
    is_grace_frame = false;
    return true;
}

void check_colisions()
{
    snake_segment *p_head_segment = (snake_segment *)p_snake_head->p_data;
    // if the snake exited the screen
    if (!(RANGE(p_head_segment->location.y, HEIGHT) && RANGE(p_head_segment->location.x, WIDTH)))
    {

        // p_head_segment->location.x = WIDTH/2; // for debug
        // p_head_segment->location.y = HEIGHT/2;// for debug
        if (is_grace_frame){

            #if !INVINCIBILITY
            not_lost = false; // death
            #endif
            
        }
        else {
            is_grace_frame = true;
        }
    }
    else
    {

        not_lost = check_self_intersect(p_head_segment->location);
        // printf("\n\n\n"); // TEMP
    }
}

void input_handle(){
    snake_segment * p_old_segment = (snake_segment *)p_snake_head->p_data;
    // if up ^ down --> direction is vertical, else direction is horizontal
    bool last_was_vertical = p_old_segment->facing.up ^ p_old_segment->facing.down;
    // ignore the input in both the direction old_facing and ~old_facing to make it so input in the direction the
    //  snake is already headed in and input back to where you came from is not counted as meaningfull input and get ignored

    up_input = up_input && !last_was_vertical;      // ignore up input if last was vertical
    down_input = down_input && !last_was_vertical;  // ignore down input if last was vertical
    right_input = right_input && last_was_vertical; // ignore right input if last was horizontal
    left_input = left_input && last_was_vertical;   // ignore left input if last was horizontal

    bool was_meaningful_input = up_input || down_input || right_input || left_input;

    // if (up=down=left=right=0) --> new_facing = old_facing
    input_buffer.up = (up_input || left_input) ||                                // if meaningful input in one of them value needs to be true
                               ((!was_meaningful_input) && input_buffer.up);     // if there wasnt any meaningful input put whatever direction the snake was going last
    input_buffer.down = (down_input || (!right_input && left_input)) ||          // if meaningful input in down value needs to be true and if in right false
                                 ((!was_meaningful_input) && input_buffer.down); // if there wasnt any meaningful input put whatever direction the snake was going last
    // printf(was_meaningful_input ? "true\n" : "false\n");
}
void update_game(float dt)
{

    // add a new head
    node *p_new_head = malloc(sizeof(node));
    p_new_head->p_next = p_snake_head;
    const snake_segment *p_old_segment = (snake_segment *)(p_snake_head->p_data); // old segment is used to check where to put new head
    p_snake_head = p_new_head;
    snake_segment *p_new_segment = malloc(sizeof(snake_segment));
    // memcpy(p_new_segment, p_old_segment, sizeof(snake_segment));
    p_new_segment->location = p_old_segment->location;

    p_new_head->p_data = p_new_segment; // update the new head segment to the correct pointer

    vec2_short delta = facing_to_vec(input_buffer);
    p_new_segment->location.y += delta.y;
    p_new_segment->location.x += delta.x;
    p_new_segment->facing = input_buffer;
    // if ((check_self_intersect(p_new_segment->location) && !(input_buffer.as_char == p_old_segment->facing.as_char))) {
    //     p_new_segment->facing = input_buffer;
    // }   
    // else {
    //     p_new_segment->location.y -= delta.y;
    //     p_new_segment->location.x -= deltad.x;
    //     vec2_short delta = facing_to_vec(p_new_segment->facing);
    //     p_new_segment->location.y += delta.y;
    //     p_new_segment->location.x += delta.x;
    // }
    
    // printf(up_input ? "true\n" : "false\n");

    // printf("%i\n", CLEAN_FACE(p_new_segment->facing.as_char));



    

    


    check_colisions();
    if (!is_grace_frame)
    {
        #if APPLE_EAT_BUTTON
        if (apple_eat){
            apple.x = p_new_segment->location.x;
            apple.y = p_new_segment->location.y;                  
        }
        #endif
        // remove tail
        if (!((p_new_segment->location.y == apple.y) && (p_new_segment->location.x == apple.x)))
        {
            delete_end(); 
        }
        else {
            score++;

            if (score + 1 != GRID_HEIGHT*GRID_WIDTH) // score + 1 to turn it from 0 to (GRID_HEIGHT*GRID_WIDTH - 1) to 1 to (GRID_HEIGHT*GRID_WIDTH)
            {
                spawn_new_apple();
            } else {
                won = true;
            }
            
            
            // printf("x: %i, y: %i \n", apple.x, apple.y);
        }
    } else {
        p_snake_head = p_new_head->p_next;
        free(p_new_head);
        free(p_new_segment);
    }
    
    
    
   

    
}

void draw_frame()
{
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

    SDL_RenderFillRect(renderer, NULL);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    node *p_cur = p_snake_head;
    SDL_Rect segment_rect = {
        .h = 2 * SEGMENT_RADIUS + 1,
        .w = 2 * SEGMENT_RADIUS + 1,
        .x = 0,
        .y = 0};

    SDL_Rect connection_rect = {
        .h = SEGMENT_SPACING,
        .w = SEGMENT_SPACING,
        .x = 0,
        .y = 0};
    // draws rest of snake segments and connects them
    while (p_cur->p_next != NULL)
    {
        snake_segment *p_segment = ((snake_segment *)(p_cur->p_data));

        // draw the segment
        segment_rect.x = p_segment->location.x - SEGMENT_RADIUS + (0.5 * SEGMENT_SPACING);
        segment_rect.y = p_segment->location.y - SEGMENT_RADIUS + (0.5 * SEGMENT_SPACING);
        SDL_RenderFillRect(renderer, &(segment_rect));

        // draw connections
        face connection_facing = p_segment->facing;
        connection_facing.as_char = ~connection_facing.as_char;
        vec2_short delta = facing_to_vec(connection_facing);
        connection_rect.x = segment_rect.x;
        connection_rect.y = segment_rect.y;
        connection_rect.h = ((signed short)delta.y) + ((2 * SEGMENT_RADIUS + 1));
        connection_rect.w = ((signed short)delta.x) + ((2 * SEGMENT_RADIUS + 1));
        // printf("\n%i\n", SIGN((signed short) delta.x));
        // printf("%i\n", ((signed short)(2*SEGMENT_RADIUS+1))*SIGN((signed short) delta.x));

        SDL_RenderFillRect(renderer, &connection_rect);

        p_cur = p_cur->p_next;
    }
    // draw last segment indpendently to not draw its connection
    snake_segment *p_segment = ((snake_segment *)(p_cur->p_data));
    segment_rect.x = p_segment->location.x - SEGMENT_RADIUS + (0.5 * SEGMENT_SPACING);
    segment_rect.y = p_segment->location.y - SEGMENT_RADIUS + (0.5 * SEGMENT_SPACING);
    SDL_RenderFillRect(renderer, &(segment_rect));

    // draw apple
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
    SDL_Rect apple_rect = {
        .x = apple.x - APPLE_RADIUS + (0.5 * SEGMENT_SPACING),
        .y = apple.y - APPLE_RADIUS + (0.5 * SEGMENT_SPACING),
        .w = 2*APPLE_RADIUS + 1,
        .h = 2*APPLE_RADIUS + 1
    };
    SDL_RenderFillRect(renderer, &(apple_rect));

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


void init()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &win, &renderer); 

    SDL_RenderSetScale(renderer, 1, 1);

    p_snake_head = malloc(sizeof(node));
    p_snake_head->p_next = NULL;
    snake_segment *p_head_segment = malloc(sizeof(snake_segment));
    p_head_segment->facing = (face){
        .up = 0,
        .down = 0};
    p_head_segment->location = grid_pos_to_screen_pos((vec2_short){
        .x = GRID_WIDTH / 2,
        .y = GRID_HEIGHT / 2});
    p_snake_head->p_data = p_head_segment;

    spawn_new_apple();

    start = SDL_GetTicks();
}
void update(float dt)
{
    
    start = SDL_GetTicks();
    update_game(dt);
    draw_frame();
}

// int SDLMAIN_DECLSPEC SDL_main(int argc, char *argv[])
int main(int argc, char *argv[])
{
    char title[64];
    init();
    // free(p_snake_head);

    SDL_SetWindowTitle(win, (char(*)) & title);

    while (not_lost && (!won))
    {
        if (SDL_PollEvent(&window_event))
        {
            if (window_event.type == SDL_QUIT)
            {
                break;
            }
        }
        check_press(&window_event);
        input_handle();
        // printf("up: %i down: %i left: %i right: %i\n", up_input, down_input, left_input, right_input);
        float dtime = DELTA_TIME_MILIS;

        if (dtime >= (1000.0 / FRAME_RATE))
        {

            update(dtime);

            snprintf((char(*)) & title, 64, "snake    fps: %f    score: %i\n", (1000.0 / dtime), score);
            SDL_SetWindowTitle(win, (char(*)) & title);
        }
        
    }
    if (won){
        printf("you won! \n");
        // win screen
    } else { // lost
        printf("you lost \nscore: %i\n", score);
        // line (*lines)[sizeof(unscaled_win_text)/sizeof(line)] = to_screen_coords(unscaled_win_text, sizeof(unscaled_win_text), (float)WIDTH, (float)HEIGHT);
        // draw_end_screen(*lines, sizeof(*lines));
        // free(lines);
    }
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
