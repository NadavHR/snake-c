// #include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

bool up_input    = false;
bool down_input  = false;
bool right_input = false;
bool left_input  = false;


bool apple_eat = false;



void check_press(SDL_Event * event){
    switch (event->type)
    {
    case SDL_KEYDOWN:
        switch (event->key.keysym.scancode)
        {
        case SDL_SCANCODE_UP:
            up_input = true;
            break;
        case SDL_SCANCODE_DOWN:
            down_input = true;
            break;
        case SDL_SCANCODE_W:
            up_input = true;
            break;
        case SDL_SCANCODE_S:
            down_input = true;
            break;
        case SDL_SCANCODE_D:
            right_input = true;
            break;
        case SDL_SCANCODE_RIGHT:
            right_input = true;
            break;
        case SDL_SCANCODE_A:
            left_input = true;
            break;
        case SDL_SCANCODE_LEFT:
            left_input = true;
            break;
        case SDL_SCANCODE_SPACE:
            apple_eat = true;
            break;
        default:
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event->key.keysym.scancode)
        {
        case SDL_SCANCODE_UP:
            up_input = false;
            break;
        case SDL_SCANCODE_DOWN:
            down_input = false;
            break;
        case SDL_SCANCODE_W:
            up_input = false;
            break;
        case SDL_SCANCODE_S:
            down_input = false;
            break;
        case SDL_SCANCODE_D:
            right_input = false;
            break;
        case SDL_SCANCODE_RIGHT:
            right_input = false;
            break;
        case SDL_SCANCODE_A:
            left_input = false;
            break;
        case SDL_SCANCODE_LEFT:
            left_input = false;
            break;
        case SDL_SCANCODE_SPACE:
            apple_eat = false;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
