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
};