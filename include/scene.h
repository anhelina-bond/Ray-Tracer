#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include <memory>

#include "vec3.h"
#include "camera.h"
#include "light.h"
#include "material.h"
#include "mesh.h"
#include "texture.h"


class Scene {
public:
    // 1. Global Settings
    int max_ray_trace_depth;
    color background_color;

    // 2. Camera
    Camera cam;

    // 3. Lights
    color ambient_light;
    std::vector<std::shared_ptr<Light>> lights;

    // 4. Materials
    std::vector<Material> materials;

    // 5. Global Data (Referenced by Mesh indices)
    std::vector<vertex> vertex_data;
    std::vector<Texture> texture_data; // UV coordinates
    std::vector<vec3> normal_data;
    std::string texture_image_name; // From <textureimage>

    // 6. Objects
    std::vector<Mesh> meshes;

    unsigned char* texture_data_buffer = nullptr;
    int tex_width = 0;
    int tex_height = 0;

    Scene() : max_ray_trace_depth(0) {}

    // Helper functions for the parser (Optional but recommended)
    void add_material(const Material& m) { materials.push_back(m); }
    void add_vertex(const vertex& v) { vertex_data.push_back(v); }
    void add_normal(const vec3& v) { normal_data.push_back(v); }
    void add_texture(const Texture& v) { texture_data.push_back(v); }
    void add_mesh(const Mesh& m) { meshes.push_back(m); }
};

#endif