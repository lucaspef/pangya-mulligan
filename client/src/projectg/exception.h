#pragma once

#include <string>

class WException
{
public:
	WException(const std::string& msg)
		: m_msg(msg)
	{
	}
	virtual ~WException() { }

	virtual void Handle();

protected:
	const std::string& m_msg;
};

class WAppException : public WException
{
public:
	WAppException(const std::string& msg)
		: WException(msg)
	{
	}
	virtual ~WAppException() { }
};

class WSysException : public WException
{
public:
	WSysException(const std::string& msg)
		: WException(msg)
	{
	}
	virtual ~WSysException() { }
};
