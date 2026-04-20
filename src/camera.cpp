#include "camera.h"

#include <iostream>

void Camera::computeCameraBasisVectors() {
    w = unit_vector(-gaze); 

    u = unit_vector(cross(up, w));

    v = cross(w, u);
}

Ray Camera::get_ray(double x, double y) const {
    point3 m = position + -w * near_distance;
    point3 q = m + left * u + top * v;

    // Use x and y directly instead of adding 0.5 inside
    double s_u = (right - left) * x / nx;
    double s_v = (top - bottom) * y / ny;

    point3 s = q + s_u * u - s_v * v;

    return Ray(position, unit_vector(s - position));
}