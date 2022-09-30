#include <iostream>
#include "ELFImage.h"
#include <cstring>
const char* fileName = "../Tests/hello_a64";



void GetTextEnd(ELFImage &elf)
{
    Elf64_Shdr* textSh = elf.GetSectHeaderByName(".text");
    Elf64_Addr textEnd = (Elf64_Addr)elf.m_fileBuffer + textSh->sh_offset + textSh->sh_size;
}


int main(int argc, char** argv)  
{
    ELFImage elf(fileName);
    Elf64_Shdr* sh = elf.GetSectHeaderByName(".text");
    elf.PushToShtr(".shell");
    Elf64_Word a=elf.GetShStrIndex(".shell");
    std::string s = elf.GetShStr(a);
    Elf64_Phdr* ph = elf.GetProgramHeader(PT_LOAD,PF_R|PF_X);
    ELFImage shell("../stub/shell.o");
    elf.AddShell(shell);
    elf.BuildFile("x86_pack");
    std::cout<<"start sh"<<std::endl;
}
