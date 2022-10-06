#include "packer.h"
uint8_t *loader_data;
size_t loader_size = 0;
typedef struct adrp
  {
    Elf64_Word rd:5;
    Elf64_Word immhi:19;
    Elf64_Word op1:5;
    Elf64_Word immlo:2;
    Elf64_Word op:1;
  }AdrpInstruction;
  typedef struct add
  {
    Elf64_Word rd:5;
    Elf64_Word rn:5;
    Elf64_Word imml:12;
    Elf64_Word shift:2;
    Elf64_Word op:8;
  }AddInstruction;
  typedef struct _b_instruction
{
	Elf64_Word offset : 24;			//偏移
	Elf64_Word op : 8;			//重定位方式)
}JmpInstruction;

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
    elf = map_elf(elf_image);
}

t_elf Packer::map_elf(ELFImage &elf_image)
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

void Packer::encypt()
{
    int id = get_section_text(&elf);
    uint8_t *data = elf.section_data[id];
    for(int i=0;i<0x100;i++)
    {
      *(data+i) = *(data+i) ^ 0x15;
    }
}

void Packer::load_shell(ELFImage &shell_image)
{
    shell = map_elf(shell_image);
}
void Packer::shell_to_loader()
{
    loader_size = shell.shdr[1].sh_size + shell.shdr[3].sh_size + 4;
    loader_data = (uint8_t*)malloc(loader_size);
    int id = get_section_text(&elf);
    Elf64_Addr b_addr = shell_va + shell.shdr[1].sh_size;
    Elf64_Addr entry = elf.header->e_entry;
    JmpInstruction b;
    b.offset = (entry - b_addr)/4;
    b.op = 0x17;
    memcpy(loader_data, shell.section_data[1], shell.shdr[1].sh_size);
    memcpy(loader_data + shell.shdr[1].sh_size, &b, 4);
    memcpy(loader_data + shell.shdr[1].sh_size + 4, shell.section_data[3],shell.shdr[3].sh_size);
}
void Packer::shell_reloc()
{
    shell_va = new_section.sh_addr;
    Elf64_Shdr sh = shell.shdr[2];
    if(sh.sh_type != SHT_RELA) return;
    uint8_t *reloc_table = shell.section_data[2];
    uint8_t *text_data = shell.section_data[1];
    for(int offset = 0 ; offset < sh.sh_size ; offset += sh.sh_entsize)
    {
      Elf64_Rela *rela = (Elf64_Rela*)(reloc_table + offset);
      u_int type = ELF64_R_TYPE(rela->r_info);
      if(type == R_AARCH64_ADR_PREL_PG_HI21)
      {
        adrp_reloc(rela);
      }else if(type == R_AARCH64_ADD_ABS_LO12_NC){
        add_reloc(rela);
      }
    }

    shell_to_loader();
}
void Packer::adrp_reloc(Elf64_Rela *rele)
{
  int shell_text_id = 1, data_id=3;
  //1. get page offset
  uint16_t elf_text_id = get_section_text(&elf);
  uint32_t sym_id = ELF64_R_SYM(rele->r_info);
  if(shell_va + shell.shdr[elf_text_id].sh_size  > (to_page(shell_va) +1) *0x1000 )
  {
    AdrpInstruction *adrp = (AdrpInstruction*)(shell.section_data[shell_text_id] + rele->r_offset);
    adrp->immlo ++;
  }
}
void Packer::add_reloc( Elf64_Rela *rela)
{
  int shell_text_id = 1;
  Elf64_Sym *symbol = (Elf64_Sym*)shell.section_data[9];
  int sys_id = ELF64_R_SYM(rela->r_info);
  //add_offset --- data's offset 
  Elf64_Addr  offset = shell.shdr[shell_text_id].sh_size + symbol[sys_id].st_value;
  uint imm = shell_va%0x1000 + offset;
  AddInstruction *add = (AddInstruction*) (shell.section_data[shell_text_id] + rela->r_offset);
  add->imml = imm;
}
uint Packer::get_shell_size()
{
  return 1;
}
void Packer::change_entry(t_elf &elf, uint16_t last_section)
{
  Elf64_Addr	last_entry;
  int32_t	jump;

  last_entry = elf.header->e_entry;
  //elf.header->e_entry = elf.shdr[last_section].sh_addr;
  jump = last_entry - (elf.header->e_entry + loader_size );
 // memcpy(elf.section_data[last_section] + loader_size - (infos_size + 4), &jump, 4);
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

  shell_reloc();


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
  uint16_t	last_section = (uint16_t)-1;
  uint16_t	last_ptload = (uint16_t)-1;

  for (uint16_t id = 0; id < elf.header->e_phnum; id += 1) {
      if (elf.phdr[id].p_type == PT_LOAD) {
	      last_ptload = id;
      }
  }
  if (last_ptload == (uint16_t)-1) {
    printf("can't find PT_LOAD section\n");
    return (-1);
  }

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

  create_section(last_section, last_ptload);

  last_section += 1;

  uint64_t	size = elf.phdr[last_ptload].p_memsz + loader_size;
  elf.phdr[last_ptload].p_memsz = size;
  elf.phdr[last_ptload].p_filesz = size;

  for (uint16_t i = 0; i < elf.header->e_phnum; i++) {
    if(elf.phdr[i].p_type == PT_LOAD) {
      elf.phdr[i].p_flags = PF_X | PF_W | PF_R;
    }
  }

  change_entry(elf, last_section);
  //6. change shdr after shell
  for (uint16_t i = last_section; i < elf.header->e_shnum - 1; i += 1) {
    elf.shdr[i + 1].sh_offset = elf.shdr[i].sh_offset + elf.shdr[i].sh_size;
  }

  if (elf.header->e_shstrndx > last_section) {
    elf.header->e_shstrndx += 1;
  }

  uint16_t	shnum = elf.header->e_shnum;
  elf.header->e_shoff = elf.shdr[shnum - 1].sh_offset + elf.shdr[shnum - 1].sh_size;

  return (0);
}


static uint64_t	off = 0;

void Packer::write_file(char* fileName)
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

  printf("file created: '%s'\n", "FILENAME");
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

 Elf64_Addr Packer::GetTextEndAddr(ELFImage &elf)
{
    /* process space addr of .text's last byte  */
    Elf64_Shdr* textSh = elf.GetSectHeaderByName(".text");
    Elf64_Addr textEnd = textSh->sh_addr + textSh->sh_size;
    return textEnd;
}