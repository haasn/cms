#include <stdio.h>
#include <stdint.h>
#include <libplacebo/colorspace.h>

int main()
{
    struct pl_color_repr repr = {
        .sys    = PL_COLOR_SYSTEM_BT_709,
        .levels = PL_COLOR_LEVELS_PC,
        .alpha  = PL_ALPHA_PREMULTIPLIED,
        .bits = {
            .sample_depth = 8,
            .color_depth = 8,
            .bit_shift = 0,
        },
    };

    struct pl_transform3x3 t = pl_color_repr_decode(&repr, NULL);
    printf("matrix:\n");
    for (int y = 0; y < 3; y++)
        printf("[%10f %10f %10f\n", t.mat.m[y][0], t.mat.m[y][1], t.mat.m[y][2]);

    printf("\noffset:\n");
    printf("[%10f %10f %10f\n", t.c[0], t.c[1], t.c[2]);
}
