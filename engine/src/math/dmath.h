#pragma once

#include "defines.h"
#include "math_types.h"

#define PI 3.14159265358979323846f
#define PI2 2.0f * PI
#define HALF_PI 0.5f * PI
#define QUARTER_PI 0.25f * PI
#define INV_PI 1.0f / PI
#define INV_2PI 1.0f / PI2
#define SQRT2 1.41421356237309504880f
#define SQRT3 1.73205080756887729352f
#define SQRT1HALF 0.70710678118654752440f
#define SQRT1THIRD 0.57735026918962576450f
#define DEG2RAD PI / 180.0f
#define RAD2DEG 180.0f / PI

// The multiplier to convert seconds to milliseconds.
#define SEC2MS 1000.0f

// The multiplier to convert milliseconds to seconds.
#define MS2SEC 0.001f

// A huge number that should be larger than any valid number used.
#define INFINITY 1e30f

// Smallest positive number where 1.0 + FLOATEPSILON != 0
#define FLOAT_EPSILON 1.192092896e-07f

DAPI f32 dsin(f32 x);
DAPI f32 dcos(f32 x);
DAPI f32 dtan(f32 x);
DAPI f32 dacos(f32 x);
DAPI f32 dsqrt(f32 x);
DAPI f32 dabs(f32 x);

DINLINE b8 IsPowOf2(u64 value){
    return (value != 0) && ((value & (value - 1)) == 0);
}

DAPI i32 drandom();
DAPI i32 drandomInRange(i32 min, i32 max);
DAPI f32 drandomF();
DAPI f32 drandomFInRange(f32 min, f32 max);

DINLINE Vec2 Vec2Create(f32 x, f32 y){
    Vec2 outVec = {};
    outVec.x = x;
    outVec.y = y;
    return outVec;
}

DINLINE Vec2 Vec2One(){
    return (Vec2){1.0f, 1.0f};
}

DINLINE Vec2 Vec2Up(){
    return (Vec2){0.0f, 1.0f};
}

DINLINE Vec2 Vec2Down(){
    return (Vec2){0.0f, -1.0f};
}

DINLINE Vec2 Vec2Left(){
    return (Vec2){-1.0f, 0.0f};
}

DINLINE Vec2 Vec2Right(){
    return (Vec2){1.0f, 0.0f};
}

DINLINE Vec2 operator+(Vec2 v1, Vec2 v2){
    Vec2 result = {};
    result.e[0] = v1.e[0] + v2.e[0];
    result.e[1] = v1.e[1] + v2.e[1];
    return result;
}

DINLINE Vec2& operator+=(Vec2& a, Vec2& b){
    a.e[0] += b.e[0];
    a.e[1] += b.e[1];
    return a;
}

DINLINE Vec2 operator-(Vec2 a, Vec2 b){
    Vec2 result = {};
    result.e[0] = a.e[0] - b.e[0];
    result.e[1] = a.e[1] - b.e[1];
    return result;
}

DINLINE Vec2 operator-(Vec2 a){
    Vec2 result = {};
    result.e[0] = -a.e[0];
    result.e[1] = -a.e[1];
    return result;
}

DINLINE Vec2 operator*(f32 a, Vec2 b){
    Vec2 result = {};
    result.e[0] = b.e[0] * a;
    result.e[1] = b.e[1] * a;
    return result;
}

DINLINE Vec2 operator*(Vec2 a, f32 b){
    Vec2 result = b * a;
    return result;
}

DINLINE Vec2& operator*=(Vec2& a, f32 b){
    a = a * b;
    return a;
}

DINLINE Vec2 HadamardProd(Vec2 a, Vec2 b){
    Vec2 result = {};
    result.e[0] = a.e[0] * b.e[0];
    result.e[1] = a.e[1] * b.e[1];
    return result;
}

DINLINE Vec2 HadamardDiv(Vec2 a, Vec2 b){
    Vec2 result = {};
    result.e[0] = a.e[0] / b.e[0];
    result.e[1] = a.e[1] / b.e[1];
    return result;
}

DINLINE Vec2 operator/(Vec2 a, f32 b){
    Vec2 result = {};
    result.e[0] = a.e[0] / b;
    result.e[1] = a.e[1] / b;
    return result;
}

