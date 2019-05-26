#include "streamlines.h"
#define _USE_MATH_DEFINES
#include <math.h>

#include "colors.h"
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <algorithm>    // std::shuffle
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock
#include <numeric>      // std::iota 

#include <windows.h>
#include "GL\include\GL.H"
#include "GL\include\glu.h"
#include "GL\include\glut.h"
#include "GL\include\glui.h"

streamlines::streamlines(int aNX, int aNY, int aNZ)
{
    NX = aNX;
    NY = aNY;
    NZ = aNZ;

    tNX = 64;
    tNY = 64;
    tNZ = 64;
}

void streamlines::get_vector_field(int fieldId, float x, float y, float z, float &vxp, float &vyp, float &vzp)
{
    if (fieldId == FIELD1)
    {
        streamlines::get_vector_field1(x, y, z, vxp, vyp, vzp);
    }
    else if (fieldId == FIELD2)
    {
        streamlines::get_vector_field2(x, y, z, vxp, vyp, vzp);
    }
    else if (fieldId == FIELD3)
    {
        streamlines::get_vector_field3(x, y, z, vxp, vyp, vzp);
    }
}

void streamlines::get_vector_field1(float x, float y, float z, float &vxp, float &vyp, float &vzp)
{
    vxp = -3 + 6.*x - 4.*x*(y + 1.) - 4.*z;
    vyp = 12.*x - 4.*x*x - 12.*z + 4.*z*z;
    vzp = 3. + 4.*x - 4.*x*(y + 1.) - 6.*z + 4.*(y + 1.)*z;
}

void streamlines::get_vector_field2(float x, float y, float z, float &vxp, float &vyp, float &vzp)
{
    vxp = sqrt(3.) * sin(z) + cos(y);
    vyp = sqrt(2.) * sin(x) + sqrt(3.) * cos(z);
    vzp = sin(y) + sqrt(2) * cos(x);
}

void streamlines::get_vector_field3(float x, float y, float z, float &vxp, float &vyp, float &vzp)
{
    vxp = -y;
    vyp = -z;
    vzp = x;
}

