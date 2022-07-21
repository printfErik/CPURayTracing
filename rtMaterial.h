class rtMaterial
{
public:
	rtMaterial();
	void set_mtlcolor(float iodr, float iodg, float iodb, float iosr, float iosg, float iosb, float ika, float ikd, float iks, float ifo, float alpha, float eta);
	void set_texture(std::string s);
	std::string get_texturefile();
	float get_alpha();
	float get_eta();
	void set_alpha(float alpha);
	void set_eta(float eta);

private:
	float m_odr;
	float m_odg;
	float m_odb;
	float m_osr;
	float m_osg;
	float m_osb;
	float m_ka;
	float m_kd;
	float m_ks;
	float m_falloff;

	std::string texturefile_;
	float alpha_;
	float eta_;
};