DINLINE Vec2& operator/=(Vec2& a, f32 b){
    a = a / b;
    return a;
}

DINLINE f32 DotProd(Vec2 a, Vec2 b){
    f32 result = a.e[0]*b.e[0] + a.e[1]*b.e[1];
    return result;
}

DINLINE f32 LengthSq(Vec2 vec){
    f32 result = DotProd(vec, vec);
    return result;
}

DINLINE f32 Length(Vec2 vec){
    f32 result = dsqrt(LengthSq(vec));
    return result;
}

DINLINE Vec2 UnitVector(Vec2 vec){
    Vec2 result = vec / Length(vec);
    return result;
}

DINLINE void Normalize(Vec2* vec){
    *vec /= Length(*vec);
}

DINLINE b8 Vec2Compare(Vec2 vec0, Vec2 vec1, f32 tolerance){
    if(dabs(vec0.e[0] - vec1.e[0]) > tolerance){
        return false;
    }
    if(dabs(vec0.e[1] - vec1.e[1]) > tolerance){
        return false;
    }
    return true;
}

DINLINE bool operator==(Vec2 vec0, Vec2 vec1){
    return Vec2Compare(vec0, vec1, FLOAT_EPSILON);
}

DINLINE f32 Distance(Vec2 vec0, Vec2 vec1){
    return Length(vec0 - vec1);
}

DINLINE Vec3 Vec3Create(f32 x, f32 y, f32 z){
    Vec3 result = {};
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}


DINLINE Vec3 CopyVec3(Vec3 src){
    Vec3 result = {};
    result.x = src.x;
    result.y = src.y;
    result.z = src.z;
    return result;
}

DINLINE Vec3 Vec3One(){
    return {1.0f, 1.0f, 1.0f};
}

DINLINE Vec3 Vec3Up(){
    return {0.0f, 1.0f, 0.0f};
}

DINLINE Vec3 Vec3Down(){
    return {0.0f, -1.0f, 0.0f};
}

DINLINE Vec3 Vec3Left(){
    return {-1.0f, 0.0f, 0.0f};
}

DINLINE Vec3 Vec3Right(){
    return {1.0f, 0.0f, 0.0f};
}

DINLINE Vec3 Vec3Forward(){
    return {0.0f, 0.0f, -1.0f};
}

DINLINE Vec3 Vec3Backward(){
    return {0.0f, 0.0f, 1.0f};
}

