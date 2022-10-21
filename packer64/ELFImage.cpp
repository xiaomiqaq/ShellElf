#include "ELFImage.h"
#include <fstream>
#include <string.h>

using namespace std;
/* Address at which the packed binary is initially loaded by the kernel on
 * exec (ie. the p_vaddr field in the binary) */
#define PACKED_BIN_ADDR 0xA00000ULL

ELFImage::ELFImage(const char* filename)
{
	size_t fsize;
	ifstream fin(filename,ios::binary);
	if(fin.fail()) return ;
	fin.seekg(0,ios::end);
	fsize = (size_t)fin.tellg();
	fin.seekg(0,ios::beg);
	m_fileBuffer = new u_char[fsize];
	fin.read((char*)m_fileBuffer,fsize);
	fin.close();


	InitStringTable();
	InitSecTable();
}
void ELFImage::InitSecTable()
{
	 
	m_eh = *(Elf64_Ehdr*)(m_fileBuffer);
	int secNum = FileHeader()->e_shnum;
	ELFSection sec;
	for(int i=0;i<secNum;i++)
	{
		sec.header=SectionHeader()[i];
		int dataSize = sec.header.sh_size;
		if(dataSize>0)
		{
			Byte* buf = new Byte[dataSize];
			memcpy(buf,m_fileBuffer+sec.header.sh_offset,dataSize);
			sec.data = buf;
		}
		m_secTable.push_back(sec);
	}

	int phNum = FileHeader()->e_phnum;
	Elf64_Phdr ph;
	for(int i=0;i<phNum;i++)
	{
		ph = ProgramHeader()[i];
		m_phTable.push_back(ph);
	}

	
}



ELFImage::~ELFImage()
{
	free(m_fileBuffer);
}
void ELFImage::InitStringTable()
{
//init string table
	Elf64_Shdr strTable = SectionHeader()[FileHeader()->e_shstrndx];
	char tmp[128];
	int tmp_index = 0;
	char* baseAddr = (char*)strTable.sh_offset;
	for(int i=1;i<strTable.sh_size;i++)
	{
		char currChar = *((size_t)m_fileBuffer + baseAddr + i);
		tmp[tmp_index++] = currChar;
		if(currChar=='\0')
		{
			m_shtrTable.push_back(tmp);
			tmp_index=0;
		}
	}
}

Elf64_Ehdr* ELFImage::FileHeader()
{
	return (Elf64_Ehdr*)m_fileBuffer;
}
Elf64_Shdr* ELFImage::SectionHeader()
{
	return (Elf64_Shdr*)(m_fileBuffer+FileHeader()->e_shoff);
}
Elf64_Phdr* ELFImage::ProgramHeader()
{
	return (Elf64_Phdr*)(m_fileBuffer+FileHeader()->e_phoff);
}
Elf64_Shdr* ELFImage::GetSectHeaderByName(std::string name)
{
	for(int i=0; i<FileHeader()->e_shnum; i++)
	{
		auto sectName = GetShStr(SectionHeader()[i].sh_name);
		if(sectName == name) return &SectionHeader()[i];
	}
	return nullptr;
}
Elf64_Phdr* ELFImage::GetProgramHeader(Elf64_Word type, Elf64_Word flag)
{
	for(int i=0; i<FileHeader()->e_phnum; i++)
	{
		if(ProgramHeader()[i].p_type == type && ProgramHeader()[i].p_flags == flag)
			return &ProgramHeader()[i];
	}
	return nullptr;
}



bool ELFImage::GenerateElfFile(const char* filename)
{
	// ofstream outputFile(filename,ofstream::out|ofstream::binary);

	// Elf64_Addr entry_vaddr = sizeof(Elf64_Ehdr) + sizeof(Elf32_Phdr);
	// Elf64_Ehdr ehdr;
	// ehdr.e_ident[EI_MAG0] = ELFMAG0;
	// ehdr.e_ident[EI_MAG1] = ELFMAG1;
	// ehdr.e_ident[EI_MAG2] = ELFMAG2;
	// ehdr.e_ident[EI_MAG3] = ELFMAG3;
	// ehdr.e_ident[EI_CLASS] = ELFCLASS64;
	// ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
	// ehdr.e_ident[EI_VERSION] = EV_CURRENT;
	// ehdr.e_ident[EI_OSABI] = ELFOSABI_SYSV;
 	// ehdr.e_ident[EI_ABIVERSION] = 0;
	// memset(ehdr.e_ident + EI_PAD, 0, EI_NIDENT - EI_PAD);
	// ehdr.e_type = ET_EXEC;
	// ehdr.e_machine = EM_X86_64;
	// ehdr.e_version = EV_CURRENT;
	// ehdr.e_entry = entry_vaddr;
	// ehdr.e_phoff = sizeof(Elf64_Ehdr);
	// ehdr.e_shoff = 0;
	// ehdr.e_flags = 0;
	// ehdr.e_ehsize = sizeof(Elf64_Ehdr);
	// ehdr.e_phentsize = sizeof(Elf64_Phdr);
	// ehdr.e_phnum = 2;
	// ehdr.e_shentsize = sizeof(Elf64_Shdr);
	// ehdr.e_shnum = 0;
	// ehdr.e_shstrndx = SHN_UNDEF;
	// outputFile.write((char*)&ehdr,sizeof(ehdr));

	// int app_offset = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) ;
	// Elf64_Phdr app_phdr;
	// app_phdr.p_type = PT_LOAD;
	// app_phdr.p_offset = app_offset;
	// app_phdr.p_vaddr = PACKED_BIN_ADDR + app_offset; /* Keep vaddr aligned */
	// app_phdr.p_paddr = app_phdr.p_vaddr;
	// app_phdr.p_filesz = m_fileBuffer.size;
	// app_phdr.p_memsz = m_fileBuffer.size;
	// app_phdr.p_flags = PF_R | PF_W;
	// app_phdr.p_align =  0x200000;
	// outputFile.write((char*)&app_phdr,sizeof(app_phdr));

	// outputFile.write((char*)m_fileBuffer,m_fileBuffer.size);

	return true;
}

