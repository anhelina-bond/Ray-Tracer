#ifndef MESH_H
#define MESH_H

#include <vector>
#include "vec3.h"
#include "ray.h"
#include "material.h"

class Scene; 

struct hit_record {
    point3 p;
    vec3 normal;      // Smooth/Interpolated normal (for shading)
    vec3 face_normal; // Geometric/Flat normal (for shadow offset)
    double t;
    double u_tex, v_tex;
    const Material* material;
};

class Mesh {
public:
    struct Face {
        int v_ids[3], t_ids[3], n_ids[3];
    };

    int material_id;
    std::vector<Face> faces;

    Mesh(int mat_id) : material_id(mat_id) {}

    void add_face(int v0, int v1, int v2, int t0, int t1, int t2, int n0, int n1, int n2);

    // 2. These functions use "const Scene&", which works with a forward declaration
    bool hit(const Ray& r, double t_min, double t_max, hit_record& rec, const Scene& scene) const;
    bool shadow_hit(const Ray& r, double t_min, double t_max, const Scene& scene) const;
};

// 3. Forward declare trace so main.cpp knows about it
color trace(const Ray& r, const Scene& scene, int depth);

#endif