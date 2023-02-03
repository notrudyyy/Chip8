#include<stdlib.h>
#include<iostream>
#include<fstream>
#include<iomanip>
#include<string.h>
#include<math.h>
#include<stack>
#include<random>

//General Outline
//Read from hex file - parse, store into RAM (do i need to use malloc here? or should i just write to drive)
//Init PC, I, Register Array and run basic logical operations
/*
First initialize these.
00E0 (clear screen)
1NNN (jump)
6XNN (set register VX)
7XNN (add value to register VX)
ANNN (set index register I)
DXYN (display/draw)
*/

typedef unsigned char uchar;
typedef std::string byte;
typedef byte* bytes;

//Global constant defining size of RAM
const int RAM_SIZE = 4095;

//Global constant defining the location of start of program in RAM
const int PROG_START = 512;

const std::string HEX = "0123456789abcdef";

//The RAM
static byte ram[RAM_SIZE];

//The stack
std::stack<int> stack;

//Register array and register i
byte registers[16];

std::string reg_i{ "0000" }; //2 bytes

//Array containing state of display
//Display starts at (0,0) and ends at (31, 63)
int display[32][64] = {};

//Function to convert the 8 bit binary input from file to a 2 character string.
void sanitize(uchar input, bytes ram, int index) {
    int inp = (int)input;
    ram[index] = "00";
    ram[index][0] = HEX[inp / 16];
    ram[index][1] = HEX[inp % 16];
}

//Function to convert integer to 2 length hex string.
void encode(int num, std::string* hex) {
    *hex = "00";
    hex[0] = HEX[num / 16];
    hex[1] = HEX[num % 16];
}

//Function to convert n length hex string to integer.
int decode(std::string hex, int len) {
    int decoded = 0;
    for (int i = 0; i < len; i++)
    {
        decoded += pow(16, len - i - 1) * (HEX.find(hex[i]));
    }
    return decoded;
}

//Function to convert a single length hex character to an integer.
int decode_char(char hex) {
    return HEX.find(hex);
}

//Function to decode a given register.
int decode_reg(char index) {
    return decode(registers[decode_char(index)], 2);
}

//Function to convert nnn to integer address
int addr(byte nib1, byte nib2) {
    std::string address = nib1 + nib2;
    address[0] = '0';
    return decode(address, 4);
}

//Function to check if program is trying to modify flag registers, outputs warning
void flag_warn(byte nib1, byte nib2, char k) {
    if (k == 'f')
    {
        std::cout << "WARNING - Program trying to access flag register using opcode - \"" << nib1 + nib2 << "\"\n";
    }
}

//Function to output error message if unknown opcode was read
void unknown_op(byte nib1, byte nib2) {
    std::cout << "ERROR - UNRECOGNIZED OPCODE \"" << nib1 << nib2 << "\"! Now exiting...\n";
    exit(EXIT_FAILURE);
}

//Function to convert hex char to 4 bit string
std::string hex_2_bin(std::string bin, char hex) {
    int decoded = decode_char(hex);
    bin[3] = decoded % 2;
    decoded /= 2;
    bin[2] = decoded % 2;
    decoded /= 2;
    bin[1] = decoded % 2;
    decoded /= 2;
    bin[0] = decoded % 2;
    return bin;
}

