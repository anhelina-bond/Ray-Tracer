#ifndef MESH_H
#define MESH_H

#include <vector>
#include "vec3.h"
#include "ray.h"
#include "material.h"

constexpr double RAY_EPSILON      = 1e-3;  // Origin offset for shadow/reflection rays
constexpr double PARALLEL_EPSILON = 1e-8;  // Near-zero determinant threshold in Möller–Trumbore

class Scene;

struct hit_record {
    point3 p;
    vec3 normal;
    vec3 face_normal;
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

    bool hit(const Ray& r, double t_min, double t_max, hit_record& rec, const Scene& scene) const;
    bool shadow_hit(const Ray& r, double t_min, double t_max, const Scene& scene) const;
};

// Free functions used by trace() and the triangular light path
bool rayTriangleIntersect(const Ray& r, const vec3& v0, const vec3& v1,
                          const vec3& v2, double& t_out);
bool is_in_shadow(const Ray& shadow_ray, double t_max, const Scene& scene);

color trace(const Ray& r, const Scene& scene, int depth);

#endif