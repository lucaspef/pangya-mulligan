#include "minatl.h"
#include "vc10_layout.h"
#include "noticeimage.h"
#include "wresrcmng.h"
#include "shareddoc.h"

C_ASSERT_VC10(sizeof(sNoticeImage) == 0x28); // 983: 0x66666667/sar 4 no tamanho do vetor

void LoadNoticeImageFile(const char* filename,
	std::vector<sNoticeImage>& images, int maxNum, const char* lang)
{
	cFile* file = g_resrcmng->GetCFile(filename, -1);

	if (!file)
		return;

	int size = file->m_nLen;
	char* buf = new char[size + 1];

	file->Read(buf, size);
	buf[size] = 0;
	CloseCFile(file);

	char* p = buf;

	images.clear();

	while (*p)
	{
		sNoticeImage info;
		std::string token;
		bool bSpace = true;

		token.clear();

		while (*p && *p != '\n')
		{
			if (*p == '(')
			{
				bSpace = false;

				if (info.name.empty())
					info.name = token.c_str();
				token.clear();
			}
			else if (*p == ')')
			{
				sNoticeImage::sBn bn;
				bSpace = true;

				sscanf(token.c_str(), "%d %f %f %f %f", &bn.typeId, &bn.rect.x,
					&bn.rect.y, &bn.rect.w, &bn.rect.h);
				info.bnList.push_back(bn);
			}
			else if (*p == '\r')
				;
			else if (*p != ' ' || !bSpace)
				token += *p;

			++p;
		}

		if (info.name.empty())
			info.name = token.c_str();

		if (info.name.size() > 0)
		{
			// 983: com idioma, o ultimo texto da linha escolhe a imagem pelo byte CSharedDoc+0x11458: ligado, so as que
			// tem o idioma; desligado, so as que nao tem
			if (lang)
			{
				bool bUse;
				if (CSharedDoc::Instance()->m_bChannelLogin && strstr(token.c_str(), lang))
					bUse = true;
				else if (!CSharedDoc::Instance()->m_bChannelLogin && !strstr(token.c_str(), lang))
					bUse = true;
				else
					bUse = false;
				if (strlen(info.name.c_str()) && bUse)
					images.push_back(info);
			}
			else
				images.push_back(info);
		}

		if (images.size() == maxNum)
			break;

		if (*p)
			++p;
	}

	delete[] buf;
}