void streamlines::get_entropy_data(int fieldId)
{
    entropy_streamline_seed_candidate.clear();
    entropy_streamline_seed.clear();
    entropy_streamlines.clear();
    chosen_entropy_streamlines.clear();
    cluster_spatial.clear();
    torsion_streamlines.clear();

    srand(time(NULL));

    float x, y, z;
    float vx, vy, vz;

    grid_3d = new streamline_entropy**[tNX];
    for (int x = 0; x < tNX; ++x) {
        grid_3d[x] = new streamline_entropy*[tNY];
        for (int y = 0; y < tNY; ++y) {
            grid_3d[x][y] = new streamline_entropy[tNZ];
        }
    }

    int total_random_points = 100000;
    int total_points = total_random_points + (tNX * tNY * tNZ);

    std::vector<streamline_bin> list_of_bins;
    GenerateEntropyBins(list_of_bins, fieldId, total_random_points);
    GenerateGrid3D(list_of_bins, fieldId, total_points);

    //float minEntropy, maxEntropy;
    //GetEntropyMinMax(minEntropy, maxEntropy);
    //float maxMinusMin = maxEntropy - minEntropy;
    //float testSampleSize = maxMinusMin + minEntropy;

    // Now we need to create streamlines based on Entropy values, with high entropy having a greater chance of being picked
    for (int i = 0; i < tNX; i++)
    {
        for (int j = 0; j < tNY; j++)
        {
            for (int k = 0; k < tNZ; k++)
            {
                entropy_streamline_seed_candidate.push_back(grid_3d[i][j][k]);
            }
        }
    }

    //std::sort(entropy_streamline_seed_candidate.begin(), entropy_streamline_seed_candidate.end(), entropy_comparison);

    //for (int i = 0; i < 10000; i++)
    //{
    //    float norm_index = 0;
    //    for (int j = 0; j < 6; j++)
    //    {
    //        float random_val = (rand() / float(RAND_MAX));
    //        if (random_val > norm_index)
    //        {
    //            norm_index = random_val;
    //        }
    //    }
    //    entropy_streamline_seed.push_back(entropy_streamline_seed_candidate[int(norm_index * entropy_streamline_seed_candidate.size()) - 1]);
    //}

    // Pick Streamlines, giving higher entropy locations more probability
    std::vector<int> shuffled_streamlines;
    for (int i = 0; i < entropy_streamline_seed_candidate.size(); i++)
    {
        int entropy = int(entropy_streamline_seed_candidate[i].entropy + 0.5);
        shuffled_streamlines.insert(shuffled_streamlines.end(), entropy, i);
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(shuffled_streamlines.begin(), shuffled_streamlines.end(), std::default_random_engine(seed));

#if DEBUG
    int number_of_seeds_to_choose = 1000;
#else
    int number_of_seeds_to_choose = 1000;
#endif

    for (int i = 0; i < number_of_seeds_to_choose; i++)
    {
        entropy_streamline_seed.push_back(entropy_streamline_seed_candidate[shuffled_streamlines[i]]);
    }
}

void streamlines::GetEntropyStreamlines(int fieldId, int numericalIntegrationTypeId, int p, int q)
{
    for (int it = 0; it < streamlines::entropy_streamline_seed.size(); it++)
    {
        float x = entropy_streamline_seed[it].x;
        float y = entropy_streamline_seed[it].y;
        float z = entropy_streamline_seed[it].z;

        std::vector<streamline_coordinate> streamline;
        streamlines::get_streamline(x, y, z, fieldId, numericalIntegrationTypeId, streamline);

        streamline_line_data line_data;
        line_data.streamline = streamline;
        line_data.startPoint = streamline[0];
        line_data.midPoint = streamline[streamline.size() / 2];
        line_data.endPoint = streamline[streamline.size() - 1];

        streamline_coordinate clusMP;
        clusMP.x = (line_data.startPoint.x + line_data.midPoint.x + line_data.endPoint.x) / 3;
        clusMP.y = (line_data.startPoint.y + line_data.midPoint.y + line_data.endPoint.y) / 3;
        clusMP.z = (line_data.startPoint.z + line_data.midPoint.z + line_data.endPoint.z) / 3;
        line_data.clusteringMidPoint = clusMP;

        if (streamline.size() <= 2)
        {
            // Can't perform this algorithm on too small streamlines so skip
            continue;
        }

        // Calculate Linear Entropy
        float linearEntropy = 0;
        int linearEntropyCount = 0;
        float L_s = 0;
        for (int i = 0; i < streamline.size() - 1; i++)
        {
            float D_j = sqrt(pow(streamline[i].x - streamline[i + 1].x, 2) + pow(streamline[i].y - streamline[i + 1].y, 2) + pow(streamline[i].z - streamline[i + 1].z, 2));

            if (D_j == 0)
            {
                continue;
            }

            L_s += D_j;

            linearEntropyCount++;
        }
        for (int i = 0; i < streamline.size() - 1; i++)
        {
            float D_j = sqrt(pow(streamline[i].x - streamline[i + 1].x, 2) + pow(streamline[i].y - streamline[i + 1].y, 2) + pow(streamline[i].z - streamline[i + 1].z, 2));

            if (D_j == 0)
            {
                continue;
            }

            linearEntropy += (D_j/L_s) * log2(D_j/L_s);
        }
        linearEntropy = -(1 / log2(linearEntropyCount)) * linearEntropy;
        line_data.linearEntropy = linearEntropy;

        // Calculate Angular Entropy
        float angularEntropy = 0;
        int angularEntropyCount = 0;
        float L_a = 0;
        for (int i = 0; i < streamline.size() - 2; i++)
        {
            float vector1[3];
            float vector2[3];

            vector1[0] = streamline[i + 1].x - streamline[i].x;
            vector1[1] = streamline[i + 1].y - streamline[i].y;
            vector1[2] = streamline[i + 1].z - streamline[i].z;
            vector2[0] = streamline[i + 2].x - streamline[i + 1].x;
            vector2[1] = streamline[i + 2].y - streamline[i + 1].y;
            vector2[2] = streamline[i + 2].z - streamline[i + 1].z;

            float vector1_magnitude = sqrt(pow(vector1[0], 2) + pow(vector1[1], 2) + pow(vector1[2], 2));
            float vector2_magnitude = sqrt(pow(vector2[0], 2) + pow(vector2[1], 2) + pow(vector2[2], 2));

            if (vector1_magnitude == 0 || vector2_magnitude == 0)
            {
                continue;
            }

            float temp_val = (vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2]) / (vector1_magnitude * vector2_magnitude);

            if (temp_val > 1)
            {
                temp_val = 1;
            }

            float A_j = acos(temp_val);

            L_a += A_j;

            angularEntropyCount++;
        }

        for (int i = 0; i < streamline.size() - 2; i++)
        {
            float vector1[3];
            float vector2[3];

            vector1[0] = streamline[i + 1].x - streamline[i].x;
            vector1[1] = streamline[i + 1].y - streamline[i].y;
            vector1[2] = streamline[i + 1].z - streamline[i].z;
            vector2[0] = streamline[i + 2].x - streamline[i + 1].x;
            vector2[1] = streamline[i + 2].y - streamline[i + 1].y;
            vector2[2] = streamline[i + 2].z - streamline[i + 1].z;

            float vector1_magnitude = sqrt(pow(vector1[0], 2) + pow(vector1[1], 2) + pow(vector1[2], 2));
            float vector2_magnitude = sqrt(pow(vector2[0], 2) + pow(vector2[1], 2) + pow(vector2[2], 2));

            if (vector1_magnitude == 0 || vector2_magnitude == 0)
            {
                continue;
            }

            float temp_val = (vector1[0] * vector2[0] + vector1[1] * vector2[1] + vector1[2] * vector2[2]) / (vector1_magnitude * vector2_magnitude);

            if (temp_val > 1)
            {
                temp_val = 1;
            }

            float A_j = acos(temp_val);

            if (A_j == 0)
            {
                continue;
            }

            angularEntropy += (A_j / L_a) * log2(A_j / L_a);
        }
        angularEntropy = -(1 / log2(angularEntropyCount)) * angularEntropy;
        line_data.angularEntropy = angularEntropy;

        if (isnan(linearEntropy) || isnan(angularEntropy))
        {
            continue;
        }

        entropy_streamlines.push_back(line_data);
    }

    // Randomize Colors
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::vector<float> vSpatialColorValues;
    std::vector<float> vShapeColorValues;
    for (int it = 0; it < p; it++)
    {
        vSpatialColorValues.push_back(float(it) / (p - 1));
    }
    for (int it = 0; it < (p * q); it++)
    {
        vShapeColorValues.push_back(float(it) / ((p * q) - 1));
    }
    std::shuffle(vSpatialColorValues.begin(), vSpatialColorValues.end(), std::default_random_engine(seed));
    std::shuffle(vShapeColorValues.begin(), vShapeColorValues.end(), std::default_random_engine(seed));

    // Do K-means for start/mid/end
    int current_cluster_color = 0;
    streamlines::k_means_spatial(p, cluster_spatial);
    for (int it = 0; it < p; it++)
    {
        GetColorFromScheme(RAINBOW, 0, cluster_spatial[it].rgb, vSpatialColorValues[it]);

        streamlines::k_means_space(q, cluster_spatial[it].streamlines, cluster_spatial[it].cluster_shape);
        for (int j = 0; j < q; j++)
        {
            GetColorFromScheme(RAINBOW, 0, cluster_spatial[it].cluster_shape[j].rgb, vShapeColorValues[current_cluster_color]);

            if (cluster_spatial[it].cluster_shape[j].streamlines.size() > 0)
            {
                float current_min_distance = 0;
                int closest_to_cluster = 0;

                for (int i = 0; i < cluster_spatial[it].cluster_shape[j].streamlines.size(); i++)
                {
                    float distance = sqrt(pow(cluster_spatial[it].cluster_shape[j].streamlines[i].linearEntropy - cluster_spatial[it].cluster_shape[j].linear_entropy, 2) + pow(cluster_spatial[it].cluster_shape[j].streamlines[i].angularEntropy - cluster_spatial[it].cluster_shape[j].angular_entropy, 2));

                    if (i == 0)
                    {
                        current_min_distance = distance;
                        closest_to_cluster = i;
                    }
                    else
                    {
                        if (distance < current_min_distance)
                        {
                            current_min_distance = distance;
                            closest_to_cluster = i;
                        }
                    }
                }

                streamline_entropy_streamline entropy_streamline;
                entropy_streamline.streamline = cluster_spatial[it].cluster_shape[j].streamlines[closest_to_cluster];
                entropy_streamline.rgb[0] = cluster_spatial[it].cluster_shape[j].rgb[0];
                entropy_streamline.rgb[1] = cluster_spatial[it].cluster_shape[j].rgb[1];
                entropy_streamline.rgb[2] = cluster_spatial[it].cluster_shape[j].rgb[2];
                chosen_entropy_streamlines.push_back(entropy_streamline);
            }

            current_cluster_color++;
        }
    }
}

