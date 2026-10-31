#pragma once

#include <string>
#include <vector>

class TiXmlNode;

class NewsReader
{
public:
	struct sArticle
	{
		std::string title;
		std::string url;
		std::string content;
		std::string regDate;
		std::string cate;
		std::string userId;
	};

	NewsReader();
	virtual ~NewsReader();

	unsigned int Load(const char* filename, unsigned int maxArticles);
	void Clear();
	unsigned int GetNumArticles() const;
	const sArticle* GetArticle(unsigned int index) const;

protected:
	static bool GetChildNodeText(const TiXmlNode* node, const char* name,
		std::string& text);

	std::vector<sArticle> m_articles;
};
