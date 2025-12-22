#include <chip8.h>
#include <SDL3\SDL.h>
#include <iostream>

int main(){
    Chip8 chip8;
    chip8.load_rom("roms/IBM Logo.ch8");
}