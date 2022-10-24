#pragma once
#include "pch.h"
#include <unistd.h>
#include <fcntl.h>
#include "ELFImage.h"
#include "ElfUtil.h"


class Packer
{
public:
    Packer(ELFImage &elf_image);

    //encypt
    void encypt(size_t size);
    //insert
    void    change_entry(uint16_t last_section);
    int32_t	create_section( uint16_t last_section, uint16_t last_ptload);
    int32_t	insert_section();
    //shell
    void load_shell(ELFImage &shell_image);
    void shell_pack(Elf64_Addr shell_va);
    void add_jump(Elf64_Addr shell_va, Elf64_Addr entry);


    
    //write
    static  void	write_to_file(int fd, void *data, uint64_t size);
    static void		pad_zero(int fd, uint64_t end);
    void	write_file(const char* fileName);
    //elf util
    static char*	get_section_name(t_elf *elf, int id);
    static uint16_t	get_section_text(t_elf *elf);
    static uint32_t to_page(Elf64_Addr addr);

public:
    t_elf elf;
    t_elf shell;
};