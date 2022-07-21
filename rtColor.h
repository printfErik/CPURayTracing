
class rtColor
{
public:
	rtColor() : m_r(0), m_g(0), m_b(0) {}
	rtVector3(float _r, float _g, float _b) :
		m_x(_r), m_y(_g), m_z(_b) {}


private:
	float m_r;
	float m_g;
	float m_b;
};