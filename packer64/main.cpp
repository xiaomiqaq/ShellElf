#include <iostream>
#include "ELFImage.h"
#include <cstring>
#include "packer.h"

typedef Elf64_Addr Addr ;

const char* fileName = "../Tests/hello_a64";
const char* outFileName = "../Tests/hello_a64_";
const char* shellName = "../stub/shell.o";

int main(int argc, char** argv)  
{
    // if (argc < 2)
    // {
    //     printf("input file path!");
    //     return 0;
    // }

    // char fileName[50] = {0};
    // char outFileName[50] = {0};
    // strncpy(fileName,argv[1],strlen(argv[1]));
    // strncpy(outFileName,argv[1],strlen(argv[1]));
    // strncat(outFileName,"_",1);
    ELFImage elf(fileName);
    ELFImage shell(shellName);
    Packer packer(elf);
    packer.load_shell(shell);
    packer.encypt();
    packer.insert_section();
    packer.write_file(outFileName);
    std::cout<<"success!"<<std::endl;
}
