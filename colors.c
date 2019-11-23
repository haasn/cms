#include <stdio.h>

enum acolor {
    BLACK = 0,
    RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE
};

enum cflags {
    NONE  = 0,
    BOLD  = 1,
    INV   = 2,
    BLINK = 4,
    UNDER = 8
};

void creset() {
    printf("\e[0;m");
}

void cansi(int fg, int bg, int flags) {
    printf("\e[%s%s%s%s%d;%dm",
            flags & BOLD  ? "1;" : "",
            flags & INV   ? "7;" : "",
            flags & UNDER ? "4;" : "",
            flags & BLINK ? "5;" : "",
            30 + fg, 40 + bg);
}

void c256(int fg, int bg) {
    printf("\e[38;5;%d;48;5;%dm", fg, bg);
}

void ctrue(int fr, int fg, int fb, int br, int bg, int bb) {
    printf("\e[38;2;%d;%d;%dm\e[48;2;%d;%d;%dm",
            fr, fg, fb, br, bg, bb);
}

#define N {printf("\n");}
void ansiline(const char *title, int bg, int flags) {
    printf("%-10s", title);
    int i;
    for (i = BLACK; i <= WHITE; i++) {
        creset(); cansi(i, bg, flags);
        printf(" %d ", i);
    }
    creset(); N;
}

int main(int argc, char **argv) {
    creset(); cansi(YELLOW, BLUE, BOLD|UNDER);
    printf("ansi - yellow on blue, bold+underline"); creset(); N;

    creset(); c256(201, 154);
    printf("256 - 201 (pinki-sh) on 154 (yellow-green-ish)"); creset(); N;

    creset(); ctrue(192, 68, 67,  131, 8, 50);
    printf("true color - dark orange on maroon"); creset(); N;

    N; printf(" -- ANSI colors --"); N;
    ansiline("Normal", BLACK, NONE);
    ansiline("Bold"  , BLACK, BOLD);
    ansiline("Inv."  , BLACK, INV);
    ansiline("Bold+Inv.", BLACK, INV|BOLD);
    ansiline("Underline", BLACK, UNDER);
    ansiline("Blink", BLACK, BLINK);

    printf("\n -- 256 colors -- \n  ");
    int i;
    for (i = 0; i < 256; i++) {
        if (i == 8 || i == 16 || i == 244) {
            printf("\n  ");
        } else if (i > 16){// && i <= 232) {
            if (! ((i - 16) % 6))
                printf("\n  ");
        }
        c256(16, i); printf("%3d ", i); creset();
    }

    printf("\n\n -- True color --\n");

    int x, y;
    for (y = 0; y < 30; y++) {
        printf("  ");
        for (x = 0; x < 64; x++) {
           int r = y * 8, g = 255 - y * 4 - x * 4, b = x * 4;
           ctrue(0, 0, 0, r, g, b); printf(" ");
        }; N; creset();
    }; N;


    return 0;
}
