#include "temperature.h"
#include "colors.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

#include <string>

temperature::temperature(int aNX, int aNY, int aNZ, int ColorSchemeId, float ColorSchemeMidPoint)
{
    NX = aNX;
    NY = aNY;
    NZ = aNZ;

    tNX = 64;
    tNY = 64;
    tNZ = 64;

    grid_3d = new node_temperature**[NX];
    for (int x = 0; x < NX; ++x) {
        grid_3d[x] = new node_temperature*[NY];
        for (int y = 0; y < NY; ++y) {
            grid_3d[x][y] = new node_temperature[NZ];
        }
    }

    Build_Temperatures();
    Calculate_Colors(ColorSchemeId, ColorSchemeMidPoint);

    Compute_Gradient();
    Build_Edge_List();
    Build_Face_List();
}

void temperature::Compute_Gradient()
{
    for (int x = 0; x < NX; ++x) {
        for (int y = 0; y < NY; ++y) {
            for (int z = 0; z < NZ; ++z) {
                if (x == 0)
                {
                    grid_3d[x][y][z].dTdx = (grid_3d[x + 1][y][z].T - grid_3d[x][y][z].T) / (grid_3d[x + 1][y][z].x - grid_3d[x][y][z].x);
                }
                else if (x == NX - 1)
                {
                    grid_3d[x][y][z].dTdx = (grid_3d[x][y][z].T - grid_3d[x - 1][y][z].T) / (grid_3d[x][y][z].x - grid_3d[x - 1][y][z].x);
                }
                else
                {
                    grid_3d[x][y][z].dTdx = (grid_3d[x + 1][y][z].T - grid_3d[x - 1][y][z].T) / (grid_3d[x + 1][y][z].x - grid_3d[x - 1][y][z].x);
                }

                if (y == 0)
                {
                    grid_3d[x][y][z].dTdy = (grid_3d[x][y + 1][z].T - grid_3d[x][y][z].T) / (grid_3d[x][y + 1][z].y - grid_3d[x][y][z].y);
                }
                else if (y == NY - 1)
                {
                    grid_3d[x][y][z].dTdy = (grid_3d[x][y][z].T - grid_3d[x][y - 1][z].T) / (grid_3d[x][y][z].y - grid_3d[x][y - 1][z].y);
                }
                else
                {
                    grid_3d[x][y][z].dTdy = (grid_3d[x][y + 1][z].T - grid_3d[x][y - 1][z].T) / (grid_3d[x][y + 1][z].y - grid_3d[x][y - 1][z].y);
                }

                if (z == 0)
                {
                    grid_3d[x][y][z].dTdz = (grid_3d[x][y][z + 1].T - grid_3d[x][y][z].T) / (grid_3d[x][y][z + 1].z - grid_3d[x][y][z].z);
                }
                else if (z == NZ - 1)
                {
                    grid_3d[x][y][z].dTdz = (grid_3d[x][y][z].T - grid_3d[x][y][z - 1].T) / (grid_3d[x][y][z].z - grid_3d[x][y][z - 1].z);
                }
                else
                {
                    grid_3d[x][y][z].dTdz = (grid_3d[x][y][z + 1].T - grid_3d[x][y][z - 1].T) / (grid_3d[x][y][z + 1].z - grid_3d[x][y][z - 1].z);
                }

                grid_3d[x][y][z].grad = pow((pow(grid_3d[x][y][z].dTdx, 2) + pow(grid_3d[x][y][z].dTdy, 2) + pow(grid_3d[x][y][z].dTdz, 2)), 0.5);
            }
        }
    }
}

void temperature::GetGradientMinMax(float &min, float &max)
{
    min = grid_3d[0][0][0].grad;
    max = grid_3d[0][0][0].grad;

    for (int x = 0; x < NX; ++x) {
        for (int y = 0; y < NY; ++y) {
            for (int z = 0; z < NZ; ++z) {
                if (grid_3d[x][y][z].grad < min)
                {
                    min = grid_3d[x][y][z].grad;
                }
                if (grid_3d[x][y][z].grad > max)
                {
                    max = grid_3d[x][y][z].grad;
                }
            }
        }
    }
}

