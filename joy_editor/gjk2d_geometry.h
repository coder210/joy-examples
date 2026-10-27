#ifndef GJK2D_GEOMETRY_HPP
#define GJK2D_GEOMETRY_HPP

#include "gjk2d_core.h"
#include <vector>
#include <cstdint>

namespace gjk2d {

// ============================================================================
// IGeometry - 几何体接口
// ============================================================================
struct IGeometry {
    virtual ~IGeometry() = default;
    virtual GeometryType type() const = 0;
    virtual bool Contains(const Vector2& pt) const = 0;
    virtual Vector2 GetFarthestProjectionPoint(const Vector2& dir) const = 0;
};

// ============================================================================
// Circle - 圆形
// ============================================================================
struct Circle : IGeometry {
    float radius = 1.0f;

    Circle() = default;
    explicit Circle(float r) : radius(r) {}

    GeometryType type() const override { return GeometryType::Circle; }

    bool Contains(const Vector2& pt) const override {
        return pt.magnitude2() <= radius * radius;
    }

    Vector2 GetFarthestProjectionPoint(const Vector2& dir) const override {
        return radius * dir.normalized();
    }
};

// ============================================================================
// Rectangle - 矩形 (中心在原点)
// ============================================================================
struct Rectangle : IGeometry {
    float width  = 2.0f;
    float height = 2.0f;

    Rectangle() = default;
    Rectangle(float w, float h) : width(w), height(h) {}

    GeometryType type() const override { return GeometryType::Rectangle; }

    bool Contains(const Vector2& pt) const override {
        float w = width * 0.5f;
        float h = height * 0.5f;
        return pt.x >= -w && pt.x <= w && pt.y >= -h && pt.y <= h;
    }

    Vector2 GetFarthestProjectionPoint(const Vector2& dir) const override {
        float w = width * 0.5f;
        float h = height * 0.5f;

        // 四个角点
        Vector2 verts[4] = {
            Vector2(-w, -h),
            Vector2( w, -h),
            Vector2( w,  h),
            Vector2(-w,  h),
        };

        Vector2 best = verts[0];
        float maxDot = Vector2::Dot(best, dir);
        for (int i = 1; i < 4; ++i) {
            float d = Vector2::Dot(verts[i], dir);
            if (d > maxDot) { maxDot = d; best = verts[i]; }
        }
        return best;
    }
};

// ============================================================================
// Polygon - 凸多边形
// ============================================================================
struct Polygon : IGeometry {
    std::vector<Vector2> verts;

    Polygon() = default;
    explicit Polygon(std::vector<Vector2> vertices) : verts(std::move(vertices)) {}

    GeometryType type() const override { return GeometryType::Polygon; }

    bool Contains(const Vector2& pt) const override {
        int n = static_cast<int>(verts.size());
        if (n < 3) return false;

        auto u = verts[n - 1] - pt;
        auto v = verts[0] - pt;
        float z = Vector2::Cross(u, v);

        for (int i = 0; i < n - 1; ++i) {
            u = verts[i] - pt;
            v = verts[i + 1] - pt;
            float w = Vector2::Cross(u, v);
            if (z * w < 0) return false;
        }
        return true;
    }

    Vector2 GetFarthestProjectionPoint(const Vector2& dir) const override {
        if (verts.empty()) return Vector2::zero();

        Vector2 best = verts[0];
        float maxDot = Vector2::Dot(best, dir);
        for (size_t i = 1; i < verts.size(); ++i) {
            float d = Vector2::Dot(verts[i], dir);
            if (d > maxDot) { maxDot = d; best = verts[i]; }
        }
        return best;
    }
};

// ============================================================================
// 前置工具函数（Capsule 等依赖）
// ============================================================================

// 点到线段的距离平方
inline float PointSegmentDistance2(const Vector2& pa, const Vector2& pb, const Vector2& pt) {
    float px = pt.x - pa.x, py = pt.y - pa.y;
    float xx = pb.x - pa.x, yy = pb.y - pa.y;
    float h = MathX::Clamp((px * xx + py * yy) / (xx * xx + yy * yy), 0.0f, 1.0f);
    float dx = px - xx * h, dy = py - yy * h;
    return dx * dx + dy * dy;
}

// ============================================================================
// Capsule - 胶囊体 (两端半圆 + 直线段)
// ============================================================================
struct Capsule : IGeometry {
    float length = 2.0f;  // 直线段长度
    float radius = 1.0f;  // 半圆半径

