#include "rtMaterial.h"

void rtMaterial::setTextureFile(const std::string& filePath)
{
	m_textureFilePath = filePath;
}

void rtMaterial::setMtlProperties(float _odr, float _odg, float _odb, float _osr, float _osg, float _osb, float _ka, float _kd, float _ks, float _falloff, float _alpha, float _eta)
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