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

private:
	float m_x;
	float m_y;
	float m_z;
};

class rtVector2
{
public:
	rtVector2() : m_x(0), m_y(0) {}
	rtVector2(float _x, float _y) :
		m_x(_x), m_y(_y) {}


private:
	float m_x;
	float m_y;
};
