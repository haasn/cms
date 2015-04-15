#include <stdlib.h>
#include <lcms2.h>

int main() {
  cmsCIExyY wp = {0.3127, 0.3290, 1.0}; // D65
  // BT709 spec
  cmsCIExyYTRIPLE prim = {
    .Red   = {0.64, 0.33, 1.0},
    .Green = {0.30, 0.60, 1.0},
    .Blue  = {0.15, 0.06, 1.0},
  };

  cmsToneCurve *tc = cmsBuildGamma(NULL, 2.2);
  cmsHPROFILE bt709 = cmsCreateRGBProfile( &wp, &prim, (cmsToneCurve*[3]) { tc, tc, tc });
  cmsFreeToneCurve(tc);

  if (!bt709) {
    printf("Failed creating profile\n");
    exit(1);
  }

  cmsMLU *mlu = cmsMLUalloc(NULL, 1);
  cmsMLUsetASCII(mlu, cmsNoLanguage, cmsNoCountry, "BT.709 with gamma 2.2");
  cmsWriteTag(bt709, cmsSigProfileDescriptionTag, mlu);
  cmsMLUfree(mlu);
  cmsSetProfileVersion(bt709, 2.0);
  cmsSaveProfileToFile(bt709, "bt709-gamma2.2.icc");
  cmsCloseProfile(bt709);
}