void temperature::Build_Edge_List()
{
    int i, j, k;

    // XY Plane
    for (k = 0; k < NZ; k++)
    {
        for (j = 0; j < NY - 1; j++)
        {
            for (i = 0; i < NX - 1; i++)
            {
                lineseg_temperature horizontal_seg;
                horizontal_seg.n1x = i;
                horizontal_seg.n1y = j;
                horizontal_seg.n1z = k;
                horizontal_seg.n2x = i + 1;
                horizontal_seg.n2y = j;
                horizontal_seg.n2z = k;
                horizontalEdgeList.push_back(horizontal_seg);

                lineseg_temperature vertical_seg;
                vertical_seg.n1x = i;
                vertical_seg.n1y = j;
                vertical_seg.n1z = k;
                vertical_seg.n2x = i;
                vertical_seg.n2y = j + 1;
                vertical_seg.n2z = k;
                verticalEdgeList.push_back(vertical_seg);
            }

            lineseg_temperature rightmost_seg;
            rightmost_seg.n1x = NX - 1;
            rightmost_seg.n1y = j;
            rightmost_seg.n1z = k;
            rightmost_seg.n2x = NX - 1;
            rightmost_seg.n2y = j + 1;
            rightmost_seg.n2z = k;
            verticalEdgeList.push_back(rightmost_seg);
        }
        // Build the edges on the top boundary
        for (i = 0; i < NX - 1; i++)
        {
            lineseg_temperature topmost_seg;
            topmost_seg.n1x = i;
            topmost_seg.n1y = NY - 1;
            topmost_seg.n1z = k;
            topmost_seg.n2x = i + 1;
            topmost_seg.n2y = NY - 1;
            topmost_seg.n2z = k;
            horizontalEdgeList.push_back(topmost_seg);
        }
    }
}

void temperature::Build_Face_List()
{
    int i, j, k;
    quadList.clear();
    //XY Plane Face List, using some index magic, we can use this for XZ and YZ planes as well!
    for (k = 0; k < NZ; k++)
    {
        for (j = 0; j < NY - 1; j++)
        {
            for (i = 0; i < NX - 1; i++)
            {
                quad_temperature q;

                // 0 = horizontal bottom
                // 1 = horizontal top
                // 2 = vertical left
                // 3 = vertical right

                q.edges[0] = i + (j * (NY - 1)) + (k * NY * (NZ - 1));
                q.edges[1] = (i + NX - 1) + (j * (NY - 1))  + (k * NY * (NZ - 1));
                q.edges[2] = i + (j * NY) + (k * NY * (NZ - 1));
                q.edges[3] = (i + 1) + (j * NY) + (k * NY * (NZ - 1));

                lineseg_temperature bottom_edge = horizontalEdgeList[q.edges[0]];
                lineseg_temperature top_edge = horizontalEdgeList[q.edges[1]];
                lineseg_temperature left_edge = verticalEdgeList[q.edges[2]];
                lineseg_temperature right_edge = verticalEdgeList[q.edges[3]];

                q.verts_x[0] = bottom_edge.n1x;
                q.verts_y[0] = bottom_edge.n1y;
                q.verts_z[0] = bottom_edge.n1z;

                q.verts_x[1] = right_edge.n1x;
                q.verts_y[1] = right_edge.n1y;
                q.verts_z[1] = right_edge.n1z;

                q.verts_x[2] = top_edge.n2x;
                q.verts_y[2] = top_edge.n2y;
                q.verts_z[2] = top_edge.n2z;

                q.verts_x[3] = left_edge.n2x;
                q.verts_y[3] = left_edge.n2y;
                q.verts_z[3] = left_edge.n2z;

                quadList.push_back(q);
            }
        }
    }
}

