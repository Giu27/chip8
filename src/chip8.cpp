#include <chip8.h>
#include <iostream>
#include <iomanip>
#include <fstream>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONT_START_ADDRESS = 0x050;

const unsigned int FONTSET_SIZE = 80;
uint8_t fontset[FONTSET_SIZE] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8(){
    PC = START_ADDRESS; //Initializes the program counter to read the first instruction

    for (int i = 0; i < FONTSET_SIZE; i++){ //Loads the font
        RAM[FONT_START_ADDRESS + i] = fontset[i];
    }

}

void Chip8::update_surf(SDL_Surface *surface) {
	uint32_t* pixels = (uint32_t*)surface->pixels;
    for (int y = 0; y < surface->h; y++) {
        for (int x = 0; x < surface->w; x++) {
            uint32_t color = (this->display[y][x]) ? 0xFFFFFFFF : 0xFF000000;
            pixels[y * (surface->pitch / 4) + x] = color;
        }
    }
}

void Chip8::load_rom(std::string path) { //Reads bytes from the rom and load it in memory
    std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file){
        std::cerr<< "Error loading the ROM!";
        return ;
    }

    std::streamsize size = file.tellg();
    char buffer[size];

    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
	file.close();

    for (unsigned int i = 0; i < size; i++) {
		RAM[START_ADDRESS + i] = buffer[i];
	}
}

void Chip8::set_keypad_value(int index, int val) {
    this->keystate[index] = val;
}
