#pragma once
class CGhostDocument;
struct GHOST_FILE_INFO;
class CGhostManager : public WSingleton<CGhostManager>
{
public:
	CGhostManager();
	virtual ~CGhostManager();
	CGhostDocument* GetDocument();
	void ReleaseDocument();
	void SetRecentSelectedGhostNick(const std::string& nick)
	{
		m_recentSelectedGhostNick = nick;
	}
	void LoadDocument(const std::string& fileName);
	void LoadDocumentFromLocalFile(const std::string& fileName);
	void SaveDocument(const std::string& fileName);
	const GHOST_FILE_INFO* GetLatestFileInfo() const
	{
		return m_pLatestFileInfo;
	}
	const std::string& GetLatestFileName() const { return m_latestFileName; }

private:
	CGhostDocument* m_pDocument;
	std::string m_recentSelectedGhostNick;
	std::string m_latestFileName;
	GHOST_FILE_INFO* m_pLatestFileInfo;
};
