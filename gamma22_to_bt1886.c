#include <stdio.h>
#include <stdlib.h>
#include <lcms2.h>
#include <math.h>

int main(int argc, const char *argv[])
{
    if (argc <= 1)
        goto error_exit;

    // hard-coded 1000 contrast ratio
    double bp = pow(1.0/1000, 1/2.4);
    cmsToneCurve *bt1886 = cmsBuildParametricToneCurve(NULL, 6,
            (double[4]){2.4, 1.0 - bp, bp / (1.0 - bp), 0.0});
    cmsToneCurve *bt1886_inv = cmsReverseToneCurve(bt1886);

    // using 2.0 instead of 2.2 makes it sliightly better
    cmsToneCurve *gamma22 = cmsBuildGamma(NULL, 2.0);

    for (int i = 1; i < argc; i++) {
        unsigned int r, g, b;
        if (sscanf(argv[i], "#%02x%02x%02x", &r, &g, &b) != 3)
            goto error_exit;

        // evaluate expanded to 16 bits
        r = cmsEvalToneCurve16(gamma22, r | (r << 8));
        g = cmsEvalToneCurve16(gamma22, g | (g << 8));
        b = cmsEvalToneCurve16(gamma22, b | (b << 8));

        // evaluate shifted down to 8 bits
        r = cmsEvalToneCurve16(bt1886_inv, r) >> 8;
        g = cmsEvalToneCurve16(bt1886_inv, g) >> 8;
        b = cmsEvalToneCurve16(bt1886_inv, b) >> 8;

        printf("#%02x%02x%02x\n", r, g, b);
    }

    cmsFreeToneCurve(gamma22);
    cmsFreeToneCurve(bt1886);
    cmsFreeToneCurve(bt1886_inv);

    return 0;

error_exit:
    printf("Usage: %s #rrggbb [#rrggbb ...]\n", argv[0]);
    exit(1);
}
