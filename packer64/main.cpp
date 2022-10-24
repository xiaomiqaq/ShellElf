#include <iostream>
#include "ELFImage.h"
#include <cstring>
#include "packer.h"

typedef Elf64_Addr Addr ;

const char* fileName = "../tests/hello_a64";
const char* outFileName = "../tests/hello_a64_";
const char* shellName = "../stub/shell.o";

void arg(int argc, char** argv)
{
    if (argc < 2)
     {
         printf("input file path!");
         return ;
     }

     char fileName[50] = {0};
     char outFileName[50] = {0};
     strncpy(fileName,argv[1],strlen(argv[1]));
     strncpy(outFileName,argv[1],strlen(argv[1]));
     strncat(outFileName,"_",1);
}
int main(int argc, char** argv)  
{
    arg(argc,argv);
    ELFImage elf(fileName);
    ELFImage shell(shellName);
    Packer packer(elf);
    packer.load_shell(shell);
    packer.encypt(0x500);
    packer.insert_section();
    packer.write_file(outFileName);
    std::cout<<"success!"<<std::endl;
}