void temperature::Build_Lines(float aScalarValue, int aNumberOfContours)
{
    intersection_lines.clear();

    // Pre-compute Min/Max
    float Smin, Smax;

    Smin = grid_3d[0][0][0].T;
    Smax = Smin;

    for (int i = 0; i < NX; i++)
    {
        for (int j = 0; j < NY; j++)
        {
            for (int k = 0; k < NZ; k++)
            {
                float S = grid_3d[i][j][k].T;

                if (S < Smin)
                {
                    Smin = S;
                }
                else if (S > Smax)
                {
                    Smax = S;
                }
            }
        }
    }

    for (int iter = 0; iter < aNumberOfContours; iter++)
    {
        float s_star = 0;
        if (aNumberOfContours == 1)
        {
            s_star = aScalarValue;
        }
        else if (aNumberOfContours > 1)
        {
            s_star = ((Smax - Smin) / (aNumberOfContours + 1)) * (iter + 1);
        }

        // Reset
        for (int i = 0; i < horizontalEdgeList.size(); i++)
        {
            horizontalEdgeList[i].foundIntersection = false;
        }
        for (int i = 0; i < verticalEdgeList.size(); i++)
        {
            verticalEdgeList[i].foundIntersection = false;
        }

        //XY
        for (int i = 0; i < quadList.size(); i++)
        {
            lineseg_temperature *bottom_edge = &horizontalEdgeList[quadList[i].edges[0]];
            lineseg_temperature *top_edge = &horizontalEdgeList[quadList[i].edges[1]];
            lineseg_temperature *left_edge = &verticalEdgeList[quadList[i].edges[2]];
            lineseg_temperature *right_edge = &verticalEdgeList[quadList[i].edges[3]];

            node_temperature *vert0 = &grid_3d[quadList[i].verts_x[0]][quadList[i].verts_y[0]][quadList[i].verts_z[0]];
            node_temperature *vert1 = &grid_3d[quadList[i].verts_x[1]][quadList[i].verts_y[1]][quadList[i].verts_z[1]];
            node_temperature *vert2 = &grid_3d[quadList[i].verts_x[2]][quadList[i].verts_y[2]][quadList[i].verts_z[2]];
            node_temperature *vert3 = &grid_3d[quadList[i].verts_x[3]][quadList[i].verts_y[3]][quadList[i].verts_z[3]];

            MarchingSquares_Algorithm(s_star, *bottom_edge, *right_edge, *top_edge, *left_edge, *vert0, *vert1, *vert2, *vert3);
        }

        // Reset
        for (int i = 0; i < horizontalEdgeList.size(); i++)
        {
            horizontalEdgeList[i].foundIntersection = false;
        }
        for (int i = 0; i < verticalEdgeList.size(); i++)
        {
            verticalEdgeList[i].foundIntersection = false;
        }

        //XZ
        for (int i = 0; i < quadList.size(); i++)
        {
            lineseg_temperature *bottom_edge = &horizontalEdgeList[quadList[i].edges[0]];
            lineseg_temperature *top_edge = &horizontalEdgeList[quadList[i].edges[1]];
            lineseg_temperature *left_edge = &verticalEdgeList[quadList[i].edges[2]];
            lineseg_temperature *right_edge = &verticalEdgeList[quadList[i].edges[3]];

            // Flip Y and Z to reuse edgeList
            node_temperature *vert0 = &grid_3d[quadList[i].verts_x[0]][quadList[i].verts_z[0]][quadList[i].verts_y[0]];
            node_temperature *vert1 = &grid_3d[quadList[i].verts_x[1]][quadList[i].verts_z[1]][quadList[i].verts_y[1]];
            node_temperature *vert2 = &grid_3d[quadList[i].verts_x[2]][quadList[i].verts_z[2]][quadList[i].verts_y[2]];
            node_temperature *vert3 = &grid_3d[quadList[i].verts_x[3]][quadList[i].verts_z[3]][quadList[i].verts_y[3]];

            MarchingSquares_Algorithm(s_star, *bottom_edge, *right_edge, *top_edge, *left_edge, *vert0, *vert1, *vert2, *vert3);
        }

        // Reset
        for (int i = 0; i < horizontalEdgeList.size(); i++)
        {
            horizontalEdgeList[i].foundIntersection = false;
        }
        for (int i = 0; i < verticalEdgeList.size(); i++)
        {
            verticalEdgeList[i].foundIntersection = false;
        }

        //YZ
        for (int i = 0; i < quadList.size(); i++)
        {
            lineseg_temperature *bottom_edge = &horizontalEdgeList[quadList[i].edges[0]];
            lineseg_temperature *top_edge = &horizontalEdgeList[quadList[i].edges[1]];
            lineseg_temperature *left_edge = &verticalEdgeList[quadList[i].edges[2]];
            lineseg_temperature *right_edge = &verticalEdgeList[quadList[i].edges[3]];

            // Flip X and Z to reuse edgeList
            node_temperature *vert0 = &grid_3d[quadList[i].verts_z[0]][quadList[i].verts_y[0]][quadList[i].verts_x[0]];
            node_temperature *vert1 = &grid_3d[quadList[i].verts_z[1]][quadList[i].verts_y[1]][quadList[i].verts_x[1]];
            node_temperature *vert2 = &grid_3d[quadList[i].verts_z[2]][quadList[i].verts_y[2]][quadList[i].verts_x[2]];
            node_temperature *vert3 = &grid_3d[quadList[i].verts_z[3]][quadList[i].verts_y[3]][quadList[i].verts_x[3]];

            MarchingSquares_Algorithm(s_star, *bottom_edge, *right_edge, *top_edge, *left_edge, *vert0, *vert1, *vert2, *vert3);
        }
    }
}

