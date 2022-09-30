#include <iostream>
#include "ELFImage.h"
#include <cstring>
typedef Elf64_Addr Addr ;

const char* fileName = "../Tests/hello_a64";
const char* outFileName = "../Tests/hello_a64_";
typedef struct _b_instruction
{
	Elf64_Word offset : 24;			//偏移
	Elf64_Word op : 8;			//重定位方式)
}JmpInstruction;

Elf64_Addr GetTextEndOff(ELFImage &elf)
{
    /* process space addr of .text's last byte  */
    Elf64_Shdr* textSh = elf.GetSectHeaderByName(".text");
    Elf64_Addr textEnd = textSh->sh_addr + textSh->sh_size;
    return textEnd;
}
void AddJmpInstruction(ELFImage &elf)
{
    /*  1. calculate b offset : offset = (entry - text_end)/4
        2. add b to text_end(file space), add .text section size
        3. check .text.off+size < (.text+1).off
        4. change entry*/
    Elf64_Addr textEnd = GetTextEndOff(elf);
    Elf64_Addr entry = elf.FileHeader()->e_entry;
    JmpInstruction b;
    b.offset = (entry - textEnd)/4;
    b.op = 0x17;

    Elf64_Shdr* textSh = elf.GetSectHeaderByName(".text");
     JmpInstruction* pB = (JmpInstruction*) 
                        ( (Addr)elf.m_fileBuffer + textSh->sh_offset +  textSh->sh_size);
    *pB = b;
    textSh->sh_size += 4;

    elf.FileHeader()->e_entry = textEnd;

}

int main(int argc, char** argv)  
{
    if (argc < 2)
    {
        printf("input file path!");
        return 0;
    }

    char fileName[50] = {0};
    char outFileName[50] = {0};
    strncpy(fileName,argv[1],strlen(argv[1]));
    strncpy(outFileName,argv[1],strlen(argv[1]));
    strncat(outFileName,"_",1);

    ELFImage elf(fileName);
    AddJmpInstruction(elf);
    elf.BuildFile(outFileName);
    std::cout<<"success!"<<std::endl;
}
