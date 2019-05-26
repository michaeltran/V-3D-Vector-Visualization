#include <vector>
#include <map>

#pragma once

typedef struct node {
    float x, y, z; // To store the coordinate of the node
    float nx, ny, nz;
    float s; // To store the scalar value
};

typedef struct lineseg
{
    int n1, n2;

    node intersection;
    bool foundIntersection = false;
};
typedef struct quad
{
    int verts[4];
    int edges[4];
    std::vector<std::vector<node>> lines;
};

class dat {
public:
    int NX, NY;
    std::vector<node> vertexList;
    std::vector<lineseg> edgeList;
    std::vector<quad> quadList;

    dat(std::string);
    void Build_Edge_List();
    void Build_Face_List();
    void Build_Lines(float aScalarValue, int aNumberOfContours);
    void MarchingSquares_FindIntersection(float s_star, lineseg &edge, node &vert0, node &vert1, std::vector<node> &intersection_locations);
};