void temperature::MarchingSquares_Algorithm(float s_star, lineseg_temperature &edge0, lineseg_temperature &edge1, lineseg_temperature &edge2, lineseg_temperature &edge3,
    node_temperature &vert0, node_temperature &vert1, node_temperature &vert2, node_temperature &vert3)
{
    std::vector<intersection_temperature> intersection_locations;

    MarchingSquares_FindIntersection(s_star, vert0, vert1, edge0, intersection_locations);
    MarchingSquares_FindIntersection(s_star, vert1, vert2, edge1, intersection_locations);
    MarchingSquares_FindIntersection(s_star, vert3, vert2, edge2, intersection_locations);
    MarchingSquares_FindIntersection(s_star, vert0, vert3, edge3, intersection_locations);

    int intersections = intersection_locations.size();
    if (intersections == 2)
    {
        // Connect Edges
        intersection_lines.push_back(intersection_locations);
    }
    else if (intersections == 0)
    {
        // Skip
    }
    else if (intersections == 1)
    {
    }
    else if (intersections == 3)
    {
    }
    else if (intersections == 4)
    {
        float M = (vert0.T + vert1.T + vert2.T + vert3.T) / 4;

        if (vert0.T <= M)
        {
            // Connect Edge 1 and 4
            // Connect Edge 2 and 3

            std::vector<intersection_temperature> newLine1;
            newLine1.push_back(intersection_locations[0]);
            newLine1.push_back(intersection_locations[3]);
            intersection_lines.push_back(newLine1);

            std::vector<intersection_temperature> newLine2;
            newLine2.push_back(intersection_locations[1]);
            newLine2.push_back(intersection_locations[2]);
            intersection_lines.push_back(newLine2);
        }
        else
        {
            // Connect Edge 1 and 2
            // Connect Edge 3 and 4

            std::vector<intersection_temperature> newLine1;
            newLine1.push_back(intersection_locations[0]);
            newLine1.push_back(intersection_locations[1]);
            intersection_lines.push_back(newLine1);

            std::vector<intersection_temperature> newLine2;
            newLine2.push_back(intersection_locations[2]);
            newLine2.push_back(intersection_locations[3]);
            intersection_lines.push_back(newLine2);
        }
    }
    else
    {
        std::cout << "Something bad has happened\n";
    }
}

