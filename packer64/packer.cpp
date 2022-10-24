#include "packer.h"
#include "pch.h"
#include "ShellUtil.h"
uint8_t *loader_data;
size_t loader_size = 0;


static Elf64_Shdr	new_section = {
  .sh_name = (uint32_t)0,
  .sh_type = (uint32_t)SHT_PROGBITS,
  .sh_flags = (uint64_t)SHF_EXECINSTR | SHF_ALLOC,
  .sh_addr = (Elf64_Addr)0,
  .sh_offset = (Elf64_Off)0,
  .sh_size = (uint64_t)0,
  .sh_link = (uint32_t)0,
  .sh_info = (uint32_t)0,
  .sh_addralign = (uint64_t)16,
  .sh_entsize = (uint64_t)0,
};

Packer::Packer(ELFImage &elf_image)
{
    elf = ElfUtil::map_elf(elf_image);
}

void Packer::encypt(size_t size)
{
    int id = get_section_text(&elf);
    elf.shdr[id].sh_flags =  elf.shdr[id].sh_flags | SHF_WRITE;
    uint8_t *data = elf.section_data[id];
    printf("text addr: %x\n",elf.shdr[id].sh_addr);
    for(int i=0; i < size; i++)
    {
      *(data+i) = *(data+i) ^ 0x15;
    }
}

void Packer::load_shell(ELFImage &shell_image)
{
    shell = ElfUtil::map_elf(shell_image);
    uint16_t last_ptload = ElfUtil::GetLastPtload(elf);
    Elf64_Addr shell_va = elf.phdr[last_ptload].p_vaddr + elf.phdr[last_ptload].p_memsz;
    ShellUtil::shell_reloc(shell,shell_va);
    shell_pack(shell_va);
    //add_jump(shell_va, elf.header->e_entry);
}
void Packer::shell_pack(Elf64_Addr shell_va)
{
  //shell = text + data + 4
    loader_size = shell.shdr[1].sh_size + shell.shdr[3].sh_size ;
    loader_data = (uint8_t*)malloc(loader_size);
    memcpy(loader_data, shell.section_data[1], shell.shdr[1].sh_size);
    memcpy(loader_data + shell.shdr[1].sh_size , shell.section_data[3],shell.shdr[3].sh_size);
}
void Packer::add_jump(Elf64_Addr shell_va, Elf64_Addr entry)
{
    Elf64_Addr b_addr = shell_va + shell.shdr[1].sh_size ;
    //Elf64_Addr b_addr = shell_va + shell.shdr[1].sh_size - 4;
    JmpInstruction b;
    b.offset = (entry - b_addr)/4;
    b.op = 0x17;
    realloc(shell.section_data[1],shell.shdr[1].sh_size + 4);
    //memcpy(shell.section_data[1] + shell.shdr[1].sh_size - 4 , &b, 4);
    memcpy(shell.section_data[1] + shell.shdr[1].sh_size - 4 , &b, 4);

    shell.shdr[1].sh_size = shell.shdr[1].sh_size + 4;
}


void Packer::change_entry(uint16_t last_section)
{
  int8_t last_ptload = ElfUtil::GetLastPtload(elf);

  elf.header->e_entry = elf.phdr[last_ptload].p_vaddr + elf.phdr[last_ptload].p_memsz;
  
}

int32_t	Packer::create_section(uint16_t last_section, uint16_t last_ptload)
{
    /* 1.realloc sh, sect data */
  Elf64_Shdr	*new_shdrs;
  uint8_t	**new_sects;
  uint8_t	*loader;

  elf.header->e_shnum += 1;

  if (!(new_shdrs = (Elf64_Shdr*) realloc(elf.shdr, sizeof(Elf64_Shdr) * elf.header->e_shnum))) {
    perror("realloc");
    return (-1);
  }

  if (!(new_sects = (uint8_t**) realloc(elf.section_data, sizeof(uint8_t *) * elf.header->e_shnum))) {
    perror("realloc");
    return (-1);
  }

  elf.shdr = new_shdrs;
  elf.section_data = new_sects;

  new_section.sh_offset = elf.phdr[last_ptload].p_offset + elf.phdr[last_ptload].p_memsz;
  new_section.sh_addr = elf.phdr[last_ptload].p_vaddr + elf.phdr[last_ptload].p_memsz;
  new_section.sh_size = loader_size;

  if (!(loader = (uint8_t*) malloc(loader_size))) {
    perror("malloc");
    return (-1);
  }

  memcpy(loader, loader_data, loader_size);
    //sh,sects data
  memmove(new_shdrs + last_section + 2, new_shdrs + last_section + 1, sizeof(Elf64_Shdr) * (elf.header->e_shnum - last_section - 2));
  memmove(new_sects + last_section + 2, new_sects + last_section + 1, sizeof(uint8_t *) * (elf.header->e_shnum - last_section - 2));

  memcpy(new_shdrs + last_section + 1, &new_section, sizeof(Elf64_Shdr));
  new_sects[last_section + 1] = loader;

  return (0);
}

