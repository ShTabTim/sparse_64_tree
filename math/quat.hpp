#ifndef VOGL_QUAT_HPP
#define VOGL_QUAT_HPP

#include "vec3.hpp"
#include "mat3.hpp"

template<typename T>
class quat {
public:
    vec3<T> v = {0, 0, 0};
    T r = 0;

    inline quat() = default;
    template<typename O> explicit quat(const vec3<O>& o) { r = 0; v = o; }
    template<typename O> explicit quat(const quat<O>& o) { r = o.r; v = o.v; }
    template<typename O> explicit quat(const O& o) { r = o; v = {0, 0, 0}; }
    template<typename O> explicit quat(const vec3<O>& oo, const O& o) { v = oo; r = o; }
    template<typename O> explicit quat(const O& xo, const O& yo, const O& zo, const O& ro) { r = ro; v = {xo, yo, zo}; }


    template <typename O> inline uint32_t operator==(const vec3<O>& o) const { return r == o.r && v == o.v; }
    template <typename O> inline uint32_t operator!=(const vec3<O>& o) const { return r != o.r || v != o.v; }

    inline quat<T> operator-() const { return quat<T>(-v, -r); }
    inline quat<T>& operator+=(const quat<T>& o) { r+=o.r; v+=o.v; return *this; }
    inline quat<T>& operator-=(const quat<T>& o) { r-=o.r; v-=o.v; return *this; }
    template <typename O> inline quat<T> operator+(const quat<O>& o) const { return quat<T>(*this)+=o; }
    template <typename O> inline quat<T> operator-(const quat<O>& o) const { return quat<T>(*this)-=o; }
    template <typename O> inline quat<T> operator*(const O& o) { return quat<T>(v * o, r * o); }
    template <typename O> inline quat<T> operator*(const quat<O>& o) { return quat<T>(r * o.v + o.r * v + cross(v, o.v), r*o.r - dot(v, o.v)); }
    template <typename O> inline quat<T>& operator/=(const O& o) { r/=o; v/=o; return *this; }
    template <typename O> inline quat<T> operator/(const O& o) { return quat<T>(*this)/=o; }
    template <typename O> inline quat<T> operator/(const quat<O>& o) { return conj(*this)/abs2(*this); }
};

template<typename T> quat<T> conj(quat<T> q) { return quat<T>(-q.v, q.r); }
template<typename T> T abs2(quat<T> q) { return q.r*q.r + abs2(q.v); }
template<typename T> float abs(quat<T> q) { return sqrtf(abs2(q)); }

template<typename T> quat<T> rot(vec3<T> normal) { T s = abs(normal)/2; return {cos(s), normal*(2*sin(s)/s)}; }
template<typename T> quat<float> rot(vec3<T> normal, float alpha) { alpha *= 0.5f; return quat<float>(normal*sinf(alpha), cosf(alpha)); }

template<typename T> vec3<T> transform(const vec3<T>& a, const quat<T>& q) { return a*(q.r*q.r - abs2(q.v)) + (2*q.r)*cross(q.v, a) + (2*dot(q.v, a))*a; }//q*a*conj(q)

template<typename T> mat3<T> rot(const quat<T>& q) {
    return mat3<T>(
            q.r*q.r+q.v.x*q.v.x-q.v.y*q.v.y-q.v.z*q.v.z, 2*(q.v.x*q.v.y-q.v.z*q.r), 2*(q.v.x*q.v.z+q.v.y*q.r),
            2*(q.v.x*q.v.y+q.v.z*q.r), q.r*q.r-q.v.x*q.v.x+q.v.y*q.v.y-q.v.z*q.v.z, 2*(q.v.y*q.v.z-q.v.x*q.r),
            2*(q.v.x*q.v.z-q.v.y*q.r), 2*(q.v.y*q.v.z+q.v.x*q.r), q.r*q.r-q.v.x*q.v.x-q.v.y*q.v.y+q.v.z*q.v.z
    );
}

template<typename T> quat<T> inv(const quat<T>& q) { return conj(q)/abs2(q); }
template<typename T> quat<float> normalize(const quat<T>& q) { return conj(q)/abs(q); }

#endif//VOGL_QUAT_HPP