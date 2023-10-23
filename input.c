// #include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

bool up   = false;
bool down = false;
bool right   = false;
bool left = false;



void check_press(SDL_Event * event){
    switch (event->type)
    {
    case SDL_KEYDOWN:
        switch (event->key.keysym.scancode)
        {
        case SDL_SCANCODE_UP:
            up = true;
            break;
        case SDL_SCANCODE_DOWN:
            down = true;
            break;
        case SDL_SCANCODE_W:
            up = true;
            break;
        case SDL_SCANCODE_S:
            down = true;
            break;
        case SDL_SCANCODE_D:
            right = true;
            break;
        case SDL_SCANCODE_RIGHT:
            right = true;
            break;
        case SDL_SCANCODE_A:
            left = true;
            break;
        case SDL_SCANCODE_LEFT:
            left = true;
            break;
        default:
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event->key.keysym.scancode)
        {
        case SDL_SCANCODE_UP:
            up = false;
            break;
        case SDL_SCANCODE_DOWN:
            down = false;
            break;
        case SDL_SCANCODE_W:
            up = false;
            break;
        case SDL_SCANCODE_S:
            down = false;
            break;
        case SDL_SCANCODE_D:
            right = false;
            break;
        case SDL_SCANCODE_RIGHT:
            right = false;
            break;
        case SDL_SCANCODE_A:
            left = false;
            break;
        case SDL_SCANCODE_LEFT:
            left = false;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