    Capsule() = default;
    Capsule(float l, float r) : length(l), radius(r) {}

    GeometryType type() const override { return GeometryType::Capsule; }

    bool Contains(const Vector2& pt) const override {
        auto p1 = Vector2( length * 0.5f, 0);
        auto p2 = Vector2(-length * 0.5f, 0);
        float d2 = PointSegmentDistance2(p1, p2, pt);
        return d2 <= radius * radius;
    }

    Vector2 GetFarthestProjectionPoint(const Vector2& dir) const override {
        auto p = radius * dir.normalized();
        p.x += (dir.x >= 0.0f) ? length * 0.5f : -length * 0.5f;
        return p;
    }
};

// ============================================================================
// Ellipse - 椭圆 (x²/a² + y²/b² = 1)
// ============================================================================
struct Ellipse : IGeometry {
    float width  = 2.0f;
    float height = 2.0f;

    Ellipse() = default;
    Ellipse(float w, float h) : width(w), height(h) {}

    float A() const { return width  * 0.5f; }
    float B() const { return height * 0.5f; }

    GeometryType type() const override { return GeometryType::Ellipse; }

    bool Contains(const Vector2& pt) const override {
        float a2 = MathX::Pow2(A());
        float b2 = MathX::Pow2(B());
        return (MathX::Pow2(pt.x) / a2 + MathX::Pow2(pt.y) / b2) <= 1.0f;
    }

    Vector2 GetFarthestProjectionPoint(const Vector2& dir) const override {
        float x = 0, y = 0;
        float a = A(), b = B();

        if (MathX::Equals(dir.x, 0.0f)) {
            y = (dir.y < 0.0f ? -1 : 1) * b;
        } else if (MathX::Equals(dir.y, 0.0f)) {
            x = (dir.x < 0.0f ? -1 : 1) * a;
        } else {
            float k  = dir.y / dir.x;
            float a2 = MathX::Pow2(a);
            float b2 = MathX::Pow2(b);
            float k2 = MathX::Pow2(k);
            float t  = MathX::Sqrt((a2 + b2 * k2) / k2);

            auto v = Vector2(0.0f, t);
            if (Vector2::Dot(v, dir) < 0) t = -t;

            x = k * t - (b2 * k2 * k * t) / (a2 + b2 * k2);
            y = (b2 * k2 * t) / (a2 + b2 * k2);
        }

        return Vector2(x, y);
    }
};

// ============================================================================
// Pie - 扇形
// ============================================================================
struct Pie : IGeometry {
    float radius = 1.0f;
    float sweep  = 90.0f;  // 张角 (度)

    Pie() = default;
    Pie(float r, float s) : radius(r), sweep(s) {}

    GeometryType type() const override { return GeometryType::Pie; }

    bool Contains(const Vector2& pt) const override {
        if (pt.magnitude2() > radius * radius) return false;
        auto p = pt;
        p.y = MathX::Abs(p.y);
        float a0 = p.Angle() * MathX::RAD2DEG;
        float a1 = MathX::Clamp360(sweep * 0.5f);
        return a0 <= a1;
    }

    Vector2 GetFarthestProjectionPoint(const Vector2& dir) const override {
        // Pie 是朝向 +X 的扇形(锥体+弧)。最远点可能是: 弧上点 / 角点 / 原点
        float half = (sweep * 0.5f) * MathX::DEG2RAD;
        float c = cosf(half), s = sinf(half);
        Vector2 best = {0, 0};  // 原点
        float maxDot = 0;

        // 检查弧上点: 方向在锥角范围内 (dir.x>0 且 |dir.y| <= dir.x * tan(half))
        if (dir.x > 0 && fabsf(dir.y) <= dir.x * tanf(half)) {
            float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
            Vector2 arcPt = {radius * dir.x / len, radius * dir.y / len};
            float d = Vector2::Dot(arcPt, dir);
            if (d > maxDot) { maxDot = d; best = arcPt; }
        }

        // 检查两个角点
        Vector2 corners[2] = {{radius * c, radius * s}, {radius * c, -radius * s}};
        for (int i = 0; i < 2; i++) {
            float d = Vector2::Dot(corners[i], dir);
            if (d > maxDot) { maxDot = d; best = corners[i]; }
        }
        return best;
    }
};

// ============================================================================
// Segment - 线段
// ============================================================================
struct Segment : IGeometry {
    float   length = 2.0f;
    Vector2 normal{0, 1};