void temperature::MarchingSquares_FindIntersection(float s_star, node_temperature &vert0, node_temperature &vert1, lineseg_temperature &edge, std::vector<intersection_temperature> &intersection_locations)
{
    if (edge.foundIntersection)
    {
        intersection_locations.push_back(edge.intersection);
    }
    else 
    {
        float t_star = (s_star - vert0.T) / (vert1.T - vert0.T);

        if (t_star >= 0 && t_star <= 1)
        {
            float x_star = (1 - t_star) * vert0.x + t_star * vert1.x;
            float y_star = (1 - t_star) * vert0.y + t_star * vert1.y;
            float z_star = (1 - t_star) * vert0.z + t_star * vert1.z;

            intersection_temperature intersection_location;
            intersection_location.x = x_star;
            intersection_location.y = y_star;
            intersection_location.z = z_star;

            intersection_locations.push_back(intersection_location);

            edge.intersection = intersection_location;
            edge.foundIntersection = true;
        }
    }
}

void temperature::Build_Temperatures()
{
    double current_x = -1;
    double current_y = -1;
    double current_z = -1;

    for (int x = 0; x < NX; x++)
    {
        current_y = -1;
        for (int y = 0; y < NY; y++)
        {
            current_z = -1;
            for (int z = 0; z < NZ; z++)
            {
                grid_3d[x][y][z].x = current_x;
                grid_3d[x][y][z].y = current_y;
                grid_3d[x][y][z].z = current_z;
                grid_3d[x][y][z].T = Temperature(current_x, current_y, current_z);

                current_z = current_z + (2.0 / double(NZ - 1));
            }
            current_y = current_y + (2.0 / double(NY - 1));
        }
        current_x = current_x + (2.0 / double(NX - 1));
    }
}

// Hardcoded Function Values
struct sources
{
    double xc, yc, zc; //
    double a; // temperature value of the source
} Sources[] =
{
    { 1.00f, 0.00f, 0.00f, 90.00f },
    { -1.00f, -0.30f, 0.00f, 120.00f },
    { 0.10f, 1.00f, 0.00f, 120.00f },
    { 0.00f, 0.40f, 1.00f, 170.00f },
};

// The following function is going to be used for the next assignment as well
double temperature::Temperature(double x, double y, double z)
{
    const float TEMPMIN = { 0.f };
    const float TEMPMAX = { 100.f };

    double t = 0.0;
    for (int i = 0; i < 4; i++)
    {
        double dx = x - Sources[i].xc;
        double dy = y - Sources[i].yc;
        double dz = z - Sources[i].zc;
        double rsqd = dx * dx + dy * dy + dz * dz;
        t += Sources[i].a * exp(-5.*rsqd);
    }
    if (t > TEMPMAX)
        t = TEMPMAX;
    return t;
}

int temperature::Get_3D_Index(int x, int y, int z)
{
    return x + NY * (y + NZ * z);
}

void temperature::Calculate_Colors(int ColorSchemeId, float ColorSchemeMidPoint)
{
    float Smin = grid_3d[0][0][0].T;
    float Smax = Smin;

    for (int i = 0; i < NX; i++)
    {
        for (int j = 0; j < NY; j++)
        {
            for (int k = 0; k < NZ; k++)
            {
                float S = grid_3d[i][j][k].T;

                //if (NonLinearOn != 0)
                //{
                //    S = S * S * S;
                //}

                if (S < Smin)
                {
                    Smin = S;
                }
                else if (S > Smax)
                {
                    Smax = S;
                }
            }
        }
    }

    float rgb[3];

    for (int x = 0; x < NX; x++)
    {
        for (int y = 0; y < NY; y++)
        {
            for (int z = 0; z < NZ; z++)
            {
                float S = grid_3d[x][y][z].T;
                float normalized_value = (S - Smin) / (Smax - Smin);

                GetColorFromScheme(ColorSchemeId, ColorSchemeMidPoint, rgb, normalized_value);

                grid_3d[x][y][z].r = rgb[0];
                grid_3d[x][y][z].g = rgb[1];
                grid_3d[x][y][z].b = rgb[2];
            }
        }
    }
}

