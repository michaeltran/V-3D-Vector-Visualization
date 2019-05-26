#pragma once

enum COLOR_SCHEME
{
    BLUE_WHITE_RED,
    HEAT_MAP,
    RAINBOW,
    DISCRETE,
};

void	HsvRgb(float[3], float[3]);

void    GetColorFromScheme(int ColorSchemeId, float ColorSchemeMidPoint, float rgb[3], float normalized_value);

void    BlueWhiteRed(float[3], float, float);
void    HeatMap(float[3], float);
void    Rainbow(float[3], float);
void    DiscreteColorScheme(float[3], float);