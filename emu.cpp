#include<stdlib.h>
#include<iostream>
#include<fstream>
#include<iomanip>
#include<string.h>
#include<math.h>
#include<stack>

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

const int RAM_SIZE = 3583;
const std::string HEX = "0123456789abcdef";
std::stack<int> stack;
byte registers[15];

//Function to convert the 8 bit binary input from file to a 2 character string.
void sanitize(uchar input, bytes ram, int index){
    int inp = (int) input;
    ram[index]="00";
    ram[index][0] = HEX[inp%16];
    ram[index][1] = HEX[inp/16];
}

//Function to convert n length hex string to integer.
int decode(std::string hex, int len){
    int decoded = 0;
    for (int i = 0; i < len;i++)
    {
        decoded+=pow(16, len-i-1)*(HEX.find(hex[i]));
    }
    return decoded;
}

//Function to convert nnn to integer address
int addr(byte nib1, byte nib2){
    std::string address=nib1+nib2;
    address[0]='0';
    return decode(address, 4);
}

void execute(int* pc, byte nib1, byte nib2){
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
                    /* code to clear display*/
                    break;
                
                case 'E': //OOEE - RET -return from subroutine
                    *pc=stack.top();
                    stack.pop();
                    break;
                default:
                    std::cout<< "Unrecognized opcode \"" <<nib1 <<nib2<<"\"! Now exiting...\n";
                    exit;
                    break;
                }
                break;
            
            default:
                break;
            }
            break;
        
        default: //0nnn - We ignore thisb keeping default blank.
            break;
        }
        break;  
    
    case '1': //1nnn - JMP addr - Jump to location at nnn
        {     //Using brackets here to bypass error. See https://stackoverflow.com/questions/5136295/switch-transfer-of-control-bypasses-initialization-of-when-calling-a-function
            
        }
        break;
    
    case '2'://2nnn - CALL addr - Call subroutine at nnn
        {
            stack.push(*pc);
            *pc=addr(nib1, nib2);
        }
        break;

    case '3'://3xkk - Skip next instruction if Vx==kk
        {
            if (registers[decode(std::string("000")+nib1[1],4)] == nib2)
            {
                *pc+=2;
            }
        }
        break;
    
    case '4'://4xkk - Skip next instruction if Vx!=kk
        {
            if (registers[decode(std::string("000")+nib1[0], 4)] != nib2)
            {
                *pc+=2;
            }
            
        }
        break;

    case '5':
        
        break;
    default:
        break;
    }
}

int main()
{
    std::string rom ="IBM Logo.ch8";
    //Open file in binary read mode
    std::ifstream data{rom, std::ios::binary};

    //We use unsigned char to store each byte of info, i.e., 2 hex values
    uchar input;

    byte ram[RAM_SIZE];
    
    int count=0;
    for (int i = 0; data.get((char&)input); i++)
    {
        sanitize(input, ram, i);
        //Debug Line.
        //std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)ram[i] << ' '<< std::dec<< count<< '\n';
        count++;
    }

    int pc = 0; //program counter
    while (1)
    {
        byte nib1 = ram[pc++];
        byte nib2 = ram[pc++];
        execute(&pc, nib1, nib2);
    }
}