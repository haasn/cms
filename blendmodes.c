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
  cmsViewingConditions cond = {
    .whitePoint = D65_XYZ,
    .Yb = 0.2,
    .La = 120,
    .surround = DIM_SURROUND,
    .D_value = D_CALCULATE,
  };
  cmsHANDLE CAM02 = cmsCIECAM02Init(NULL, &cond);

#define SIZE 1000

  unsigned char start[3] = {0,255,0}, end[3] = {255,0,0};
  double a[3], b[3];

  double buf[SIZE][3];
  unsigned char out[SIZE][3];

  cmsDoTransform(forw, start, a, 1);
  cmsDoTransform(forw, end,   b, 1);

  cmsCIEXYZ tXYZ;
  cmsJCh tJCh;

  tXYZ.X = a[0];
  tXYZ.Y = a[1];
  tXYZ.Z = a[2];

  cmsCIECAM02Forward(CAM02, &tXYZ, &tJCh);

  a[0] = tJCh.J;
  a[1] = tJCh.C;
  a[2] = tJCh.h;

  tXYZ.X = b[0];
  tXYZ.Y = b[1];
  tXYZ.Z = b[2];

  cmsCIECAM02Forward(CAM02, &tXYZ, &tJCh);

  b[0] = tJCh.J;
  b[1] = tJCh.C;
  b[2] = tJCh.h;


  /*
  double Ca = sqrt(a[1]*a[1] + a[2]*a[2]);
  double Cb = sqrt(b[1]*b[1] + b[2]*b[2]);

  double ha = atan2(a[2], a[1]);
  double hb = atan2(b[2], b[1]);

  double C, h;
  */

  //printf("LCh: %f %f %f\n", a[0], C, h);
  //printf("Lab': %f %f %f\n", a[0], C*cos(h), C*sin(h));

  for (int i=0; i<SIZE; i++) {
    double v = (double)i / (SIZE-1);

    tJCh.J = v * a[0] + (1-v) * b[0];
    tJCh.C = v * a[1] + (1-v) * b[1];
    tJCh.h = v * a[2] + (1-v) * b[2];

    cmsCIECAM02Reverse(CAM02, &tJCh, &tXYZ);

    buf[i][0] = tXYZ.X;
    buf[i][1] = tXYZ.Y;
    buf[i][2] = tXYZ.Z;

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

  cmsDoTransform(back, buf, out, SIZE);

  for (int h=0; h<200; h++) {
    for (int i=0; i<SIZE; i++) {
      printf("%d %d %d ", out[i][0], out[i][1], out[i][2]);
    }
    printf("\n");
  }

  cmsCIECAM02Done(CAM02);
  cmsDeleteTransform(forw);
  cmsDeleteTransform(back);
  cmsCloseProfile(sRGB);
  cmsCloseProfile(Lab);
  cmsCloseProfile(XYZ);
}
