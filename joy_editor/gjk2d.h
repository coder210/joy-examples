#ifndef GJK2D_COLLIDER_HPP
#define GJK2D_COLLIDER_HPP

#include "gjk2d_geometry.h"

namespace gjk2d {

// ============================================================================
// ICollider - 碰撞器接口
// ============================================================================
struct ICollider {
    virtual ~ICollider() = default;
    virtual GeometryType geometryType() const = 0;

    // 获取在指定方向(dir)上投影最远的点（给定旋转角度）
    virtual Vector2 GetFarthestProjectionPoint(float rotation, const Vector2& dir) const = 0;

    // 获取在指定方向(dir)上投影最远的点（给定完整变换）
    virtual Vector2 GetFarthestProjectionPoint(const Transform& transform, const Vector2& dir) const = 0;
};

// ============================================================================
// BaseCollider<T> - 碰撞器基类模板
// ============================================================================
template<typename T>
struct BaseCollider : ICollider {
    static_assert(std::is_base_of<IGeometry, T>::value, "T must derive from IGeometry");

    T geometry;

    static const Vector2 CARDINAL_DIRS[4];

    explicit BaseCollider(const T& geom) : geometry(geom) {}

    GeometryType geometryType() const override { return geometry.type(); }

    // 刷新几何信息（这里简化为计算包围盒，实际可扩展）
    void RefreshGeometry(float rotation) {
        // 此处简化，不实现 AABB 计算。实际可参考 C# 版本
    }

    Vector2 GetFarthestProjectionPoint(float rotation, const Vector2& dir) const override {
        return helper::GetFarthestProjectionPoint(geometry, rotation, dir);
    }

    Vector2 GetFarthestProjectionPoint(const Transform& transform, const Vector2& dir) const override {
        return helper::GetFarthestProjectionPoint(geometry, transform, dir);
    }
};

template<typename T>
const Vector2 BaseCollider<T>::CARDINAL_DIRS[4] = {
    Vector2::up(), Vector2::right(), Vector2::down(), Vector2::left()
};

// ============================================================================
// Minkowski - 闵可夫斯基差
// ============================================================================
struct Minkowski {
    const ICollider* collider1;
    Transform        transform1;
    const ICollider* collider2;
    Transform        transform2;

    Minkowski(const ICollider* c1, const Transform& t1,
              const ICollider* c2, const Transform& t2)
        : collider1(c1), transform1(t1)
        , collider2(c2), transform2(t2) {}

    // Support 函数：返回闵可夫斯基差上沿 dir 方向的最远点
    Vector2 Support(const Vector2& dir) const {
        auto p1 = collider1->GetFarthestProjectionPoint(transform1, dir);
        auto ndir = -dir;
        auto p2 = collider2->GetFarthestProjectionPoint(transform2, ndir);
        return p1 - p2;
    }
};

// ============================================================================
// Simplex - 单纯形
// ============================================================================
struct Simplex {
    std::vector<Vector2> verts;

    explicit Simplex(int capacity = 3) {
        verts.reserve(capacity);
    }

    int count() const { return static_cast<int>(verts.size()); }

    Vector2& a() { return verts[count() - 1]; }              // 最近加入
    Vector2& b() { return verts[count() - 2]; }              // 次近
    Vector2& c() { return verts[count() - 3]; }              // 第三近

    const Vector2& a() const { return verts[count() - 1]; }
    const Vector2& b() const { return verts[count() - 2]; }
    const Vector2& c() const { return verts[count() - 3]; }

    const Vector2& operator[](int i) const { return verts[i]; }
    Vector2&       operator[](int i)       { return verts[i]; }

    void Add(const Vector2& pt) { verts.push_back(pt); }

