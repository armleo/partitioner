#include "geom.hpp"

// Point2D implementation
Point2D::Point2D() : x(0), y(0) {}

Point2D::Point2D(float x, float y) : x(x), y(y) {}

// BoundingBox implementation
BoundingBox::BoundingBox() : ll(), ur() {}

BoundingBox::BoundingBox(const Point2D& ll, const Point2D& ur) : ll(ll), ur(ur) {}

BoundingBox::BoundingBox(float minX, float minY, float maxX, float maxY)
    : ll(minX, minY), ur(maxX, maxY) {}