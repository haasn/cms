#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
#define gc getchar_unlocked
#define pc putchar_unlocked

inline double get_uint() {
  return (gc() | (gc()<<8))/65535.0;
}

inline void put_uint(unsigned short x) {
  pc(x & 0xFF);
  pc(x >> 8);
}
*/

static const double alpha = 1.0993, beta = 0.0181;

inline double linearize(double E) {
  if (E < 4.5*beta)
    return E/4.5;
  else
    return pow((E + (alpha-1))/alpha, 1/0.45);
}

inline double compand(double e) {
  if (e < beta)
    return 4.5*e;
  else
    return alpha*pow(e,0.45) - (alpha-1);
}

#define WIDTH 4096
#define HEIGHT 2160
#define SIZE (WIDTH*HEIGHT)

typedef struct {
  unsigned short a;
  unsigned short b;
  unsigned short c;
} abc;

int main() {
  double R, G, B, r, g, b, Yc, Cbc, Crc;
  unsigned short DYc, DCbc, DCrc;

  abc *img;
  unsigned short *out;

  if (sizeof(abc) != 6) {
    printf("wrong sizeof(abc): %lu\n", sizeof(abc));
    exit(1);
  }

  img = malloc(sizeof(abc) * SIZE);
  out = malloc(sizeof(unsigned short) * SIZE * 3);

  if (!img || !out) {
    printf("failed allocating buffers\n");
    exit(1);
  }

  fread(img, sizeof(abc), SIZE, stdin);

  for (int i = 0; i < SIZE; i++) {
    R = (img[i].a/256.0 - 16.0)/219.0;
    G = (img[i].b/256.0 - 16.0)/219.0;
    B = (img[i].c/256.0 - 16.0)/219.0;

    r = linearize(R);
    g = linearize(G);
    b = linearize(B);

    Yc = compand(0.2627*r + 0.6780*g + 0.0593*b);

    Cbc = B - Yc;
    Cbc /= (Cbc > 0) ? 1.5816 : 1.9404;

    Crc = R - Yc;
    Crc /= (Crc > 0) ? 0.9936 : 1.7184;

    DYc =  (unsigned short) ( (219.0*Yc  +  16.0) * 256 );
    DCbc = (unsigned short) ( (224.0*Cbc + 128.0) * 256 );
    DCrc = (unsigned short) ( (224.0*Crc + 128.0) * 256 );

    img[i] = (abc) { DYc, DCbc, DCrc };
  }

  for (int i = 0; i < SIZE; i++) {
    out[SIZE*0 + i] = img[i].a;
    out[SIZE*1 + i] = img[i].b;
    out[SIZE*2 + i] = img[i].c;
  }

  fwrite(out, sizeof(unsigned short), SIZE*3, stdout);

  free(img);
  free(out);
}
