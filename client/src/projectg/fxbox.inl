inline std::map<CFxSpray*, CFxSpray*>& CFxBox::GetSprayList()
{
	return m_spray;
}

#ifdef REBANG_FXSPRAY_CPP // HACK: see fxspray.h
inline float CFxBox::Random(float minimum, float maximum)
{
	return (maximum - minimum) * (float)(rand() % 20001) * 0.00005f + minimum;
}

__declspec(noinline) inline WVector CFxBox::Random(const WVector& minimum,
	const WVector& maximum)
{
	return WVector(Random(minimum.x, maximum.x), Random(minimum.y, maximum.y),
		Random(minimum.z, maximum.z));
}
#endif

#ifdef REBANG_FX_CPP
inline std::map<CFxSequence*, CFxSequence*>& CFxBox::GetSequenceList()
{
	return m_sequence;
}

inline CFxSpray* CFxBox::GetSpray(int index)
{
	if (index < 0)
		return NULL;

	for (std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.begin();
		it != m_spray.end(); it++, --index)
	{
		if (index == 0)
			return (*it).second;
	}
	return NULL;
}

inline bool CFxBox::FindSequence(CFxSequence* sequence)
{
	return m_sequence.find(sequence) != m_sequence.end();
}

inline bool CFxBox::FindSpray(CFxSpray* spray)
{
	return m_spray.find(spray) != m_spray.end();
}

inline bool CFxBox::GetOutput(WVector& pos, unsigned char& alpha)
{
	for (std::map<CFxSpray*, CFxSpray*>::iterator it = m_spray.begin();
		it != m_spray.end(); ++it)
	{
		CFxSpray* spray = (*it).second;
		WVector diff = pos - spray->m_quakeCenter;
		float scale = 1.0f / (diff.Magnitude() + 1.0f);
		pos += spray->m_quakeShake * scale;
		int sum = spray->m_flashAlpha + alpha;
		alpha = sum > 255 ? 255 : sum;
	}
	return m_spray.size() != 0;
}
#endif

#ifdef REBANG_FXBOX_CPP
inline bool CFxBox::CheckGroundCollision(const WVector& from, const WVector& to,
	float& height, float radius)
{
	if (!(to.y - height < 0.00001f && to.y - height > -0.00001f))
	{
		if (to.y < height)
		{
			if ((from.y - height < 0.00001f && from.y - height > -0.00001f) ||
				from.y < height)
			{
				float top = radius + to.y;
				return (top - height < 0.00001f && top - height > -0.00001f) ||
					top > height;
			}
		}
		else
		{
			if ((from.y - height < 0.00001f && from.y - height > -0.00001f) ||
				from.y > height)
			{
				float bottom = to.y - radius;
				return (bottom - height < 0.00001f &&
						   bottom - height > -0.00001f) ||
					bottom < height;
			}
		}
	}
	return true;
}

inline void CFxBox::OnSequenceCreated(CFxSequence* sequence)
{
}

inline void CFxBox::OnSequenceDeleted(CFxSequence* sequence)
{
}

inline void CFxBox::OnSprayCreated(CFxSpray* spray)
{
}

inline void CFxBox::OnSprayDeleted(CFxSpray* spray)
{
}
#endif
