#include <stdlib.h>
#include <math.h>
#include <lcms2.h>

double clamp(double x) {
  if (x < 0)
    return 0;
  if (x > 1)
    return 1;

  return x;
}

enum {
    RAMP_RGB = 0,
    RAMP_XYZ,
    RAMP_LAB,
    RAMP_JAB,
    RAMP_CAM,
    RAMP_NUM,
};

int main() {
  cmsCIExyY D65 = {0.31271, 0.32902, 1};
  cmsCIEXYZ D65_XYZ;
  cmsxyY2XYZ(&D65_XYZ, &D65);
  cmsHPROFILE sRGB = cmsCreate_sRGBProfile();
  cmsHPROFILE Lab = cmsCreateLab4Profile(&D65);
  cmsHPROFILE XYZ = cmsCreateXYZProfile();
  cmsHTRANSFORM forw = cmsCreateTransform( sRGB, TYPE_RGB_8
                                         , XYZ, TYPE_XYZ_DBL
                                         , INTENT_RELATIVE_COLORIMETRIC
                                         , cmsFLAGS_HIGHRESPRECALC);

  cmsHTRANSFORM back = cmsCreateTransform( XYZ, TYPE_XYZ_DBL
                                         , sRGB, TYPE_RGB_8
                                         , INTENT_RELATIVE_COLORIMETRIC
                                         , cmsFLAGS_HIGHRESPRECALC);

  cmsHTRANSFORM forwlab = cmsCreateTransform( sRGB, TYPE_RGB_8
                                            , Lab, TYPE_Lab_DBL
                                            , INTENT_RELATIVE_COLORIMETRIC
                                            , cmsFLAGS_HIGHRESPRECALC);

  cmsHTRANSFORM backlab = cmsCreateTransform( Lab, TYPE_Lab_DBL
                                            , sRGB, TYPE_RGB_8
                                            , INTENT_RELATIVE_COLORIMETRIC
                                            , cmsFLAGS_HIGHRESPRECALC);
  cmsViewingConditions cond = {
    .whitePoint = D65_XYZ,
    .Yb = 0.2,
    .La = 120,
    .surround = DIM_SURROUND,
    .D_value = D_CALCULATE,
  };
  cmsHANDLE CAM02 = cmsCIECAM02Init(NULL, &cond);

#define SIZE 1000

  unsigned char start[3] = {0,255,0},
                end[3] = {255,0,0};

  double buf[RAMP_NUM][SIZE][3];
  unsigned char out[RAMP_NUM][SIZE][3];

  double a[RAMP_NUM][3], b[RAMP_NUM][3];
  for (int i = 0; i < 3; i++) {
      a[RAMP_RGB][i] = start[i] / 255.0;
      b[RAMP_RGB][i] = end[i] / 255.0;
  }

  cmsDoTransform(forw, start, a[RAMP_XYZ], 1);
  cmsDoTransform(forw, end,   b[RAMP_XYZ], 1);

  cmsDoTransform(forwlab, start, a[RAMP_LAB], 1);
  cmsDoTransform(forwlab, end,   b[RAMP_LAB], 1);

  cmsCIEXYZ tXYZ;
  cmsJCh tJCh;

  tXYZ.X = a[RAMP_XYZ][0];
  tXYZ.Y = a[RAMP_XYZ][1];
  tXYZ.Z = a[RAMP_XYZ][2];

  cmsCIECAM02Forward(CAM02, &tXYZ, &tJCh);

  a[RAMP_CAM][0] = tJCh.J;
  a[RAMP_CAM][1] = tJCh.C;
  a[RAMP_CAM][2] = tJCh.h;

  a[RAMP_JAB][0] = tJCh.J;
  a[RAMP_JAB][1] = tJCh.C * cos(tJCh.h * M_PI / 180.0);
  a[RAMP_JAB][2] = tJCh.C * sin(tJCh.h * M_PI / 180.0);

  //printf("JCh: %f %f %f\n", tJCh.J, tJCh.C, tJCh.h);

  tXYZ.X = b[RAMP_XYZ][0];
  tXYZ.Y = b[RAMP_XYZ][1];
  tXYZ.Z = b[RAMP_XYZ][2];

  cmsCIECAM02Forward(CAM02, &tXYZ, &tJCh);

  b[RAMP_CAM][0] = tJCh.J;
  b[RAMP_CAM][1] = tJCh.C;
  b[RAMP_CAM][2] = tJCh.h;

  b[RAMP_JAB][0] = tJCh.J;
  b[RAMP_JAB][1] = tJCh.C * cos(tJCh.h * M_PI / 180.0);
  b[RAMP_JAB][2] = tJCh.C * sin(tJCh.h * M_PI / 180.0);

  //printf("JCh: %f %f %f\n", tJCh.J, tJCh.C, tJCh.h);

  /*
  double Ca = sqrt(a[RAMP_XYZ][1]*a[1] + a[2]*a[2]);
  double Cb = sqrt(b[RAMP_XYZ][1]*b[1] + b[2]*b[2]);

  double ha = atan2(a[2], a[1]);
  double hb = atan2(b[2], b[1]);

  double C, h;

  //printf("LCh: %f %f %f\n", a[0], C, h);
  //printf("Lab': %f %f %f\n", a[0], C*cos(h), C*sin(h));
  */

  for (int i=0; i<SIZE; i++) {
    double v = (double)i / (SIZE-1);

    tJCh.J = v * a[RAMP_CAM][0] + (1-v) * b[RAMP_CAM][0];
    tJCh.C = v * a[RAMP_CAM][1] + (1-v) * b[RAMP_CAM][1];
    tJCh.h = v * a[RAMP_CAM][2] + (1-v) * b[RAMP_CAM][2];

    cmsCIECAM02Reverse(CAM02, &tJCh, &tXYZ);

    buf[RAMP_CAM][i][0] = tXYZ.X;
    buf[RAMP_CAM][i][1] = tXYZ.Y;
    buf[RAMP_CAM][i][2] = tXYZ.Z;

    tJCh.J = v * a[RAMP_JAB][0] + (1-v) * b[RAMP_JAB][0];
    double Ja = v * a[RAMP_JAB][1] + (1-v) * b[RAMP_JAB][1];
    double Jb = v * a[RAMP_JAB][2] + (1-v) * b[RAMP_JAB][2];
    tJCh.C = sqrt(Ja*Ja + Jb*Jb);
    tJCh.h = 180.0 / M_PI * atan2(Jb, Ja);

    cmsCIECAM02Reverse(CAM02, &tJCh, &tXYZ);

    buf[RAMP_JAB][i][0] = tXYZ.X;
    buf[RAMP_JAB][i][1] = tXYZ.Y;
    buf[RAMP_JAB][i][2] = tXYZ.Z;

    for (int c = 0; c < 3; c++) {
        buf[RAMP_XYZ][i][c] = v * a[RAMP_XYZ][c] + (1-v) * b[RAMP_XYZ][c];
        buf[RAMP_LAB][i][c] = v * a[RAMP_LAB][c] + (1-v) * b[RAMP_LAB][c];
        buf[RAMP_RGB][i][c] = v * a[RAMP_RGB][c] + (1-v) * b[RAMP_RGB][c];
    }

    /*
    buf[i][0] = v * a[0] + (1-v) * b[0];
    buf[i][1] = v * a[1] + (1-v) * b[1];
    buf[i][2] = v * a[2] + (1-v) * b[2];
    */

    /*
    C = v * Ca + (1-v) * Cb;
    h = v * ha + (1-v) * hb;

    buf[i][1] = C*cos(h);
    buf[i][2] = C*sin(h);
    */
  }

  cmsDoTransform(back, buf[RAMP_CAM], out[RAMP_CAM], SIZE);
  cmsDoTransform(back, buf[RAMP_JAB], out[RAMP_JAB], SIZE);
  cmsDoTransform(back, buf[RAMP_XYZ], out[RAMP_XYZ], SIZE);
  cmsDoTransform(backlab, buf[RAMP_LAB], out[RAMP_LAB], SIZE);

  for (int i = 0; i < SIZE; i++) {
      for (int c = 0; c < 3; c++)
          out[RAMP_RGB][i][c] = round(buf[RAMP_RGB][i][c] * 255.0);
  }

  printf("P3\n1000\n800\n255\n");
  for (int r = 0; r < RAMP_CAM; r++) {
      for (int h=0; h<200; h++) {
        for (int i=0; i<SIZE; i++) {
          printf("%d %d %d ", out[r][i][0], out[r][i][1], out[r][i][2]);
        }
        printf("\n");
      }
  }

  cmsCIECAM02Done(CAM02);
  cmsDeleteTransform(forw);
  cmsDeleteTransform(back);
  cmsCloseProfile(sRGB);
  cmsCloseProfile(Lab);
  cmsCloseProfile(XYZ);
}
