#pragma once

#include <map>

class CAztecHoleinEffect : public WSingleton<CAztecHoleinEffect>
{
public:
	CAztecHoleinEffect();
	virtual ~CAztecHoleinEffect();

	bool LoadEffectFromIFF();
	bool Initialize();
	bool PlayEffect(unsigned long effect);

protected:
	void Clear();
	const char* GetEffectName(unsigned long effect);
	bool RegisterEffect(unsigned long effect, const char* name);

protected:
	unsigned long m_pad983_28; // 983 so zera no Clear (007a28e7), sem leitura
	unsigned long m_pad983_2c; // 983 so zera no Clear (007a28ea), sem leitura
	std::map<unsigned long, const char*> m_iffEffectMap;
	std::map<unsigned long, const char*> m_effectMap;
};
