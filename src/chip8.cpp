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

int Chip8::extract_nibbles(int opcode, int bits, int val_to_binary_and = 0xFFFF){
    return ((opcode & val_to_binary_and) >> bits);
}

void Chip8::cycle(){
    opcode = ((uint16_t)RAM[PC] << 8) | RAM[PC + 1];
    int first_nibble = extract_nibbles(opcode, 12, 0xF000);
    int x = extract_nibbles(opcode, 8, 0x0F00); //Look up a register
    int y = extract_nibbles(opcode, 4, 0x00F0); //Look up a register
    int n = extract_nibbles(opcode, 0, 0x000F); //4 bit number
    int nn = extract_nibbles(opcode, 0, 0x00FF); //8 bit number
    int nnn = extract_nibbles(opcode, 0, 0x0FFF); //12 bit memory address
    PC += 2;
    
    switch(first_nibble){
        case 0x0:
            switch (opcode) {
                case 0x00E0: //Clean the screen
                    memset(display, 0, sizeof(display));
                    break;
                case 0x00EE: //return from the subroutine
                    PC = stack[sp];
                    sp--;
            }
            break;
        case 0x1: //jump to nnn
            PC = nnn;
            break;
        case 0x2: //call a subroutine in nnn
            stack[sp] = PC;
            PC = nnn;
            break;
        case 0x3: //skip next instruction if Vx = nn
            if (V[x] == nn) {
                PC += 2;
            }
            break;
        case 0x4:
            if (V[x] != nn) { //skip next instruction if Vx != nn
                PC += 2;
            }
            break;
        case 0x5:
            if (V[x] == V[y]) { //skip next instruction if Vx = Vy
                PC += 2;
            }
            break;
        case 0x6:
            V[x] = nn;
            break;
        case 0x7:
            V[x] += nn;
            break;
        case 0x8:
            std::cout<<(first_nibble)<<std::endl;
            break;
            std::cout<<(first_nibble)<<std::endl;
        case 0x9: //skip next instruction if Vx != Vy
            if (V[x] != V[y]) {
                PC += 2;
            }
            break;
        case 0xA:
            I = nnn;
            break;
        case 0xB:
            std::cout<<(first_nibble)<<std::endl;
            break;
        case 0xC:
            std::cout<<(first_nibble)<<std::endl;
            break;
        case 0xD: { //Draws to the screen
            V[0xF] = 0;
            for (int i = 0; i < n; i++) {
                uint8_t sprite_byte = RAM[I + i];
    
                int x_coord = V[x] & 63; 
                int y_coord = (V[y] + i) & 31; 

                for (int j = 0; j < 8; j++) {
                    if ((sprite_byte & (0x80 >> j)) != 0) {
                        if (x_coord >= 64) {
                            break; 
                        }

                        if (display[y_coord][x_coord] == true) {
                            display[y_coord][x_coord] = false;
                            V[0xF] = 1; 
                        } else {
                            display[y_coord][x_coord] = true;
                        }
                }   
                x_coord++;
                }
            }
            break;
        }
        case 0xE:
            std::cout<<(first_nibble)<<std::endl;
            break;
        case 0xF:
            std::cout<<(first_nibble)<<std::endl;
            break;
        default:
            std::cout<<"Default";
            break;
    }
}