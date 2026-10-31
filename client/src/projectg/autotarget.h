#pragma once

#include <vector>

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
class CAutoTarget
{
public:
	CAutoTarget() { }
	~CAutoTarget();

	void Reset();
	float GetAngle();
	int CalcDropPos(const WVector& pos);
	WVector GetPoint(const char* club);
	WVector GetDropPos() { return m_dropPos; }

	void SetList(const std::vector<WVector>& list, const WVector& pos);

	const std::vector<WVector>& GetPointList() { return m_pointList; }

	friend class CAuxMenuUI; // OnDropDlgResult le m_dropPos por referencia (VC10 nao embute acessor aqui)

protected:
	float InArea(const WVector& a, const WVector& b, float radius) const;
	WVector GetNearest(unsigned char index);
	float GetDistance(const WVector& a, const WVector& b);
	WVector SolveEquation(unsigned char index, float dist);

	std::vector<WVector> m_pointList;
	WVector m_dropPos;
};
