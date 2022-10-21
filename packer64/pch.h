#pragma once
#include <elf.h>
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

typedef struct	s_elf
{
  Elf64_Ehdr	*header;
  Elf64_Phdr	*phdr;
  Elf64_Shdr	*shdr;
  uint8_t	**section_data;
}	t_elf;