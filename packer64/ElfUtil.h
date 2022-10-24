#pragma once 
#include <elf.h>
#include "pch.h"

#include "ELFImage.h"

class ElfUtil
{
public:
    static t_elf map_elf(ELFImage &elf_image);
    static uint16_t  GetLastPtload(t_elf &elf);
    static uint16_t  GetLastSection(t_elf &elf);
    static uint16_t GetSect(t_elf& elf, const char* name)
    {
        int num = elf.header->e_shnum;
        for(int i=0; i < num ;i++)
        {
            
        }
    }
    static char*  get_section_name(t_elf *elf, int id)
    {
        uint16_t shname = elf->header->e_shstrndx;
        if(!shname) return nullptr;
        return ((char*)(elf->section_data[shname] + elf->shdr[id].sh_name));
    }

    static uint32_t to_page(Elf64_Addr addr)
    {
        return (uint32_t)(addr/0x1000);
    }

};