//Function to load font sprite data into RAM
void load_font(bytes ram) {
    //Loading 0
    ram[0] = "f0";
    ram[1] = "90";
    ram[2] = "90";
    ram[3] = "90";
    ram[4] = "f0";

    //Loading 1
    ram[5] = "20";
    ram[6] = "60";
    ram[7] = "20";
    ram[8] = "20";
    ram[9] = "70";

    //Loading 2
    ram[10] = "f0";
    ram[11] = "10";
    ram[12] = "f0";
    ram[13] = "80";
    ram[14] = "f0";

    //Loading 3
    ram[15] = "f0";
    ram[16] = "10";
    ram[17] = "f0";
    ram[18] = "10";
    ram[19] = "f0";

    //Loading 4
    ram[20] = "90";
    ram[21] = "91";
    ram[22] = "f0";
    ram[23] = "10";
    ram[24] = "10";

    //Loading 5
    ram[25] = "f0";
    ram[26] = "80";
    ram[27] = "f0";
    ram[28] = "10";
    ram[29] = "f0";

    //Loading 6
    ram[30] = "f0";
    ram[31] = "80";
    ram[32] = "f0";
    ram[33] = "90";
    ram[34] = "f0";

    //Loading 7
    ram[35] = "f0";
    ram[36] = "10";
    ram[37] = "20";
    ram[38] = "40";
    ram[39] = "40";

    //Loading 8
    ram[40] = "f0";
    ram[41] = "90";
    ram[42] = "f0";
    ram[43] = "90";
    ram[44] = "f0";

    //Loading 9
    ram[45] = "f0";
    ram[46] = "90";
    ram[47] = "f0";
    ram[48] = "10";
    ram[49] = "f0";

    //Loading A
    ram[50] = "f0";
    ram[51] = "90";
    ram[52] = "f0";
    ram[53] = "90";
    ram[54] = "90";

    //Loading B
    ram[55] = "e0";
    ram[56] = "90";
    ram[57] = "e0";
    ram[58] = "90";
    ram[59] = "e0";

    //Loading C
    ram[60] = "f0";
    ram[61] = "80";
    ram[62] = "80";
    ram[63] = "80";
    ram[64] = "f0";

    //Loading D
    ram[65] = "e0";
    ram[66] = "90";
    ram[67] = "90";
    ram[68] = "90";
    ram[69] = "e0";

    //Loading E
    ram[70] = "f0";
    ram[71] = "80";
    ram[72] = "f0";
    ram[73] = "80";
    ram[74] = "f0";

    //Loading F
    ram[75] = "f0";
    ram[76] = "80";
    ram[77] = "f0";
    ram[78] = "80";
    ram[79] = "80";
}

//Function to recursively print the stack
void print_stack() {
    if (stack.empty())
    {
        return;
    }
    int x = stack.top();
    stack.pop();
    print_stack();
    stack.push(x);
    std::cout << x << '\n';
}

//Function to output each opcode read OR that + stack and registers and pc based on user input
void debug_print(byte nib1, byte nib2, int pc, int verbosity) {
    if (verbosity == 0 || verbosity == 1)
    {
        std::cout << "Current opcode - \"" << nib1 + nib2 << "\"\n";
        if (verbosity == 1)
        {
            std::cout << "REGISTERS\n\n";
            std::cout << "Program Counter - \"" << pc << "\"\n";
            for (int i = 0; i < 16; i++)
            {
                std::cout << "V" << HEX[i] << " - \"" << registers[i] << "\"\n";
            }
            std::cout << "\nSTACK\n\n";
            print_stack();
            std::cout << "\n";
        }
    }
}

