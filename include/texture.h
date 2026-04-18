#ifndef TEXTURE_H
#define TEXTURE_H

class Texture {
public:
    double u, v;

    Texture() : u(0), v(0) {}
    Texture(double _u, double _v) : u(_u), v(_v) {}
};

#endif