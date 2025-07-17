#include <stdio.h>
#include <stdint.h>
#include <libplacebo/colorspace.h>

int main()
{
    /*
    struct pl_color_repr repr = {
        .sys    = PL_COLOR_SYSTEM_BT_2100_PQ,
        .levels = PL_COLOR_LEVELS_PC,
        .alpha  = PL_ALPHA_PREMULTIPLIED,
        .bits = {
            .sample_depth = 8,
            .color_depth = 8,
            .bit_shift = 0,
        },
    };

    struct pl_transform3x3 t = pl_color_repr_decode(&repr, NULL);
    */

    /*
    struct pl_matrix3x3 lms2xyz = {{
        {1.95856, -1.22173, 0.213693},
        {0.362909, 0.658327, -0.0212727},
        {-0.0232335, -0.231735, 1.13539},
    }};

    struct pl_matrix3x3 m = pl_get_xyz2rgb_matrix(pl_raw_primaries_get(PL_COLOR_PRIM_BT_2020));
    pl_matrix3x3_mul(&m, &lms2xyz);
    */

    struct pl_matrix3x3 m = pl_get_xyz2lms_matrix(PL_LMS_HPE, 0.02);
    const struct pl_matrix3x3 rgb2xyz = pl_get_rgb2xyz_matrix(pl_raw_primaries_get(PL_COLOR_PRIM_BT_2020));
    //const struct pl_matrix3x3 xyz2rgb = pl_get_xyz2rgb_matrix(pl_raw_primaries_get(PL_COLOR_PRIM_BT_709));

    //struct pl_matrix3x3 m = xyz2rgb;
    pl_matrix3x3_mul(&m, &rgb2xyz);
    //pl_matrix3x3_mul(&m, &rgb2xyz);
    //pl_matrix3x3_invert(&m);

    printf("matrix:\n");

    /*
    for (int y = 0; y < 3; y++)
        printf("[%10f, %10f, %10f]\n", m.m[y][0], m.m[y][1], m.m[y][2]);
        */
    for (int x = 0; x < 3; x++)
        printf("\"%10f, %10f, %10f%c \"\n", m.m[0][x], m.m[1][x], m.m[2][x], x == 2 ? ')' : ',');

    /* printf("\noffset:\n"); */
    /* printf("[%10f %10f %10f\n", t.c[0], t.c[1], t.c[2]); */
}
