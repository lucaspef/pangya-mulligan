inline WVector CFxSpray::GetBgPos(const CFxParticle* particle)
{
	return m_param.m_bBackground ? GetBgPos() : particle->m_pos;
}

inline WMatrix CFxSpray::GetBgRot(const CFxParticle* particle)
{
	return m_param.m_bBackground ? m_bgRot : particle->m_rot;
}

inline void CFxSpray::GetBoneMatrix(WMatrix* mat)
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
