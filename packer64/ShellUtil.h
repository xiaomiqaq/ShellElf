#pragma once
#include "pch.h"
#include <unistd.h>
#include <fcntl.h>
#include "ElfUtil.h"
class ShellUtil
{
public:
    ShellUtil(/* args */);
    ~ShellUtil();
    static Elf64_Addr GetSymAddr(t_elf& shell, Elf64_Addr shell_va, Elf64_Rela *rela)
    {
        uint32_t sym_id = ELF64_R_SYM(rela->r_info);
        Elf64_Sym* symt = (Elf64_Sym*)shell.section_data[symbol];
        Elf64_Sym sym = symt[sym_id];
        Elf64_Addr sym_addr = shell_va + shell.shdr[text].sh_size + sym.st_value;
    }
    static void adrp_reloc(t_elf& shell, Elf64_Rela *rela,Elf64_Addr shell_va)
    {
        uint32_t sym_id = ELF64_R_SYM(rela->r_info);
        Elf64_Addr sym_addr = ShellUtil::GetSymAddr(shell, shell_va, rela);
        Elf64_Addr cur_addr = shell_va + rela->r_offset;
        Elf64_Addr sym_page = ElfUtil::to_page(sym_addr);
        Elf64_Addr cur_page = ElfUtil::to_page(cur_addr);
        if(sym_page == cur_page)
        {
            printf("no use change adrp\n");
            return ;
        }
        else
        {
            int offset = sym_page - cur_page;
            AdrpInstruction *adrp = (AdrpInstruction*) (shell.section_data[text] + rela->r_offset);
            adrp->immlo += offset;
        }
    }
    static void add_reloc(t_elf& shell, Elf64_Rela *rela,Elf64_Addr shell_va)
    {
        Elf64_Addr sym_addr = GetSymAddr(shell,shell_va,rela);
        uint imm = sym_addr % 0x1000;
        AddInstruction *add = (AddInstruction*) (shell.section_data[text] + rela->r_offset);
        add->imml = imm;
    }
    static void shell_reloc(t_elf& shell, Elf64_Addr shell_va)
    {
        Elf64_Shdr sh = shell.shdr[text_reloc];
        if(sh.sh_type != SHT_RELA) return;
        uint8_t *reloc_table = shell.section_data[text_reloc];
        for(int offset = 0 ; offset < sh.sh_size ; offset += sh.sh_entsize)
        {
            Elf64_Rela *rela = (Elf64_Rela*)(reloc_table + offset);
            u_int type = ELF64_R_TYPE(rela->r_info);
            if(type == R_AARCH64_ADR_PREL_PG_HI21)
            {
                ShellUtil::adrp_reloc(shell,rela,shell_va);
                printf("reloc adrp\n");
            }else if(type == R_AARCH64_ADD_ABS_LO12_NC){
                ShellUtil::add_reloc(shell, rela, shell_va);
                printf("reloc add\n");
            }
        }
    }

private:
    static const int text = 1;
    static const int text_reloc = 2;
    static const int data = 3;
    static const int symbol = 9;


};

ShellUtil::ShellUtil(/* args */)
{
}

ShellUtil::~ShellUtil()
{
}
