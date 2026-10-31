#pragma once
#include <string>
#include "singleton.h"

class Bitmap;
struct sCharacterInfo;

// JP 983: tabelas fixas por personagem (RTTI .?AVCreateCharacterOption@@, vtable 00d045e0; base WSingleton, vtable 00d045d8).
// Instância única em 00e73e84 (WSingleton<CreateCharacterOption>::m_pInstance), new 0xb60 no CreateSingletonClasses do 983
// (00a6592b). O construtor 004d0e00 chama Load (004d0de0) que preenche as tabelas; nada é lido de arquivo. Unidade
// 004ce760–004d1050 (jp/createcharacteroption.cpp; 004ce560–004ce740 antes dela são da oficina de tacos e 004d1050 em
// diante, do vizinho). Existe no 839 (004d2870…), não no 645 (lá as tabelas eram estáticas nas janelas: s_chatFacials do
// ntAvatarChatMain, figuras do InitUI). Os nomes dos métodos e dos campos são meus. Sessão B124.
class CreateCharacterOption : public WSingleton<CreateCharacterOption>
{
public:
	enum { CHAR_COUNT983 = 15, SET_MOTION_COUNT983 = 5, SET_PART_COUNT983 = 10 };

	// movimento de conjunto do bate-papo: o comando ("/" + command no texto) e o movimento do boneco ("chat_*")
	struct sSetMotion983			// 0x8
	{
		const char* command;		// +0x0
		const char* motion;			// +0x4
	};
	// peça que libera um movimento de conjunto (FindEquipped983/GetSetMotion983)
	struct sSetPart983				// 0x10
	{
		unsigned long tid;			// +0x0 (0 = vazio)
		int kind;					// +0x4 = índice em m_setMotions
		sSetMotion983 motion;		// +0x8 (cópia de m_setMotions[kind])

		void Set(unsigned long t, int k, const sSetMotion983& m) { tid = t; kind = k; motion = m; }
	};

	CreateCharacterOption();		// 004d0e00
	virtual ~CreateCharacterOption(); // 004d0dc0 (vazio; com `delete`: 004d0e60)

	// testes de peça (tid exatos, um por personagem)
	bool IsPartGroupA983(unsigned long tid);	// 004ce760 (sai do boneco de quem tem má conduta: multiplayroomdlg/grandprixroomdlg)
	bool IsPartGroupB983(unsigned long tid);	// 004ce7e0 (chamador 0052e260)
	bool IsPartGroupC983(unsigned long tid);	// 004ce860 (sem chamador)
	bool IsPartGroupD983(unsigned long tid);	// 004ce8e0 (chamador 0052e360)

	void LoadPortraits983(Bitmap** out);		// 004ce960 ("*_def.tga"; o 645 fazia no ntAvatarChatMain::InitUI)
	const char* GetBarIcon983(int charIndex);	// 004ceb40 ("bar_*_def")
	const char* GetChatFacial983(int charIndex, int kind); // 004ced70
	const char* GetName983(int index);			// 004cee50 (nome coreano; pula o índice 13)
	int GetNameCount983() { return m_nameCount; }	// inline no 983 ([inst+0x28], ex. 008126d0)
	const char* GetSubBone983(int charIndex);	// 004cef20 ("*_WI_11_SUB")
	void SetDefaultParts983(unsigned long* parts, int charIndex); // 004cef40
	void RandomParts983(unsigned long* parts, unsigned long charTid); // 004cf170
	void GetCharName983(char* out, int charIndex); // 004cf610 (K2L do nome)

	bool FindSetCommand983(const char* text, unsigned int* pKind); // 004d0b10 (strstr de "/" + comando)
	bool FindSetMotionExact983(const char* motion, unsigned int* pKind); // 004d0b70 (strcmp)
	unsigned long FindEquipped983(unsigned long charTid, const unsigned long* parts, int kind); // 004d0bd0
	// D788: o 983 passa o ntAvatar do Lounge e lê +0x489/+0x495 = m_charInfo.tid/tidParts (no nosso ntAvatar m_charInfo fica em 0x421)
	bool FindEquippedSetCommand983(const char* text, char* motion, unsigned int* pKind, const sCharacterInfo* pInfo); // 004d0c40
	unsigned int GetSetCount983();				// 004d0ce0 (5)
	bool IsSetMotion983(const char* motion, int* pKind); // 004d0cf0 (_stricmp)
	void GetCharFileNames983(std::string* names); // 004d0e90 (15 nomes: "nuri_old"…"cecilia")
	void GetSetMotion983(int charIndex, const unsigned long* parts, int kind, std::string& motion,
		std::string& command);					// 004d0fa0

protected:
	void Load983();							// 004d0de0
	void InitBarIcons983();					// 004cead0
	void InitChatFacials983();				// 004ceb50
	void InitNames983();					// 004ced90
	void InitSubBones983();					// 004cee80
	void InitSetParts983();					// 004cf830

	int m_nameCount;										// +0x28 (0x11)
	const char* m_barIcons[CHAR_COUNT983];					// +0x2c
	const char* m_chatFacials[CHAR_COUNT983][4];			// +0x68 (o 4º = "*reset.png")
	const char* m_names[17];								// +0x158
	const char* m_subBones[CHAR_COUNT983];					// +0x19c
	sSetMotion983 m_setMotions[SET_MOTION_COUNT983];		// +0x1d8
	sSetPart983 m_setParts[CHAR_COUNT983][SET_PART_COUNT983]; // +0x200
};