    // 检测原点的位置关系
    // 返回 true 表示原点被包含（碰撞）；false 则更新 dir 为下一次搜索方向
    bool Check(Vector2& dir) {
        const auto ao = -a();  // 从 a 指向原点（拷贝，非引用）
        const auto ab = b() - a();

        if (count() == 2) {
            // 线段情况：用三重积求垂直于 ab 且指向原点的方向
            dir = Vector2::Mul3(ab, ao, ab);
            if (dir.magnitude2() < MathX::EPSILON) {
                dir = ab.perpendicular();
            }
        } else if (count() == 3) {
            const auto ac = c() - a();
            // v 是垂直于 ac 且指向 ab 侧的向量
            auto v = Vector2::Mul3(ab, ac, ac);

            if (Vector2::Dot(v, ao) >= 0.0f) {
                // 原点在 ac 外侧，沿 v 方向搜索
                dir = v;
            } else {
                // u 是垂直于 ab 且指向 ac 侧的向量
                auto u = Vector2::Mul3(ac, ab, ab);
                if (Vector2::Dot(u, ao) < 0.0f) {
                    // 原点在三角形内部 → 碰撞！
                    return true;
                }
                // 原点在 ab 外侧，去掉 a，保留 c→b，沿 u 方向搜索
                c() = b();
                dir = u;
            }
            // 去掉最旧的顶点 a
            b() = a();
            verts.pop_back();
        }

        return false;
    }
};

// ============================================================================
// ExpandingSimplex - 扩展多面体（EPA 用）
// ============================================================================
enum class WindingType : int8_t {
    Unknown          = 0,
    Clockwise        = -1,
    CounterClockwise = 1,
};

struct ExpandingSimplex {
    struct Edge {
        Vector2     p1, p2;
        Vector2     normal;
        WindingType winding = WindingType::Unknown;
        float       distance = 0.0f;

        Edge(const Vector2& p1_, const Vector2& p2_, WindingType w)
            : p1(p1_), p2(p2_), winding(w)
        {
            // 边方向向量
            normal = Vector2(p2.x - p1.x, p2.y - p1.y);
            if (winding == WindingType::Clockwise) {
                normal.CW90();   // 顺时针缠绕时，向外法线为 CW90
            } else {
                normal.CCW90();  // 逆时针缠绕时，向外法线为 CCW90
            }
            normal.Normalize();

            // 原点到该边的距离
            distance = MathX::Abs(p1.x * normal.x + p1.y * normal.y);
        }
    };

    WindingType winding = WindingType::Unknown;
    std::vector<Edge> edges;

    ExpandingSimplex(const Simplex& simplex) {
        DetermineWinding(simplex);
        BuildEdges(simplex);
    }

    // 获取离原点最近的边
    const Edge& GetClosestEdge() const {
        return edges[0];
    }

    // 向多面体中插入新点，扩展边
    void Expand(const Vector2& pt) {
        auto edge = edges[0];
        edges.erase(edges.begin());

        edges.emplace_back(edge.p1, pt, winding);
        edges.emplace_back(pt, edge.p2, winding);

        // 按距离排序
        std::sort(edges.begin(), edges.end(),
            [](const Edge& a, const Edge& b) {
                return a.distance < b.distance;
            });
    }

private:
    void DetermineWinding(const Simplex& simplex) {
        int n = simplex.count();
        for (int i = 0; i < n; ++i) {
            const auto& p1 = simplex[i];
            const auto& p2 = simplex[(i + 1) % n];
            float c = Vector2::Cross(p1, p2);
            winding = (c > 0) ? WindingType::Clockwise
                              : WindingType::CounterClockwise;
        }
    }

    void BuildEdges(const Simplex& simplex) {
        int n = simplex.count();
        for (int i = 0; i < n; ++i) {
            const auto& p1 = simplex[i];
            const auto& p2 = simplex[(i + 1) % n];
            edges.emplace_back(p1, p2, winding);
        }

        std::sort(edges.begin(), edges.end(),
            [](const Edge& a, const Edge& b) {
                return a.distance < b.distance;
            });
    }
};

// ============================================================================
// EPA - 扩展多面体算法
// ============================================================================
struct EPA {
    static constexpr int MAX_ITERATIONS = 100;

