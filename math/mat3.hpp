#ifndef VOGL_MAT3_HPP
#define VOGL_MAT3_HPP

#include <cassert>
#include <cinttypes>
#include <iostream>
#include "vec3.hpp"

template <typename T>
class mat3 {
    T A[9];
public:
    mat3(T aii = 0) {
        A[0] = aii; A[1] =   0; A[2] =   0;
        A[3] =   0; A[4] = aii; A[5] =   0;
        A[6] =   0; A[7] =   0; A[8] = aii;
    }

    mat3(T a00, T a11, T a22) {
        A[0] = a00; A[1] =   0; A[2] =   0;
        A[3] =   0; A[4] = a11; A[5] =   0;
        A[6] =   0; A[7] =   0; A[8] = a22;
    }

    mat3(T a00, T a01, T a02, T a10, T a11, T a12, T a20, T a21, T a22) {
        A[0] = a00; A[1] = a01; A[2] = a02;
        A[3] = a10; A[4] = a11; A[5] = a12;
        A[6] = a20; A[7] = a21; A[8] = a22;
    }

    T& operator() (uint32_t i, uint32_t j) { return A[i * 3 + j]; }
    T& operator[] (uint32_t i) { return A[i]; }

    template<typename O>
    mat3<T> operator=(const mat3<O>& o) {
        A[0] = static_cast<O>(o[0]);
        A[1] = static_cast<O>(o[1]);
        A[2] = static_cast<O>(o[2]);
        A[3] = static_cast<O>(o[3]);
        A[4] = static_cast<O>(o[4]);
        A[5] = static_cast<O>(o[5]);
        A[6] = static_cast<O>(o[6]);
        A[7] = static_cast<O>(o[7]);
        A[8] = static_cast<O>(o[8]);
        return *this;
    }

    template<typename O>
    mat3<T> operator+=(const mat3<O>& o) {
        A[0] += static_cast<O>(o[0]);
        A[1] += static_cast<O>(o[1]);
        A[2] += static_cast<O>(o[2]);
        A[3] += static_cast<O>(o[3]);
        A[4] += static_cast<O>(o[4]);
        A[5] += static_cast<O>(o[5]);
        A[6] += static_cast<O>(o[6]);
        A[7] += static_cast<O>(o[7]);
        A[8] += static_cast<O>(o[8]);
        return *this;
    }

    template<typename O>
    mat3<T> operator-=(const mat3<O>& o) {
        A[0] -= static_cast<O>(o[0]);
        A[1] -= static_cast<O>(o[1]);
        A[2] -= static_cast<O>(o[2]);
        A[3] -= static_cast<O>(o[3]);
        A[4] -= static_cast<O>(o[4]);
        A[5] -= static_cast<O>(o[5]);
        A[6] -= static_cast<O>(o[6]);
        A[7] -= static_cast<O>(o[7]);
        A[8] -= static_cast<O>(o[8]);
        return *this;
    }

    template<typename O>
    mat3<T> operator-() {
        mat3<T> m;
        m[0] = -A[0];
        m[1] = -A[1];
        m[2] = -A[2];
        m[3] = -A[3];
        m[4] = -A[4];
        m[5] = -A[5];
        m[6] = -A[6];
        m[7] = -A[7];
        m[8] = -A[8];
        return m;
    }

    template <typename O>
    mat3<T> operator*=(const mat3<O>& m) {
        return operator=((*this) * m);
    }

    mat3<T> adj() {
        mat3<T> m;
        m(0, 0) = (operator()(1, 1) * operator()(2, 2) - operator()(2, 1) * operator()(1, 2));
        m(0, 1) = (operator()(0, 2) * operator()(2, 1) - operator()(0, 1) * operator()(2, 2));
        m(0, 2) = (operator()(0, 1) * operator()(1, 2) - operator()(0, 2) * operator()(1, 1));
        m(1, 0) = (operator()(1, 2) * operator()(2, 0) - operator()(1, 0) * operator()(2, 2));
        m(1, 1) = (operator()(0, 0) * operator()(2, 2) - operator()(0, 2) * operator()(2, 0));
        m(1, 2) = (operator()(1, 0) * operator()(0, 2) - operator()(0, 0) * operator()(1, 2));
        m(2, 0) = (operator()(1, 0) * operator()(2, 1) - operator()(2, 0) * operator()(1, 1));
        m(2, 1) = (operator()(2, 0) * operator()(0, 1) - operator()(0, 0) * operator()(2, 1));
        m(2, 2) = (operator()(0, 0) * operator()(1, 1) - operator()(1, 0) * operator()(0, 1));
        return m;
    }
};

template <typename T>
float det(mat3<T>& m) {
    return
        m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) -
        m(0, 1) * (m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0)) +
        m(0, 2) * (m(1, 0) * m(2, 1) - m(1, 1) * m(2, 0));
}

template <typename M, typename V>
inline vec3<M> operator*(mat3<M> m, vec3<V> v) { return vec3<M>(
            (m(0, 0))*(v.x) + (m(1, 0))*(v.y) + (m(2, 0))*(v.z),
            (m(0, 1))*(v.x) + (m(1, 1))*(v.y) + (m(2, 1))*(v.z),
            (m(0, 2))*(v.x) + (m(1, 2))*(v.y) + (m(2, 2))*(v.z)); }

template <typename M, typename C>
inline mat3<M> operator*(mat3<M> m, mat3<C> c) { return mat3<M>(
            m(0, 0)*c(0, 0) + m(1, 0)*c(0, 1) + m(2, 0)*c(0, 2),        m(0, 1)*c(0, 0) + m(1, 1)*c(0, 1) + m(2, 1)*c(0, 2),        m(0, 2)*c(0, 0) + m(1, 2)*c(0, 1) + m(2, 2)*c(0, 2),
            m(0, 0)*c(1, 0) + m(1, 0)*c(1, 1) + m(2, 0)*c(1, 2),        m(0, 1)*c(1, 0) + m(1, 1)*c(1, 1) + m(2, 1)*c(1, 2),        m(0, 2)*c(1, 0) + m(1, 2)*c(1, 1) + m(2, 2)*c(1, 2),
            m(0, 0)*c(2, 0) + m(1, 0)*c(2, 1) + m(2, 0)*c(2, 2),        m(0, 1)*c(2, 0) + m(1, 1)*c(2, 1) + m(2, 1)*c(2, 2),        m(0, 2)*c(2, 0) + m(1, 2)*c(2, 1) + m(2, 2)*c(2, 2)
    ); }

inline mat3<float> rot_xy(float alpha) {
    return {
            cosf(alpha), sinf(alpha), 0,
            -sinf(alpha), cosf(alpha), 0,
            0         , 0         , 1
    };
}
inline mat3<float> rot_yz(float alpha) {
    return {
            1,  0         , 0         ,
            0,  cosf(alpha), sinf(alpha),
            0, -sinf(alpha), cosf(alpha)
    };
}
inline mat3<float> rot_zx(float alpha) {
    return {
            cosf(alpha), 0, -sinf(alpha),
            0         , 1,  0         ,
            sinf(alpha), 0,  cosf(alpha)
    };
}

#endif//VOGL_MAT3_HPP