void streamlines::GetEntropyMinMax(float &min, float &max)
{
    min = grid_3d[0][0][0].entropy;
    max = grid_3d[0][0][0].entropy;

    for (int x = 0; x < tNX; ++x) {
        for (int y = 0; y < tNY; ++y) {
            for (int z = 0; z < tNZ; ++z) {
                if (grid_3d[x][y][z].entropy < min)
                {
                    min = grid_3d[x][y][z].entropy;
                }
                if (grid_3d[x][y][z].entropy > max)
                {
                    max = grid_3d[x][y][z].entropy;
                }
            }
        }
    }
}

void streamlines::k_means_spatial(int p, std::vector<streamline_cluster_spatial> &cluster_spatial)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::vector<int> vZeroToSize(entropy_streamlines.size());
    std::iota(vZeroToSize.begin(), vZeroToSize.end(), 0);
    std::shuffle(vZeroToSize.begin(), vZeroToSize.end(), std::default_random_engine(seed));

    for (int i = 0; i < p; i++)
    {
        if (i > vZeroToSize.size() - 1)
        {
            break;
        }
        else
        {
            streamline_cluster_spatial clus;
            clus.x = entropy_streamlines[vZeroToSize[i]].clusteringMidPoint.x;
            clus.y = entropy_streamlines[vZeroToSize[i]].clusteringMidPoint.y;
            clus.z = entropy_streamlines[vZeroToSize[i]].clusteringMidPoint.z;
            cluster_spatial.push_back(clus);
        }
    }

    bool anyChanges;

    do
    {
        for (int i = 0; i < p; i++)
        {
            cluster_spatial[i].streamlines.clear();
        }

        for (int i = 0; i < entropy_streamlines.size(); i++)
        {
            float current_min_distance = 0;
            int current_cluster = 0;

            for (int j = 0; j < p; j++)
            {
                float distance = sqrt(pow(entropy_streamlines[i].clusteringMidPoint.x - cluster_spatial[j].x, 2) + pow(entropy_streamlines[i].clusteringMidPoint.y - cluster_spatial[j].y, 2) + pow(entropy_streamlines[i].clusteringMidPoint.z - cluster_spatial[j].z, 2));

                if (j == 0)
                {
                    current_min_distance = distance;
                    current_cluster = j;
                }
                else
                {
                    if (distance < current_min_distance)
                    {
                        current_min_distance = distance;
                        current_cluster = j;
                    }
                }
            }

            cluster_spatial[current_cluster].streamlines.push_back(entropy_streamlines[i]);
        }

        anyChanges = false;

        // Recompute Centroid
        for (int i = 0; i < p; i++)
        {
            float x = 0;
            float y = 0;
            float z = 0;
            for (int j = 0; j < cluster_spatial[i].streamlines.size(); j++)
            {
                x += cluster_spatial[i].streamlines[j].clusteringMidPoint.x;
                y += cluster_spatial[i].streamlines[j].clusteringMidPoint.y;
                z += cluster_spatial[i].streamlines[j].clusteringMidPoint.z;
            }

            x = x / cluster_spatial[i].streamlines.size();
            y = y / cluster_spatial[i].streamlines.size();
            z = z / cluster_spatial[i].streamlines.size();

            if (cluster_spatial[i].streamlines.size() == 0)
            {
                continue;
            }

            if (abs(x - cluster_spatial[i].x) < 0.01 && abs(y - cluster_spatial[i].y) < 0.01 && abs(z - cluster_spatial[i].z) < 0.01)
            {
                cluster_spatial[i].x = x;
                cluster_spatial[i].y = y;
                cluster_spatial[i].z = z;
            }
            else
            {
                anyChanges = true;
                cluster_spatial[i].x = x;
                cluster_spatial[i].y = y;
                cluster_spatial[i].z = z;
            }
        }
    } while (anyChanges);
}