void execute(int* pc, byte nib1, byte nib2) {
    debug_print(nib1, nib2, *pc - 2, 1);
    switch (nib1[0])
    {
    case '0':
        switch (nib1[1])
        {
        case '0':
            switch (nib2[0])
            {
            case 'e':
                switch (nib2[1])
                {
                case '0': //00E0 - CLS -Clear Display
                    for (int i = 0; i < 32; i++) {
                        for (int j = 0; j < 64; j++) {
                            display[i][j] = 0;
                        }
                    }
                    break;

                case 'E': //00EE - RET -return from subroutine
                    *pc = stack.top();
                    stack.pop();
                    break;
                default:
                    unknown_op(nib1, nib2);
                    break;
                }
                break;

            default:
                unknown_op(nib1, nib2);
                break;
            }
            break;

        default: //0nnn - We ignore thisb keeping default blank.
            break;
        }
        break;

    case '1': //1nnn - JMP addr - Jump to location at nnn
    {     //Using brackets here to bypass error. See https://stackoverflow.com/questions/5136295/switch-transfer-of-control-bypasses-initialization-of-when-calling-a-function
        *pc = addr(nib1, nib2);
    }
    break;

    case '2'://2nnn - CALL addr - Call subroutine at nnn
    {
        stack.push(*pc);
        *pc = addr(nib1, nib2);
    }
    break;

    case '3'://3xkk - Skip next instruction if Vx==kk
    {
        if (registers[decode_char(nib1[1])] == nib2)
        {
            *pc += 2;
        }
    }
    break;

    case '4'://4xkk - Skip next instruction if Vx!=kk
    {
        if (registers[decode_char(nib1[0])] != nib2)
        {
            *pc += 2;
        }

    }
    break;

    case '5'://5xy0 - Skip next instruction if Vx==Vy
    {
        if (nib2[1] == '0')
        {
            //NOTE - Can increase performance (basically by a nanosec lol) by removing call to
            //       decode_reg and instead comparing the 2 strings stored in the registers
            //       (using decode_char to get index)
            if (decode_reg(nib1[1]) == decode_reg(nib2[0]))
            {
                *pc += 2;
            }
        }
    }
    break;

    case '6'://6xkk - Loads Vx with kk
    {
        flag_warn(nib1, nib2, nib1[1]);
        registers[decode_char(nib1[1])] = nib2;
    }
    break;

    case '7'://7xkk - Vx += kk
    {
        flag_warn(nib1, nib2, nib1[1]);
        encode((decode_reg(nib1[1]) + decode(nib2, 2)), &registers[decode_char(nib1[1])]);
    }
    break;

    case '8':
        switch (nib2[1])
        {
        case '0'://8xy0 - Set Vx = Vy
        {
            flag_warn(nib1, nib2, nib1[1]);
            registers[decode_char(nib1[1])] = registers[decode_char(nib2[0])];
        }
        break;

        case '1'://8xy1 - Vx = Vy OR Vx (bitwise OR)
        {
            flag_warn(nib1, nib2, nib1[1]);
            encode(decode_reg(nib2[0]) | decode_reg(nib1[1]), &registers[decode_char(nib1[1])]);
        }
        break;

        case '2'://8xy2 - Vx = Vx AND Vy
        {
            flag_warn(nib1, nib2, nib1[1]);
            encode(decode_reg(nib2[0]) & decode_reg(nib1[1]), &registers[decode_char(nib1[1])]);
        }
        break;

        case '3'://8xy3 - Vx = Vx XOR Vy
        {
            flag_warn(nib1, nib2, nib1[1]);
            encode(decode_reg(nib2[0]) ^ decode_reg(nib1[1]), &registers[decode_char(nib1[1])]);
        }
        break;

        case '4'://8xy4 - Vx = (Vx + Vy) mod 256 and Vf = 1 if Vx+Vy<=255, else 0
        {
            flag_warn(nib1, nib2, nib1[1]);
            if (decode_reg(nib2[0]) + decode_reg(nib1[1]) > 255)
            {
                //Could have just used 15 as the index, but this makes it more clear what the index is.
                registers[decode_char('f')] = "00";
            }
            else
            {
                registers[decode_char('f')] = "01";
            }
            encode((decode_reg(nib2[0]) + decode_reg(nib1[1])) % 256, &registers[decode_char(nib1[1])]);
        }
        break;

        case '5'://8xy5 - Vx = mod(Vx-Vy) and Vf = 1 if Vx>Vy, else 0
        {
            flag_warn(nib1, nib2, nib1[1]);
            if (decode_reg(nib2[0]) - decode_reg(nib1[1]) < 0)
            {
                registers[decode_char('f')] = "01";
            }
            else
            {
                registers[decode_char('f')] = "00";
            }
            encode(abs(decode_reg(nib2[0]) - decode_reg(nib1[1])), &registers[decode_char(nib1[1])]);
        }
        break;

        case '6'://8xy6 - Vx = Vx/2 and Vf=1 if LSB of Vx=1, else 0
        {
            flag_warn(nib1, nib2, nib1[1]);
            if (decode_reg(nib1[1]) % 2 == 1)
            {
                registers[decode_char('f')] = "01";
            }
            else
            {
                registers[decode_char('f')] = "00";
            }

            encode(decode_reg(nib1[1]) / 2, &registers[decode_char(nib1[1])]);
        }
        break;

        case '7'://8xy7 - Vx = mod(Vx-Vy) and Vf = 1 if Vy>Vx, else 0
        {
            flag_warn(nib1, nib2, nib1[1]);
            if (decode_reg(nib2[0]) > decode_reg(nib1[1]))
            {
                registers[decode_char('f')] = "01";
            }
            else
            {
                registers[decode_char('f')] = "00";
            }
            encode(abs(decode_reg(nib1[1]) - decode_reg(nib2[0])), &registers[decode_char(nib1[1])]);
        }
        break;

        case 'e'://8xyE - Vx = Vx*2 and Vf = 1 if MSB of Vx = 1, else 0
        {
            flag_warn(nib1, nib2, nib1[1]);
            if (decode_reg(nib1[1]) >= 128)
            {
                registers[decode_char('f')] = "01";
            }
            else
            {
                registers[decode_char('f')] = "00";
            }
            encode((decode_reg(nib1[1]) * 2) % 256, &registers[decode_char(nib1[1])]);
        }
        break;

        default:
            unknown_op(nib1, nib2);
            break;
        }
        break;
        
    case '9': //9xy0 - Skip next if Vx != Vy
    {
        if (nib2[1] == '0')
        {
            if (decode_reg(nib1[1]) != decode_reg(nib2[0]))
            {
                *pc += 2;
            }
        }
        else
        {
            unknown_op(nib1, nib2);
        }
    }
    break;

    case 'a'://Annn - Set I = nnn
    {
        reg_i = "0" + nib1[1] + nib2;
    }
    break;

    case 'b'://bnnn - Jump to nnn + V0
    {
        *pc = decode_reg('0') + decode(nib1[1] + nib2, 3);
    }
    break;

    case 'c'://cxkk - Set Vx = Random byte AND kk
    {
        //Some code to generate random number, see https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, 255);

        encode(distr(gen) & decode(nib2, 2), &registers[decode_char(nib1[1])]);
    }
    break;

    case 'd'://dxyn - Draw
    {
        //We read n bytes from memory, starting from address store in reg i
        //these bytes are then XOR'd onto the screen starting from (Vx, Vy)
        //if we reach display boundaries, we loop around to other side
        int col = decode_reg(nib1[1]);
        int row = decode_reg(nib2[0]);
        int no_of_bytes = decode_char(nib2[1]);
        int ram_start = decode(reg_i, 4);

        for (int i = ram_start; i < ram_start + no_of_bytes; i++)
        {
            std::string bin = hex_2_bin("0000", ram[i][0]) + hex_2_bin("0000", ram[i][1]);

            for (int j = 0; j < 8; j++) {
                display[row][(col + j) % 64] = display[row][(col + j) % 64] ^ (int)bin[j];
            }

            row = (row + 1) % 32;
        }
    }
    break;

    default:
        unknown_op(nib1, nib2);
        break;
    }
}

int main()
{
    std::string rom = "IBM Logo.ch8";
    //Open file in binary read mode
    std::ifstream data{ rom, std::ios::binary };

    //We use unsigned char to store each byte of info, i.e., 2 hex values
    uchar input{};

    int count = 0;
    for (int i = PROG_START; data.get((char&)input); i++)
    {
        sanitize(input, ram, i);
        //Debug Line.
        //std::cout << std::setw(2) << std::setfill('0') << ram[i] << ' '<< count<< '\n';
        count++;
    }


    //We set currently unused memory to 00
    for (int i = 0; i < PROG_START; i++)
    {
        ram[i] = "00";
    }

    for (int i = count + PROG_START; i < RAM_SIZE; i++)
    {
        ram[i] = "00";
    }

    //We now load in the sprites for the Chip-8 font
    load_font(ram);

    int pc = 0; //program counter

    while (1)
    {
        byte nib1 = ram[pc++];
        byte nib2 = ram[pc++];
        execute(&pc, nib1, nib2);
    }
}