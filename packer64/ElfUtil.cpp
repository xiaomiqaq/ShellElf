#include "ElfUtil.h"
#include "stdio.h"
#include "ELFImage.h"

t_elf ElfUtil::map_elf(ELFImage &elf_image)
{
    t_elf elf;
    //header
    size_t eh_size = elf_image.FileHeader()->e_ehsize;
    elf.header = (Elf64_Ehdr*)malloc(elf_image.FileHeader()->e_ehsize);
    memcpy(elf.header,elf_image.FileHeader(),elf_image.FileHeader()->e_ehsize);


    size_t ph_size = elf_image.FileHeader()->e_phentsize *elf_image.FileHeader()->e_phnum;
    if(ph_size>0)
    {
      elf.phdr = (Elf64_Phdr*)malloc(ph_size);
      memcpy(elf.phdr,elf_image.ProgramHeader(),ph_size);
    }


    size_t sh_size = elf_image.FileHeader()->e_shentsize * elf_image.FileHeader()->e_shnum;
    elf.shdr = (Elf64_Shdr*)malloc(sh_size);
    memcpy(elf.shdr,elf_image.SectionHeader(),sh_size);
    
    //
    elf.section_data = (uint8_t**)malloc(sizeof(uint8_t*) * elf.header->e_shnum);
    for(int i=0;i<elf.header->e_shnum;i++)
    {
        if(elf.shdr[i].sh_type == SHT_NOBITS)
        {
            elf.section_data[i] = (uint8_t*)0;
        }
        size_t data_size = elf.shdr[i].sh_size;
        elf.section_data[i] = (uint8_t*) malloc(data_size);
        memcpy(elf.section_data[i],elf_image.m_fileBuffer + elf.shdr[i].sh_offset, data_size);
    }
    return elf;
}

uint16_t ElfUtil::GetLastPtload(t_elf &elf)
{
    uint16_t last_ptload = -1;
    for (uint16_t id = 0; id < elf.header->e_phnum; id += 1) {
      if (elf.phdr[id].p_type == PT_LOAD) {
	      last_ptload = id;
      }
  }
  if (last_ptload == (uint16_t)-1) {
    printf("can't find PT_LOAD section\n");
    return (-1);
  }
  return last_ptload;
}

uint16_t ElfUtil::GetLastSection(t_elf& elf)
{
    uint16_t last_section = -1;
    uint16_t last_ptload = ElfUtil::GetLastPtload(elf);
     for (uint16_t id = 0; id < elf.header->e_shnum; id += 1) {

    Elf64_Phdr	*phdr = elf.phdr + last_ptload;
    Elf64_Shdr	*shdr = elf.shdr + id;

    if (shdr->sh_addr + shdr->sh_size >= phdr->p_vaddr + phdr->p_memsz) {
      last_section = id;
    }
  }
  if (last_section == (uint16_t)-1) {
    printf("can't find last section\n");
    return (-1);
  }
  return last_section;
}