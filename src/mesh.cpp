#include "mesh.h"
#include "scene.h"
#include <limits>

void Mesh::add_face(int v0, int v1, int v2, int t0, int t1, int t2, int n0, int n1, int n2) {
    Face f;
    f.v_ids[0] = v0; f.v_ids[1] = v1; f.v_ids[2] = v2;
    f.t_ids[0] = t0; f.t_ids[1] = t1; f.t_ids[2] = t2;
    f.n_ids[0] = n0; f.n_ids[1] = n1; f.n_ids[2] = n2;
    faces.push_back(f);
}

// Möller–Trumbore helper 
static bool moller_trumbore(const Ray& r,
                             const vec3& v0, const vec3& v1, const vec3& v2,
                             double t_min, double t_max,
                             double& t_out, double& u_out, double& v_out)
{
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    vec3 h = cross(r.direction(), edge2);
    double a = dot(edge1, h);
    if (a > -1e-8 && a < 1e-8) return false;

    double f = 1.0 / a;
    vec3 s = r.origin() - v0;
    double u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return false;

    vec3 q = cross(s, edge1);
    double v = f * dot(r.direction(), q);
    if (v < 0.0 || u + v > 1.0) return false;

    double t = f * dot(edge2, q);
    if (t <= t_min || t >= t_max) return false;

    t_out = t;
    u_out = u;
    v_out = v;
    return true;
}

bool Mesh::hit(const Ray& r, double t_min, double t_max, hit_record& rec, const Scene& scene) const {
    bool hit_anything = false;
    double closest_so_far = t_max;

    for (const auto& face : faces) {
        const vec3& v0 = scene.vertex_data[face.v_ids[0] - 1];
        const vec3& v1 = scene.vertex_data[face.v_ids[1] - 1];
        const vec3& v2 = scene.vertex_data[face.v_ids[2] - 1];

        double t, u, v;
        if (!moller_trumbore(r, v0, v1, v2, t_min, closest_so_far, t, u, v))
            continue;

        //  Commit the hit 
        hit_anything = true;
        closest_so_far = t;
        rec.t = t;
        rec.p = r.at(t);

        vec3 edge1 = v1 - v0;
        vec3 edge2 = v2 - v0;

        // Geometric (face) normal — used only for shadow ray offset
        vec3 geom_normal = unit_vector(cross(edge1, edge2));

        // orient geometric normal toward the incoming ray
        if (dot(geom_normal, r.direction()) > 0.0)
            geom_normal = -geom_normal;
        rec.face_normal = geom_normal;

        //  Smooth normal interpolation 
        if (!scene.normal_data.empty()) {
            const vec3& n0v = scene.normal_data[face.n_ids[0] - 1];
            const vec3& n1v = scene.normal_data[face.n_ids[1] - 1];
            const vec3& n2v = scene.normal_data[face.n_ids[2] - 1];

            // Barycentric blend:  w0=(1-u-v), w1=u, w2=v
            vec3 smooth_n = (1.0 - u - v) * n0v + u * n1v + v * n2v;

            double len = smooth_n.length();
            if (len < 1e-6) {
                rec.normal = geom_normal;
            } else {
                smooth_n = smooth_n / len; // normalize

                if (dot(smooth_n, geom_normal) < 0.0)
                    smooth_n = -smooth_n;

                // orient toward the viewer
                if (dot(smooth_n, r.direction()) > 0.0)
                    smooth_n = -smooth_n;

                rec.normal = smooth_n;
            }
        } else {
            rec.normal = geom_normal;
        }

        //  Texture coordinate interpolation 
        if (!scene.texture_data.empty()) {
            const Texture& tc0 = scene.texture_data[face.t_ids[0] - 1];
            const Texture& tc1 = scene.texture_data[face.t_ids[1] - 1];
            const Texture& tc2 = scene.texture_data[face.t_ids[2] - 1];
            rec.u_tex = (1.0 - u - v) * tc0.u + u * tc1.u + v * tc2.u;
            rec.v_tex = (1.0 - u - v) * tc0.v + u * tc1.v + v * tc2.v;
        }

        //  Material 
        if (!scene.materials.empty() &&
            (size_t)(this->material_id - 1) < scene.materials.size()) {
            rec.material = &scene.materials[this->material_id - 1];
        } else {
            static Material default_mat(color(1,1,1), color(1,1,1),
                                        color(0,0,0), 1, color(0,0,0), 0);
            rec.material = &default_mat;
        }
    }

    return hit_anything;
}

