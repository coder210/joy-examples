#ifndef GJK_H
#define GJK_H
#include <cstdint>
#include <cfloat>
#include <array>
#include <vector>

// ============ 定点数 FP 类型 ============
struct FP {
	FP();
	FP(float value);
	FP(int64_t value);
	operator float() const;
	operator int64_t() const;
	FP operator-() const;
	FP& operator+=(const FP& other);
	FP& operator-=(const FP& other);
	FP& operator*=(const FP& other);
	FP& operator/=(const FP& other);
	FP operator+(const FP& other) const;
	FP operator-(const FP& other) const;
	FP operator*(const FP& other) const;
	FP operator/(const FP& other) const;
	bool operator==(const FP& other);
	bool operator!=(const FP& other);
	bool operator>=(const FP& other);
	bool operator<=(const FP& other);
	bool operator>(const FP& other);
	bool operator<(const FP& other);

	static FP MinValue();
	static FP MaxValue();
	static FP Zero();
	static FP Half();
	static FP One();
	static FP Max(FP a, FP b);
	static FP Min(FP a, FP b);
	static FP Pow2(FP x);
	static int Sign(FP x);
	static FP Clamp(FP a, FP low, FP high);
	static FP Sqrt(FP x);
	static FP Abs(FP x);
	static FP Cos(FP x);
	static FP Sin(FP x);

private:
	int64_t rawValue;
};

// ============ 定点数 3D/4D 向量和矩阵 ============
struct FixedVec3 {
    FP x, y, z;
    FixedVec3() : x(FP::Zero()), y(FP::Zero()), z(FP::Zero()) {}
    FixedVec3(const FP& x_, const FP& y_, const FP& z_) : x(x_), y(y_), z(z_) {}
};

struct FixedVec4 {
	FP x, y, z, w;
	FixedVec4() : x(FP::Zero()), y(FP::Zero()), z(FP::Zero()), w(FP::Zero()) {}
	FixedVec4(const FP& x_, const FP& y_, const FP& z_, const FP& w_) : x(x_), y(y_), z(z_), w(w_) {}
	FixedVec4(FixedVec3 v) : x(v.x), y(v.y), z(v.z), w(FP::One()) {}
};

struct FixedMat4 {
	FP m0, m4, m8, m12;
	FP m1, m5, m9, m13;
	FP m2, m6, m10, m14;
	FP m3, m7, m11, m15;
};

