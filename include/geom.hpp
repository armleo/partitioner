#pragma once

// Simple 2D point class
class Point2D {
public:
    float x;
    float y;

    Point2D() : x(0), y(0) {}
    Point2D(float x, float y) : x(x), y(y) {}
};

// Bounding box class with lower-left (ll) and upper-right (ur) corners
class BoundingBox {
public:
    Point2D ll; // lower-left
    Point2D ur; // upper-right

    BoundingBox() : ll(), ur() {}
    BoundingBox(const Point2D& ll, const Point2D& ur) : ll(ll), ur(ur) {}
    BoundingBox(float minX, float minY, float maxX, float maxY)
        : ll(minX, minY), ur(maxX, maxY) {}
};