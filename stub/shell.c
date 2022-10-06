unsigned char* start = (unsigned char*)0x400580;
int size=0x100;

void encypt()
{
    for(int i=0; i<size;i++)
    {
        *(start+i) = *start ^ 0x15;
    }
}
