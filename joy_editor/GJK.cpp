#include "GJK.h"
#include "../joy/jlut.h"

#define FP_NUM_BITS 64
#define FP_FRACTIONAL_PLACES 32
#define FP_ONE (1LL << FP_FRACTIONAL_PLACES)
#define FP_MAX_VALUE 9223372036854775807LL
#define FP_MIN_VALUE -9223372036854775808LL
#define FP_PI 0x3243F6A88
#define FP_PI_OVER_2 0x1921FB544

// ============ FP 实现 ============

static int CountLeadingZeroes(uint64_t x) {
	int result = 0;
	while ((x & 0xF000000000000000) == 0) { result += 4; x <<= 4; }
	while ((x & 0x8000000000000000) == 0) { result += 1; x <<= 1; }
	return result;
}

FP::FP() : rawValue(0) {}
FP::FP(float value) { rawValue = (int64_t)(FP_ONE * value); }
FP::FP(int64_t value) : rawValue(value) {}
FP::operator float() const { return (float)rawValue / FP_ONE; }
FP::operator int64_t() const { return rawValue; }
FP FP::operator-() const { return -rawValue; }

FP& FP::operator+=(const FP& other) { rawValue += other.rawValue; return *this; }
FP& FP::operator-=(const FP& other) { rawValue -= other.rawValue; return *this; }

FP& FP::operator*=(const FP& other) {
	int64_t xl = rawValue, yl = other.rawValue;
	uint64_t xlo = (uint64_t)(xl & 0x00000000FFFFFFFF), ylo = (uint64_t)(yl & 0x00000000FFFFFFFF);
	int64_t xhi = xl >> FP_FRACTIONAL_PLACES, yhi = yl >> FP_FRACTIONAL_PLACES;
	uint64_t lolo = xlo * ylo;
	int64_t lohi = (int64_t)xlo * yhi, hilo = xhi * (int64_t)ylo, hihi = xhi * yhi;
	uint64_t loResult = lolo >> FP_FRACTIONAL_PLACES;
	int64_t midResult1 = lohi, midResult2 = hilo, hiResult = hihi << FP_FRACTIONAL_PLACES;
	rawValue = (int64_t)loResult + midResult1 + midResult2 + hiResult;
	return *this;
}

FP& FP::operator/=(const FP& other) {
	int64_t xl = rawValue, yl = other.rawValue;
	if (yl == 0) { rawValue = 0; return *this; }
	uint64_t remainder = (uint64_t)(xl >= 0 ? xl : -xl);
	uint64_t divider = (uint64_t)(yl >= 0 ? yl : -yl);
	uint64_t quotient = 0;
	int bitPos = FP_NUM_BITS / 2 + 1;
	while ((divider & 0xF) == 0 && bitPos >= 4) { divider >>= 4; bitPos -= 4; }
	while (remainder != 0 && bitPos >= 0) {
		int shift = CountLeadingZeroes(remainder);
		if (shift > bitPos) shift = bitPos;
		remainder <<= shift; bitPos -= shift;
		uint64_t div = remainder / divider;
		remainder = remainder % divider;
		quotient += div << bitPos;
		if ((div & ~(0xFFFFFFFFFFFFFFFF >> bitPos)) != 0) {
			rawValue = ((xl ^ yl) & FP_MIN_VALUE) == 0 ? FP::MaxValue() : FP::MinValue();
			return *this;
		}
		remainder <<= 1; --bitPos;
	}
	++quotient;
	int64_t result = (int64_t)(quotient >> 1);
	if (((xl ^ yl) & FP_MIN_VALUE) != 0) result = -result;
	rawValue = result;
	return *this;
}

FP FP::operator+(const FP& other) const { return rawValue + other.rawValue; }
FP FP::operator-(const FP& other) const { return rawValue - other.rawValue; }

FP FP::operator*(const FP& other) const {
	int64_t xl = rawValue, yl = other.rawValue;
	uint64_t xlo = (uint64_t)(xl & 0x00000000FFFFFFFF), ylo = (uint64_t)(yl & 0x00000000FFFFFFFF);
	int64_t xhi = xl >> FP_FRACTIONAL_PLACES, yhi = yl >> FP_FRACTIONAL_PLACES;
	uint64_t lolo = xlo * ylo;
	int64_t lohi = (int64_t)xlo * yhi, hilo = xhi * (int64_t)ylo, hihi = xhi * yhi;
	uint64_t loResult = lolo >> FP_FRACTIONAL_PLACES;
	return (int64_t)loResult + lohi + hilo + (hihi << FP_FRACTIONAL_PLACES);
}

