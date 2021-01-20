#ifndef __HW1__PARSER__
#define __HW1__PARSER__

#include <string>
#include <vector>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>

namespace parser
{
    //Notice that all the structures are as simple as possible
    //so that you are not enforced to adopt any style or design.
    struct Vec3f
    {
        float x, y, z;
    };

    struct Vec3i
    {
        int x, y, z;
    };

    struct Vec4f
    {
        float x, y, z, w;
    };

    struct Camera
    {
        Vec3f position;
        Vec3f gaze;
        Vec3f up;
        Vec4f near_plane;
        float near_distance;
        float far_distance;
        int image_width, image_height;
    };

    struct PointLight
    {
        Vec3f position;
        Vec3f intensity;
        bool status;
    };

    struct Material
    {
        Vec3f ambient;
        Vec3f diffuse;
        Vec3f specular;
        float phong_exponent;
    };

    struct Transformation
    {
        std::string transformation_type;
        int id;
    };

    struct Face
    {
        int v0_id;
        int v1_id;
        int v2_id;
    };

    struct Mesh
    {
        int material_id;
        std::vector<unsigned int> indices;
        std::vector<Face> faces;
        std::vector<Transformation> transformations;
        std::string mesh_type;

        GLfloat *normals;

        GLuint vertexAttribBuffer, indexBuffer;
    };

    struct Scene
    {
        //Data
        Vec3i background_color;//done
        int culling_enabled;//done
        int culling_face;//done
        Camera camera;//done
        Vec3f ambient_light; // done
        std::vector<PointLight> point_lights; //done
        std::vector<Material> materials; //done
        std::vector<Vec3f> vertex_data;//done
        std::vector<Vec3f> translations;//done
        std::vector<Vec3f> scalings;//done
        std::vector<Vec4f> rotations;//done
        std::vector<Mesh> meshes;//done

        //Functions
        void loadFromXml(const std::string& filepath);
    };
}

#endif