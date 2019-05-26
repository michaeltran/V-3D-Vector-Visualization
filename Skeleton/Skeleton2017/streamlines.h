#include <vector>

#pragma once

enum FIELD_TYPE
{
    FIELD1,
    FIELD2,
    FIELD3,
};

enum NUMERICAL_INTEGRATION_TECHNIQUES
{
    EULER,
    RK2,
    RK4,
};

typedef struct streamline_coordinate
{
    float x, y, z;
};

typedef struct streamline_entropy
{
    int i, j, k;
    float r, g, b;
    float x, y, z;
    float vx, vy, vz;
    float probability;
    float entropy;
};

typedef struct streamline_bin
{
    int hits;

    // phi, theta
    float center_phi;
    float center_theta;
};

typedef struct streamline_line_data
{
    std::vector<streamline_coordinate> streamline;
    streamline_coordinate startPoint;
    streamline_coordinate midPoint;
    streamline_coordinate endPoint;

    streamline_coordinate clusteringMidPoint;

    //// 0 = Linear Entropy
    //// 1 = Angular Entropy
    //float entropy[2];
    float linearEntropy;
    float angularEntropy;
};

typedef struct streamline_cluster_shape
{
    float linear_entropy, angular_entropy;
    float rgb[3];
    std::vector<streamline_line_data> streamlines;
};

typedef struct streamline_cluster_spatial
{
    float x, y, z;
    float rgb[3];
    std::vector<streamline_line_data> streamlines;
    std::vector<streamline_cluster_shape> cluster_shape;
};

typedef struct streamline_entropy_streamline
{
    float rgb[3];
    streamline_line_data streamline;
};

typedef struct streamline_derivative
{
    float x, y, z;
};

typedef struct streamline_torsion_part
{
    float x, y, z;

    float binormal_x;
    float binormal_y;
    float binormal_z;

    float normalized_binormal_x;
    float normalized_binormal_y;
    float normalized_binormal_z;

    float determinant;
    float torsion;

    float normalized_torsion;
    float normalized_width;
};

typedef struct streamline_torsion
{
    float rgb[3];
    std::vector<streamline_torsion_part> tor;
};

class streamlines {
public:
    int NX, NY, NZ;
    int tNX, tNY, tNZ;

    streamline_entropy ***grid_3d;

    std::vector<streamline_entropy> entropy_streamline_seed_candidate;
    std::vector<streamline_entropy> entropy_streamline_seed;

    std::vector<streamline_cluster_spatial> cluster_spatial;

    std::vector<streamline_line_data> entropy_streamlines;
    std::vector<streamline_entropy_streamline> chosen_entropy_streamlines;

    std::vector<streamline_torsion> torsion_streamlines;

    unsigned char TextureXY[64][64][64][4];
    unsigned char TextureXZ[64][64][64][4];
    unsigned char TextureYZ[64][64][64][4];

    streamlines::streamlines(int aNX, int aNY, int aNZ);

    void streamlines::get_vector_field(int fieldId, float x, float y, float z, float &vxp, float &vyp, float &vzp);
    void streamlines::get_vector_field1(float x, float y, float z, float &vxp, float &vyp, float &vzp);
    void streamlines::get_vector_field3(float x, float y, float z, float &vxp, float &vyp, float &vzp);
    void streamlines::get_vector_field2(float x, float y, float z, float &vxp, float &vyp, float &vzp);

    void streamlines::get_entropy_data(int fieldId);

    float streamlines::ConvertToPositiveDegrees(float x);

    void streamlines::GenerateEntropyBins(std::vector<streamline_bin> &list_of_bins, int fieldId, int total_random_points);
    void streamlines::GenerateGrid3D(std::vector<streamline_bin> &list_of_bins, int fieldId, int total_points);
    void streamlines::AddEntropyToBin(std::vector<streamline_bin> &bins, float phi, float theta);
    int streamlines::FindEntropyBin(std::vector<streamline_bin> &bins, float phi, float theta);

    void streamlines::Calculate_Colors(int ColorSchemeId, float ColorSchemeMidPoint);
    void streamlines::CompositeXY(float MaxAlpha, float minT, float maxT);
    void streamlines::CompositeXZ(float MaxAlpha, float minT, float maxT);
    void streamlines::CompositeYZ(float MaxAlpha, float minT, float maxT);

    void streamlines::CalculatePhiTheta(int fieldId, float x, float y, float z, float &phi_degree, float &theta_degree);

    void streamlines::GetEntropyMinMax(float &min, float &max);

    void streamlines::GetEntropyStreamlines(int fieldId, int numericalIntegrationTypeId, int p, int q);

    void streamlines::k_means_space(int q, std::vector<streamline_line_data> &streamlines, std::vector<streamline_cluster_shape> &cluster_shape);
    void streamlines::k_means_spatial(int p, std::vector<streamline_cluster_spatial> &cluster_spatial);

    void streamlines::get_streamline(float aX, float aY, float aZ, int FieldId, int NumericalIntegrationTypeId, std::vector<streamline_coordinate> &streamline);
    void streamlines::get_streamribbon(int FieldId, int NumericalIntegrationTypeId, std::vector<streamline_coordinate> &streamline, std::vector<streamline_coordinate> &streamribbon);
    void streamlines::get_streamtapes();
    void streamlines::get_streamtape(std::vector<streamline_coordinate> &streamline, std::vector<streamline_torsion_part> &tor);
};