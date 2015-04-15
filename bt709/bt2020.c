#include <stdlib.h>
#include <lcms2.h>

int main() {
  cmsCIExyY wp = {0.3127, 0.3290, 1.0}; // D65
  // BT2020 spec
  cmsCIExyYTRIPLE prim = {
    .Red   = {0.708, 0.292},
    .Green = {0.170, 0.797},
    .Blue  = {0.131, 0.046},
  };

  cmsToneCurve *tc = cmsBuildGamma(NULL, 2.2);
  cmsHPROFILE bt2020 = cmsCreateRGBProfile( &wp, &prim, (cmsToneCurve*[3]) { tc, tc, tc });
  cmsFreeToneCurve(tc);

  if (!bt2020) {
    printf("Failed creating profile\n");
    exit(1);
  }

  cmsSetProfileVersion(bt2020, 2.0);
  cmsSaveProfileToFile(bt2020, "bt2020-gamma2.2.icc");
  cmsCloseProfile(bt2020);
}
