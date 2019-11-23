#include <stdio.h>
#include <libplacebo/dither.h>

#define SIZE 256

float data[SIZE][SIZE];

int main()
{
    pl_generate_blue_noise(&data[0][0], SIZE);

    printf("P2\%d %d\n255\n", SIZE, SIZE);
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++)
            printf("%d ", (int) (255.0 * data[y][x]));
        printf("\n");
    }
}