void streamlines::k_means_space(int q, std::vector<streamline_line_data> &streamlines, std::vector<streamline_cluster_shape> &cluster_shape)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::vector<int> vZeroToSize(streamlines.size());
    std::iota(vZeroToSize.begin(), vZeroToSize.end(), 0);
    std::shuffle(vZeroToSize.begin(), vZeroToSize.end(), std::default_random_engine(seed));

    for (int i = 0; i < q; i++)
    {
        if (i > vZeroToSize.size() - 1)
        {
            break;
        }
        else
        {
            streamline_cluster_shape clus;
            clus.linear_entropy = streamlines[vZeroToSize[i]].linearEntropy;
            clus.angular_entropy = streamlines[vZeroToSize[i]].angularEntropy;
            cluster_shape.push_back(clus);
        }
    }

    bool anyChanges;

    do
    {
        for (int i = 0; i < q; i++)
        {
            cluster_shape[i].streamlines.clear();
        }

        for (int i = 0; i < streamlines.size(); i++)
        {
            float current_min_distance = 0;
            int current_cluster = 0;

            for (int j = 0; j < q; j++)
            {
                float distance = sqrt(pow(streamlines[i].linearEntropy - cluster_shape[j].linear_entropy, 2) + pow(streamlines[i].angularEntropy - cluster_shape[j].angular_entropy, 2));

                if (j == 0)
                {
                    current_min_distance = distance;
                    current_cluster = j;
                }
                else
                {
                    if (distance < current_min_distance)
                    {
                        current_min_distance = distance;
                        current_cluster = j;
                    }
                }
            }

            cluster_shape[current_cluster].streamlines.push_back(streamlines[i]);
        }

        anyChanges = false;

        // Recompute Centroid
        for (int i = 0; i < q; i++)
        {
            float x = 0;
            float y = 0;
            for (int j = 0; j < cluster_shape[i].streamlines.size(); j++)
            {
                x += cluster_shape[i].streamlines[j].linearEntropy;
                y += cluster_shape[i].streamlines[j].angularEntropy;
            }

            x = x / cluster_shape[i].streamlines.size();
            y = y / cluster_shape[i].streamlines.size();

            if (cluster_shape[i].streamlines.size() == 0)
            {
                continue;
            }

            if (abs(x - cluster_shape[i].linear_entropy) < 0.01 && abs(y - cluster_shape[i].angular_entropy) < 0.01)
            {
                cluster_shape[i].linear_entropy = x;
                cluster_shape[i].angular_entropy = y;
            }
            else
            {
                anyChanges = true;
                cluster_shape[i].linear_entropy = x;
                cluster_shape[i].angular_entropy = y;
            }
        }
    } while (anyChanges);
}

void streamlines::CalculatePhiTheta(int fieldId, float x, float y, float z, float &phi_degree, float &theta_degree)
{
    float vx, vy, vz;

    streamlines::get_vector_field(fieldId, x, y, z, vx, vy, vz);

    float r = sqrt((vx * vx) + (vy * vy) + (vz * vz));
    float phi = atan2(vy, vx);
    float theta = acos(vz / r);

    phi_degree = streamlines::ConvertToPositiveDegrees(phi);
    theta_degree = streamlines::ConvertToPositiveDegrees(theta);
}

float streamlines::ConvertToPositiveDegrees(float x)
{
    float radians = 0;
    if (x > 0) 
    { 
        radians = x; 
    }
    else 
    {
        radians = 2 * M_PI + x; 
    }

    return radians * (180 / M_PI);
}

void streamlines::GenerateEntropyBins(std::vector<streamline_bin> &list_of_bins, int fieldId, int total_random_points)
{
    float x, y, z;
    float vx, vy, vz;

    float bins_x_y = 6;
    int number_of_bins = bins_x_y * bins_x_y;

    // Split Phi by 360
    float phi_delta = 360 / (bins_x_y * 2);
    // Split Theta by 180
    float theta_delta = 180 / (bins_x_y * 2);

    // Generate Bins
    for (int i = 0; i < number_of_bins; i++)
    {
        streamline_bin s_bin;
        s_bin.hits = 0;
        s_bin.center_phi = phi_delta + (phi_delta * 2) * (i % int(bins_x_y));
        s_bin.center_theta = theta_delta + (theta_delta * 2) * (int(i / bins_x_y) % int(bins_x_y));
        list_of_bins.push_back(s_bin);
    }

    // Pick Random Locations and Bin
    for (int i = 0; i < total_random_points; i++)
    {
        x = ((rand() % 2000) / 1000.) - 1;
        y = ((rand() % 2000) / 1000.) - 1;
        z = ((rand() % 2000) / 1000.) - 1;

        float phi_degree = 0;
        float theta_degree = 0;

        streamlines::CalculatePhiTheta(fieldId, x, y, z, phi_degree, theta_degree);
        streamlines::AddEntropyToBin(list_of_bins, phi_degree, theta_degree);
    }
    // Uniformly Sample and Bin
    for (int i = 0; i < tNX; i++)
    {
        for (int j = 0; j < tNY; j++)
        {
            for (int k = 0; k < tNZ; k++)
            {
                x = -1. + ((i * 2.) / tNX);
                y = -1. + ((j * 2.) / tNY);
                z = -1. + ((k * 2.) / tNZ);

                streamlines::get_vector_field(fieldId, x, y, z, vx, vy, vz);

                float phi_degree = 0;
                float theta_degree = 0;

                streamlines::CalculatePhiTheta(fieldId, x, y, z, phi_degree, theta_degree);
                streamlines::AddEntropyToBin(list_of_bins, phi_degree, theta_degree);
            }
        }
    }
}