bool Mesh::shadow_hit(const Ray& r, double t_min, double t_max, const Scene& scene) const {
    for (const auto& face : faces) {
        const vec3& v0 = scene.vertex_data[face.v_ids[0] - 1];
        const vec3& v1 = scene.vertex_data[face.v_ids[1] - 1];
        const vec3& v2 = scene.vertex_data[face.v_ids[2] - 1];

        double t, u, v;
        if (moller_trumbore(r, v0, v1, v2, t_min, t_max, t, u, v))
            return true;
    }
    return false;
}

bool rayTriangleIntersect(const Ray& r, const vec3& v0, const vec3& v1,
                          const vec3& v2, double& t_out) {
    double u, v;
    return moller_trumbore(r, v0, v1, v2, 0.001, std::numeric_limits<double>::max(), t_out, u, v);
}

bool is_in_shadow(const Ray& shadow_ray, double t_max, const Scene& scene) {
    for (const auto& mesh : scene.meshes)
        if (mesh.shadow_hit(shadow_ray, 0.001, t_max, scene))
            return true;
    return false;
}

color trace(const Ray& r, const Scene& scene, int depth) {
    if (depth > scene.max_ray_trace_depth) return color(0, 0, 0);

    hit_record rec;
    double closest_so_far = std::numeric_limits<double>::max();
    bool hit = false;
    for (const auto& mesh : scene.meshes) {
        if (mesh.hit(r, 0.001, closest_so_far, rec, scene)) {
            hit = true;
            closest_so_far = rec.t;
        }
    }

    if (!hit) return scene.background_color;

    const Material* mat = rec.material;
    vec3 V = unit_vector(-r.direction());
    vec3 N = unit_vector(rec.normal); 

    // Texture Sampling
    color texture_color(1, 1, 1);
    if (scene.texture_data_buffer != nullptr) {
        double u_c = std::max(0.0, std::min(1.0, rec.u_tex));
        double v_c = std::max(0.0, std::min(1.0, rec.v_tex));
        int x = static_cast<int>(u_c * (scene.tex_width - 1));
        int y = static_cast<int>((1.0 - v_c) * (scene.tex_height - 1));
        int idx = (y * scene.tex_width + x) * 3;
        if (idx >= 0 && idx < (scene.tex_width * scene.tex_height * 3 - 2)) {
            texture_color = color(scene.texture_data_buffer[idx]/255.0, 
                                  scene.texture_data_buffer[idx+1]/255.0, 
                                  scene.texture_data_buffer[idx+2]/255.0);
        }
    }

    // Base Surface Color (Texture + Material Diffuse blend)
    color modulator = (1.0 - mat->textureFactor) * color(1,1,1) + mat->textureFactor * texture_color;
    color surface_color = mat->diffuse * modulator;


    // Ambient Shading (Ambient light * Surface color)
    color final_color = scene.ambient_light * surface_color;

    for (const auto& light_ptr : scene.lights) {
        vec3 L;
        double t_light_dist;

        if (auto pLight = std::dynamic_pointer_cast<pointLight>(light_ptr)) {
            vec3 to_l = pLight->position - rec.p;
            t_light_dist = to_l.length();
            L = unit_vector(to_l);
        } else if (auto tLight = std::dynamic_pointer_cast<triangularLight>(light_ptr)) {
            L = unit_vector(-tLight->light_dir);
            Ray shadow_r(rec.p, L);
            if (!rayTriangleIntersect(shadow_r, tLight->v1, tLight->v2, tLight->v3, t_light_dist)) continue;
        } else continue;

        // Shadow acne check
        if (dot(rec.face_normal, L) <= 0) continue;

        Ray shadow_ray(rec.p + rec.face_normal * 0.001, L);
        if (!is_in_shadow(shadow_ray, t_light_dist - 0.001, scene)) {
            
            color intensity = light_ptr->intensity;

            // Diffuse
            double cos_theta = std::max(0.0, dot(N, L));
            final_color += (surface_color * intensity) * cos_theta;

            // Specular
            vec3 H = unit_vector(L + V);
            double cos_alpha = std::max(0.0, dot(N, H));
            final_color += (mat->specular * intensity) * std::pow(cos_alpha, mat->phongExponent);
        }
    }

    // 4. Reflections (Recursive)
    if (mat->mirrorReflectance.e[0] > 0 || mat->mirrorReflectance.e[1] > 0 || mat->mirrorReflectance.e[2] > 0) {
        vec3 reflect_dir = r.direction() - 2.0 * dot(r.direction(), N) * N;
        Ray reflect_ray(rec.p + rec.face_normal * 0.001, unit_vector(reflect_dir));
        final_color += mat->mirrorReflectance * trace(reflect_ray, scene, depth + 1);
    }

    return final_color;
}