class rtVector3
{
	rtVector3() : m_x(0), m_y(0), m_z(0){}
	rtVector3(float _x, float _y, float _z):
		m_x(_x), m_y(_y), m_z(_z) {}


private:
	float m_x;
	float m_y;
	float m_z;
};

class rtVector2
{
	rtVector2() : m_x(0), m_y(0) {}
	rtVector2(float _x, float _y) :
		m_x(_x), m_y(_y) {}


private:
	float m_x;
	float m_y;
};