void ELFImage::InsertShell(Byte* shellBuf)
{
	//make shell
	//modi eh
	// FileHeader()->e_shnum = FileHeader()->e_shnum + 1;
	// //m_fileBuffer.size = m_fileBuffer.size + FileHeader()->e_shentsize + shellBuf.siz;
	// Elf64_Phdr* ph = GetProgramHeader(PT_LOAD,PF_R|PF_X);

	// Elf64_Shdr* textSec = GetSectHeaderByName(".text");
	// size_t textSecHeaderOff = (size_t)textSec-(size_t)m_fileBuffer;
	// Elf64_Shdr shellSH;
	// PushToShtr(".shell");
	// shellSH.sh_name = GetShStrIndex(".shell");
	// shellSH.sh_addr = SHN_UNDEF;
	// shellSH.sh_type = SHT_PROGBITS;
	// shellSH.sh_flags = SHF_EXECINSTR;
	// shellSH.sh_addr =0;
	// shellSH.sh_offset = 0;
	// shellSH.sh_link =0;
	// shellSH.sh_info=0;
	// shellSH.sh_addralign = 1;
	// shellSH.sh_entsize = sizeof(shellSH);
	// m_fileBuffer.Insert({(Byte*)&shellSH,sizeof(shellSH)},m_fileBuffer.size);

	// Elf64_Off shellOff = textSec->sh_offset + textSec->sh_size;
	// Byte* newBuf = (Byte*)malloc(m_fileBuffer.size);
	
}

void ELFImage::AddShell(ELFImage shell)
{
	// Elf64_Shdr *shellTextSH = shell.GetSectHeaderByName(".text");
	// Byte *shellTextData = (Byte*)malloc(shellTextSH->sh_size);
	// memcpy(shellTextData,shell.m_fileBuffer + shellTextSH->sh_offset , shellTextSH->sh_size);

	// Elf64_Phdr *ph = GetProgramHeader(PT_LOAD,PF_R|PF_X);
	// ph->p_memsz += shellTextSH->sh_size;
	// ph->p_filesz += shellTextSH->sh_size;
	// memcpy(m_fileBuffer+ph->p_offset+ph->p_memsz, shellTextData, shellTextSH->sh_size);

	// free(shellTextData);

}

void ELFImage::BuildFile(const char* name)
{
	ofstream ofs(name,ofstream::out|ofstream::binary);
	
	//get file size
	int fileSize = 0;
	fileSize+= (FileHeader()->e_shoff +FileHeader()->e_shentsize*FileHeader()->e_shnum);

	ofs.write((char*)m_fileBuffer,fileSize);
	
}


std::string ELFImage::GetShStr(Elf64_Word index)
{
	Elf64_Shdr shStrTab = SectionHeader()[FileHeader()->e_shstrndx];
	char tmp[128];
	std::string ret;
	Byte* dataAddr = (Byte*)(m_fileBuffer + shStrTab.sh_offset);
	for(int i=0;i<128;i++)
	{
		char c = *(char*)(dataAddr + index + i);
		tmp[i]=c;
		if(c=='\0')
		{
			return tmp;
		}
	}
	return NULL;
}

void ELFImage::PushToShtr(std::string str)
{
	Elf64_Shdr* header = GetSectHeaderByName(".shstrtab");
	// size_t addPos = header->sh_offset+header->sh_size;
	// size_t newSize = str.size() + 1;

	
	// m_fileBuffer.Insert({(Byte*)str.c_str(),newSize},addPos);
	// FileHeader()->e_shoff = FileHeader()->e_shoff + newSize;
	// GetSectHeaderByName(".shstrtab")->sh_size+= newSize;
}

Elf64_Word ELFImage::GetShStrIndex(std::string str)
{
	Elf64_Shdr* header = GetSectHeaderByName(".shstrtab");
	char tmp[128];
	int tmp_ind = 0;
	std::string ret;
	Byte* dataAddr = (Byte*)(m_fileBuffer + header->sh_offset);
	for(int i=0;i<header->sh_size ;i++)
	{
		tmp[tmp_ind] = *(char*)(dataAddr+i);
		if(tmp[tmp_ind] == '\0')
		{
			if( str == tmp ) return i - str.size();
			tmp_ind = 0;
		}else{
			tmp_ind++;
		}
	}
	return 0;
}
