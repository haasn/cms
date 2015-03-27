#include <stdio.h>
#include <string.h>
#include <magick/MagickCore.h>

int main(int argc, char **argv)
{
  if (argc != 2) exit(1);

  MagickCoreGenesis(*argv, MagickFalse);

  ImageInfo *info = AcquireImageInfo();
  ExceptionInfo *err = AcquireExceptionInfo();

  strcpy(info->filename, argv[1]);
  Image *img = PingImage(info, err);

  if (img == NULL) {
    printf("failed opening\n");
    exit(0);
  }

  StringInfo *pinfo = GetImageProfile(img, "ICC");

  if (pinfo == NULL) {
    printf("NULL\n");
    exit(0);
  }

  //printf("path=%s, len=%zu, sig=%zu\n", pinfo->datum, pinfo->length, pinfo->signature);
  fwrite(pinfo->datum, sizeof(unsigned char), pinfo->length, stdout);

  DestroyStringInfo(pinfo);
  DestroyImage(img);
  DestroyImageInfo(info);
  DestroyExceptionInfo(err);
  MagickCoreTerminus();
}
