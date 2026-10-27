#ifndef GJK2D_CORE_HPP
#define GJK2D_CORE_HPP

#include <cmath>
#include <algorithm>
#include <cstdint>

namespace gjk2d {

// ============================================================================
// MathX - 数学工具
// ============================================================================
struct MathX {
    static constexpr float PI       = 3.14159265358979323846f;
    static constexpr float DEG2RAD  = PI / 180.0f;
    static constexpr float RAD2DEG  = 180.0f / PI;

    static float EPSILON;

    static float Abs(float v)              { return std::fabs(v); }
    static int   Abs(int v)                { return std::abs(v); }
    static float Sin(float rad)            { return std::sin(rad); }
    static float Cos(float rad)            { return std::cos(rad); }
    static float ACos(float v)             { return std::acos(v); }
    static float Atan2(float y, float x)   { return std::atan2(y, x); }
    static float Sqrt(float v)             { return std::sqrt(v); }
    static float Pow(float v, int n)       { return std::pow(v, n); }
    static float Pow2(float v)             { return v * v; }
    static float Floor(float v)            { return std::floor(v); }
    static float Ceiling(float v)          { return std::ceil(v); }

    static float Min(float a, float b)                      { return a < b ? a : b; }
    static float Min(float a, float b, float c)             { return Min(Min(a, b), c); }
    static float Min(float a, float b, float c, float d)    { return Min(Min(a, b, c), d); }
    static float Max(float a, float b)                      { return a > b ? a : b; }
    static float Max(float a, float b, float c)             { return Max(Max(a, b), c); }
    static float Max(float a, float b, float c, float d)    { return Max(Max(a, b, c), d); }

    static int Min(int a, int b)            { return a < b ? a : b; }
    static int Max(int a, int b)            { return a > b ? a : b; }

    static float Clamp(float v, float lo, float hi) { return Max(lo, Min(v, hi)); }
    static float Clamp01(float v)                   { return Clamp(v, 0.0f, 1.0f); }
    static int   Sign(float v)                      { return v >= 0.0f ? 1 : -1; }

    static bool Equals(float a, float b) { return Abs(a - b) < EPSILON; }

    static float Clamp360(float angle) {
        while (angle < 0.0f)   angle += 360.0f;
        while (angle >= 360.0f) angle -= 360.0f;
        return angle;
    }

    static int Round(float v) { return static_cast<int>(v + 0.5f); }
};

// 计算机器 epsilon
inline float compute_machine_epsilon() {
    float eps = 0.5f;
    while (1.0f + eps > 1.0f)
        eps *= 0.5f;
    return eps;
}

inline float MathX::EPSILON = compute_machine_epsilon();

// ============================================================================
// Vector2 - 2D 向量
// ============================================================================
struct Vector2 {
    float x, y;
    float w;  // 齐次坐标 w 分量，用于矩阵变换

    Vector2() : x(0), y(0), w(1) {}
    Vector2(float x, float y) : x(x), y(y), w(1) {}
    explicit Vector2(float x, float y, float w) : x(x), y(y), w(w) {}

    // 常用常量
    static const Vector2& zero()  { static Vector2 v(0, 0); return v; }
    static const Vector2& one()   { static Vector2 v(1, 1); return v; }
    static const Vector2& left()  { static Vector2 v(-1, 0); return v; }
    static const Vector2& right() { static Vector2 v(1, 0); return v; }
    static const Vector2& up()    { static Vector2 v(0, -1); return v; }   // 屏幕坐标系
    static const Vector2& down()  { static Vector2 v(0, 1); return v; }

    // 模长
    float magnitude()  const { return MathX::Sqrt(magnitude2()); }
    float magnitude2() const { return x * x + y * y; }

    // 归一化
    Vector2 normalized() const {
        float m = magnitude();
        return (m > MathX::EPSILON) ? Vector2(x / m, y / m) : Vector2(0, 0);
    }

    void Normalize() {
        float m = magnitude();
        if (m > MathX::EPSILON) { x /= m; y /= m; }
    }

    // 取反
    Vector2 negatived() const { return Vector2(-x, -y); }
    void Negative() { x = -x; y = -y; }

    // 垂直向量 (x, y) -> (y, -x)
    Vector2 perpendicular() const { return Vector2(y, -x); }

    // 90度旋转
    void CW90()  { float t = x; x = -y; y = t; }   // (x,y) -> (-y, x)
    void CCW90() { float t = x; x = y;  y = -t; }  // (x,y) -> (y, -x)  [修复原 C# bug]

    // 点乘
    float Dot(const Vector2& v) const { return x * v.x + y * v.y; }
    static float Dot(const Vector2& a, const Vector2& b) { return a.x * b.x + a.y * b.y; }

