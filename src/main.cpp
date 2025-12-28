#include <iostream>
#include <chip8.h>
#define SDL_MAIN_USE_CALLBACKS 1  
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Surface* surface;
static Chip8 chip8;
static bool original;

int scancode_mask(SDL_Scancode scancode) {
    int index;
    switch (scancode) {
        case SDL_SCANCODE_1: index = 1; break;
        case SDL_SCANCODE_2: index = 2; break;
        case SDL_SCANCODE_3: index = 3; break;
        case SDL_SCANCODE_4: index = 12; break;
        case SDL_SCANCODE_Q: index = 4; break;
        case SDL_SCANCODE_W: index = 5; break;
        case SDL_SCANCODE_E: index = 6; break;
        case SDL_SCANCODE_R: index = 13; break;
        case SDL_SCANCODE_A: index = 7; break;
        case SDL_SCANCODE_S: index = 8; break;
        case SDL_SCANCODE_D: index = 9; break;
        case SDL_SCANCODE_F: index = 14; break;
        case SDL_SCANCODE_Z: index = 10; break;
        case SDL_SCANCODE_X: index = 0; break;
        case SDL_SCANCODE_C: index = 11; break;
        case SDL_SCANCODE_V: index = 15; break;
        default: return 0;
    }
    return index;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("CHIP-8 Interpreter", "0.1", "it.Giu27");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("CHIP-8 Interpreter", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, 64, 32, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    surface = SDL_CreateSurface(64, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) {
    SDL_Log("CreateRGBSurface failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
}
    if (argc == 1) {
        std::cout<<"You must specify the rom path!";
        return SDL_APP_FAILURE;
    }
    chip8.load_rom(argv[1]);
    if (argc > 2){
        if (std::string(argv[2]) == "--original"){
            original = true;
            std::cout<<"Using original interpretation"<<std::endl;
            return SDL_APP_CONTINUE; 
        }
    }
    original = false;
    std::cout<<"Using modern interpretation"<<std::endl;

    return SDL_APP_CONTINUE;  
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;

        case SDL_EVENT_KEY_DOWN:
            chip8.set_keypad_value(scancode_mask(event->key.scancode), true);
            break;
        
        case SDL_EVENT_KEY_UP: 
            chip8.set_keypad_value(scancode_mask(event->key.scancode), false);
            break;
    }
    return SDL_APP_CONTINUE;  
}


SDL_AppResult SDL_AppIterate(void *appstate)
{   
    chip8.cycle(original);

    chip8.update_surf(surface);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (texture == NULL) {
        SDL_Log("CreateTextureFromSurface failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);

    return SDL_APP_CONTINUE; 
}


void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroySurface(surface);
}