FP FP::operator/(const FP& other) const {
	int64_t xl = rawValue, yl = other.rawValue;
	if (yl == 0) return FP::Zero();
	uint64_t remainder = (uint64_t)(xl >= 0 ? xl : -xl);
	uint64_t divider = (uint64_t)(yl >= 0 ? yl : -yl);
	uint64_t quotient = 0;
	int bitPos = FP_NUM_BITS / 2 + 1;
	while ((divider & 0xF) == 0 && bitPos >= 4) { divider >>= 4; bitPos -= 4; }
	while (remainder != 0 && bitPos >= 0) {
		int shift = CountLeadingZeroes(remainder);
		if (shift > bitPos) shift = bitPos;
		remainder <<= shift; bitPos -= shift;
		uint64_t div = remainder / divider;
		remainder = remainder % divider;
		quotient += div << bitPos;
		if ((div & ~(0xFFFFFFFFFFFFFFFF >> bitPos)) != 0)
			return ((xl ^ yl) & FP_MIN_VALUE) == 0 ? FP::MaxValue() : FP::MinValue();
		remainder <<= 1; --bitPos;
	}
	++quotient;
	int64_t result = (int64_t)(quotient >> 1);
	if (((xl ^ yl) & FP_MIN_VALUE) != 0) result = -result;
	return result;
}

bool FP::operator==(const FP& other) { return rawValue == other.rawValue; }
bool FP::operator!=(const FP& other) { return rawValue != other.rawValue; }
bool FP::operator>=(const FP& other) { return rawValue >= other.rawValue; }
bool FP::operator<=(const FP& other) { return rawValue <= other.rawValue; }
bool FP::operator>(const FP& other) { return rawValue > other.rawValue; }
bool FP::operator<(const FP& other) { return rawValue < other.rawValue; }

FP FP::MinValue() { FP r; r.rawValue = FP_MIN_VALUE; return r; }
FP FP::MaxValue() { FP r; r.rawValue = FP_MAX_VALUE; return r; }
FP FP::Zero() { return FP(0.0f); }
FP FP::Half() { return FP(0.5f); }
FP FP::One() { return FP(1.0f); }

FP FP::Abs(FP x) {
	if (x == FP::MinValue()) return FP::MaxValue();
	int64_t mask = x >> 63;
	return (x.rawValue + mask) ^ mask;
}
FP FP::Max(FP a, FP b) { return a > b ? a : b; }
FP FP::Min(FP a, FP b) { return a < b ? a : b; }
FP FP::Pow2(FP x) { return x * x; }
int FP::Sign(FP x) { return x < FP::Zero() ? -1 : 1; }
FP FP::Clamp(FP a, FP low, FP high) { return FP::Max(low, FP::Min(a, high)); }

FP FP::Sqrt(FP x) {
	int64_t xl = x; if (xl < 0) return FP::Zero();
	uint64_t num = (uint64_t)xl, result = 0, bit = (uint64_t)1 << (FP_NUM_BITS - 2);
	while (bit > num) bit >>= 2;
	for (int i = 0; i < 2; ++i) {
		while (bit != 0) {
			if (num >= result + bit) { num -= result + bit; result = (result >> 1) + bit; }
			else { result >>= 1; }
			bit >>= 2;
		}
		if (i == 0) {
			if (num > ((uint64_t)1 << (FP_NUM_BITS / 2)) - 1) { num -= result; num = (num << (FP_NUM_BITS / 2)) - 0x80000000; result = (result << (FP_NUM_BITS / 2)) + 0x80000000; }
			else { num <<= (FP_NUM_BITS / 2); result <<= (FP_NUM_BITS / 2); }
			bit = (uint64_t)1 << (FP_NUM_BITS / 2 - 2);
		}
	}
	if (num > result) ++result;
	return (int64_t)result;
}

#define FP_PI_TIMES_2 0x6487ED511LL
#define FP_LUT_SIZE (int)(FP_PI >> 16)

static int64_t ClampSinValue(int64_t angle, bool* flipHorizontal, bool* flipVertical) {
	int64_t largePI = 7244019458077122842LL;
	int64_t clamped2Pi = angle;
	for (int i = 0; i < 29; ++i) clamped2Pi %= (largePI >> i);
	if (angle < 0) clamped2Pi += FP_PI_TIMES_2;
	*flipVertical = clamped2Pi >= FP_PI;
	int64_t clampedPi = clamped2Pi;
	while (clampedPi >= FP_PI) clampedPi -= FP_PI;
	*flipHorizontal = clampedPi >= FP_PI_OVER_2;
	int64_t clampedPiOver2 = clampedPi;
	if (clampedPiOver2 >= FP_PI_OVER_2) clampedPiOver2 -= FP_PI_OVER_2;
	return clampedPiOver2;
}