void streamlines::GenerateGrid3D(std::vector<streamline_bin> &list_of_bins, int fieldId, int total_points)
{
    float x, y, z;
    float vx, vy, vz;

    // Uniformly Sample and Calculate Probability
    for (int i = 0; i < tNX; i++)
    {
        for (int j = 0; j < tNY; j++)
        {
            for (int k = 0; k < tNZ; k++)
            {
                x = -1. + ((i * 2.) / tNX);
                y = -1. + ((j * 2.) / tNY);
                z = -1. + ((k * 2.) / tNZ);

                streamlines::get_vector_field(fieldId, x, y, z, vx, vy, vz);

                float phi_degree = 0;
                float theta_degree = 0;

                streamlines::CalculatePhiTheta(fieldId, x, y, z, phi_degree, theta_degree);

                grid_3d[i][j][k].i = i;
                grid_3d[i][j][k].j = j;
                grid_3d[i][j][k].k = k;
                grid_3d[i][j][k].x = x;
                grid_3d[i][j][k].y = y;
                grid_3d[i][j][k].z = z;
                grid_3d[i][j][k].vx = vx;
                grid_3d[i][j][k].vy = vy;
                grid_3d[i][j][k].vz = vz;

                int bin_number = streamlines::FindEntropyBin(list_of_bins, phi_degree, theta_degree);

                float probability = list_of_bins[bin_number].hits / float(total_points);

                grid_3d[i][j][k].probability = probability;
            }
        }
    }

    // Local Neighborhood Entropy Calculations
    int sample_i, sample_j, sample_k;
    for (int i = 0; i < tNX; i++)
    {
        for (int j = 0; j < tNY; j++)
        {
            for (int k = 0; k < tNZ; k++)
            {
                // Sample a Cube Around i, j, k

                float entropy_sum = 0;

                int delta_i = -1;
                int delta_j = -1;
                int delta_k = -1;

                for (int it = 0; it < 27; it++)
                {
                    //std::cout << delta_i << ", " << delta_j << ", " << delta_k << "\n";

                    sample_i = i + delta_i;
                    sample_j = j + delta_j;
                    sample_k = k + delta_k;

                    if (sample_i < 0 || sample_i >= tNX || sample_j < 0 || sample_j >= tNY || sample_k < 0 || sample_k >= tNZ)
                    {
                        // Calculate outer points

                        x = -1. + ((sample_i * 2.) / tNX);
                        y = -1. + ((sample_j * 2.) / tNY);
                        z = -1. + ((sample_k * 2.) / tNZ);

                        streamlines::get_vector_field(fieldId, x, y, z, vx, vy, vz);

                        float phi_degree = 0;
                        float theta_degree = 0;

                        streamlines::CalculatePhiTheta(fieldId, x, y, z, phi_degree, theta_degree);

                        int bin_number = streamlines::FindEntropyBin(list_of_bins, phi_degree, theta_degree);

                        float probability = list_of_bins[bin_number].hits / float(total_points);

                        entropy_sum += probability * log2(probability);
                    }
                    else
                    {
                        float probability = grid_3d[sample_i][sample_j][sample_k].probability;

                        entropy_sum += probability * log2(probability);
                    }

                    // After Main Stuff Is Completed
                    delta_i++;
                    if (delta_i > 1)
                    {
                        delta_i = -1;
                        delta_j++;
                    }
                    if (delta_j > 1)
                    {
                        delta_j = -1;
                        delta_k++;
                    }
                }

                grid_3d[i][j][k].entropy = -entropy_sum;
            }
        }
    }
}

void streamlines::AddEntropyToBin(std::vector<streamline_bin> &bins, float phi, float theta)
{
    int bin_number = streamlines::FindEntropyBin(bins, phi, theta);
    bins[bin_number].hits++;
}

int streamlines::FindEntropyBin(std::vector<streamline_bin> &bins, float phi, float theta)
{
    float current_min_distance = 0;
    int current_bin = 0;
    for (int i = 0; i < bins.size(); i++)
    {
        float distance = sqrt(pow(phi - bins[i].center_phi, 2) + pow(theta - bins[i].center_theta, 2));

        if (i == 0)
        {
            current_min_distance = distance;
            current_bin = i;
        }
        else
        {
            if (distance < current_min_distance)
            {
                current_min_distance = distance;
                current_bin = i;
            }
        }
    }

    return current_bin;
}

