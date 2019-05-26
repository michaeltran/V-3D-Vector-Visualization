#include "dat.h"
#include <iostream>
#include <fstream>
#include <vector>

#include <string>

dat::dat(std::string fileName)
{
    int i;
    FILE *fp = fopen(fileName.c_str(), "r");
    if (fp == NULL) return;
    fscanf(fp, "%d %d\n", &NX, &NY);
    vertexList.clear();
    for (i = 0; i < NX*NY; i++)
    {
        node tmp;
        tmp.nx = 0;
        tmp.ny = 0;
        tmp.nz = 1;
        fscanf(fp, "%f, %f, %f, %f \n", &tmp.x, &tmp.y, &tmp.z, &tmp.s);
        vertexList.push_back(tmp);
    }
    fclose(fp);
}

void dat::Build_Edge_List()
{
    int i, j;
    int cur = 0;
    edgeList.clear();
    for (j = 0; j < NY - 1; j++)
    {
        cur = j * NX;
        for (i = 0; i < NX - 1; i++)
        {
            lineseg horizontal_seg;
            horizontal_seg.n1 = cur;
            horizontal_seg.n2 = cur + 1;
            edgeList.push_back(horizontal_seg);

            lineseg vertical_seg;
            vertical_seg.n1 = cur;
            vertical_seg.n2 = cur + NX;
            edgeList.push_back(vertical_seg);

            cur++;
        }

        lineseg rightmost_seg;
        rightmost_seg.n1 = cur;
        rightmost_seg.n2 = cur + NX;
        edgeList.push_back(rightmost_seg);
    }
    // Build the edges on the top boundary
    cur = (NY - 1) * NX;
    for (i = 0; i < NX - 1; i++)
    {
        lineseg topmost_seg;
        topmost_seg.n1 = cur;
        topmost_seg.n2 = cur + 1;
        edgeList.push_back(topmost_seg);

        cur++;
    }
}

void dat::Build_Face_List()
{
    int i, j;
    int cur = 0;
    quadList.clear();
    for (j = 0; j < NY - 1; j++)
    {
        cur = j * NX;
        for (i = 0; i < NX - 1; i++)
        {
            quad q;
            q.verts[0] = cur;
            q.verts[1] = cur + 1;
            q.verts[2] = cur + NX + 1;
            q.verts[3] = cur + NX;

            int found = 0;
            for (int z = 0; z < edgeList.size(); z++)
            {
                if (edgeList[z].n1 == cur && edgeList[z].n2 == cur + 1)
                {
                    q.edges[0] = z;
                    found++;
                }
                else if (edgeList[z].n1 == cur + 1 && edgeList[z].n2 == cur + NX + 1)
                {
                    q.edges[1] = z;
                    found++;
                }
                else if (edgeList[z].n1 == cur + NX && edgeList[z].n2 == cur + NX + 1)
                {
                    q.edges[2] = z;
                    found++;
                }
                else if (edgeList[z].n1 == cur && edgeList[z].n2 == cur + NX)
                {
                    q.edges[3] = z;
                    found++;
                }

                if (found == 4)
                {
                    break;
                }
            }

            quadList.push_back(q);
            cur++;
        }
    }
}

