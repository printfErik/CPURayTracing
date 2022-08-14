#include "rtMaterial.h"

void rtMaterial::setTextureFile(const std::string& filePath)
{
	m_textureFilePath = filePath;
}

void rtMaterial::setMtlProperties(double _odr, double _odg, double _odb, double _osr, double _osg, double _osb, double _ka, double _kd, double _ks, double _falloff, double _alpha, double _eta)
{
	m_odr = _odr;
	m_odg = _odg;
	m_odb = _odb;

	m_osr = _osr;
	m_osg = _osg;
	m_osb = _osb;

	m_ka = _ka;
	m_kd = _kd;
	m_ks = _ks;

	m_falloff = _falloff;
	m_alpha = _alpha;
	m_eta = _eta;
}

std::string rtMaterial::getTextureFile()
{
	return m_textureFilePath;
}