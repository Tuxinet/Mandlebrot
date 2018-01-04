
#ifndef __GRADIENT_H__
#define __GRADIENT_H__
#include <vector>
#include <math.h>
namespace Gradient {
    template<class T>
    class GradientStop {
        float t;
        T value;
        template <class Ta>
        friend class Gradient;
    public:
        GradientStop(float _t, T _val):t(_t), value(_val){
        }

    };

    template<class T>
    T lerp(T s, T e, float t){
        return s + (e - s) * t;
    }
    template<class T>
    T cerp(T a,T b,float x){
        float ft = x * M_PI;
        float f = (1.0-cos(ft)) / 2.0;
        return a * (1.0-f) + b*f;
    }

    class GradientColor {
    public:
        float r,g,b,a;
        GradientColor(int, int, int, int);
        GradientColor();
        const GradientColor & operator+=(const GradientColor &);
        const GradientColor & operator-=(const GradientColor &);
        const GradientColor & operator*=(const float &);
        const GradientColor operator+(const GradientColor&) const;
        const GradientColor operator-(const GradientColor&) const;
        const GradientColor operator*(const float&) const;
    };
    template<class T>
    class Gradient {
        std::vector<GradientStop<T>*> stops;
    public:
        void addColorStop(float t, T val){
            typename std::vector<GradientStop<T>*>::iterator it;
            for(it = stops.begin(); it != stops.end(); it++){
                if((*it)->t > t)break;
            }

            stops.insert(it, new GradientStop<T>(t, val));
        }
        T getColorAt(float t){
            typename std::vector<GradientStop<T>*>::iterator it;
            GradientStop<T> *start, *stop;
            for(it = stops.begin(); it != stops.end(); it++){
                stop = *it;
                if(stop->t > t)
                    break;
            }
            if(it == stops.begin() || it == stops.end()) return stop->value;
            start = *(--it);
            float frac = (t - start->t) / (stop->t - start->t);
            return lerp(start->value, stop->value, frac);
        }
        ~Gradient(){
            for(typename std::vector<GradientStop<T>*>::iterator it = stops.begin(); it != stops.end(); it++){
                delete *it;
            }
        }
    };
}
#endif//__GRADIENT_H__