void streamlines::Calculate_Colors(int ColorSchemeId, float ColorSchemeMidPoint)
{
    float Smin = grid_3d[0][0][0].entropy;
    float Smax = Smin;

    for (int i = 0; i < tNX; i++)
    {
        for (int j = 0; j < tNY; j++)
        {
            for (int k = 0; k < tNZ; k++)
            {
                float S = grid_3d[i][j][k].entropy;

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

    for (int x = 0; x < tNX; x++)
    {
        for (int y = 0; y < tNY; y++)
        {
            for (int z = 0; z < tNZ; z++)
            {
                float S = grid_3d[x][y][z].entropy;
                float normalized_value = (S - Smin) / (Smax - Smin);

                GetColorFromScheme(ColorSchemeId, ColorSchemeMidPoint, rgb, normalized_value);

                grid_3d[x][y][z].r = rgb[0];
                grid_3d[x][y][z].g = rgb[1];
                grid_3d[x][y][z].b = rgb[2];
            }
        }
    }
}

void streamlines::CompositeXY(float MaxAlpha, float minT, float maxT)
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

                float T = grid_3d[x][y][z].entropy;
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

void streamlines::CompositeXZ(float MaxAlpha, float minT, float maxT)
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

                float T = grid_3d[x][z][y].entropy;
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

void streamlines::CompositeYZ(float MaxAlpha, float minT, float maxT)
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

                float T = grid_3d[z][x][y].entropy;
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

void streamlines::get_streamline(float aX, float aY, float aZ, int FieldId, int NumericalIntegrationTypeId, std::vector<streamline_coordinate> &streamline)
{
    float stepSize = 0.01;
    int maximumSteps = 1000;
    float exit_threshold = 0.;
    float x, y, z;
    float vx, vy, vz;

    // Backwards
    streamline_coordinate first_coordinate;
    first_coordinate.x = aX;
    first_coordinate.y = aY;
    first_coordinate.z = aZ;
    streamline.push_back(first_coordinate);

    x = aX;
    y = aY;
    z = aZ;

    for (int i = 0; i < maximumSteps; i++)
    {
        if (NumericalIntegrationTypeId == EULER)
        {
            streamlines::get_vector_field(FieldId, x, y, z, vx, vy, vz);
            x = x - vx * stepSize;
            y = y - vy * stepSize;
            z = z - vz * stepSize;
        }
        else if (NumericalIntegrationTypeId == RK2)
        {
            streamlines::get_vector_field(FieldId, x, y, z, vx, vy, vz);
            float k1_vx = stepSize * vx;
            float k1_vy = stepSize * vy;
            float k1_vz = stepSize * vz;

            streamlines::get_vector_field(FieldId, x - k1_vx, y - k1_vy, z - k1_vz, vx, vy, vz);
            float k2_vx = stepSize * vx;
            float k2_vy = stepSize * vy;
            float k2_vz = stepSize * vz;

            x = x - 0.5 * (k1_vx + k2_vx);
            y = y - 0.5 * (k1_vy + k2_vy);
            z = z - 0.5 * (k1_vz + k2_vz);
        }
        else if (NumericalIntegrationTypeId == RK4)
        {
            streamlines::get_vector_field(FieldId, x, y, z, vx, vy, vz);
            float k1_vx = stepSize * vx;
            float k1_vy = stepSize * vy;
            float k1_vz = stepSize * vz;

            streamlines::get_vector_field(FieldId, x - (k1_vx / 2), y - (k1_vy / 2), z - (k1_vz / 2), vx, vy, vz);
            float k2_vx = stepSize * vx;
            float k2_vy = stepSize * vy;
            float k2_vz = stepSize * vz;

            streamlines::get_vector_field(FieldId, x - (k2_vx / 2), y - (k2_vy / 2), z - (k2_vz / 2), vx, vy, vz);
            float k3_vx = stepSize * vx;
            float k3_vy = stepSize * vy;
            float k3_vz = stepSize * vz;

            streamlines::get_vector_field(FieldId, x - k3_vx, y - k3_vy, z - k3_vz, vx, vy, vz);
            float k4_vx = stepSize * vx;
            float k4_vy = stepSize * vy;
            float k4_vz = stepSize * vz;

            x = x - (1 / 6.) * (k1_vx + 2 * k2_vx + 2 * k3_vx + k4_vx);
            y = y - (1 / 6.) * (k1_vy + 2 * k2_vy + 2 * k3_vy + k4_vy);
            z = z - (1 / 6.) * (k1_vz + 2 * k2_vz + 2 * k3_vz + k4_vz);
        }

        if (x < -1.25 || x > 1.25)
        {
            break;
        }
        if (y < -1.25 || y > 1.25)
        {
            break;
        }
        if (z < -1.25 || z > 1.25)
        {
            break;
        }
        if (abs(vx) <= exit_threshold && abs(vy) <= exit_threshold && abs(vz) <= exit_threshold)
        {
            break;
        }

        streamline_coordinate coordinate;
        coordinate.x = x;
        coordinate.y = y;
        coordinate.z = z;
        streamline.push_back(coordinate);
    }

    std::reverse(streamline.begin(), streamline.end());

    // Forwards
    x = aX;
    y = aY;
    z = aZ;

    for (int i = 0; i < maximumSteps; i++)
    {
        if (NumericalIntegrationTypeId == EULER)
        {
            streamlines::get_vector_field(FieldId, x, y, z, vx, vy, vz);
            x = x + vx * stepSize;
            y = y + vy * stepSize;
            z = z + vz * stepSize;
        }
        else if (NumericalIntegrationTypeId == RK2)
        {
            streamlines::get_vector_field(FieldId, x, y, z, vx, vy, vz);
            float k1_vx = stepSize * vx;
            float k1_vy = stepSize * vy;
            float k1_vz = stepSize * vz;

            streamlines::get_vector_field(FieldId, x + k1_vx, y + k1_vy, z + k1_vz, vx, vy, vz);

            float k2_vx = stepSize * vx;
            float k2_vy = stepSize * vy;
            float k2_vz = stepSize * vz;

            x = x + 0.5 * (k1_vx + k2_vx);
            y = y + 0.5 * (k1_vy + k2_vy);
            z = z + 0.5 * (k1_vz + k2_vz);
        }
        else if (NumericalIntegrationTypeId == RK4)
        {
            streamlines::get_vector_field(FieldId, x, y, z, vx, vy, vz);
            float k1_vx = stepSize * vx;
            float k1_vy = stepSize * vy;
            float k1_vz = stepSize * vz;

            streamlines::get_vector_field(FieldId, x + (k1_vx / 2), y + (k1_vy / 2), z + (k1_vz / 2), vx, vy, vz);
            float k2_vx = stepSize * vx;
            float k2_vy = stepSize * vy;
            float k2_vz = stepSize * vz;

            streamlines::get_vector_field(FieldId, x + (k2_vx / 2), y + (k2_vy / 2), z + (k2_vz / 2), vx, vy, vz);
            float k3_vx = stepSize * vx;
            float k3_vy = stepSize * vy;
            float k3_vz = stepSize * vz;

            streamlines::get_vector_field(FieldId, x + k3_vx, y + k3_vy, z + k3_vz, vx, vy, vz);
            float k4_vx = stepSize * vx;
            float k4_vy = stepSize * vy;
            float k4_vz = stepSize * vz;

            x = x + (1 / 6.) * (k1_vx + 2 * k2_vx + 2 * k3_vx + k4_vx);
            y = y + (1 / 6.) * (k1_vy + 2 * k2_vy + 2 * k3_vy + k4_vy);
            z = z + (1 / 6.) * (k1_vz + 2 * k2_vz + 2 * k3_vz + k4_vz);
        }

        if (x < -1.25 || x > 1.25)
        {
            break;
        }
        if (y < -1.25 || y > 1.25)
        {
            break;
        }
        if (z < -1.25 || z > 1.25)
        {
            break;
        }
        if (abs(vx) <= exit_threshold && abs(vy) <= exit_threshold && abs(vz) <= exit_threshold)
        {
        break;
        }

        streamline_coordinate coordinate;
        coordinate.x = x;
        coordinate.y = y;
        coordinate.z = z;
        streamline.push_back(coordinate);
    }
}

void streamlines::get_streamribbon(int FieldId, int NumericalIntegrationTypeId,
    std::vector<streamline_coordinate> &streamline, std::vector<streamline_coordinate> &streamribbon)
{
    float x, y, z;

    float d = 0.1;
    float vx, vy, vz;

    float stepSize = 0.01;
    float exit_threshold = 0.;

    x = streamline[0].x + 0.05;
    y = streamline[0].y + 0.05;
    z = streamline[0].z + 0.05;

    streamline_coordinate first_coordinate;
    first_coordinate.x = streamline[0].x;
    first_coordinate.y = streamline[0].y;
    first_coordinate.z = streamline[0].z;
    streamribbon.push_back(first_coordinate);

    for (int i = 1; i < streamline.size(); i++)
    {
        if (NumericalIntegrationTypeId == EULER)
        {
            streamlines::get_vector_field(FieldId, x, y, z, vx, vy, vz);
            x = x + vx * stepSize;
            y = y + vy * stepSize;
            z = z + vz * stepSize;
        }
        else if (NumericalIntegrationTypeId == RK2)
        {
            streamlines::get_vector_field(FieldId, x, y, z, vx, vy, vz);
            float k1_vx = stepSize * vx;
            float k1_vy = stepSize * vy;
            float k1_vz = stepSize * vz;

            streamlines::get_vector_field(FieldId, x + k1_vx, y + k1_vy, z + k1_vz, vx, vy, vz);

            float k2_vx = stepSize * vx;
            float k2_vy = stepSize * vy;
            float k2_vz = stepSize * vz;

            x = x + 0.5 * (k1_vx + k2_vx);
            y = y + 0.5 * (k1_vy + k2_vy);
            z = z + 0.5 * (k1_vz + k2_vz);
        }

        float temp_x = x - streamline[i].x;
        float temp_y = y - streamline[i].y;
        float temp_z = z - streamline[i].z;
        float d_temp = sqrt(pow(temp_x, 2) + pow(temp_y, 2) + pow(temp_z, 2));
        if (d_temp == 0)
        {
            int gdf = 4;
        }
        float factor = d / d_temp;

        x = streamline[i].x + factor * temp_x;
        y = streamline[i].y + factor * temp_y;
        z = streamline[i].z + factor * temp_z;

        if (x < -1 || x > 1)
        {
            break;
        }
        if (y < -1 || y > 1)
        {
            break;
        }
        if (z < -1 || z > 1)
        {
            break;
        }
        if (abs(vx) <= exit_threshold && abs(vy) <= exit_threshold && abs(vz) <= exit_threshold)
        {
            break;
        }

        streamline_coordinate coordinate;
        coordinate.x = x;
        coordinate.y = y;
        coordinate.z = z;
        streamribbon.push_back(coordinate);
    }
}

void streamlines::get_streamtapes()
{
    float normalized_value;
    for (int i = 0; i < chosen_entropy_streamlines.size(); i++)
    {
        streamline_torsion tor_streamline;

        streamlines::get_streamtape(chosen_entropy_streamlines[i].streamline.streamline, tor_streamline.tor);

        tor_streamline.rgb[0] = chosen_entropy_streamlines[i].rgb[0];
        tor_streamline.rgb[1] = chosen_entropy_streamlines[i].rgb[1];
        tor_streamline.rgb[2] = chosen_entropy_streamlines[i].rgb[2];

        torsion_streamlines.push_back(tor_streamline);
    }
}

void streamlines::get_streamtape(std::vector<streamline_coordinate> &streamline, std::vector<streamline_torsion_part> &tor)
{
    std::vector<streamline_derivative> firstDerivative;
    std::vector<streamline_derivative> secondDerivative;
    std::vector<streamline_derivative> thirdDerivative;

    std::vector<streamline_coordinate> split_streamline;

    float split_size = 0.0;
    float current_split_size = 0;

    for (int i = 0; i < streamline.size() - 1; i++)
    {
        float dx = streamline[i].x - streamline[i + 1].x;
        float dy = streamline[i].y - streamline[i + 1].y;
        float dz = streamline[i].z - streamline[i + 1].z;

        float magnitude = sqrt(dx * dx + dy * dy + dz * dz);

        current_split_size += magnitude;

        if (current_split_size > split_size)
        {
            split_streamline.push_back(streamline[i]);

            current_split_size = 0;
        }
    }

    for (int i = 0; i < split_streamline.size() - 1; i++)
    {
        streamline_derivative derivative;
        derivative.x = split_streamline[i].x - split_streamline[i + 1].x;
        derivative.y = split_streamline[i].y - split_streamline[i + 1].y;
        derivative.z = split_streamline[i].z - split_streamline[i + 1].z;
        firstDerivative.push_back(derivative);
    }

    for (int i = 0; i < firstDerivative.size() - 1; i++)
    {
        streamline_derivative derivative;
        derivative.x = firstDerivative[i].x - firstDerivative[i + 1].x;
        derivative.y = firstDerivative[i].y - firstDerivative[i + 1].y;
        derivative.z = firstDerivative[i].z - firstDerivative[i + 1].z;
        secondDerivative.push_back(derivative);
    }

    for (int i = 0; i < secondDerivative.size() - 1; i++)
    {
        streamline_derivative derivative;
        derivative.x = secondDerivative[i].x - secondDerivative[i + 1].x;
        derivative.y = secondDerivative[i].y - secondDerivative[i + 1].y;
        derivative.z = secondDerivative[i].z - secondDerivative[i + 1].z;
        thirdDerivative.push_back(derivative);
    }

    for (int i = 0; i < thirdDerivative.size(); i++)
    {
        streamline_torsion_part s_torsion;

        s_torsion.x = split_streamline[i + 1].x;
        s_torsion.y = split_streamline[i + 1].y;
        s_torsion.z = split_streamline[i + 1].z;

        float part1 = firstDerivative[i].x * (secondDerivative[i].y * thirdDerivative[i].z - secondDerivative[i].z * thirdDerivative[i].y);
        float part2 = firstDerivative[i].y * (secondDerivative[i].x * thirdDerivative[i].z - secondDerivative[i].z * thirdDerivative[i].x);
        float part3 = firstDerivative[i].z * (secondDerivative[i].x * thirdDerivative[i].y - secondDerivative[i].y * thirdDerivative[i].x);

        float determinant = part1 - part2 + part3;

        s_torsion.determinant = determinant;

        float binormal_x = firstDerivative[i].y * secondDerivative[i].z - firstDerivative[i].z * secondDerivative[i].y;
        float binormal_y = -(firstDerivative[i].x * secondDerivative[i].z - firstDerivative[i].z * secondDerivative[i].x);
        float binormal_z = firstDerivative[i].x * secondDerivative[i].y - firstDerivative[i].y * secondDerivative[i].x;

        s_torsion.binormal_x = binormal_x;
        s_torsion.binormal_y = binormal_y;
        s_torsion.binormal_z = binormal_z;

        float binormal_magnitude = sqrt(binormal_x * binormal_x + binormal_y * binormal_y + binormal_z * binormal_z);
        s_torsion.normalized_binormal_x = binormal_x / binormal_magnitude;
        s_torsion.normalized_binormal_y = binormal_y / binormal_magnitude;
        s_torsion.normalized_binormal_z = binormal_z / binormal_magnitude;

        float torsion = determinant / (binormal_magnitude * binormal_magnitude);
        s_torsion.torsion = torsion;

        tor.push_back(s_torsion);
    }

    // Get Min/Max
    float min = tor[0].torsion;
    float max = min;
    for (int i = 0; i < tor.size(); i++) {
        if (tor[i].torsion < min)
        {
            min = tor[i].torsion;
        }
        if (tor[i].torsion > max)
        {
            max = tor[i].torsion;
        }
    }

    // Get Normalized Values
    for (int i = 0; i < tor.size(); i++)
    {
        float normalized_value = (tor[i].torsion - min) / (max - min);

        tor[i].normalized_torsion = normalized_value;
        tor[i].normalized_width = 1 - normalized_value;
    }
}
