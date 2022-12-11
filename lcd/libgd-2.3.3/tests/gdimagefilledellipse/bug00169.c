/**
 * add test case for gdImageFilledEllipse
 */

#include "gd.h"
#include "gdtest.h"

int main()
{
	gdImagePtr im;
	int white = 0;

	im = gdImageCreateTrueColor(2200, 2200);
	white = gdImageColorAllocate(im, 255, 255, 255);

	gdImageFilledEllipse(im, 1100, 1100, 2200, 2200, white);
	gdAssertImageEqualsToFile("gdimagefilledellipse/bug00169_exp.png", im);

	gdImageDestroy(im);

	return gdNumFailures();
}
