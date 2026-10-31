#pragma once
#include <map>
#include <string>
#include <typeinfo>

struct typename_pair
{
	const char* m_from;
	const char* m_to;

	typename_pair() { }

	bool operator<(const typename_pair& other) const
	{
		return (std::string(m_from) + m_to) <
			(std::string(other.m_from) + other.m_to);
	}
};

class type_convert
{
public:
	typedef void(__fastcall* convert_fn)(const void*, void*);
	typedef std::map<typename_pair, convert_fn> convert_map;

	template <class From, class To>
	type_convert(const From* from, To* to)
		: m_failed(false)
	{
		typename_pair key;
		key.m_from = typeid(From).name();
		key.m_to = typeid(To).name();
		convert_map::iterator it = convmap().find(key);
		if (it != convmap().end())
		{
			(*it).second(from, to);
		}
		else
		{
			m_failed = true;
		}
	}

	bool failed() const { return m_failed; }

	template <class From, class To>
	static bool insert_converter(convert_fn fn)
	{
		typename_pair key;
		key.m_from = typeid(From).name();
		key.m_to = typeid(To).name();
		convert_map::iterator it = convmap().find(key);
		if (it == convmap().end())
			convmap().insert(convert_map::value_type(key, fn));
		return true;
	}

private:
	static convert_map& __fastcall convmap()
	{
		static convert_map _cvmap;
		return _cvmap;
	}

	bool m_failed;
};