static inline FP FixedVec3Dot(const FixedVec3& a, const FixedVec3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
static inline FixedVec3 FixedVec3Cross(const FixedVec3& a, const FixedVec3& b) {
	return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
static inline FP FixedVec3SquareLength(const FixedVec3& a) { return a.x * a.x + a.y * a.y + a.z * a.z; }
static inline FP FixedVec3Length(const FixedVec3& a) { return FP::Sqrt(FixedVec3SquareLength(a)); }
static inline FixedVec3 FixedVec3Normalize(const FixedVec3& a) {
	FixedVec3 r = { FP::Zero(), FP::Zero(), FP::Zero() };
	FP length = FixedVec3Length(a);
	if (length != FP::Zero()) {
		FP invLength = FP::One() / length;
		r.x = a.x * invLength; r.y = a.y * invLength; r.z = a.z * invLength;
	}
	return r;
}
static inline FixedVec3 operator-(FixedVec3 a) { return { -a.x, -a.y, -a.z }; }
static inline FixedVec3 operator-(const FixedVec3& a, const FixedVec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline FixedVec3 operator+(const FixedVec3& a, const FixedVec3& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline FixedVec3 operator*(const FixedVec3& a, FP s) { return { a.x * s, a.y * s, a.z * s }; }

static inline FixedMat4 FixedMat4Multiply(FixedMat4 left, FixedMat4 right) {
	// Column-major multiply: result = left * right (FixedMat4 is column-major, m0,m4,m8,m12 form column 1)
	FixedMat4 r = { FP::Zero() };
	r.m0  = left.m0 * right.m0 + left.m4 * right.m1 + left.m8 * right.m2 + left.m12 * right.m3;
	r.m1  = left.m1 * right.m0 + left.m5 * right.m1 + left.m9 * right.m2 + left.m13 * right.m3;
	r.m2  = left.m2 * right.m0 + left.m6 * right.m1 + left.m10 * right.m2 + left.m14 * right.m3;
	r.m3  = left.m3 * right.m0 + left.m7 * right.m1 + left.m11 * right.m2 + left.m15 * right.m3;
	r.m4  = left.m0 * right.m4 + left.m4 * right.m5 + left.m8 * right.m6 + left.m12 * right.m7;
	r.m5  = left.m1 * right.m4 + left.m5 * right.m5 + left.m9 * right.m6 + left.m13 * right.m7;
	r.m6  = left.m2 * right.m4 + left.m6 * right.m5 + left.m10 * right.m6 + left.m14 * right.m7;
	r.m7  = left.m3 * right.m4 + left.m7 * right.m5 + left.m11 * right.m6 + left.m15 * right.m7;
	r.m8  = left.m0 * right.m8 + left.m4 * right.m9 + left.m8 * right.m10 + left.m12 * right.m11;
	r.m9  = left.m1 * right.m8 + left.m5 * right.m9 + left.m9 * right.m10 + left.m13 * right.m11;
	r.m10 = left.m2 * right.m8 + left.m6 * right.m9 + left.m10 * right.m10 + left.m14 * right.m11;
	r.m11 = left.m3 * right.m8 + left.m7 * right.m9 + left.m11 * right.m10 + left.m15 * right.m11;
	r.m12 = left.m0 * right.m12 + left.m4 * right.m13 + left.m8 * right.m14 + left.m12 * right.m15;
	r.m13 = left.m1 * right.m12 + left.m5 * right.m13 + left.m9 * right.m14 + left.m13 * right.m15;
	r.m14 = left.m2 * right.m12 + left.m6 * right.m13 + left.m10 * right.m14 + left.m14 * right.m15;
	r.m15 = left.m3 * right.m12 + left.m7 * right.m13 + left.m11 * right.m14 + left.m15 * right.m15;
	return r;
}
static inline FixedVec4 FixedMat4Multiply(FixedMat4 left, FixedVec4 right) {
	FixedVec4 r;
	r.x = left.m0 * right.x + left.m4 * right.y + left.m8 * right.z + left.m12 * right.w;
	r.y = left.m1 * right.x + left.m5 * right.y + left.m9 * right.z + left.m13 * right.w;
	r.z = left.m2 * right.x + left.m6 * right.y + left.m10 * right.z + left.m14 * right.w;
	r.w = left.m3 * right.x + left.m7 * right.y + left.m11 * right.z + left.m15 * right.w;
	return r;
}
static inline FixedMat4 FixedMat4Translate(FP x, FP y, FP z) {
	return { FP::One(), FP::Zero(), FP::Zero(), x, FP::Zero(), FP::One(), FP::Zero(), y, FP::Zero(), FP::Zero(), FP::One(), z, FP::Zero(), FP::Zero(), FP::Zero(), FP::One() };
}
static inline FixedMat4 FixedMat4Rotate(FixedVec3 axis, FP angle) {
	FixedMat4 r = { FP::Zero() };
	FP x = axis.x, y = axis.y, z = axis.z;
	FP lengthSquared = x * x + y * y + z * z;
	if (lengthSquared != FP::One() && lengthSquared != FP::Zero()) {
		FP ilength = FP::One() / FP::Sqrt(lengthSquared);
		x *= ilength; y *= ilength; z *= ilength;
	}
	FP sinres = FP::Sin(angle), cosres = FP::Cos(angle), t = FP::One() - cosres;
	r.m0 = x * x * t + cosres;           r.m1 = y * x * t + z * sinres;         r.m2 = z * x * t - y * sinres;
	r.m4 = x * y * t - z * sinres;       r.m5 = y * y * t + cosres;             r.m6 = z * y * t + x * sinres;
	r.m8 = x * z * t + y * sinres;       r.m9 = y * z * t - x * sinres;         r.m10 = z * z * t + cosres;
	r.m15 = FP::One();
	return r;
}
static inline FixedMat4 FixedMat4Scale(FP x, FP y, FP z) {
	return { x, FP::Zero(), FP::Zero(), FP::Zero(), FP::Zero(), y, FP::Zero(), FP::Zero(), FP::Zero(), FP::Zero(), z, FP::Zero(), FP::Zero(), FP::Zero(), FP::Zero(), FP::One() };
}

// ============ GJK 碰撞检测类型 ============
struct FixedNormal { FP x, y, z, w; };
struct FixedContact { FixedVec3 Normal; FP Depth; };

struct FixedSimplex {
private:
	std::array<FixedVec3, 4> m_points;
	int m_size;
public:
	FixedSimplex() : m_size(0) {}
	FixedSimplex& operator=(std::initializer_list<FixedVec3> list) {
		m_size = 0;
		for (FixedVec3 p : list) m_points[m_size++] = p;
		return *this;
	}
	void push_front(FixedVec3 p) {
		m_points = { p, m_points[0], m_points[1], m_points[2] };
		m_size = std::min(m_size + 1, 4);
	}
	FixedVec3& operator[](int i) { return m_points[i]; }
	size_t size() const { return m_size; }
	auto begin() const { return m_points.begin(); }
	auto end() const { return m_points.end() - (4 - m_size); }
};

enum ShapeType { SHPERE, BOX };

struct Shape {
protected:
	ShapeType m_shapeType;
public:
	virtual ShapeType GetShapeType() = 0;
	virtual FixedVec3 FindFurthestPoint(FixedVec3 direction) = 0;
	virtual void UpdateVertices(FixedVec3 axis, FP angle, const FixedVec3& position) = 0;
};

struct SphereShape : Shape {
private:
	FP m_radius;
	FixedVec3 m_position;
public:
	SphereShape(FP radius) { m_shapeType = SHPERE; m_radius = radius; m_position = { FP::Zero(), FP::Zero(), FP::Zero() }; }
	ShapeType GetShapeType() override { return m_shapeType; }
	FP GetRadius() { return m_radius; }
	FixedVec3 FindFurthestPoint(FixedVec3 direction) override {
		direction = FixedVec3Normalize(direction);
		return m_position + (direction * m_radius);
	}
	void UpdateVertices(FixedVec3 axis, FP angle, const FixedVec3& position) override { m_position = position; }
};

struct MeshShape : Shape {
protected:
	std::vector<FixedVec3> m_vertices;
	std::vector<FixedVec3> m_localVertices;
public:
	FixedVec3 FindFurthestPoint(FixedVec3 direction) override {
		FixedVec3 maxPoint = { FP::Zero(), FP::Zero(), FP::Zero() };
		FP maxDistance = -FLT_MAX;
		for (size_t i = 0; i < m_vertices.size(); i++) {
			FP distance = FixedVec3Dot(m_vertices[i], direction);
			if (distance > maxDistance) { maxDistance = distance; maxPoint = m_vertices[i]; }
		}
		return maxPoint;
	}
	void UpdateVertices(FixedVec3 axis, FP angle, const FixedVec3& position) override {
		FixedMat4 t = FixedMat4Translate(position.x, position.y, position.z);
		FixedMat4 r = FixedMat4Rotate(axis, angle);
		// Rotate around origin first, then translate to position: worldVerts = T * R * v
		FixedMat4 c = FixedMat4Multiply(t, r);
		for (size_t i = 0; i < m_localVertices.size(); i++) {
			FixedVec4 result = FixedMat4Multiply(c, m_localVertices[i]);
			m_vertices[i] = { result.x, result.y, result.z };
		}
	}
	std::vector<FixedVec3> GetVertices() { return m_vertices; }
};

struct BoxShape : MeshShape {
private:
	FixedVec3 m_HalfSize;
public:
	ShapeType GetShapeType() override { return m_shapeType; }
	FixedVec3 GetHalfSize() { return m_HalfSize; }
	BoxShape(FixedVec3 halfSize) {
		m_shapeType = BOX; m_HalfSize = halfSize;
		m_localVertices = {
			{ -halfSize.x, halfSize.y, -halfSize.z }, { halfSize.x, halfSize.y, -halfSize.z },
			{ halfSize.x, halfSize.y, halfSize.z },   { -halfSize.x, halfSize.y, halfSize.z },
			{ -halfSize.x, -halfSize.y, -halfSize.z },{ halfSize.x, -halfSize.y, -halfSize.z },
			{ halfSize.x, -halfSize.y, halfSize.z },  { -halfSize.x, -halfSize.y, halfSize.z }
		};
		m_vertices = m_localVertices;
	}
};

// ============ GJK 声明 ============
bool Gjk(Shape* shapeA, Shape* shapeB, FixedVec3 initalDir, FixedContact* contect);
bool Gjk(Shape* shapeA, Shape* shapeB, FixedContact* contect);

#endif // !GJK_H
