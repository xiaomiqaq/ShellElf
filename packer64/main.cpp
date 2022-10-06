#include <iostream>
#include "ELFImage.h"
#include <cstring>
#include "packer.h"

typedef Elf64_Addr Addr ;

const char* fileName = "../Tests/hello_a64";
const char* outFileName = "../Tests/hello_a64_";
const char* shellName = "../stub/shell.o";

typedef struct _b_instruction
{
	Elf64_Word offset : 24;			//偏移
	Elf64_Word op : 8;			//重定位方式)
}JmpInstruction;

static Elf64_Addr GetTextEndAddr(ELFImage &elf)
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
    Elf64_Addr textEnd = GetTextEndAddr(elf);
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

void InsertDataInText(ELFImage &elf)
{
    /*  1.add data to buff
        2.change elf header:sh_off,
        3.change text sh:size
        4.change sh after the textsh*/
    Elf64_Shdr* textSh = elf.GetSectHeaderByName(".text");
    Addr endOff = (Addr)elf.m_fileBuffer + textSh->sh_offset + textSh->sh_size;
    Byte data[0x100];
    memset(data,0x88,0x100);
    endOff+=0x100;
    Elf64_Shdr* sh = elf.SectionHeader();
    Elf64_Phdr* ph = elf.ProgramHeader();
    if(ph->p_vaddr+ph->p_memsz+0x100 > ph[1].p_vaddr)
    {
        printf("over segment\n");
    }else{
        printf("no over segment\n");
    }
    //change secion afert text offs addr
    for(int i=0;i<elf.FileHeader()->e_shnum;i++)
    {
        if(sh[i].sh_offset < endOff) continue;
        sh[i].sh_offset+=0x100;
        if(sh[i].sh_addr<ph[1].p_vaddr) sh[i].sh_addr+=0x100;
        
    }
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
    ELFImage shell(shellName);
    Packer packer(elf);
    packer.load_shell(shell);
    packer.encypt();
    packer.insert_section();
    packer.write_file(outFileName);
    std::cout<<"success!"<<std::endl;
}
