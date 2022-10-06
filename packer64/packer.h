#pragma once
#include <elf.h>
# include <unistd.h>
# include <fcntl.h>
#include "ELFImage.h"

typedef struct	s_elf
{
  Elf64_Ehdr	*header;
  Elf64_Phdr	*phdr;
  Elf64_Shdr	*shdr;
  uint8_t	**section_data;
}	t_elf;

class Packer
{
public:
    Packer(ELFImage &elf_image);
    static t_elf map_elf(ELFImage &elf_image);
  
    //encypt
    void encypt();
    //insert
    static void	change_entry(t_elf &elf, uint16_t last_section);
    int32_t	create_section( uint16_t last_section, uint16_t last_ptload);
    int32_t	insert_section();
    //shell
    void load_shell(ELFImage &shell_image);
    void shell_to_loader();
    void shell_reloc();
    uint get_shell_size();
    void adrp_reloc(Elf64_Rela *rele);
    void add_reloc( Elf64_Rela *rela);
    
    //write
    static  void	write_to_file(int fd, void *data, uint64_t size);
    static void		pad_zero(int fd, uint64_t end);
    void	write_file(char* fileName);
    //elf util
    static char*	get_section_name(t_elf *elf, int id);
    static uint16_t	get_section_text(t_elf *elf);
    static uint32_t to_page(Elf64_Addr addr);
    static Elf64_Addr GetTextEndAddr(ELFImage &elf);

public:
    t_elf elf;
    t_elf shell;
    Elf64_Addr shell_va;
};