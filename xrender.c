/*******************************************************************************
**3456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
**      10        20        30        40        50        60        70        80
**
** Copyright (C) 2006-2007 Mirco "MacSlow" Mueller <macslow@gmail.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public
** License along with this program; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA 02111-1307, USA.
**
** CAUTION: Running this code on anything but Xgl or anything below Xorg 7.1
** will crash your X11-session!!! This is mainly due to some non-implemented
** things (filters, conical-gradients) in the earlier X11-servers.
**
*******************************************************************************/

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/extensions/Xrender.h>

#define WIN_WIDTH 384
#define WIN_HEIGHT 384

void
repaint (Display*	pDisplay,
	 Window		window,
	 Visual*	pVisual,
	 int		iWidth,
	 int		iHeight)
{
	XRenderPictFormat*	 pPictFormatDefault = NULL;
	XRenderPictureAttributes pictAttribs;
	XRenderColor		 colorBlack = {0x0000, 0x0000, 0x0000, 0xffff};
	XRenderColor		 colorWhite = {0xffff, 0xffff, 0xffff, 0xffff};
	Picture			 destPict;
	Picture			 gradientPict;
	XTrapezoid		 trapezoid;
	XLinearGradient		 linearGradient;
	int			 iKernelSize;

	/* get some typical formats */
	pPictFormatDefault = XRenderFindVisualFormat (pDisplay, pVisual);

	/* this is my target- or destination-drawable derived from the window */
	destPict = XRenderCreatePicture (pDisplay,
					 window,
					 pPictFormatDefault,
					 0,
					 &pictAttribs);
	/* define a as rectangle/square as trapezoid */
	trapezoid.top = XDoubleToFixed (30.0f);
	trapezoid.bottom = XDoubleToFixed ((double) iHeight - 30.0f);
	trapezoid.left.p1.x = XDoubleToFixed (30.0f);
	trapezoid.left.p1.y = XDoubleToFixed (30.0f);
	trapezoid.left.p2.x = XDoubleToFixed (30.0f);
	trapezoid.left.p2.y = XDoubleToFixed ((double) iHeight - 30.0f);
	trapezoid.right.p1.x = XDoubleToFixed ((double) iWidth - 30.0f);
	trapezoid.right.p1.y = XDoubleToFixed (30.0f);
	trapezoid.right.p2.x = XDoubleToFixed ((double) iWidth - 30.0f);
	trapezoid.right.p2.y = XDoubleToFixed ((double) iHeight - 30.0f);

	/* coordinates for the start- and end-point of the linear gradient are
	** in  window-space, they are not normalized like in cairo... here using
	** a 10th of the width so it gradient will repeat 10 times sideways if
	** the repeat-attribute is used (see further below) */
	linearGradient.p1.x = XDoubleToFixed (0.0f);
	linearGradient.p1.y = XDoubleToFixed (0.0f);
	linearGradient.p2.x = XDoubleToFixed ((double) iWidth);
	linearGradient.p2.y = XDoubleToFixed (0.0f);

        gradientPict = XRenderCreateLinearGradient (pDisplay, &linearGradient,
                (XFixed[]) {XDoubleToFixed(0.0f), XDoubleToFixed(1.0f)},
                (XRenderColor[]) {colorBlack, colorWhite}, 2);

	/* make the gradient in the gradient picture repeat */
	pictAttribs.repeat = True;
	XRenderChangePicture (pDisplay,
			      gradientPict,
			      CPRepeat,
			      &pictAttribs);

        XRenderComposite (pDisplay,
                          PictOpSrc,
                          gradientPict,
                          None,
                          destPict,
                          0, 0,
                          0, 0,
                          0, 0,
                          iWidth, iHeight);

	XRenderFreePicture (pDisplay, destPict);
	XRenderFreePicture (pDisplay, gradientPict);
}

int
main (int    argc,
      char** argv)
{
	Display*	     pDisplay = NULL;
	XEvent		     event;
	Bool		     bKeepGoing = True;
	int		     iEventBase;
	int		     iErrorBase;
	int		     iMajorVersion;
	int		     iMinorVersion;
	XSetWindowAttributes values;
	Window		     window;
	Visual*		     pVisual = NULL;
	int		     iWindowAttribsMask;
	Atom		     property;
	Atom		     type;
	unsigned char	     acWindowTitle[] = "mpv - foo";

	/* connect to display */
	pDisplay = XOpenDisplay (NULL);
	if (!pDisplay)
	{
		printf ("Could not open display\n");
		return 1;
	}

	/* check for Xrender-extension */
	if (!XRenderQueryExtension (pDisplay, &iEventBase, &iErrorBase))
	{
		XCloseDisplay (pDisplay);
		printf ("Xrender-extension not found\n");
		return 2;
	}

	/* see which version is available */
	XRenderQueryVersion (pDisplay, &iMajorVersion, &iMinorVersion);
	if (iMajorVersion == 0 && iMinorVersion < 10)
	{
		XCloseDisplay (pDisplay);
		printf ("At least Xrender 0.10 needed\n");
		return 3;
	}
	printf ("Found Xrender %d.%d\n", iMajorVersion, iMinorVersion);

	/* make operations asynchronous */
	XSynchronize (pDisplay, True);

	/* get the default visual for this display */
        XVisualInfo vinfo;
        XMatchVisualInfo(pDisplay, 0, 32, TrueColor, &vinfo);
        pVisual = vinfo.visual;
        printf("pVisual->bits_per_rgb = %d\n", pVisual->bits_per_rgb);
        printf("VisualUD = %d\n", XVisualIDFromVisual(pVisual));

	/* create and open window */
	values.background_pixel = WhitePixel (pDisplay, 0);
	values.event_mask = KeyPressMask | ExposureMask;
        values.border_pixel = 0;
        values.colormap = XCreateColormap(pDisplay, XDefaultRootWindow(pDisplay), pVisual, AllocNone);
	iWindowAttribsMask = CWEventMask | CWBackPixel | CWColormap | CWBorderPixel;
	window = XCreateWindow (pDisplay,
				RootWindow (pDisplay, 0),
				50, 50, WIN_WIDTH, WIN_HEIGHT, 0,
				32,
                                InputOutput,
				pVisual,
				iWindowAttribsMask,
				&values);
	XMapWindow (pDisplay, window);

	/* this is indeed how you change the window-title under pure X11 */
	property = XInternAtom (pDisplay, "_NET_WM_NAME", True);
	type = XInternAtom (pDisplay, "UTF8_STRING", True);
	XChangeProperty (pDisplay,
			 window,
			 property,
			 type,
			 8,
			 PropModeReplace,
			 acWindowTitle,
			 30);

	XSelectInput (pDisplay, window, KeyPressMask | ExposureMask);

	while (bKeepGoing)
	{
		XNextEvent (pDisplay, &event);
		switch (event.type)
		{
			case KeyPress :
				if (XLookupKeysym (&event.xkey, 0) == XK_Escape)
					bKeepGoing = False;
			break;

			case Expose :
				if (event.xexpose.count < 1)
					repaint (pDisplay,
						 window,
						 pVisual,
						 event.xexpose.width,
						 event.xexpose.height);
			break;
		}
	}

	XUnmapWindow (pDisplay, window);
	XDestroyWindow (pDisplay, window);
	XCloseDisplay (pDisplay);

	return 0;
}


