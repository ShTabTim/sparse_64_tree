#ifndef VOGL_VEC3_HPP
#define VOGL_VEC3_HPP

#include <cstdint>
#include <cmath>

template <typename T>
class vec3 {
public:
    T x = 0, y = 0, z = 0;
    inline vec3() {}
    template <typename O> inline vec3(const vec3<O>& o) { x = static_cast<T>(o.x); y = static_cast<T>(o.y); z = static_cast<T>(o.z); }
    inline vec3(T x, T y, T z) { this->x = x; this->y = y; this->z = z; }
    template <typename O> inline vec3<T> operator=(const vec3<O>& o) { x = static_cast<T>(o.x); y = static_cast<T>(o.y); z = static_cast<T>(o.z); return *this; }

    vec3<T>& zero() { x = 0; y = 0; z = 0; return *this; }
    static vec3<T> Zero() { return vec3<T>().zero(); }
    inline T volume() const { return x*y*z; }
    ////logical
    template <typename O> inline uint32_t operator==(const vec3<O>& o) const { return x == o.x && y == o.y && z == o.z; }
    template <typename O> inline uint32_t operator!=(const vec3<O>& o) const { return x != o.x || y != o.y || z != o.z; }
    ////v-v
    inline vec3<T> operator-() const { return vec3<T>(-x, -y, -z); }
    inline vec3<T>& operator+=(const vec3<T>& o) { x+=o.x; y+=o.y; z+=o.z; return *this; }
    inline vec3<T>& operator-=(const vec3<T>& o) { x-=o.x; y-=o.y; z-=o.z; return *this; }
    template <typename O> inline vec3<T> operator+(const vec3<O>& o) const { return vec3<T>(*this)+=o; }
    template <typename O> inline vec3<T> operator-(const vec3<O>& o) const { return vec3<T>(*this)-=o; }
    ////v-s
    template <typename O> inline vec3<T>& operator*=(const O& o) { x*=o; y*=o; z*=o; return *this; }
    template <typename O> inline vec3<T>& operator/=(const O& o) { x/=o; y/=o; z/=o; return *this; }
};

template <typename V, typename O> inline vec3<V> operator*(const vec3<V>& v, const O& k) { return vec3<V>(v.x * k, v.y * k, v.z * k); }
template <typename V, typename O> inline vec3<V> operator*(const O& k, const vec3<V>& v) { return vec3<V>(k * v.x, k * v.y, k * v.z); }
template <typename V, typename O> inline vec3<V> operator/(const vec3<V>& v, const O& k) { return vec3<V>(v.x / k, v.y / k, v.z / k); }

template <typename V, typename W> inline vec3<V> operator*(const vec3<V>& v, const vec3<W>& w) { return vec3<V>(v.x * w.x, v.y * w.y, v.z * w.z); }
template <typename V, typename W> inline vec3<V> operator/(const vec3<V>& v, const vec3<W>& w) { return vec3<V>(v.x / w.x, v.y / w.y, v.z / w.z); }

template<typename O> inline O dot(const vec3<O>& a, const vec3<O>& b) {
    return
    a.x * b.x +
    a.y * b.y +
    a.z * b.z
    ;
}
template<typename O> inline O abs2(const vec3<O>& v) { return dot(v, v); }
template<typename O> inline O abs(const vec3<O>& v) { return sqrt(abs2(v)); }
template<typename O> inline O normalize(const vec3<O>& v) { return v/abs(v); }

template<typename O> inline vec3<O> cross(const vec3<O>& a, const vec3<O>& b) {
    return vec3<O>(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
            );
}

#endif//VOGL_VEC3_HPP