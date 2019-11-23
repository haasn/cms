#include <stdint.h>
#include <stdio.h>

uint8_t dither(uint8_t x, uint8_t y)
{
    x = x ^ y;

    x = (x | 4*x) & 0x33333333;
    x = (x | 2*x) & 0x55555555;

    y = (y | 4*y) & 0x33333333;
    y = (y | 2*y) & 0x55555555;

    uint16_t b = x + 2 * y;
    b = (((b * 0x0802LU & 0x22110LU) | (b * 0x8020LU & 0x88440LU)) * 0x10101LU) >> 16;
    return b;
}

int main()
{
    const int shift = 4;
    const int size = 1 << shift;

    for (int y = 0; y < size; y++) {
        printf("   ");
        for (int x = 0; x < size; x++)
            printf("%4d", dither(x, y));
        printf("\n");
    }

    printf(" X=%u, Y=%u:\n", size, size);
    for(unsigned y=0; y<size; ++y)
    {
        printf("   ");
        for(unsigned x=0; x<size; ++x)
        {
            unsigned v = 0, mask = shift-1, xc = x ^ y, yc = y;
            for(unsigned bit=0; bit < 2*shift; --mask)
            {
                v |= ((yc >> mask)&1) << bit++;
                v |= ((xc >> mask)&1) << bit++;
            }
            printf("%4d", v);
        }
        printf(" |");
        if(y == 0) printf(" 1/%u", size * size);
        printf("\n");
    }
}
