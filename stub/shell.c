unsigned char* start = (unsigned char*)0x400340;
int size=0x500;

void encypt()
{
    for(int i = 0; i < size;i++)
    {
        *(start + i) = *(start + i ) ^ 0x15;
    }
    long  a = 0x400580;
    
    asm volatile(
        "mov x30, x0\n"
    );
}
