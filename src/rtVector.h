#pragma once
class rtVector3
{
public:
	rtVector3() : m_x(0), m_y(0), m_z(0){}
	rtVector3(double _x, double _y, double _z):
		m_x(_x), m_y(_y), m_z(_z) {}

	void setX(double _x) { m_x = _x; }
	void setY(double _y) { m_y = _y; }
	void setZ(double _z) { m_z = _z; }

	void operator = (const rtVector3& p) 
	{
		m_x = p.m_x;
		m_y = p.m_y;
		m_z = p.m_z;
	}

	static rtVector3 crossProduct(const rtVector3& v1, const rtVector3& v2);
	static double dotProduct(const rtVector3& v1, const rtVector3& v2);
	static double area(const rtVector3& v1, const rtVector3& v2);

	void twoNorm();
	void selfScale(double s);
	rtVector3 scale(double s) const;
	rtVector3 add(const rtVector3& v) const;
	rtVector3 subtract(const rtVector3& v) const;
	rtVector3 getTwoNorm() const;
	double length();

	double m_x;
	double m_y;
	double m_z;

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
