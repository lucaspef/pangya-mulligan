inline CGolfBall& GolfBall()
{
	static CGolfBall me;
	return me;
}

inline const CGolfBall::sExpected* CGolfBall::PreData(int index) const
{
	if (index < 0 || index > m_renderNum)
	{
		return NULL;
	}

	if (index < MAX_EXPECTED)
		return &m_expected[index];
	else
		return &m_pExtra[index - MAX_EXPECTED];
}

inline CGolfBall::sExpected* CGolfBall::PreDataToWrite(int index)
{
	if (index < 0 || index > m_renderNum)
		return NULL;

	if (index < MAX_EXPECTED)
		return &m_expected[index];

	if ((index -= MAX_EXPECTED) == m_extraNum * EXTRA_EXPECTED)
	{
		++m_extraNum;

		if (index == 0)
		{
			m_pExtra = new sExpected[EXTRA_EXPECTED];
			memset(m_pExtra, 0, sizeof(sExpected) * EXTRA_EXPECTED);
		}
		else
		{
			sExpected* temp = new sExpected[index];
			memcpy(temp, m_pExtra, sizeof(sExpected) * index);

			delete[] m_pExtra;
			m_pExtra = new sExpected[index + EXTRA_EXPECTED];
			memcpy(m_pExtra, temp, sizeof(sExpected) * index);
			memset(m_pExtra + index, 0, sizeof(sExpected) * EXTRA_EXPECTED);

			delete[] temp;
		}
	}

	return &m_pExtra[index];
}

inline void CGolfBall::operator=(const CGolfBall& ball)
{
	if (m_pExtra)
	{
		delete[] m_pExtra;
		m_pExtra = NULL;
	}

	memcpy(this, &ball, sizeof(CGolfBall));
	if (ball.m_pExtra)
	{
		sExpected* temp = new sExpected[ball.m_extraNum * EXTRA_EXPECTED];
		memcpy(temp, ball.m_pExtra,
			sizeof(sExpected) * EXTRA_EXPECTED * ball.m_extraNum);

		m_pExtra = temp;
	}
}

inline CGolfBall::~CGolfBall()
{
	if (m_pExtra)
	{
		delete[] m_pExtra;
		m_pExtra = NULL;
	}
}

inline CGolfBall::sExpected* CGolfBall::NewPreData()
{
	++m_renderNum;
	return PreDataToWrite(m_renderNum);
}
