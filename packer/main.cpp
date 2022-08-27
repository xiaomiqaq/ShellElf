#include <iostream>
#include "ELFImage.h"
#include <cstring>

int main(int, char**) 
{

    ELFImage elf("TestProgram");
    Elf64_Shdr* sh = elf.GetSectHeaderByName(".text");
    elf.PushToShtr(".shell");
    Elf64_Word a=elf.GetShStrIndex(".shell");
    std::string s = elf.GetShStr(a);
    Elf64_Phdr* ph = elf.GetProgramHeader(PT_LOAD,PF_R|PF_X);
    elf.BuildFile("x86_out");
    std::cout<<"start sh"<<std::endl;
}
