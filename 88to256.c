#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int map_col(int n) {
    if (n < 16)
        return n;

    n -= 16;
    if (n < 64) {
        // 4x4x4 rgb color cube -> 6x6x6
        // maps 0->0, 139 -> 135, 205->215, 255->255
        static const int cmap[4] = {0, 2, 4, 5};
        int r = cmap[(n / 16) % 4], g = cmap[(n / 4) % 4], b = cmap [(n / 1) % 4];

        return 16 + r*6*6 + g*6 + b;
    }

    n -= 64;
    if (n < 8) {
        // maps 46 -> 48, 92 -> 88, 115 -> 118, 139 -> 138, 162 -> 158,
        //      185 -> 188, 208 -> 208, 231 -> 228
        static const int gmap[8] = {4, 8, 11, 13, 15, 18, 20, 22};

        return 232 + gmap[n];
    }

    // rest are black
    return 16;
}

int main(int argc, const char *argv[])
{
    if (argc <= 1)
        goto error_exit;

    for (int i = 1; i < argc; i++) {
        int n;
        if (sscanf(argv[i], "%d", &n) != 1)
            goto error_exit;

        printf("%d\n", map_col(n));
    }

    return 0;

error_exit:
    printf("Usage: %s col [col ...]\n", argv[0]);
    exit(1);
}
