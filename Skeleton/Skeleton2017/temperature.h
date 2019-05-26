#include <vector>
#include <map>

#pragma once

typedef struct node_temperature
{
    float x, y, z;          // location
    float T;                // temperature
    float r, g, b;          // the assigned color
    float rad;              // radius
    float dTdx, dTdy, dTdz; // can store these if you want, or not
    float grad;             // total gradient
};
typedef struct intersection_temperature
{
    float x, y, z;
};
typedef struct lineseg_temperature
{
    int n1x, n1y, n1z;
    int n2x, n2y, n2z;

    intersection_temperature intersection;
    bool foundIntersection = false;
};
typedef struct quad_temperature
{
    int verts_x[4];
    int verts_y[4];
    int verts_z[4];
    int edges[4];
    std::vector<std::vector<intersection_temperature>> lines;
};

class temperature {
public:
    int NX, NY, NZ;
    int tNX, tNY, tNZ;

    unsigned char TextureXY[64][64][64][4];
    unsigned char TextureXZ[64][64][64][4];
    unsigned char TextureYZ[64][64][64][4];

    std::vector<std::vector<intersection_temperature>> intersection_lines;
    node_temperature ***grid_3d;

    std::vector<lineseg_temperature> horizontalEdgeList;
    std::vector<lineseg_temperature> verticalEdgeList;
    std::vector<quad_temperature> quadList;

    void Build_Edge_List();
    void Build_Face_List();

    temperature::temperature(int aNX, int aNY, int aNZ, int ColorSchemeId, float ColorSchemeMidPoint);
    void temperature::Compute_Gradient();
    void temperature::GetGradientMinMax(float &min, float &max);
    void Build_Lines(float aScalarValue, int aNumberOfContours);
    void MarchingSquares_Algorithm(float s_star, lineseg_temperature &edge0, lineseg_temperature &edge1, lineseg_temperature &edge2, lineseg_temperature &edge3,
        node_temperature &vert0, node_temperature &vert1, node_temperature &vert2, node_temperature &vert3);
    void MarchingSquares_FindIntersection(float s_star, node_temperature &vert0, node_temperature &vert1, lineseg_temperature &edge, std::vector<intersection_temperature> &intersection_locations);
    void Build_Temperatures();

    double Temperature(double x, double y, double z);

    int Get_3D_Index(int x, int y, int z);

    void temperature::Calculate_Colors(int ColorSchemeId, float ColorSchemeMidPoint);
    void temperature::CompositeXY(float MaxAlpha, float minT, float maxT);
    void temperature::CompositeXZ(float MaxAlpha, float minT, float maxT);
    void temperature::CompositeYZ(float MaxAlpha, float minT, float maxT);
};