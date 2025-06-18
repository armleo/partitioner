#pragma once

// Simple 2D point class
class Point2D {
public:
    float x;
    float y;

    Point2D();
    Point2D(float x, float y);
};

// Bounding box class with lower-left (ll) and upper-right (ur) corners
class BoundingBox {
public:
    Point2D ll; // lower-left
    Point2D ur; // upper-right

    BoundingBox();
    BoundingBox(const Point2D& ll, const Point2D& ur);
    BoundingBox(float minX, float minY, float maxX, float maxY);
};