#pragma once
#include <string>

class rtMaterial
{
public:
	rtMaterial() {}
	rtMaterial(double _odr, double _odg, double _odb, double _osr, double _osg, double _osb, double _ka, double _kd, double _ks, double _falloff, double _alpha, double _eta) :
		m_odr(_odr), m_odg(_odg), m_odb(_odb), m_osr(_osr), m_osg(_osg), m_osb(_osb), m_ka(_ka), m_kd(_kd), m_ks(_ks), m_falloff(_falloff), m_alpha(_alpha), m_eta(_eta) {}

	std::string getTextureFile();
	void setTextureFile(const std::string& filePath);
	void setMtlProperties(double _odr, double _odg, double _odb, double _osr, double _osg, double _osb, double _ka, double _kd, double _ks, double _falloff, double _alpha, double _eta);

	double m_odr;
	double m_odg;
	double m_odb;
	double m_osr;
	double m_osg;
	double m_osb;
	double m_ka;
	double m_kd;
	double m_ks;
	double m_falloff;
	double m_alpha;
	double m_eta;

private:
	std::string m_textureFilePath;
	
};