void temperature::CompositeXY(float MaxAlpha, float minT, float maxT)
{
    int x, y, z, zz;
    float alpha; /* opacity at this voxel */
    float r, g, b; /* running color composite */
    for (x = 0; x < tNX; x++)
    {
        for (y = 0; y < tNY; y++)
        {
            r = g = b = 0.;
            for (zz = 0; zz < tNZ; zz++)
            {
                z = zz;

                float T = grid_3d[x][y][z].T;
                if (T < minT || T > maxT) // determine whether the value is out of the
                // range set by the range slider
                {
                    r = g = b = 0.;
                    alpha = 0.;
                }
                else
                {
                    r = grid_3d[x][y][z].r;
                    g = grid_3d[x][y][z].g;
                    b = grid_3d[x][y][z].b;
                    alpha = MaxAlpha;
                }

                TextureXY[zz][y][x][0] = (unsigned char)(255.*r + .5);
                TextureXY[zz][y][x][1] = (unsigned char)(255.*g + .5);
                TextureXY[zz][y][x][2] = (unsigned char)(255.*b + .5);
                TextureXY[zz][y][x][3] = (unsigned char)(255.*alpha + .5);
            }
        }
    }
}

void temperature::CompositeXZ(float MaxAlpha, float minT, float maxT)
{
    int x, y, z, zz;
    float alpha; /* opacity at this voxel */
    float r, g, b; /* running color composite */
    for (x = 0; x < tNX; x++)
    {
        for (y = 0; y < tNZ; y++)
        {
            r = g = b = 0.;
            for (zz = 0; zz < tNY; zz++)
            {
                z = zz;

                float T = grid_3d[x][z][y].T;
                if (T < minT || T > maxT) // determine whether the value is out of the
                // range set by the range slider
                {
                    r = g = b = 0.;
                    alpha = 0.;
                }
                else
                {
                    r = grid_3d[x][z][y].r;
                    g = grid_3d[x][z][y].g;
                    b = grid_3d[x][z][y].b;
                    alpha = MaxAlpha;
                }

                TextureXZ[zz][y][x][0] = (unsigned char)(255.*r + .5);
                TextureXZ[zz][y][x][1] = (unsigned char)(255.*g + .5);
                TextureXZ[zz][y][x][2] = (unsigned char)(255.*b + .5);
                TextureXZ[zz][y][x][3] = (unsigned char)(255.*alpha + .5);
            }
        }
    }
}

void temperature::CompositeYZ(float MaxAlpha, float minT, float maxT)
{
    int x, y, z, zz;
    float alpha; /* opacity at this voxel */
    float r, g, b; /* running color composite */
    for (x = 0; x < tNY; x++)
    {
        for (y = 0; y < tNZ; y++)
        {
            r = g = b = 0.;
            for (zz = 0; zz < tNX; zz++)
            {
                z = zz;

                float T = grid_3d[z][x][y].T;
                if (T < minT || T > maxT) // determine whether the value is out of the
                // range set by the range slider
                {
                    r = g = b = 0.;
                    alpha = 0.;
                }
                else
                {
                    r = grid_3d[z][x][y].r;
                    g = grid_3d[z][x][y].g;
                    b = grid_3d[z][x][y].b;
                    alpha = MaxAlpha;
                }

                TextureYZ[zz][y][x][0] = (unsigned char)(255.*r + .5);
                TextureYZ[zz][y][x][1] = (unsigned char)(255.*g + .5);
                TextureYZ[zz][y][x][2] = (unsigned char)(255.*b + .5);
                TextureYZ[zz][y][x][3] = (unsigned char)(255.*alpha + .5);
            }
        }
    }
}