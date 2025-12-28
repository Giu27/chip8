#pragma once

#include <cstdint>
#include <SDL3\SDL.h>
#include <string>
class Chip8{
    private:
        uint8_t RAM[4096]; //4kB of RAM
        uint16_t PC, I; //16 bits Program Counter and Index Register
        uint8_t V[16]; //Vx Registers, V0 through VF

        uint16_t stack[16]; //Stack
        uint8_t sp; //stack pointer

        uint8_t delay_timer, sound_timer; //timers

        uint16_t opcode;

        bool display[32][64]; //display
        bool keystate[16]; // hex keypad
    
    public:
        Chip8();
        void update_surf(SDL_Surface *surface);
        void load_rom(std::string path);
        void set_keypad_value(int index, int val);
        int extract_nibbles(int opcode, int bits, int val_to_binary_and);
        void cycle(bool original);
        void decrement_timers();
};