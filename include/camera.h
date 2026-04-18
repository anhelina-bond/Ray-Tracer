#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"
#include "ray.h"

class Camera {
public:
    // <position>, <gaze>, <up>
    point3 position;
    vec3 gaze;
    vec3 up;

    // <nearplane> left, right, bottom, top
    double left, right, bottom, top;

    // <neardistance>
    double near_distance;

    // <imageresolution> nx, ny
    int nx, ny;

    // Constructor to initialize all fields
    Camera(point3 pos, vec3 g, vec3 u, 
           double l, double r, double b, double t, 
           double dist, int res_x, int res_y) 
        : position(pos), gaze(g), up(u), 
          left(l), right(r), bottom(b), top(t), 
          near_distance(dist), nx(res_x), ny(res_y) 
    {
        computeCameraBasisVectors();
    }

    // Default constructor
    Camera() : left(0), right(0), bottom(0), top(0), near_distance(0), nx(0), ny(0) {}

    

    Ray get_ray(int i, int j) const;


private:
    vec3 u;
    vec3 v;
    vec3 w;

    void computeCameraBasisVectors();
};

#endif