    // 叉乘 (2D 中返回标量，表示 z 分量)
    float Cross(const Vector2& v) const { return x * v.y - y * v.x; }
    static float Cross(const Vector2& a, const Vector2& b) { return a.x * b.y - a.y * b.x; }

    // 向量三重积：b * (a·c) - a * (b·c)
    static Vector2 Mul3(const Vector2& a, const Vector2& b, const Vector2& c) {
        float ac = Dot(a, c);
        float bc = Dot(b, c);
        return Vector2(b.x * ac - a.x * bc,
                       b.y * ac - a.y * bc);
    }

    // 与 X 轴夹角 (弧度)
    float Angle() const { return Angle(*this, right()); }
    static float Angle(const Vector2& a, const Vector2& b) {
        float d = Dot(a.normalized(), b.normalized());
        return MathX::ACos(d);
    }

    // 极角 (弧度)
    float Theta() const { return MathX::Atan2(y, x); }

    // 反射向量
    static Vector2 Reflect(const Vector2& input, const Vector2& normal) {
        auto I = input;
        auto N = normal.normalized();
        return (I - (2.0f * Dot(I, N)) * N).normalized();
    }

    // 距离
    float Distance(const Vector2& v) const { return Distance(*this, v); }
    static float Distance(const Vector2& a, const Vector2& b) {
        return MathX::Sqrt(Distance2(a, b));
    }
    float Distance2(const Vector2& v) const { return Distance2(*this, v); }
    static float Distance2(const Vector2& a, const Vector2& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return dx * dx + dy * dy;
    }

    // 运算符重载
    Vector2 operator-() const { return Vector2(-x, -y); }
    Vector2 operator+(const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
    Vector2 operator-(const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
    Vector2 operator*(float k) const { return Vector2(x * k, y * k); }
    Vector2 operator/(float k) const { return Vector2(x / k, y / k); }

    Vector2& operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
    Vector2& operator-=(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
    Vector2& operator*=(float k) { x *= k; y *= k; return *this; }
    Vector2& operator/=(float k) { x /= k; y /= k; return *this; }

    friend Vector2 operator*(float k, const Vector2& v) { return Vector2(v.x * k, v.y * k); }
};

// ============================================================================
// Matrix - 3x3 仿射变换矩阵 (齐次坐标)
// ============================================================================
struct Matrix {
    float m11, m12, m13;
    float m21, m22, m23;
    float m31, m32, m33;

    Matrix()
        : m11(1), m12(0), m13(0)
        , m21(0), m22(1), m23(0)
        , m31(0), m32(0), m33(1) {}

    Matrix(float m11, float m12, float m13,
           float m21, float m22, float m23,
           float m31, float m32, float m33)
        : m11(m11), m12(m12), m13(m13)
        , m21(m21), m22(m22), m23(m23)
        , m31(m31), m32(m32), m33(m33) {}

    static Matrix CreateTranslationMatrix(float x, float y) {
        return Matrix(1, 0, 0,
                      0, 1, 0,
                      x, y, 1);
    }

    static Matrix CreateTranslationMatrix(const Vector2& p) {
        return CreateTranslationMatrix(p.x, p.y);
    }

    static Matrix CreateRotationMatrix(float radian) {
        float s = MathX::Sin(radian);
        float c = MathX::Cos(radian);
        return Matrix( c, s, 0,
                      -s, c, 0,
                       0, 0, 1);
    }

    static Vector2 Transform(const Vector2& v, const Matrix& m) {
        float x = v.x * m.m11 + v.y * m.m21 + v.w * m.m31;
        float y = v.x * m.m12 + v.y * m.m22 + v.w * m.m32;
        return Vector2(x, y);
    }

    Matrix operator*(const Matrix& rhs) const {
        return Matrix(
            m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31,
            m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32,
            m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33,
            m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31,
            m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32,
            m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33,
            m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31,
            m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32,
            m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33
        );
    }
};

// ============================================================================
// Transform - 变换
// ============================================================================
struct Transform {
    Vector2 position{0, 0};
    float rotation = 0.0f;  // 角度 (度)
    float scale    = 1.0f;

    void Move(const Vector2& delta) { position = position + delta; }
    void Rotate(float delta)        { rotation += delta; }
};

// ============================================================================
// 几何体类型枚举
// ============================================================================
enum class GeometryType : uint8_t {
    Circle    = 0,
    Rectangle = 1,
    Polygon   = 2,
    Capsule   = 3,
    Ellipse   = 4,
    Pie       = 5,
    Segment   = 6,
};

// ============================================================================
// Penetration - 穿透数据
// ============================================================================
struct Penetration {
    Vector2 normal{0, 0};
    float   depth = 0.0f;

    Penetration() = default;
    Penetration(const Vector2& n, float d) : normal(n), depth(d) {}
    void Clear() { normal = Vector2::zero(); depth = 0.0f; }
};

} // namespace gjk2d

#endif // GJK2D_CORE_HPP
