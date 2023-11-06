#ifndef INCLUDE_AKFSFSIM_UTILS_H
#define INCLUDE_AKFSFSIM_UTILS_H

#include <vector>

struct Vector2 {
    double x, y;
    Vector2():x(0.0),y(0.0){};
    Vector2(double _x, double _y):x(_x),y(_y){};
};

std::vector<Vector2> offsetPoints(const std::vector<Vector2>& points, const Vector2& offset);
std::vector<std::vector<Vector2>> offsetPoints(const std::vector<std::vector<Vector2>>& dataset, const Vector2& offset);

double wrapAngle(double angle);
double calculateMean(const std::vector<double>& dataset);
double calculateRMSE(const std::vector<double>& dataset);

std::vector<Vector2> generateEllipse(double x, double y, double sigma_xx, double sigma_yy, double sigma_xy, int num_points = 50);
std::vector<Vector2> generateCircle(double x, double y, double radius, int num_points = 50);

#endif  // INCLUDE_AKFSFSIM_UTILS_H