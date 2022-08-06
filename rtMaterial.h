#pragma once
class rtMaterial
{
public:
	rtMaterial() {}
	rtMaterial(float _odr, float _odg, float _odb, float _osr, float _osg, float _osb, float _ka, float _kd, float _ks, float _falloff, float _alpha, float _eta) :
		m_odr(_odr), m_odg(_odg), m_odb(_odb), m_osr(_osr), m_osg(_osg), m_osb(_osb), m_ka(_ka), m_kd(_kd), m_ks(_ks), m_falloff(_falloff), m_alpha(_alpha), m_eta(_eta) {}

	std::string get_texturefile();
	float get_alpha();
	float get_eta();
	void set_alpha(float alpha);
	void set_eta(float eta);

	void setTextureFile(const std::string& filePath);
	void setMtlProperties(float _odr, float _odg, float _odb, float _osr, float _osg, float _osb, float _ka, float _kd, float _ks, float _falloff, float _alpha, float _eta);


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
	float m_alpha;
	float m_eta;

private:
	std::string m_textureFilePath;
	
};