#include "colors.h"
#include <math.h>

//
// routine to convert HSV to RGB
//
// Reference:  Foley, van Dam, Feiner, Hughes,
//		"Computer Graphics Principles and Practices,"
//		Additon-Wesley, 1990, pp592-593.
void HsvRgb(float hsv[3], float rgb[3])
{
    float h, s, v;			// hue, sat, value
    float r, g, b;			// red, green, blue
    float i, f, p, q, t;		// interim values


    // guarantee valid input:

    h = hsv[0] / 60.;
    while (h >= 6.)	h -= 6.;
    while (h < 0.) 	h += 6.;

    s = hsv[1];
    if (s < 0.)
        s = 0.;
    if (s > 1.)
        s = 1.;

    v = hsv[2];
    if (v < 0.)
        v = 0.;
    if (v > 1.)
        v = 1.;


    // if sat==0, then is a gray:

    if (s == 0.0)
    {
        rgb[0] = rgb[1] = rgb[2] = v;
        return;
    }


    // get an rgb from the hue itself:

    i = floor(h);
    f = h - i;
    p = v * (1. - s);
    q = v * (1. - s * f);
    t = v * (1. - (s * (1. - f)));

    switch ((int)i)
    {
    case 0:
        r = v;	g = t;	b = p;
        break;

    case 1:
        r = q;	g = v;	b = p;
        break;

    case 2:
        r = p;	g = v;	b = t;
        break;

    case 3:
        r = p;	g = q;	b = v;
        break;

    case 4:
        r = t;	g = p;	b = v;
        break;

    case 5:
        r = v;	g = p;	b = q;
        break;
    }


    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

void GetColorFromScheme(int ColorSchemeId, float ColorSchemeMidPoint, float rgb[3], float normalized_value)
{
    if (normalized_value > 1)
    {
        normalized_value = 1;
    }
    else if (normalized_value < 0)
    {
        normalized_value = 0;
    }

    if (ColorSchemeId == RAINBOW)
    {
        Rainbow(rgb, normalized_value);
    }
    else if (ColorSchemeId == HEAT_MAP)
    {
        HeatMap(rgb, normalized_value);
    }
    else if (ColorSchemeId == BLUE_WHITE_RED)
    {
        BlueWhiteRed(rgb, normalized_value, ColorSchemeMidPoint);
    }
    else if (ColorSchemeId == DISCRETE)
    {
        DiscreteColorScheme(rgb, normalized_value);
    }
}

void BlueWhiteRed(float rgb[3], float val, float midPoint)
{
    float hsv[3];

    if (val < midPoint) // Blue-White
    {
        hsv[0] = 240;
        hsv[1] = 1 - val / midPoint;
    }
    else if (val >= midPoint) // White-Red
    {
        hsv[0] = 0;
        hsv[1] = val / midPoint - 1;
    }

    hsv[2] = 1;

    HsvRgb(hsv, rgb);
}

void HeatMap(float rgb[3], float val)
{
    if (val < 1.0 / 3.0)
    {
        rgb[0] = val * 3.0;
        rgb[1] = 0;
        rgb[2] = 0;
    }
    else if (val >= 1.0 / 3.0 && val < 2.0 / 3.0)
    {
        rgb[0] = 1;
        rgb[1] = (val - 1.0 / 3.0) * 3.0;
        rgb[2] = 0;
    }
    else
    {
        rgb[0] = 1;
        rgb[1] = 1;
        rgb[2] = (val - 2.0 / 3.0) * 3.0;
    }
}

void Rainbow(float rgb[3], float val)
{
    float hsv[3];
    hsv[0] = (1.0 - val) * 240.0;
    hsv[1] = hsv[2] = 1;

    HsvRgb(hsv, rgb);
}

void DiscreteColorScheme(float rgb[3], float val)
{
    float hsv[3];

    if (val < 1.0 / 7.0)
    {
        hsv[0] = 240;
    }
    else if (val < 2.0 / 7.0)
    {
        hsv[0] = 200;
    }
    else if (val < 3.0 / 7.0)
    {
        hsv[0] = 160;
    }
    else if (val < 4.0 / 7.0)
    {
        hsv[0] = 120;
    }
    else if (val < 5.0 / 7.0)
    {
        hsv[0] = 80;
    }
    else if (val < 6.0 / 7.0)
    {
        hsv[0] = 40;
    }
    else
    {
        hsv[0] = 0;
    }

    hsv[1] = 1;
    hsv[2] = 1;

    HsvRgb(hsv, rgb);
}