FP FP::Sin(FP x) {
	bool flipHorizontal = false;
	bool flipVertical = false;
	int64_t clampedL = ClampSinValue(x, &flipHorizontal, &flipVertical);
	uint32_t rawIndex = (uint32_t)(clampedL >> 15);
	rawIndex = rawIndex >= FP_LUT_SIZE ? FP_LUT_SIZE - 1 : rawIndex;
	int64_t nearestValue = SIN_LUT[flipHorizontal ? FP_LUT_SIZE - 1 - (int)rawIndex : (int)rawIndex];
	return flipVertical ? -nearestValue : nearestValue;
}
FP FP::Cos(FP x) {
	int64_t xl = x;
	FP angle = xl + (xl > 0 ? -FP_PI - FP_PI_OVER_2 : FP_PI_OVER_2);
	return FP::Sin(angle);
}

// ============ GJK 核心算法 ============
static FixedVec3 Support(Shape* shapeA, Shape* shapeB, FixedVec3 direction) {
	FixedVec3 aFurthestPoint = shapeA->FindFurthestPoint(direction);
	FixedVec3 bFurthestPoint = shapeB->FindFurthestPoint(-direction);
	return aFurthestPoint - bFurthestPoint;
}

static bool SameDirection(const FixedVec3& direction, const FixedVec3& ao) {
	return FixedVec3Dot(direction, ao) > FP::Zero();
}

static std::pair<std::vector<FixedNormal>, size_t> GetFaceNormals(const std::vector<FixedVec3>& polytope, const std::vector<size_t>& faces) {
	std::vector<FixedNormal> normals;
	size_t minTriangle = 0;
	FP minDistance = FLT_MAX;
	for (size_t i = 0; i < faces.size(); i += 3) {
		FixedVec3 a = polytope[faces[i]], b = polytope[faces[i + 1]], c = polytope[faces[i + 2]];
		FixedVec3 ba = FixedVec3Normalize(b - a), ca = FixedVec3Normalize(c - a);
		FixedVec3 normal = FixedVec3Normalize(FixedVec3Cross(ba, ca));
		FP distance = FixedVec3Dot(normal, a);
		if (distance < FP::Zero()) { normal = -normal; distance = -distance; }
		normals.push_back({ normal.x, normal.y, normal.z, distance });
		if (distance < minDistance) { minTriangle = i / 3; minDistance = distance; }
	}
	return { normals, minTriangle };
}

static void AddIfUniqueEdge(std::vector<std::pair<size_t, size_t>>& edges, const std::vector<size_t>& faces, size_t a, size_t b) {
	auto reverse = std::find(edges.begin(), edges.end(), std::make_pair(faces[b], faces[a]));
	if (reverse != edges.end()) edges.erase(reverse);
	else edges.emplace_back(faces[a], faces[b]);
}

static FixedContact EPA(const FixedSimplex& simplex, Shape* shapeA, Shape* shapeB) {
	std::vector<FixedVec3> polytope(simplex.begin(), simplex.end());
	std::vector<size_t> faces = { 0, 1, 2, 0, 3, 1, 0, 2, 3, 1, 3, 2 };
	auto faceNormals = GetFaceNormals(polytope, faces);
	std::vector<FixedNormal> normals = faceNormals.first;
	size_t minFace = faceNormals.second;
	FixedVec3 bakeMinNormal = { normals[minFace].x, normals[minFace].y, normals[minFace].z };
	FP bakeMinDistance = normals[minFace].w;
	FixedVec3 minNormal; FP minDistance = FP::MaxValue();
	while (minDistance == FP::MaxValue()) {
		minNormal = { normals[minFace].x, normals[minFace].y, normals[minFace].z };
		minDistance = normals[minFace].w;
		FixedVec3 support = Support(shapeA, shapeB, minNormal);
		FP sDistance = FixedVec3Dot(minNormal, support);
		if (FP::Abs(sDistance - minDistance) > FP(0.001f)) {
			minDistance = FP::MaxValue();
			std::vector<std::pair<size_t, size_t>> uniqueEdges;
			for (size_t i = 0; i < normals.size(); i++) {
				FixedVec3 direction = { normals[i].x, normals[i].y, normals[i].z };
				size_t f = i * 3;
				if (FixedVec3Dot(direction, support) > FixedVec3Dot(direction, polytope[faces[f]])) {
					AddIfUniqueEdge(uniqueEdges, faces, f, f + 1);
					AddIfUniqueEdge(uniqueEdges, faces, f + 1, f + 2);
					AddIfUniqueEdge(uniqueEdges, faces, f + 2, f);
					faces[f + 2] = faces.back(); faces.pop_back();
					faces[f + 1] = faces.back(); faces.pop_back();
					faces[f] = faces.back(); faces.pop_back();
					normals[i] = normals.back(); normals.pop_back();
					i--;
				}
			}
			std::vector<size_t> newFaces;
			for (size_t i = 0; i < uniqueEdges.size(); i++) {
				newFaces.push_back(uniqueEdges[i].first);
				newFaces.push_back(uniqueEdges[i].second);
				newFaces.push_back(polytope.size());
			}
			polytope.push_back(support);
			auto newFaceNormals = GetFaceNormals(polytope, newFaces);
			std::vector<FixedNormal> newNormals = newFaceNormals.first;
			size_t newMinFace = newFaceNormals.second;
			FP oldMinDistance = FP::MaxValue();
			for (size_t i = 0; i < normals.size(); i++) if (normals[i].w < oldMinDistance) { oldMinDistance = normals[i].w; minFace = i; }
			if (newNormals.size() <= 0) { minNormal = bakeMinNormal; minDistance = bakeMinDistance; break; }
			if (newNormals[newMinFace].w < oldMinDistance) minFace = newMinFace + normals.size();
			faces.insert(faces.end(), newFaces.begin(), newFaces.end());
			normals.insert(normals.end(), newNormals.begin(), newNormals.end());
		}
	}
	FixedContact contact; contact.Normal = minNormal; contact.Depth = minDistance + FP(0.001f);
	return contact;
}

