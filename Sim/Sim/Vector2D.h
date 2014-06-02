#pragma once

#include <cmath>

const float EPSILON = 0.00001f;

class Vector2D
{
	float x;
	float y;

public:
	Vector2D() : x(0.0f), y(0.0f) {}
	Vector2D(float xCoord, float yCoord) : x(xCoord), y(yCoord) { }

	float getX() const { return x; }
	float getY() const { return y; }

	Vector2D operator-() const { return Vector2D(-x, -y); }
	float operator*(const Vector2D &vector) const { return x * vector.x + y * vector.y; }
	Vector2D operator*(float num) const { return Vector2D(x * num, y * num); }
	Vector2D operator/(float s) const { float invS = 1.0f / s; return Vector2D(x * invS, y * invS); }
	Vector2D operator+(const Vector2D &vector) const { return Vector2D(x + vector.x, y + vector.y); }
	Vector2D operator-(const Vector2D &vector) const { return Vector2D(x - vector.x, y - vector.y); }
	bool operator==(const Vector2D &vector) const { return x == vector.x && y == vector.y; }
	bool operator!=(const Vector2D &vector) const { return x != vector.x || y != vector.y; }
	Vector2D &operator*=(float num) { x *= num; y *= num; return *this; }
	Vector2D &operator/=(float num) { float invNum = 1.0f / num; x *= invNum; y *= invNum; return *this; }
	Vector2D &operator+=(const Vector2D &vector) { x += vector.x; y += vector.y; return *this; }
	Vector2D &operator-=(const Vector2D &vector) { x -= vector.x; y -= vector.y; return *this; }

	~Vector2D() {}
};

/* =========================================================================================== */

class Line
{
	Vector2D point;
	Vector2D dir;
public:
	Line() {}
	Line(const Vector2D& pointVec, const Vector2D& dirVec): point(pointVec), dir(dirVec) {}
	
	Vector2D getPoint() const { return point; }
	Vector2D getDir() const { return dir; }

	void setPoint(const Vector2D& pointVec) { point = pointVec; }
	void setDir(const Vector2D& dirVec) { dir = dirVec; }
};

/* =========================================================================================== */


inline Vector2D operator*(float num, const Vector2D &vector)
{
	return Vector2D(num * vector.getX(), num * vector.getY());
}

inline float abs(const Vector2D &vector)
{
	return std::sqrt(vector * vector);
}

inline float absSq(const Vector2D &vector)
{
	return vector * vector;
}

inline float prVectMul(const Vector2D &vector1, const Vector2D &vector2)
{
	return vector1.getX() * vector2.getY() - vector1.getY() * vector2.getX();
}

inline Vector2D normalize(const Vector2D &vector) {
	return vector / abs(vector);
}

inline float distSqPointLineSegment(const Vector2D &a, const Vector2D &b, const Vector2D &c)
{
	const float r = ((c - a) * (b - a)) / absSq(b - a);

	if (r < 0.0f) {
		return absSq(c - a);
	}
	else if (r > 1.0f) {
		return absSq(c - b);
	}
	else {
		return absSq(c - (a + r * (b - a)));
	}
}

/* =========================================================================================== */

inline bool isLeftOf(const Vector2D &a, const Vector2D &b, const Vector2D &c)
{	
	return (prVectMul(a - c, b - a) >= -EPSILON) ? true : false;
}

inline bool isLeftOf(const Vector2D &a, const Vector2D &b, const Vector2D &c, float &signDistToLine)
{	
	return ((signDistToLine = prVectMul(a - c, b - a)) >= -EPSILON) ? true : false;
}

inline bool isLeftOf(const Vector2D &a, const Vector2D &b)
{
	return (prVectMul(a, b) >= -EPSILON) ? true : false;
}

inline float sqr(float a)
{
	return a * a;
}

inline bool isParallelLines2(const Vector2D& dirLine1, const Vector2D& dirLine2)
{
	float prVectMulValue = prVectMul(dirLine1, dirLine2);
	return (std::fabs(prVectMulValue) <= EPSILON);
}

inline bool getPointOfLineIntersection2(const Line& line1, const Line& line2, Vector2D& point)
{
	float denom = prVectMul(line1.getDir(), -line2.getDir());
	if (std::fabs(denom) <= EPSILON) 
		return false;

	point = line1.getPoint() + prVectMul(line2.getDir(), line1.getPoint() - line2.getPoint()) / 
		denom * line1.getDir();
	return true;
}

inline Vector2D getNormal(const Vector2D &v)
{
	return normalize(Vector2D(-v.getY(), v.getX()));
}