DINLINE Vec3 operator+(Vec3 a, Vec3 b){
    Vec3 result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

DINLINE Vec3& operator+=(Vec3& a, Vec3 b){
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

DINLINE Vec3 operator-(Vec3 a, Vec3 b){
    Vec3 result = {};
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

DINLINE Vec3 operator-(Vec3 a){
    Vec3 result = {};
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    return result;
}

DINLINE Vec3 operator*(f32 a, Vec3 b){
    Vec3 result = {};
    result.x = b.x * a;
    result.y = b.y * a;
    result.z = b.z * a;
    return result;
}

DINLINE Vec3 operator*(Vec3 a, f32 b){
    Vec3 result = b * a;
    return result;
}

DINLINE Vec3& operator*=(Vec3& a, f32 b){
    a = a * b;
    return a;
}

DINLINE Vec3 HadamardProd(Vec3 a, Vec3 b){
    Vec3 result = {};
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    return result;
}

DINLINE Vec3 HadamardDiv(Vec3 a, Vec3 b){
    Vec3 result = {};
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;
    return result;
}

DINLINE Vec3 operator/(Vec3 a, f32 b){
    Vec3 result = {};
    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    return result;
}

DINLINE Vec3& operator/=(Vec3& a, f32 b){
    a = a / b;
    return a;
}

DINLINE f32 DotProd(Vec3 a, Vec3 b){
    f32 result = a.x*b.x + a.y*b.y + a.z*b.z;
    return result;
}

DINLINE f32 LengthSq(Vec3 vec){
    f32 result = DotProd(vec, vec);
    return result;
}

DINLINE f32 Length(Vec3 vec){
    f32 result = dsqrt(LengthSq(vec));
    return result;
}

DINLINE Vec3 CrossProd(Vec3 a, Vec3 b){
    Vec3 result = {};
    result.e[0] = a.e[1] * b.e[2] - a.e[2] * b.e[1];
    result.e[1] = a.e[2] * b.e[0] - a.e[0] * b.e[2];
    result.e[2] = a.e[0] * b.e[1] - a.e[1] * b.e[0];
    return result;
}

DINLINE void Normalize(Vec3* vec){
    *vec /= Length(*vec);
}

DINLINE Vec3 UnitVector(Vec3 vec){
    Vec3 result = vec * (1.0f / Length(vec));
    return result;
}

DINLINE b8 Vec3Compare(Vec3 vec0, Vec3 vec1, f32 tolerance){
    if(dabs(vec0.e[0] - vec1.e[0]) > tolerance){
        return false;
    }
    if(dabs(vec0.e[1] - vec1.e[1]) > tolerance){
        return false;
    }
    if(dabs(vec0.e[2] - vec1.e[2]) > tolerance){
        return false;
    }
    return true;
}

DINLINE bool operator==(Vec3 vec0, Vec3 vec1){
    return Vec3Compare(vec0, vec1, FLOAT_EPSILON);
}

DINLINE f32 Distance(Vec3 vec0, Vec3 vec1){
    return Length(vec0 - vec1);
}

DINLINE Vec4 Vec4Create(f32 x, f32 y, f32 z, f32 w){
    Vec4 result = {};
#if defined(DUSESIMD)
    result.data = mmsetrps(x, y, z, w);
#else
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
#endif
    return result;
}

DINLINE Vec4 Vec4Create(Vec3 vec, f32 f){
#if defined(DUSESIMD)
    Vec4 result = {};
    result.data = mmsetrps(x, y, z, w);
    return result;
#else
    return {vec.x, vec.y, vec.z, f};
#endif
}

DINLINE Vec3 Vec4ToVec3(Vec4 vec){
    return {vec.x, vec.y, vec.z};
}

DINLINE Vec4 CopyVec4(Vec4 src){
    Vec4 result = {};
    result.x = src.x;
    result.y = src.y;
    result.z = src.z;
    result.w = src.w;
    return result;
}

DINLINE Vec4 Vec4Zero(){
    return {0.0f, 0.0f, 0.0f, 0.0f};
}

DINLINE Vec4 Vec4One(){
    return {1.0f, 1.0f, 1.0f, 1.0f};
}

DINLINE Vec4 operator+(Vec4 a, Vec4 b){
    Vec4 result = {};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

DINLINE Vec4& operator+=(Vec4& a, Vec4& b){
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

DINLINE Vec4 operator-(Vec4 a, Vec4 b){
    Vec4 result = {};
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

DINLINE Vec4 operator-(Vec4 a){
    Vec4 result = {};
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;
    return result;
}

DINLINE Vec4 operator*(f32 a, Vec4 b){
    Vec4 result = {};
    result.x = b.x * a;
    result.y = b.y * a;
    result.z = b.z * a;
    result.w = b.w * a;
    return result;
}

DINLINE Vec4 operator*(Vec4 a, f32 b){
    Vec4 result = b * a;
    return result;
}

DINLINE Vec4& operator*=(Vec4& a, f32 b){
    a = a * b;
    return a;
}

DINLINE Vec4 HadamardProd(Vec4 a, Vec4 b){
    Vec4 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    result.w = a.w * b.w;
    return result;
}

DINLINE Vec4 HadamardDiv(Vec4 a, Vec4 b){
    Vec4 result = {};
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;
    result.w = a.w / b.w;
    return result;
}

inline Vec4 operator/(Vec4 a, f32 b){
    Vec4 result;
    result.x = a.x / b;
    result.y = a.y / b;
    result.z = a.z / b;
    result.w = a.w / b;
    return result;
}

inline Vec4& operator/=(Vec4& a, f32 b){
    a = a / b;
    return a;
}

inline f32 DotProd(Vec4 a, Vec4 b){
    f32 result = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    return result;
}

inline f32 LengthSq(Vec4 vec){
    f32 result = DotProd(vec, vec);
    return result;
}

inline f32 Length(Vec4 vec){
    f32 result = dsqrt(LengthSq(vec));
    return result;
}

DINLINE Vec4 UnitVector(Vec4 vec){
    Vec4 result = vec / Length(vec);
    return result;
}

DINLINE void Normalize(Vec4* vec){
    *vec /= Length(*vec);
}

DINLINE b8 Vec4Compare(Vec4 vec0, Vec4 vec1, f32 tolerance){
    if(dabs(vec0.e[0] - vec1.e[0]) > tolerance){
        return false;
    }
    if(dabs(vec0.e[1] - vec1.e[1]) > tolerance){
        return false;
    }
    if(dabs(vec0.e[2] - vec1.e[2]) > tolerance){
        return false;
    }
    if(dabs(vec0.e[3] - vec1.e[3]) > tolerance){
        return false;
    }
    return true;
}

DINLINE bool operator==(Vec4 vec0, Vec4 vec1){
    return Vec4Compare(vec0, vec1, FLOAT_EPSILON);
}

DINLINE f32 Distance(Vec4 vec0, Vec4 vec1){
    return Length(vec0 - vec1);
}

DINLINE Mat4 Mat4Identity(){
    Mat4 result = {};
    result.data[0] = 1.0f;
    result.data[5] = 1.0f;
    result.data[10] = 1.0f;
    result.data[15] = 1.0f;
    return result;
}

DINLINE Mat4 Mat4Mult(Mat4 Mat0, Mat4 Mat1){
    Mat4 result = Mat4Identity();
    f32* m0Ptr = Mat0.data;
    f32* m1Ptr = Mat1.data;
    f32* dstPtr = result.data;

    for(i32 i = 0; i < 4; i++){
        for(i32 j = 0; j < 4; j++){
            *dstPtr = 
                m0Ptr[0] * m1Ptr[0 + j] +
                m0Ptr[1] * m1Ptr[4 + j] +
                m0Ptr[2] * m1Ptr[8 + j] +
                m0Ptr[3] * m1Ptr[12 + j];
            dstPtr++;
        }
        m0Ptr += 4;
    }
    return result;
}

DINLINE Mat4 Mat4Orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearClip, f32 farClip){
    Mat4 result = Mat4Identity();

    f32 lr = 1.0f / (left - right);
    f32 bt = 1.0f / (bottom - top);
    f32 nf = 1.0f / (nearClip - farClip);

    result.data[0] = -2.0f * lr;
    result.data[5] = -2.0f * bt;
    result.data[10] = 2.0f * nf;

    result.data[12] = (left + right) * lr;
    result.data[13] = (top + bottom) * bt;
    result.data[14] = (farClip + nearClip) * nf;
    return result;
}

DINLINE Mat4 Mat4Perspective(f32 fovRadians, f32 aspectRatio, f32 nearClip, f32 farClip){
    f32 halfTanFov = dtan(fovRadians * 0.5f);
    Mat4 result = {};
    result.data[0] = 1.0f / (aspectRatio * halfTanFov);
    result.data[5] = 1.0f / halfTanFov;
    result.data[10] = -((farClip + nearClip) / (farClip - nearClip));
    result.data[11] = -1.0f;
    result.data[14] = -((2.0f * farClip * nearClip) / (farClip - nearClip));
    return result;
}

DINLINE Mat4 Mat4LookAt(Vec3 pos, Vec3 target, Vec3 up){
    Mat4 result = {};
    Vec3 zAxis = UnitVector(target - pos);
    
    Vec3 xAxis = UnitVector(CrossProd(zAxis, up));
    Vec3 yAxis = CrossProd(xAxis, zAxis);
    
    result.data[0] = xAxis.x;
    result.data[1] = yAxis.x;
    result.data[2] = -zAxis.x;
    result.data[3] = 0;
    result.data[4] = xAxis.y;
    result.data[5] = yAxis.y;
    result.data[6] = -zAxis.y;
    result.data[7] = 0;
    result.data[8] = xAxis.z;
    result.data[9] = yAxis.z;
    result.data[10] = -zAxis.z;
    result.data[11] = 0;
    result.data[12] = -DotProd(xAxis, pos);
    result.data[13] = -DotProd(yAxis, pos);
    result.data[14] = DotProd(zAxis, pos);
    result.data[15] = 1.0f;

    return result;
}

DINLINE Mat4 Mat4Transpose(Mat4 Matrix) {
    Mat4 result = Mat4Identity();
    result.data[0] = Matrix.data[0];
    result.data[1] = Matrix.data[4];
    result.data[2] = Matrix.data[8];
    result.data[3] = Matrix.data[12];
    result.data[4] = Matrix.data[1];
    result.data[5] = Matrix.data[5];
    result.data[6] = Matrix.data[9];
    result.data[7] = Matrix.data[13];
    result.data[8] = Matrix.data[2];
    result.data[9] = Matrix.data[6];
    result.data[10] = Matrix.data[10];
    result.data[11] = Matrix.data[14];
    result.data[12] = Matrix.data[3];
    result.data[13] = Matrix.data[7];
    result.data[14] = Matrix.data[11];
    result.data[15] = Matrix.data[15];
    return result;
}

DINLINE Mat4 Mat4Inverse(Mat4 Matrix) {
    f32* m = Matrix.data;

    f32 t0 = m[10] * m[15];
    f32 t1 = m[14] * m[11];
    f32 t2 = m[6] * m[15];
    f32 t3 = m[14] * m[7];
    f32 t4 = m[6] * m[11];
    f32 t5 = m[10] * m[7];
    f32 t6 = m[2] * m[15];
    f32 t7 = m[14] * m[3];
    f32 t8 = m[2] * m[11];
    f32 t9 = m[10] * m[3];
    f32 t10 = m[2] * m[7];
    f32 t11 = m[6] * m[3];
    f32 t12 = m[8] * m[13];
    f32 t13 = m[12] * m[9];
    f32 t14 = m[4] * m[13];
    f32 t15 = m[12] * m[5];
    f32 t16 = m[4] * m[9];
    f32 t17 = m[8] * m[5];
    f32 t18 = m[0] * m[13];
    f32 t19 = m[12] * m[1];
    f32 t20 = m[0] * m[9];
    f32 t21 = m[8] * m[1];
    f32 t22 = m[0] * m[5];
    f32 t23 = m[4] * m[1];

    Mat4 result = {};
    f32* o = result.data;

    o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] + t2 * m[9] + t5 * m[13]);
    o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
    o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
    o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

    f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

    o[0] = d * o[0];
    o[1] = d * o[1];
    o[2] = d * o[2];
    o[3] = d * o[3];
    o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
    o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
    o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
    o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
    o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
    o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
    o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
    o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
    o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
    o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
    o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
    o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

    return result;
}

DINLINE Mat4 Mat4Translation(Vec3 pos){
    Mat4 result = Mat4Identity();
    result.data[12] = pos.x;
    result.data[13] = pos.y;
    result.data[14] = pos.z;
    return result;
}

DINLINE Mat4 Mat4Scale(Vec3 scale){
    Mat4 result = Mat4Identity();
    result.data[0] = scale.x;
    result.data[5] = scale.y;
    result.data[10] = scale.z;
    return result;
}

DINLINE Mat4 Mat4EulerX(f32 rads){
    Mat4 result = Mat4Identity();
    f32 c = dcos(rads);
    f32 s = dsin(rads);
    result.data[5] = c;
    result.data[6] = s;
    result.data[9] = -s;
    result.data[10] = c;
    return result;
}

DINLINE Mat4 Mat4EulerY(f32 rads){
    Mat4 result = Mat4Identity();
    f32 c = dcos(rads);
    f32 s = dsin(rads);
    result.data[0] = c;
    result.data[2] = -s;
    result.data[8] = s;
    result.data[10] = c;
    return result;
}

DINLINE Mat4 Mat4EulerZ(f32 rads){
    Mat4 result = Mat4Identity();
    f32 c = dcos(rads);
    f32 s = dsin(rads);
    result.data[0] = c;
    result.data[1] = s;
    result.data[4] = -s;
    result.data[5] = c;
    return result;
}

DINLINE Mat4 Mat4EulerXyz(f32 xRads, f32 yRads, f32 zRads){
    Mat4 rotX = Mat4EulerX(xRads);
    Mat4 rotY = Mat4EulerY(yRads);
    Mat4 rotZ = Mat4EulerZ(zRads);
    Mat4 result = Mat4Mult(rotX, rotY);
    result = Mat4Mult(result, rotZ);
    return result;
}

DINLINE Vec3 Mat4Forward(Mat4 Matrix){
    Vec3 result = {};
    result.x = -Matrix.data[2];
    result.y = -Matrix.data[6];
    result.z = -Matrix.data[10];
    Normalize(&result);
    return result;
}

DINLINE Vec3 Mat4Backward(Mat4 Matrix){
    Vec3 result = {};
    result.x = Matrix.data[2];
    result.y = Matrix.data[6];
    result.z = Matrix.data[10];
    Normalize(&result);
    return result;
}

DINLINE Vec3 Mat4Up(Mat4 Matrix){
    Vec3 result = {};
    result.x = Matrix.data[1];
    result.y = Matrix.data[5];
    result.z = Matrix.data[9];
    Normalize(&result);
    return result;
}

DINLINE Vec3 Mat4Down(Mat4 Matrix){
    Vec3 result = {};
    result.x = -Matrix.data[1];
    result.y = -Matrix.data[5];
    result.z = -Matrix.data[9];
    Normalize(&result);
    return result;
}

DINLINE Vec3 Mat4Left(Mat4 Matrix){
    Vec3 result = {};
    result.x = -Matrix.data[0];
    result.y = -Matrix.data[4];
    result.z = -Matrix.data[8];
    Normalize(&result);
    return result;
}

DINLINE Vec3 Mat4Right(Mat4 Matrix){
    Vec3 result = {};
    result.x = Matrix.data[0];
    result.y = Matrix.data[4];
    result.z = Matrix.data[8];
    Normalize(&result);
    return result;
}

//QUATERNIONS

DINLINE Quat QuatIdentity(){
    return {0, 0, 0, 1.0f};
}

DINLINE f32 QuatNormal(Quat q){
    return dsqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
}

DINLINE Quat QuatNormalize(Quat q){
    f32 normal = QuatNormal(q);
    return {q.x/normal, q.y/normal, q.z/normal, q.w/normal};
}

DINLINE Quat QuatConjugate(Quat q){
    return {-q.x, -q.y, -q.z, q.w};
}

DINLINE Quat QuatInverse(Quat q){
    return QuatNormalize(QuatConjugate(q));
}

DINLINE Quat QuatMul(Quat q0, Quat q1) {
    Quat result = {};

    result.x = q0.x * q1.w +
               q0.y * q1.z -
               q0.z * q1.y +
               q0.w * q1.x;

    result.y = -q0.x * q1.z +
               q0.y * q1.w +
               q0.z * q1.x +
               q0.w * q1.y;

    result.z = q0.x * q1.y -
               q0.y * q1.x +
               q0.z * q1.w +
               q0.w * q1.z;

    result.w = -q0.x * q1.x -
                q0.y * q1.y -
                q0.z * q1.z +
                q0.w * q1.w;

    return result;
}

DINLINE f32 DotProd(Quat q0, Quat q1) {
    return q0.x * q1.x +
           q0.y * q1.y +
           q0.z * q1.z +
           q0.w * q1.w;
}

DINLINE Mat4 QuatToMat4(Quat q) {
    Mat4 result = Mat4Identity();

    // https://stackoverflow.com/questions/1556260/convert-Quaternion-rotation-to-rotation-Matrix

    Quat n = QuatNormalize(q);

    result.data[0] = 1.0f - 2.0f * n.y * n.y - 2.0f * n.z * n.z;
    result.data[1] = 2.0f * n.x * n.y - 2.0f * n.z * n.w;
    result.data[2] = 2.0f * n.x * n.z + 2.0f * n.y * n.w;

    result.data[4] = 2.0f * n.x * n.y + 2.0f * n.z * n.w;
    result.data[5] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.z * n.z;
    result.data[6] = 2.0f * n.y * n.z - 2.0f * n.x * n.w;

    result.data[8] = 2.0f * n.x * n.z - 2.0f * n.y * n.w;
    result.data[9] = 2.0f * n.y * n.z + 2.0f * n.x * n.w;
    result.data[10] = 1.0f - 2.0f * n.x * n.x - 2.0f * n.y * n.y;

    return result;
}

// Calculates a rotation Matrix based on the Quaternion and the passed in center point.
DINLINE Mat4 QuatToRotationMatrix(Quat q, Vec3 center) {
    Mat4 result = {};

    f32* o = result.data;
    o[0] = (q.x * q.x) - (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[1] = 2.0f * ((q.x * q.y) + (q.z * q.w));
    o[2] = 2.0f * ((q.x * q.z) - (q.y * q.w));
    o[3] = center.x - center.x * o[0] - center.y * o[1] - center.z * o[2];

    o[4] = 2.0f * ((q.x * q.y) - (q.z * q.w));
    o[5] = -(q.x * q.x) + (q.y * q.y) - (q.z * q.z) + (q.w * q.w);
    o[6] = 2.0f * ((q.y * q.z) + (q.x * q.w));
    o[7] = center.y - center.x * o[4] - center.y * o[5] - center.z * o[6];

    o[8] = 2.0f * ((q.x * q.z) + (q.y * q.w));
    o[9] = 2.0f * ((q.y * q.z) - (q.x * q.w));
    o[10] = -(q.x * q.x) - (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
    o[11] = center.z - center.x * o[8] - center.y * o[9] - center.z * o[10];

    o[12] = 0.0f;
    o[13] = 0.0f;
    o[14] = 0.0f;
    o[15] = 1.0f;
    return result;
}

DINLINE Quat QuatFromAxisAngle(Vec3 axis, f32 angle, b8 normalize) {
    const f32 halfAngle = 0.5f * angle;
    f32 s = dsin(halfAngle);
    f32 c = dcos(halfAngle);

    Quat q = {s * axis.x, s * axis.y, s * axis.z, c};
    if (normalize) {
        return QuatNormalize(q);
    }
    return q;
}

DINLINE Quat QuatsLerp(Quat q0, Quat q1, f32 percentage) {
    Quat result = {};
    // Source: https://en.wikipedia.org/wiki/Slerp
    // Only unit Quaternions are valid rotations.
    // Normalize to avoid undefined behavior.
    Quat v0 = QuatNormalize(q0);
    Quat v1 = QuatNormalize(q1);

    // Compute the cosine of the angle between the two vectors.
    f32 dot = DotProd(v0, v1);

    // If the dot product is negative, slerp won't take
    // the shorter path. Note that v1 and -v1 are equivalent when
    // the negation is applied to all four components. Fix by
    // reversing one Quaternion.
    if (dot < 0.0f) {
        v1.x = -v1.x;
        v1.y = -v1.y;
        v1.z = -v1.z;
        v1.w = -v1.w;
        dot = -dot;
    }

    const f32 DOT_THRESHOLD = 0.9995f;
    if (dot > DOT_THRESHOLD) {
        // If the inputs are too close for comfort, linearly interpolate
        // and normalize the result.
        result = (Quat){
            v0.x + ((v1.x - v0.x) * percentage),
            v0.y + ((v1.y - v0.y) * percentage),
            v0.z + ((v1.z - v0.z) * percentage),
            v0.w + ((v1.w - v0.w) * percentage)};

        return QuatNormalize(result);
    }

    // Since dot is in range [0, DOTTHRESHOLD], acos is safe
    f32 theta0 = dacos(dot);          // theta0 = angle between input vectors
    f32 theta = theta0 * percentage;  // theta = angle between v0 and result
    f32 sinTheta = dsin(theta);       // compute this value only once
    f32 sinTheta0 = dsin(theta0);   // compute this value only once

    f32 s0 = dcos(theta) - dot * sinTheta / sinTheta0;  // == sin(theta0 - theta) / sin(theta0)
    f32 s1 = sinTheta / sinTheta0;

    return {
        (v0.x * s0) + (v1.x * s1),
        (v0.y * s0) + (v1.y * s1),
        (v0.z * s0) + (v1.z * s1),
        (v0.w * s0) + (v1.w * s1)};
}

/**
 * @brief Converts provided degrees to radians.
 * 
 * @param degrees The degrees to be converted.
 * @return The amount in radians.
 */
DINLINE f32 DegToRad(f32 degrees) {
    return degrees * DEG2RAD;
}

/**
 * @brief Converts provided radians to degrees.
 * 
 * @param radians The radians to be converted.
 * @return The amount in degrees.
 */
DINLINE f32 RadToDeg(f32 radians) {
    return radians * RAD2DEG;
}