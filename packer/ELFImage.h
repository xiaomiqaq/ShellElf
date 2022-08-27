#pragma once
#include <elf.h>
#include <vector>
#include <string>
#include <cstring>
#define Byte unsigned char

typedef struct 
{
    Elf64_Shdr header;
    Byte* data;
}ELFSection;


typedef struct Buffer
{
    Byte* buf;
    size_t size;

    void Insert(Buffer appendBuf,size_t index)
    {
        size_t newSize = size+appendBuf.size;
        Byte* newBuf = (Byte*)malloc(newSize);

        memcpy(newBuf,buf,index);
        memcpy(newBuf+index,appendBuf.buf,appendBuf.size);
        memcpy(newBuf+index+appendBuf.size,buf+index,size-index);
        free(buf);
        size=newSize;
        buf = newBuf;
    }
}Buffer;

class ELFImage
{
public:
    ELFImage(const char* filename);
    ~ELFImage();

    void InitStringTable();
    void InitSecTable();
    Elf64_Ehdr* FileHeader();
    Elf64_Shdr* SectionHeader();
    Elf64_Phdr* ProgramHeader();
    Elf64_Phdr* GetProgramHeader(Elf64_Word type, Elf64_Word flag);

    bool DeleteUseless();
    bool GenerateElfFile(const char* filename);

    std::string GetShStr(Elf64_Word index);
    Elf64_Word GetShStrIndex(std::string str);
    void PushToShtr(std::string str);

    Elf64_Shdr* GetSectHeaderByName(std::string name);

    //write
    void InsertShell(Buffer shellBuf);
    void BuildFile(const char *name);

    
private:
    Buffer m_fileBuffer;  

    std::vector<ELFSection> m_secTable;
    std::vector<Elf64_Phdr> m_phTable;
    Elf64_Ehdr m_eh;


    //int m_fileBuffer.size ;
    std::vector<std::string> m_shtrTable;
};