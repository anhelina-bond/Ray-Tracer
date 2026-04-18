#include "camera.h"

#include <iostream>

void Camera::computeCameraBasisVectors() {
    w = unit_vector(-gaze); 

    u = unit_vector(cross(up, w));

    v = cross(w, u);
}

Ray Camera::get_ray(int i, int j) const {
    point3 m = position + -w*near_distance;
    point3 q = m + left*u + top*v;

    double s_u = (right-left) * (i+0.5)/nx;
    double s_v = (top - bottom) * (j+0.5)/ny;

    point3 s = q + s_u*u - s_v*v;

    return Ray(position, unit_vector(s - position));
}