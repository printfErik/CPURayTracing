#pragma once
class rtVector3
{
public:
	rtVector3() : m_x(0), m_y(0), m_z(0){}
	rtVector3(float _x, float _y, float _z):
		m_x(_x), m_y(_y), m_z(_z) {}

	void setX(float _x) { m_x = _x; }
	void setY(float _y) { m_y = _y; }
	void setZ(float _z) { m_z = _z; }

	void operator = (const rtVector3& p) 
	{
		m_x = p.m_x;
		m_y = p.m_y;
		m_z = p.m_z;
	}

	static rtVector3 crossProduct(const rtVector3& v1, const rtVector3& v2);
	static float dotProduct(const rtVector3& v1, const rtVector3& v2);
	static float area(const rtVector3& v1, const rtVector3& v2);

	void twoNorm();
	void selfScale(float s);
	rtVector3 scale(float s);
	rtVector3 add(const rtVector3& v);
	rtVector3 subtract(const rtVector3& v);
	rtVector3 getTwoNorm();
	float length();

	float m_x;
	float m_y;
	float m_z;

private:

};

template <typename T>
class rtVector2
{
public:
	rtVector2() : m_x(0), m_y(0) {}
	rtVector2(T _x, T _y) :
		m_x(_x), m_y(_y) {}

	bool operator < (const rtVector2& v) const
	{
		return (m_x < v.m_x) || (m_x == v.m_x && m_y < v.m_y);
	}

	T m_x;
	T m_y;
};
