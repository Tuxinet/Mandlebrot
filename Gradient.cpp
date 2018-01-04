#include "Gradient.h"
#include <vector>
Gradient::GradientColor::GradientColor(int _r, int _g, int _b, int _a):r((float)_r / 255), g((float)_g / 255), b((float)_b / 255), a((float)_a / 255) {}
Gradient::GradientColor::GradientColor():r(), g(0), b(0), a(0) {}

const Gradient::GradientColor & Gradient::GradientColor::operator+=(const GradientColor &lhs){
    r += lhs.r;
    g += lhs.g;
    b += lhs.b;
    a += lhs.a;
    return *this;
}

const Gradient::GradientColor & Gradient::GradientColor::operator-=(const GradientColor &lhs){
    r -= lhs.r;
    g -= lhs.g;
    b -= lhs.b;
    a -= lhs.a;
    return *this;
}

const Gradient::GradientColor & Gradient::GradientColor::operator*=(const float &lhs){
    r *= lhs;
    g *= lhs;
    b *= lhs;
    a *= lhs;
    return *this;
}
const Gradient::GradientColor Gradient::GradientColor::operator+(const GradientColor& lhs) const{
    GradientColor res = *this;
    res += lhs;
    return res;
}

const Gradient::GradientColor Gradient::GradientColor::operator-(const GradientColor& lhs) const {
    GradientColor res = *this;
    res -= lhs;
    return res;
}

const Gradient::GradientColor Gradient::GradientColor::operator*(const float& lhs) const {
    GradientColor res = *this;
    res *= lhs;
    return res;
}