    void CheckPenetration(const Simplex& simplex, const Minkowski& minkowski,
                          Penetration& penetration)
    {
        ExpandingSimplex expandingSimplex(simplex);
        ExpandingSimplex::Edge edge{Vector2::zero(), Vector2::zero(), WindingType::Unknown};
        Vector2 point = Vector2::zero();

        for (int i = 0; i < MAX_ITERATIONS; ++i) {
            edge = expandingSimplex.GetClosestEdge();
            point = minkowski.Support(edge.normal);

            float projection = Vector2::Dot(point, edge.normal);
            if (projection - edge.distance < MathX::EPSILON) {
                // 收敛：新点不再扩展多面体
                penetration.normal = edge.normal;
                penetration.depth  = projection;
                return;
            }

            expandingSimplex.Expand(point);
        }

        // 达到最大迭代次数，用当前结果
        penetration.normal = edge.normal;
        penetration.depth  = Vector2::Dot(point, edge.normal);
    }
};

// ============================================================================
// GJK - 主算法
// ============================================================================
struct GJK {
    static constexpr int MAX_DETECT_ITERATIONS = 100;

    // 检测碰撞（无穿透信息）
    static bool Detect(const ICollider* c1, const Transform& t1,
                       const ICollider* c2, const Transform& t2)
    {
        // 圆-圆短路优化
        if (c1->geometryType() == GeometryType::Circle &&
            c2->geometryType() == GeometryType::Circle)
        {
            auto& circle1 = static_cast<const BaseCollider<Circle>*>(c1)->geometry;
            auto& circle2 = static_cast<const BaseCollider<Circle>*>(c2)->geometry;
            return helper::IsCircleOverlapsWithCircle(circle1, t1, circle2, t2);
        }

        Simplex simplex(3);
        Minkowski minkowski(c1, t1, c2, t2);
        auto dir = t1.position - t2.position;

        return Detect(simplex, minkowski, dir);
    }

    // 检测碰撞（带穿透信息）
    static bool Detect(const ICollider* c1, const Transform& t1,
                       const ICollider* c2, const Transform& t2,
                       Penetration& penetration)
    {
        Simplex simplex(3);
        Minkowski minkowski(c1, t1, c2, t2);
        auto dir = t1.position - t2.position;

        if (Detect(simplex, minkowski, dir)) {
            EPA epa;
            epa.CheckPenetration(simplex, minkowski, penetration);
            return true;
        }

        return false;
    }

private:
    static bool Detect(Simplex& simplex, const Minkowski& minkowski, Vector2 dir) {
        if (dir.magnitude2() < MathX::EPSILON) {
            dir = Vector2::right();
        }

        // 第一个 Support 点
        auto pt = minkowski.Support(dir);
        simplex.Add(pt);

        if (Vector2::Dot(pt, dir) <= 0.0f) {
            return false;  // 原点不在闵可夫斯基差中
        }

        dir.Negative();

        for (int i = 0; i < MAX_DETECT_ITERATIONS; ++i) {
            pt = minkowski.Support(dir);
            simplex.Add(pt);

            if (Vector2::Dot(pt, dir) <= 0.0f) {
                return false;
            }

            if (simplex.Check(dir)) {
                return true;  // 碰撞！
            }
        }

        return false;
    }
};

// ============================================================================
// ColliderFactory - 碰撞器工厂
// ============================================================================
struct ColliderFactory {
    static BaseCollider<Circle>*    CreateCollider(const Circle& circle)    { return new BaseCollider<Circle>(circle); }
    static BaseCollider<Rectangle>* CreateCollider(const Rectangle& rect)   { return new BaseCollider<Rectangle>(rect); }
    static BaseCollider<Polygon>*   CreateCollider(const Polygon& poly)     { return new BaseCollider<Polygon>(poly); }
    static BaseCollider<Capsule>*   CreateCollider(const Capsule& cap)      { return new BaseCollider<Capsule>(cap); }
    static BaseCollider<Ellipse>*   CreateCollider(const Ellipse& ell)      { return new BaseCollider<Ellipse>(ell); }
    static BaseCollider<Pie>*       CreateCollider(const Pie& pie)          { return new BaseCollider<Pie>(pie); }
    static BaseCollider<Segment>*   CreateCollider(const Segment& seg)      { return new BaseCollider<Segment>(seg); }
};

} // namespace gjk2d

#endif // GJK2D_COLLIDER_HPP