static bool Line(FixedSimplex& points, FixedVec3& direction) {
	FixedVec3 a = points[0], b = points[1];
	FixedVec3 ab = FixedVec3Normalize(b - a), ao = FixedVec3Normalize(-a);
	if (SameDirection(ab, ao)) direction = FixedVec3Normalize(FixedVec3Cross(FixedVec3Cross(ab, ao), ab));
	else { points = { a }; direction = ao; }
	return false;
}

static bool Triangle(FixedSimplex& points, FixedVec3& direction) {
	FixedVec3 a = points[0], b = points[1], c = points[2];
	FixedVec3 ab = FixedVec3Normalize(b - a), ac = FixedVec3Normalize(c - a), ao = FixedVec3Normalize(-a);
	FixedVec3 abc = FixedVec3Normalize(FixedVec3Cross(ab, ac));
	if (SameDirection(FixedVec3Cross(abc, ac), ao)) {
		if (SameDirection(ac, ao)) { points = { a, c }; direction = FixedVec3Normalize(FixedVec3Cross(FixedVec3Cross(ac, ao), ac)); }
		else return Line(points = { a, b }, direction);
	} else {
		if (SameDirection(FixedVec3Cross(ab, abc), ao)) return Line(points = { a, b }, direction);
		else { if (SameDirection(abc, ao)) direction = abc; else { points = { a, c, b }; direction = -abc; } }
	}
	return false;
}

static bool Tetrahedron(FixedSimplex& points, FixedVec3& direction) {
	FixedVec3 a = points[0], b = points[1], c = points[2], d = points[3];
	FixedVec3 ab = FixedVec3Normalize(b - a), ac = FixedVec3Normalize(c - a), ad = FixedVec3Normalize(d - a), ao = FixedVec3Normalize(-a);
	FixedVec3 abc = FixedVec3Normalize(FixedVec3Cross(ab, ac)), acd = FixedVec3Normalize(FixedVec3Cross(ac, ad)), adb = FixedVec3Normalize(FixedVec3Cross(ad, ab));
	if (SameDirection(abc, ao)) return Triangle(points = { a, b, c }, direction);
	if (SameDirection(acd, ao)) return Triangle(points = { a, c, d }, direction);
	if (SameDirection(adb, ao)) return Triangle(points = { a, d, b }, direction);
	return true;
}

static bool NextSimplex(FixedSimplex& points, FixedVec3& direction) {
	switch (points.size()) {
	case 2: return Line(points, direction);
	case 3: return Triangle(points, direction);
	case 4: return Tetrahedron(points, direction);
	}
	return false;
}

bool Gjk(Shape* shapeA, Shape* shapeB, FixedVec3 initalDir, FixedContact* contect) {
	FixedVec3 support = Support(shapeA, shapeB, initalDir);
	FixedSimplex points;
	points.push_front(support);
	FixedVec3 direction = FixedVec3Normalize(-support);
	while (true) {
		support = Support(shapeA, shapeB, direction);
		if (FixedVec3Dot(support, direction) < FP::Zero()) return false;
		points.push_front(support);
		if (NextSimplex(points, direction)) { *contect = EPA(points, shapeA, shapeB); return true; }
	}
}

bool Gjk(Shape* shapeA, Shape* shapeB, FixedContact* contect) {
	FixedVec3 initalDir = { FP::One(), FP::Zero(), FP::Zero() };
	return Gjk(shapeA, shapeB, initalDir, contect);
}