    Segment() = default;
    Segment(float l, const Vector2& n) : length(l), normal(n) {}

    GeometryType type() const override { return GeometryType::Segment; }

    bool Contains(const Vector2& pt) const override {
        auto pa = Vector2( length * 0.5f, 0.0f);
        auto pb = Vector2(-length * 0.5f, 0.0f);
        float t = Vector2::Dot(pa - pt, pb - pt);
        return MathX::Equals(t, -1.0f);
    }

    Vector2 GetFarthestProjectionPoint(const Vector2& dir) const override {
        return (dir.x >= 0.0f)
            ? Vector2( length * 0.5f, 0.0f)
            : Vector2(-length * 0.5f, 0.0f);
    }
};

// ============================================================================
// GeometryHelper - 几何工具
// ============================================================================
namespace helper {

// 获取矩形顶点
inline std::vector<Vector2> GetRectanglePoints(float width, float height, float rotation) {
    float w = width * 0.5f;
    float h = height * 0.5f;

    Vector2 pts[4] = {
        Vector2(-w, -h), Vector2(w, -h),
        Vector2( w,  h), Vector2(-w,  h),
    };

    auto mt = Matrix::CreateRotationMatrix(rotation * MathX::DEG2RAD);
    std::vector<Vector2> result;
    result.reserve(4);
    for (auto& p : pts)
        result.push_back(Matrix::Transform(p, mt));
    return result;
}

// 获取胶囊端点
inline std::vector<Vector2> GetCapsulePoints(float length, float rotation) {
    auto mt = Matrix::CreateRotationMatrix(rotation * MathX::DEG2RAD);
    std::vector<Vector2> result(2);
    result[0] = Matrix::Transform(Vector2( length * 0.5f, 0), mt);
    result[1] = Matrix::Transform(Vector2(-length * 0.5f, 0), mt);
    return result;
}

// 圆与圆是否重叠
inline bool IsCircleOverlapsWithCircle(const Circle& c1, const Transform& t1,
                                       const Circle& c2, const Transform& t2) {
    float d2 = Vector2::Distance2(t1.position, t2.position);
    float r2 = (c1.radius + c2.radius) * (c1.radius + c2.radius);
    return d2 <= r2;
}

// 获取图形旋转后在给定方向上的最大投影点
template<typename T>
Vector2 GetFarthestProjectionPoint(const T& geometry, float rotation, const Vector2& dir) {
    auto m1 = Matrix::CreateRotationMatrix(-rotation * MathX::DEG2RAD);
    auto localDir = Matrix::Transform(dir, m1);
    auto pt = geometry.GetFarthestProjectionPoint(localDir);

    auto m2 = Matrix::CreateRotationMatrix(rotation * MathX::DEG2RAD);
    return Matrix::Transform(pt, m2);
}

// 获取图形变换后在给定方向上的最大投影点
template<typename T>
Vector2 GetFarthestProjectionPoint(const T& geometry, const Transform& transform, const Vector2& dir) {
    auto m1 = Matrix::CreateRotationMatrix(-transform.rotation * MathX::DEG2RAD);
    auto localDir = Matrix::Transform(dir, m1);
    auto pt = geometry.GetFarthestProjectionPoint(localDir);

    auto m2 = Matrix::CreateRotationMatrix(transform.rotation * MathX::DEG2RAD);
    auto m3 = Matrix::CreateTranslationMatrix(transform.position);
    return Matrix::Transform(pt, m2 * m3);
}

} // namespace helper

} // namespace gjk2d

#endif // GJK2D_GEOMETRY_HPP
