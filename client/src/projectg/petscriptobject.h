#pragma once

// 983: interface nova dos comandos de script de boneco ("*snd(", "*shot", ...). No 645 o CGolfPlayer::ParsePuppetScript tinha um switch
// com as 15 palavras; no 983 o analisador saiu para esta classe (ParseScript, 005ac510) e chama um virtual por palavra.
// RTTI .?AVIPetScriptObject@@, vtable 00d0466c (14 slots: destrutor 004d1060 + 13 vazios 005ac830..005ac8f0; `ret 4`, `ret 8` ou
// `ret 0xc` dao o numero de argumentos). Nomes dos metodos e dos argumentos sao NOSSOS (provisorios); a ordem e a da vtable.
// Corpos (ParseScript e os auxiliares) na src/projectg/jp/pktviews983.cpp da frente C (C-PKT lote 64).
class CPetFrame;
class WPuppet;

class IPetScriptObject
{
public:
	virtual ~IPetScriptObject() { }                                      // 0  004d1060
	virtual void OnScriptSnd(const char*& p) { }                         // 1  005ac830 "*snd("
	virtual void OnScriptShot(const char*& p) { }                        // 2  005ac840 "*shot"
	virtual void OnScriptTail(const char*& p) { }                        // 3  005ac850 "*tail"
	virtual void OnScriptShowClub(const char*& p) { }                    // 4  005ac860 "*showclub"
	virtual void OnScriptHideClub(const char*& p) { }                    // 5  005ac870 "*hideclub"
	virtual void OnScriptSwing(const char*& p) { }                       // 6  005ac880 "*swing"
	virtual void OnScriptLockClub(const char*& p) { }                    // 7  005ac890 "*lockclub"
	virtual void OnScriptFx(const char*& p, bool bSkip, int cmd) { }     // 8  005ac8a0 "*fx("
	virtual void OnScriptPTex(const char*& p) { }                        // 9  005ac8b0 "*ptex("
	virtual void OnScriptShowBone(const char*& p, int cmd) { }           // 10 005ac8c0 "*showbone(" / "*hidebone("
	virtual void OnScriptPlayIf(const char*& p) { }                      // 11 005ac8d0 "*playif("
	virtual void OnScriptOpa(const char*& p) { }                         // 12 005ac8e0 "*opa("
	virtual void OnScriptShowVis(const char*& p, int cmd) { }            // 13 005ac8f0 "*showvis(" / "*hidevis("

	void ParseScript(char* script, bool bSkip); // 005ac510

protected:
	void ScriptShowBone(CPetFrame* pFrame, int cmd, const char*& p);     // 005ac900 (cmd 9 = mostrar)
	void ScriptShowBone(WPuppet* pPuppet, int cmd, const char*& p);      // 005ac9b0 (o mesmo com o boneco direto; corpo: frente C)
	const char* ScriptPlayIf(CPetFrame* pFrame, const char*& p);         // 005aca50 (devolve o nome do movimento ou NULL)
	void ScriptShowVis(CPetFrame* pFrame, bool bShow, const char*& p);   // 005acbf0

	CPetFrame* m_pScriptFrame; // +4: gravado por quem chama o ParseScript (0073da85)
};