void dat::Build_Lines(float aScalarValue, int aNumberOfContours)
{
    float Smin, Smax;

    for (int i = 0; i < vertexList.size(); i++)
    {
        float S = vertexList[i].s;

        if (i == 0)
        {
            Smin = S;
            Smax = S;
        }
        else
        {
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

    for (int i = 0; i < quadList.size(); i++)
    {
        quadList[i].lines.clear();
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

        for (int jj = 0; jj < edgeList.size(); jj++)
        {
            edgeList[jj].foundIntersection = false;
        }

        for (int i = 0; i < quadList.size(); i++)
        {
            //MARCHING SQUARES ALGORITHM
            node vert0 = vertexList[quadList[i].verts[0]];
            node vert1 = vertexList[quadList[i].verts[1]];
            node vert2 = vertexList[quadList[i].verts[2]];
            node vert3 = vertexList[quadList[i].verts[3]];

            lineseg &edge0 = edgeList[quadList[i].edges[0]];
            lineseg &edge1 = edgeList[quadList[i].edges[1]];
            lineseg &edge2 = edgeList[quadList[i].edges[2]];
            lineseg &edge3 = edgeList[quadList[i].edges[3]];

            std::vector<node> intersection_locations;

            MarchingSquares_FindIntersection(s_star, edge0, vert0, vert1, intersection_locations);
            MarchingSquares_FindIntersection(s_star, edge1, vert1, vert2, intersection_locations);
            MarchingSquares_FindIntersection(s_star, edge2, vert2, vert3, intersection_locations);
            MarchingSquares_FindIntersection(s_star, edge3, vert3, vert0, intersection_locations);

            int intersections = intersection_locations.size();
            if (intersections == 2)
            {
                // Connect Edges
                //MarchingSquares_DrawIntersection(intersection_locations[0], intersection_locations[1]);

                quadList[i].lines.push_back(intersection_locations);
            }
            else if (intersections == 0)
            {
                // Skip
            }
            else if (intersections == 1)
            {
                std::cout << "IMPOSSIBLE 1: " << aNumberOfContours << "\n";
            }
            else if (intersections == 3)
            {
                std::cout << "IMPOSSIBLE 3: " << aNumberOfContours << "\n";
            }
            else if (intersections == 4)
            {
                std::cout << "4: " << aNumberOfContours << "\n";

                float M = (vert0.s + vert1.s + vert2.s + vert3.s) / 4;

                if (vert0.s <= M)
                {
                    // Connect Edge 1 and 4
                    //MarchingSquares_DrawIntersection(intersection_locations[0], intersection_locations[3]);
                    // Connect Edge 2 and 3
                    //MarchingSquares_DrawIntersection(intersection_locations[1], intersection_locations[2]);

                    std::vector<node> newLine1;
                    newLine1.push_back(intersection_locations[0]);
                    newLine1.push_back(intersection_locations[3]);
                    quadList[i].lines.push_back(newLine1);

                    std::vector<node> newLine2;
                    newLine2.push_back(intersection_locations[1]);
                    newLine2.push_back(intersection_locations[2]);
                    quadList[i].lines.push_back(newLine2);
                }
                else
                {
                    // Connect Edge 1 and 2
                    //MarchingSquares_DrawIntersection(intersection_locations[0], intersection_locations[1]);
                    // Connect Edge 3 and 4
                    //MarchingSquares_DrawIntersection(intersection_locations[2], intersection_locations[3]);

                    std::vector<node> newLine1;
                    newLine1.push_back(intersection_locations[0]);
                    newLine1.push_back(intersection_locations[1]);
                    quadList[i].lines.push_back(newLine1);

                    std::vector<node> newLine2;
                    newLine2.push_back(intersection_locations[2]);
                    newLine2.push_back(intersection_locations[3]);
                    quadList[i].lines.push_back(newLine2);
                }
            }
            else
            {
                std::cout << "Something bad has happened\n";
            }
        }
    }
}

void dat::MarchingSquares_FindIntersection(float s_star, lineseg &edge, node &vert0, node &vert1, std::vector<node> &intersection_locations)
{
    if (edge.foundIntersection)
    {
        intersection_locations.push_back(edge.intersection);
    }
    else 
    {
        float t_star = (s_star - vert0.s) / (vert1.s - vert0.s);

        if (t_star >= 0 && t_star <= 1)
        {
            float x_star = (1 - t_star) * vert0.x + t_star * vert1.x;
            float y_star = (1 - t_star) * vert0.y + t_star * vert1.y;

            node intersection_location;
            intersection_location.x = x_star;
            intersection_location.y = y_star;
            intersection_location.z = 0;

            intersection_locations.push_back(intersection_location);

            edge.intersection = intersection_location;
            edge.foundIntersection = true;
        }
    }
}