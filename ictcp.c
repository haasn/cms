#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

void convert(float color[3])
{
    float r = color[0], g = color[1], b = color[2];

    // RGB -> LMS
    float l = (1688 * r + 2146 * g +  262 * b) / 4096.0,
          m = ( 683 * r + 2951 * g +  462 * b) / 4096.0,
          s = (  99 * r +  309 * g + 3688 * b) / 4096.0;

    // HLG OETF (SDR part only)
    float l_ = 0.5 * sqrt(l),
          m_ = 0.5 * sqrt(m),
          s_ = 0.5 * sqrt(s);

    // L'M'S' -> ICtCp
    float i = ( 2048 * l_ +  2048 * m_            ) / 4096.0,
          t = ( 6610 * l_ - 13613 * m_ + 7003 * s_) / 4096.0,
          p = (17933 * l_ - 17390 * m_ -  543 * s_) / 4096.0;

    color[0] = i;
    color[1] = t + 128/255.0;
    color[2] = p + 128/255.0;
}

int main()
{
    int width, height;
    char type;
    if (scanf("P%c\n%d %d\n-1.0\n", &type, &width, &height) != 3 ||
        toupper(type) != 'F')
    {
        fprintf(stderr, "Got unexpected file header? Input must be PF format\n");
        return 1;
    }

    // basic sanitization
    if (width <= 0 || height <= 0 || width > 1<<20 || height > 1<<20) {
        fprintf(stderr, "Got nonsensical dimensions %dx%d!\n", width, height);
        return 1;
    }

    printf("PF\n%d %d\n-1.0\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float texel[3];

            int num = 0;
            switch (type) {
            case 'F': num = 3; break; // RGB
            case 'f': num = 1; break; // grayscale
            }

            if (fread(texel, sizeof(float), num, stdin) != num) {
                fprintf(stderr, "Expected %d floats, got fewer!\n", num);
                return 1;
            }

            if (num == 1)
                texel[2] = texel[1] = texel[0];

            convert(texel);

            // Write the result to stdout
            fwrite(texel, sizeof(float), 3, stdout);
        }
    }
}
