#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec3.h"
#include <iostream>

class Material {
public:
    color ambient, diffuse, specular, mirrorReflectance;
    double phongExponent, textureFactor;

    Material(color _amb, color _diff, color _spec, double _phong, color _mirr, double _tex) 
        : ambient(_amb), 
          diffuse(_diff), 
          specular(_spec), 
          mirrorReflectance(_mirr), // mirrorReflectance comes before phongExponent
          phongExponent(_phong),    // phongExponent comes after
          textureFactor(_tex)
    {}
};


#endif