int32_t	Packer::insert_section()
{
  ElfUtil e;
  uint16_t	last_section = ElfUtil::GetLastSection(elf);
  uint16_t	last_ptload = ElfUtil::GetLastPtload(elf);
  if( last_section == -1 || last_ptload == -1) return -1;

  create_section(last_section, last_ptload);

  last_section += 1;


  change_entry(last_section);
  uint64_t	size = elf.phdr[last_ptload].p_memsz + loader_size;
  elf.phdr[last_ptload].p_memsz = size;
  elf.phdr[last_ptload].p_filesz = size;

  for (uint16_t i = 0; i < elf.header->e_phnum; i++) {
    if(elf.phdr[i].p_type == PT_LOAD) {
      elf.phdr[i].p_flags = PF_X | PF_W | PF_R;
    }
  }

  
  //6. change shdr after shell
  for (uint16_t i = last_section; i < elf.header->e_shnum - 1; i += 1) 
  {
    elf.shdr[i + 1].sh_offset = elf.shdr[i].sh_offset + elf.shdr[i].sh_size;
  }

  if (elf.header->e_shstrndx > last_section) 
  {
    elf.header->e_shstrndx += 1;
  }

  uint16_t	shnum = elf.header->e_shnum;
  elf.header->e_shoff = elf.shdr[shnum - 1].sh_offset + elf.shdr[shnum - 1].sh_size;

  return (0);
}


static uint64_t	off = 0;

void Packer::write_file(const char* fileName)
{
  int id = get_section_text(&elf);
  uint8_t *data = elf.section_data[id];
  int fd;

  if ((fd = open(fileName, O_CREAT | O_WRONLY, 0744)) < 0) {
    perror("open");
    return ;
  }

  write_to_file(fd, elf.header, sizeof(Elf64_Ehdr));
  pad_zero(fd, elf.header->e_phoff);
  write_to_file(fd, elf.phdr, sizeof(Elf64_Phdr) * elf.header->e_phnum);

  //write section data
  for (uint16_t id = 0; id < elf.header->e_shnum; id += 1) {
    if (elf.shdr[id].sh_type != SHT_NOBITS) {
      pad_zero(fd, elf.shdr[id].sh_offset);
      write_to_file(fd, elf.section_data[id], elf.shdr[id].sh_size);
    }
  }
  pad_zero(fd, elf.header->e_shoff);

  for (uint16_t id = 0; id < elf.header->e_shnum; id += 1) {
    write_to_file(fd, &elf.shdr[id], sizeof(Elf64_Shdr));
  }

  close(fd);

  printf("file created: '%s'\n", fileName);
}

 void Packer::write_to_file(int fd, void *data, uint64_t size)
{
  if (write(fd, data, size) != (ssize_t)size) {
    perror("write");
    exit(-1);
  }
  off += size;
}

 void Packer::pad_zero(int fd, uint64_t end)
{
  static const char	c = 0;

  while (off < end) {
    write_to_file(fd, (void *)&c, 1);
  }
}

//elf util
char*	Packer::get_section_name(t_elf *elf, int id)
 {
      uint16_t	shname;

      shname = elf->header->e_shstrndx;
      return ((char*)(elf->section_data[shname] + elf->shdr[id].sh_name));
}
uint16_t Packer::get_section_text(t_elf *elf)
{
  uint16_t	id;

  for (id = 0; id < elf->header->e_shnum; id += 1) {
    if (!strcmp(".text", get_section_name(elf, id))) {
      return (id);
    }
   }
  return -1;
}

uint32_t Packer::to_page(Elf64_Addr addr)
{
   return (uint32_t)(addr/0x1000);
}
