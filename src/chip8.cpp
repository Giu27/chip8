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
    sp = 0;
    I = 0;
    opcode = 0;

    memset(RAM, 0, sizeof(RAM));

    for (int i = 0; i < FONTSET_SIZE; i++){ //Loads the font
        RAM[FONT_START_ADDRESS + i] = fontset[i];
    }

    memset(display, 0, sizeof(display));
    memset(stack, 0, sizeof(stack));
    memset(keystate, 0, sizeof(keystate));
    memset(V, 0, sizeof(V));
    
    delay_timer = 0;
    sound_timer = 0;

    srand(time(0));
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

void Chip8::cycle(bool original = true, bool debug = false){
    opcode = ((uint16_t)RAM[PC] << 8) | RAM[PC + 1];
    int first_nibble = extract_nibbles(opcode, 12, 0xF000);
    int x = extract_nibbles(opcode, 8, 0x0F00); //Look up a register, second nibble
    int y = extract_nibbles(opcode, 4, 0x00F0); //Look up a register, third nibble
    int n = extract_nibbles(opcode, 0, 0x000F); //4 bit number, fourth nibble
    int nn = extract_nibbles(opcode, 0, 0x00FF); //8 bit number, third and fourth nibble
    int nnn = extract_nibbles(opcode, 0, 0x0FFF); //12 bit memory address, second, third and fourth nibble
    PC += 2;
    
    if (debug) {
        std::cout<<"Pre"<<std::endl;
        print_registers(x,y);
    }

    switch(first_nibble){
        case 0x0:
            switch (opcode) {
                case 0x00E0: //Clean the screen
                    memset(display, 0, sizeof(display));
                    break;
                case 0x00EE: //return from the subroutine
                    sp--;
                    PC = stack[sp];
                    break;
            }
            break;
        case 0x1: //jump to nnn
            PC = nnn;
            break;
        case 0x2: //call a subroutine in nnn
            stack[sp] = PC;
            PC = nnn;
            sp++;
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
        case 0x6: //set V[x] to nn
            V[x] = nn;
            break;
        case 0x7: //add nn to V[x] 
            V[x] += nn;
            break;
        case 0x8:
            switch (n){
                case 0: //Set Vx to Vy
                    V[x] = V[y];
                    break;
                case 1: //Binary Or
                    V[x] = V[x] | V[y];
                    if (original) {
                        V[0xF] = 0;
                    }
                    break;
                case 2: //Binary And
                    V[x] = V[x] & V[y];
                    if (original) {
                        V[0xF] = 0;
                    }
                    break;
                case 3: //Binary Xor
                    V[x] = V[x] ^ V[y];
                    if (original) {
                        V[0xF] = 0;
                    }
                    break;
                case 4: //Add
                    int carry;
                    if (V[x] + V[y] > 255){
                        carry = 1;
                    } else {
                        carry = 0;
                    }
                    V[x] = V[x] + V[y];
                    V[0xF] = carry;
                    break;
                case 5: //Subtract Vy from Vx
                    if (V[x] > V[y]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[x] = V[x] - V[y];
                    break;
                case 7: //Subtract Vx from Vy
                    if (V[y] > V[x]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[x] = V[y] - V[x];
                    break;
                case 6: { //Shifts 1 bit to the right
                    int lsb = V[x] & 0x1;
                    if (original){
                        lsb = V[y] & 0x1;
                        V[x] = V[y];
                    }
                    V[x] = V[x] >> 1;
                    V[0xF] = lsb;
                    break;
                }
                case 0xE: { //Shits 1 bit to the left
                    int msb = (V[x] >> 7);
                    if (original){
                        msb = (V[y] >> 7);
                        V[x] = V[y];
                    }
                    V[x] = V[x] << 1;
                    V[0xF] = msb;
                    break;
                }
                default:
                    std::cout<<(first_nibble)<<"XY"<<(n)<<std::endl;
                    break;
            }
            break;
            std::cout<<(first_nibble)<<std::endl;
        case 0x9: //skip next instruction if Vx != Vy
            if (V[x] != V[y]) {
                PC += 2;
            }
            break;
        case 0xA: //set Index Register
            I = nnn;
            break;
        case 0xB: //jump with offset
            if (original){
                PC = nnn + V[0];
            } else {
                PC = nnn + V[x];
            }
            break;
        case 0xC: //Generate a random number
            V[x] = rand() & nn;
            break;
        case 0xD: { //Draws to the screen
            int x_coord = V[x] & 63; 
            int y_coord = (V[y]) & 31;
            V[0xF] = 0;

            for (int i = 0; i < n; i++) {
                uint8_t sprite_byte = RAM[I + i];
                x_coord = V[x] & 63;

                for (int j = 0; j < 8; j++) {
                    if ((sprite_byte & (0x80 >> j)) != 0) {
                        if (display[y_coord][x_coord] == true) {
                            display[y_coord][x_coord] = false;
                            V[0xF] = 1; 
                        } else {
                            display[y_coord][x_coord] = true;
                        }
                    }
                    if (x_coord >= 63) {
                        break;
                    }
                    x_coord++;   
                }
                y_coord++;
                if (y_coord > 31) {
                    break;
                }
            }
            break;
        }
        case 0xE: 
            switch (nn) {
                case 0x9E: //skip if pressed
                    if (keystate[V[x]]){
                        PC += 2;
                    }
                    break;
                case 0xA1: //skip if not pressed
                    if (!keystate[V[x]]){
                        PC += 2;
                    }
                    break;
                default:
                    std::cout<<(first_nibble)<<"X"<<(nn)<<std::endl;
                    break;
            }
            break;
        case 0xF:
            switch (nn) {
                case 0x07: //set Vx to delay timer
                    V[x] = delay_timer;
                    break;
                case 0x15: //set delay timer to Vx
                    delay_timer = V[x]; 
                    break;
                case 0x18: //set sound timer to Vx
                    sound_timer = V[x];
                    break;
                case 0x1E: //add to index
                    if (I + V[x] >= 0x1000 && !original){
                        V[0xF] = 1;
                    }
                    I += V[x];
                    break;
                case 0x0A: { //get key
                    bool key_pressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (keystate[i]) {
                            key_pressed = true;
                            V[x] = i;
                        }
                    }
                    if (!key_pressed) {
                        PC -= 2;
                    }
                    break;
                }
                case 0x29: //font character
                    I = FONT_START_ADDRESS + (V[x] * 0x5);
                    break;
                case 0x33: //binary-coded decimal conversion
                    RAM[I] =  V[x] / 100;
                    RAM[I + 1] = (V[x] / 10) % 10;
                    RAM[I + 2] = V[x] % 10;
                    break;
                case 0x55: //store in memory
                    for (int i = 0; i <= x; i++) {
                        RAM[I + i] = V[i];
                    }
                    if (original){
                        I = I + x + 1;
                    }
                    break;
                case 0x65:
                    for (int i = 0; i <= x; i++) {
                        V[i] = RAM[I + i];
                    }
                    if (original){
                        I = I + x + 1;
                    }
                    break;
                default:
                    std::cout<<(first_nibble)<<"X"<<(nn)<<std::endl;
                    break;
                }
            break;
        default:
            std::cout<<"Default";
            break;
    }
    if (debug) {
        std::cout<<"Post"<<std::endl;
        print_registers(x,y);
    }
}

void Chip8::decrement_timers() {
    if (delay_timer > 0) {
        delay_timer--;
    }
    if (sound_timer > 0) {
        sound_timer--;
    }
}

void Chip8::print_registers(int x, int y){
    std::cout<<std::hex<<std::uppercase<<std::showbase;
    std::cout<<"OPCODE: "<<opcode<<std::endl;
    std::cout<<"PC: "<<PC<<std::endl;
    std::cout<<"I: "<<I<<std::endl;
    std::cout<<"SP: "<<(int)sp<<std::endl;
    std::cout<<"VX: "<<(int) V[x]<<std::endl;
    std::cout<<"VY: "<<(int) V[y]<<std::endl;
    std::cout<<"VF: "<<(int) V[0xF]<<std::endl;
}