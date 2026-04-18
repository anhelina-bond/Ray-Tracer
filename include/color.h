#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include <iostream>


inline double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

void write_color(std::ostream& out, const color& pixel_color) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Translate [0,1] to [0,255]
    int rbyte = int(256 * clamp(r, 0.0, 0.999));
    int gbyte = int(256 * clamp(g, 0.0, 0.999));
    int bbyte = int(256 * clamp(b, 0.0, 0.999));

    // Write out the pixel color components
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif

// ----------------- usage ---------------------------
// for (int i = 0; i < image_width; i++) {
//     auto pixel_color = color(double(i)/(image_width-1), double(j)/(image_height-1), 0);
//     write_color(std::cout, pixel_color);
// }