#ifndef LIGHT_H
#define LIGHT_H

#include <vector>
#include <memory> // Required for smart pointers
#include "vec3.h"

// Base Class
class Light {
public:
    color intensity;

    Light(double c0, double c1, double c2) : intensity(c0, c1, c2) {}
    
    virtual ~Light() = default; 

    // Returns the direction FROM the hit point TO the light source
    // and calculates the distance to the light (for attenuation).
    virtual vec3 get_direction_and_dist(const point3& hit_p, double& out_dist) const = 0;
};

class ambientLight : public Light {
public:
    using Light::Light; // Inherit the constructor
};

class pointLight : public Light {
public:
    point3 position;

    pointLight(double c0, double c1, double c2, double x, double y, double z) 
        : Light(c0, c1, c2), position(x, y, z) {}

    vec3 get_direction_and_dist(const point3& hit_p, double& out_dist) const override {
        vec3 dir = position - hit_p;
        out_dist = dir.length(); // Distance used for 1/r^2 attenuation
        return unit_vector(dir);
    }
};

class triangularLight : public Light {
public: 
    vertex v1, v2, v3; // Assuming vertex is a point3
    vec3 light_dir;

    triangularLight(double c0, double c1, double c2, point3 _v1, point3 _v2, point3 _v3) 
        : Light(c0, c1, c2), v1(_v1), v2(_v2), v3(_v3) {
            vec3 normal = cross(v1-v2 , v1-v3);
            light_dir = unit_vector(normal);
        }

    vec3 get_direction_and_dist(const point3& hit_p, double& out_dist) const override {
        // For a planar/directional light, distance is usually treated as infinity 
        // or a constant since rays are parallel.
        out_dist = 1e30; 
        return -light_dir; // Vector toward the light
    }
};

#endif