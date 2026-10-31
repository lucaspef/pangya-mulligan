inline void CFxSequence::SetActive(bool bActive, bool bChild)
{
	if (m_bActive == bActive)
		return;

	m_bActive = bActive;
	if (!bChild)
		return;

	for (std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.begin();
		it != m_spray.end(); ++it)
	{
		(*it).second->m_bActive = bActive;
	}
}

inline WVector& CFxSequence::Pos()
{
	return m_pos;
}

inline std::map<CFxSpray*, CFxSpray*>& CFxSequence::GetSprayList()
{
	return m_spray;
}

inline void CFxSequence::SetDelay(float delay)
{
	m_delay = delay;
	if (delay > 0.0f)
		SetActive(false, false);
}

inline void CFxSequence::GetBoneMatrix(WMatrix* mat)
{
	WMatrix rot;

	mat->Reset();

	for (int i = 0; i < 4; i++)
	{
		if (m_attachMat[i] == NULL)
			break;

		m_attachMat[i]->GetRotMatrix(&rot);
		*mat = *mat * rot;
	}
}
