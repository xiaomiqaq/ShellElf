unsigned char* start = (unsigned char*)0x400340;
int size=0x1000;

void encypt()
{
    for(int i=0; i<size;i++)
    {
        *(start+i) = *(start + i) ^ 0x15;
    }
}
