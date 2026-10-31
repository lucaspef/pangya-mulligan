#include "minatl.h"
#include "fortunedlg.h"
#include "frarea.h"
#include "frstatic.h"

IMPLEMENT_OBJECT(FrFortuneDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrFortuneDlg, FrForm)

ON_FRESH_VI("portrait", FRCMD_INIT, FrFortuneDlg::OnPortraitInit)
ON_FRESH_VI("fortune_img", FRCMD_INIT, FrFortuneDlg::OnPortraitInit) // 983 00e36028
ON_FRESH_VI("text_name", FRCMD_INIT, FrFortuneDlg::OnTextNameInit983)
ON_FRESH_VI("text_open", FRCMD_INIT, FrFortuneDlg::OnTextOpenInit983)
ON_FRESH_VI("caption_img", FRCMD_INIT, FrFortuneDlg::OnCaptionImgInit983)
ON_FRESH_VI("desc_name", FRCMD_INIT, FrFortuneDlg::OnDescNameInit)
ON_FRESH_VI("desc_level", FRCMD_INIT, FrFortuneDlg::OnDescLevelInit)
ON_FRESH_VI("desc_desc", FRCMD_INIT, FrFortuneDlg::OnDescDescInit)

END_FRESH_MSGMAP()

void FrFortuneDlg::OnPortraitInit(int param)
{
	m_pPortrait = DYNAMIC_CAST(FrArea, param);
}

void FrFortuneDlg::OnDescNameInit(int param)
{
	m_pDescName = DYNAMIC_CAST(FrStatic, param);
}

void FrFortuneDlg::OnDescLevelInit(int param)
{
	m_pDescLevel = DYNAMIC_CAST(FrStatic, param);
}

void FrFortuneDlg::OnDescDescInit(int param)
{
	m_pDescDesc = DYNAMIC_CAST(FrStatic, param);
}

void FrFortuneDlg::OnTextNameInit983(int param)
{
	m_pTextName983 = DYNAMIC_CAST(FrStatic, param);
}

void FrFortuneDlg::OnTextOpenInit983(int param)
{
	m_pTextOpen983 = DYNAMIC_CAST(FrStatic, param);
	if (m_pTextOpen983) // 983 00aa9287: texto inicial (00d47dd0)
		m_pTextOpen983->SetCaption(K2L_Compatibility("\xc7\xe0\xbf\xee\xc0\xc7 \xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf\xb8\xa6 \xbf\xad\xbe\xee \xb4\xd9\xc0\xbd\xb0\xfa \xb0\xb0\xc0\xba \xbe\xc6\xc0\xcc\xc5\xdb\xb5\xe9\xc0\xbb \xc8\xb9\xb5\xe6\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."));
}

void FrFortuneDlg::OnCaptionImgInit983(int param)
{
	m_pCaptionImg983 = DYNAMIC_CAST(FrArea, param);
}

FrFortuneDlg::FrFortuneDlg()
{
	m_typeId = 0;

	m_pPortrait = NULL;
	m_pDescName = NULL;
	m_pDescLevel = NULL;
	m_pDescDesc = NULL;
	m_pTextName983 = NULL;
	m_pTextOpen983 = NULL;
	m_pCaptionImg983 = NULL;
}

FrFortuneDlg::~FrFortuneDlg()
{
}

void FrFortuneDlg::SetTypeId(unsigned long typeId, IFF_ITEM_COMMON* item)
{
	m_typeId = typeId;

	if (item == NULL)
		return;

	if (m_pPortrait)
	{
		m_pPortrait->SetBgImg(item->Icon);
	}

	if (m_pDescName)
	{
		m_pDescName->SetCaption(MakeStr(K2L_Compatibility("\xc0\xcc\xb8\xa7 : %s"), item->Name));
	}

	if (m_pDescLevel)
	{
		m_pDescLevel->SetCaption(
			"\xb7\xb9\xba\xa7 : \xb4\xa9\xb1\xb8\xb3\xaa \xbb\xe7\xbf\xeb \xb0\xa1\xb4\xc9");
	}

	if (m_pDescDesc)
	{
		m_pDescDesc->SetCaption(MakeStr(
			K2L_Compatibility("\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 %s\xc0\xbb \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
			item->Name));
	}
}

// 983 00aa7860: portrait(+0x110)->SetBgImg(icone do item)
void FrFortuneDlg::SetIcon983(const char* icon)
{
	if (m_pPortrait && icon)
		m_pPortrait->SetBgImg(icon);
}

// 983 00aa78a0: caixa com texto proprio (K2L) e imagem
static const struct { unsigned long tid; const char* name; const char* open; const char* img; } s_fortuneBoxes983[] = {
	{ 0x1a00004d, "\xc0\xcc\xb8\xa7 : 3\xc1\xd6\xb3\xe2 \xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 3\xc1\xd6\xb3\xe2 \xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "fortune_lucky_ponta" },
	{ 0x1a000054, "\xc0\xcc\xb8\xa7 : 2008\xb3\xe2\xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 2008\xb3\xe2\xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "lucky_ponta2008" },
	{ 0x1a0000b5, "\xc0\xcc\xb8\xa7 : 2009\xb3\xe2\xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 2009\xb3\xe2\xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "lucky_ponta2009" },
	{ 0x1a0000bb, "\xc0\xcc\xb8\xa7 : \xc4\xed\xc0\xc7\xbc\xb1\xb9\xb0\xbb\xf3\xc0\xda", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc4\xed\xc0\xc7\xbc\xb1\xb9\xb0\xbb\xf3\xc0\xda \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "kooh_present" },
	{ 0x1a0000d4, "\xc0\xcc\xb8\xa7 : \xc6\xce\xbe\xdf\xc0\xc7\xb3\xaf\xb1\xe2\xb3\xe4\xbb\xf3\xc0\xda", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc6\xce\xbe\xdf\xc0\xc7\xb3\xaf\xb1\xe2\xb3\xe4\xbb\xf3\xc0\xda \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "pangya_box" },
	{ 0x1a0000d5, "\xc0\xcc\xb8\xa7 : ThankYou\xbb\xf3\xc0\xda", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 ThankYou\xbb\xf3\xc0\xda \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "thank_you" },
	{ 0x1a0000e5, "\xc0\xcc\xb8\xa7 : \xc7\xd2\xb7\xce\xc0\xa9\xb1\xe2\xc7\xc1\xc6\xae\xb9\xda\xbd\xba", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc7\xd2\xb7\xce\xc0\xa9\xb1\xe2\xc7\xc1\xc6\xae\xb9\xda\xbd\xba \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "hw_giftbox" },
	{ 0x1a0000e6, "\xc0\xcc\xb8\xa7 : \xc7\xd2\xb7\xce\xc0\xa9\xc4\xda\xbd\xba\xc3\xf5\xb9\xda\xbd\xba", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc7\xd2\xb7\xce\xc0\xa9\xc4\xda\xbd\xba\xc3\xf5\xb9\xda\xbd\xba \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "hw_cosbox" },
	{ 0x1a0000ec, "\xc0\xcc\xb8\xa7 : 5\xc1\xd6\xb3\xe2\xb1\xe2\xb3\xe4\xbb\xf3\xc0\xda", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 5\xc1\xd6\xb3\xe2\xb1\xe2\xb3\xe4\xbb\xf3\xc0\xda \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "5th_box" },
	{ 0x1a0000fd, "\xc0\xcc\xb8\xa7 : \xbe\xc8\xc1\xa9\xb8\xae\xb3\xaa\xc0\xc7\xbe\xcb", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xbe\xc8\xc1\xa9\xb8\xae\xb3\xaa\xc0\xc7\xbe\xcb \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "angel_egg" },
	{ 0x1a00010f, "\xc0\xcc\xb8\xa7 : 2010\xb3\xe2\xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 2010\xb3\xe2\xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "lucky_ponta2010" },
	{ 0x1a000116, "\xc0\xcc\xb8\xa7 : \xb4\xeb\xbf\xee\xb5\xbf\xc8\xb8\xb9\xda\xbd\xba", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xb4\xeb\xbf\xee\xb5\xbf\xc8\xb8\xb9\xda\xbd\xba \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "undoukai_box" },
	{ 0x1a000118, "\xc0\xcc\xb8\xa7 : \xc4\xed\xc0\xc7\xbc\xb1\xb9\xb0\xbb\xf3\xc0\xda""2ND", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc4\xed\xc0\xc7\xbc\xb1\xb9\xb0\xbb\xf3\xc0\xda""2ND \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "kooh_present2nd" },
	{ 0x1a000138, "\xc0\xcc\xb8\xa7 : \xc7\xd2\xb7\xce\xc0\xa9\xb1\xe2\xc7\xc1\xc6\xae\xb9\xda\xbd\xba", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc7\xd2\xb7\xce\xc0\xa9\xb1\xe2\xc7\xc1\xc6\xae\xb9\xda\xbd\xba \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "hw_giftbox" },
	{ 0x1a00013e, "\xc0\xcc\xb8\xa7 : 2011\xb3\xe2\xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 2011\xb3\xe2\xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "lucky_ponta2011" },
	{ 0x1a00013f, "\xc0\xcc\xb8\xa7 : \xc4\xed\xc0\xc7 \xbc\xbf\xb7\xba\xc6\xae\xb9\xda\xbd\xba 2011", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc4\xed\xc0\xc7 \xbc\xbf\xb7\xba\xc6\xae \xb9\xda\xbd\xba 2011 \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "kooh_selectbox_2011" },
	{ 0x1a000165, "\xc0\xcc\xb8\xa7 : 2012\xb3\xe2\xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 2012\xb3\xe2\xba\xb9\xc1\xd6\xb8\xd3\xb4\xcf \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "lucky_ponta2012" },
	{ 0x1a00016a, "\xc0\xcc\xb8\xa7 : \xc4\xed\xc0\xc7 \xbc\xbf\xb7\xba\xc6\xae\xb9\xda\xbd\xba 2012", "\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc4\xed\xc0\xc7 \xbc\xbf\xb7\xba\xc6\xae \xb9\xda\xbd\xba 2012 \xc1\xdf \xc7\xcf\xb3\xaa\xb8\xa6 \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?", "kooh_selectbox_2011" },
};
// 983 00aa78a0 -> 00aa77e0: texto pelo nome do item; img "" = sem imagem
static const struct { unsigned long tid; const char* img; } s_fortuneNamed983[] = {
	{ 0x1a000149, "swimwear_female_2011" }, { 0x1a00018b, "swimwear_female_2011" },
	{ 0x1a00014c, "pangyabox_2011" }, { 0x1a00018c, "pangyabox_2011" },
	{ 0x1a000185, "fortune_lucky_lucia" },
	{ 0x1a000148, "swimwear_male_2011" }, { 0x1a00018a, "swimwear_male_2011" },
	{ 0x1a000195, "8th_box" }, { 0x1a000196, "8th_box" }, { 0x1a000197, "8th_box" }, { 0x1a000198, "8th_box" },
	{ 0x1a000199, "8th_box" }, { 0x1a00019a, "8th_box" }, { 0x1a00019b, "8th_box" }, { 0x1a00019c, "8th_box" },
	{ 0x1a00019d, "8th_box" }, { 0x1a00019e, "8th_box" },
	{ 0x1a0001a2, "madoka_magika_pouch" },
	{ 0x1a0001cf, "fortune_pouch_2013" },
	{ 0x1a0001d6, "kooh_selectbox_2013" },
	{ 0x1a000212, "vividred_2013" },
	{ 0x1a00021d, "pangyabox_2013" },
	{ 0x1a00021e, "male_swimsuit_2013" },
	{ 0x1a00021f, "female_swimsuit_2013" },
	{ 0x1a000220, "club_2013" },
	{ 0x1a00023c, "thai_2013" },
	{ 0x1a000271, "" }, { 0x1a00027b, "" }, { 0x1a00027c, "" }, { 0x1a00027d, "" },
	{ 0x1a00027e, "" }, { 0x1a00028d, "" }, { 0x1a000294, "" }, { 0x1a00029c, "" },
	{ 0x1a00029d, "" }, { 0x1a0002a1, "" }, { 0x1a0002a3, "" }, { 0x1a0002a5, "" },
	{ 0x1a0002aa, "" }, { 0x1a0002b4, "" }, { 0x1a0002cc, "" }, { 0x1a0002cd, "" },
	{ 0x1a0002ce, "" }, { 0x1a0002cf, "" }, { 0x1a0002d0, "" }, { 0x1a0002d1, "" },
	{ 0x1a0002d2, "" }, { 0x1a0002d3, "" }, { 0x1a0002d4, "" }, { 0x1a0002d5, "" },
	{ 0x1a0002d6, "" }, { 0x1a0002d8, "" }, { 0x1a0002da, "" }, { 0x1a0002db, "" },
	{ 0x1a0002dc, "" }, { 0x1a0002dd, "" }, { 0x1a0002de, "" }, { 0x1a0002df, "" },
	{ 0x1a0002e0, "" }, { 0x1a0002e1, "" }, { 0x1a0002e2, "" }, { 0x1a0002e3, "" },
	{ 0x1a0002e4, "" }, { 0x1a0002e5, "" }, { 0x1a0002ee, "" }, { 0x1a0002ef, "" },
	{ 0x1a0002fa, "" }, { 0x1a0002fb, "" }, { 0x1a0002fc, "" }, { 0x1a000309, "" },
	{ 0x1a00030b, "" }, { 0x1a00030d, "" }, { 0x1a00030e, "" }, { 0x1a00030f, "" },
	{ 0x1a000310, "" }, { 0x1a000311, "" }, { 0x1a000312, "" }, { 0x1a000313, "" },
	{ 0x1a000317, "" }, { 0x1a000318, "" }, { 0x1a00031b, "" }, { 0x1a00031c, "" },
	{ 0x1a00031d, "" }, { 0x1a00031e, "" }, { 0x1a00031f, "" }, { 0x1a000320, "" },
	{ 0x1a00032d, "" }, { 0x1a00032e, "" }, { 0x1a00032f, "" }, { 0x1a000330, "" },
	{ 0x1a000331, "" }, { 0x1a000332, "" }, { 0x1a000333, "" }, { 0x1a000334, "" },
	{ 0x1a000335, "" }, { 0x1a000336, "" }, { 0x1a000337, "" }, { 0x1a000339, "" },
	{ 0x1a00033a, "" }, { 0x1a00033b, "" }, { 0x1a00033c, "" }, { 0x1a00033d, "" },
	{ 0x1a00033e, "" }, { 0x1a00033f, "" }, { 0x1a000340, "" }, { 0x1a000341, "" },
	{ 0x1a000342, "" }, { 0x1a000343, "" }, { 0x1a000349, "" }, { 0x1a00034c, "" },
	{ 0x1a00034d, "" }, { 0x1a00034f, "" }, { 0x1a000350, "" }, { 0x1a000351, "" },
	{ 0x1a000355, "" }, { 0x1a000356, "" }, { 0x1a00035a, "" }, { 0x1a00035c, "" },
	{ 0x1a00035d, "" }, { 0x1a00035e, "" }, { 0x1a00035f, "" }, { 0x1a000360, "" },
	{ 0x1a000361, "" }, { 0x1a000363, "" }, { 0x1a000364, "" }, { 0x1a000366, "" },
	{ 0x1a000367, "" }, { 0x1a000369, "" }, { 0x1a00036a, "" }, { 0x1a00036b, "" },
	{ 0x1a00036e, "" }, { 0x1a00036f, "" }, { 0x1a000370, "" }, { 0x1a000371, "" },
	{ 0x1a000372, "" }, { 0x1a000373, "" }, { 0x1a000374, "" }, { 0x1a000375, "" },
	{ 0x1a000376, "" }, { 0x1a000377, "" }, { 0x1a000378, "" }, { 0x1a000379, "" },
	{ 0x1a00037a, "" }, { 0x1a00037b, "" }, { 0x1a00037c, "" }, { 0x1a00037d, "" },
	{ 0x1a00037e, "" }, { 0x1a00037f, "" }, { 0x1a000380, "" }, { 0x1a000381, "" },
	{ 0x1a000382, "" }, { 0x1a000383, "" }, { 0x1a000384, "" }, { 0x1a000385, "" },
	{ 0x1a000386, "" }, { 0x1a000387, "" }, { 0x1a000388, "" }, { 0x1a000389, "" },
	{ 0x1a00038a, "" }, { 0x1a00038c, "" }, { 0x1a00038d, "" }, { 0x1a00038f, "" },
	{ 0x1a000391, "" }, { 0x1a000397, "" }, { 0x1a000398, "" }, { 0x1a00039c, "" },
	{ 0x1a00039d, "" }, { 0x1a0003a2, "" }, { 0x1a0003a3, "" }, { 0x1a0003a5, "" },
	{ 0x1a0003a6, "" }, { 0x1a0003b5, "" }, { 0x1a0003b8, "" }, { 0x1a0003ba, "" },
	{ 0x1a0003bb, "" }, { 0x1a0003bc, "" }, { 0x1a0003bd, "" }, { 0x1a0003be, "" },
	{ 0x1a0003c0, "" }, { 0x1a0003c3, "" }, { 0x1a0003c4, "" }, { 0x1a0003c5, "" },
	{ 0x1a0003c6, "" }, { 0x1a0003c7, "" }, { 0x1a0003c8, "" }, { 0x1a0003c9, "" },
	{ 0x1a0003ca, "" }, { 0x1a0003cb, "" }, { 0x1a0003cc, "" }, { 0x1a0003cd, "" },
	{ 0x1a0003ce, "" }, { 0x1a0003cf, "" }, { 0x1a0003d0, "" }, { 0x1a0003d1, "" },
	{ 0x1a0003d2, "" }, { 0x1a0003d3, "" }, { 0x1a0003d4, "" }, { 0x1a0003d5, "" },
	{ 0x1a0003d6, "" }, { 0x1a0003d7, "" }, { 0x1a0003d8, "" }, { 0x1a0003d9, "" },
	{ 0x1a0003da, "" },
};
// 983 00aa78a0 (saltam direto a 00aa8146): tudo vazio
static const unsigned long s_fortuneBlank983[] = { 0x1a000240, 0x1a000242, 0x1a000248, 0x1a000249, 0x1a00024a, 0x1a000263 };

// 983 00aa78a0: icone, nome/abertura (FrStatic +0x118/+0x11c) e imagem do titulo (+0x120) por tid; tid fora das tabelas = so o icone
void FrFortuneDlg::SetBox983(unsigned long typeId, const char* icon, const char* itemName)
{
	m_typeId = typeId;
	SetIcon983(icon);
	std::string name, open;
	const char* img = NULL;
	for (size_t i = 0; i < sizeof(s_fortuneBoxes983) / sizeof(s_fortuneBoxes983[0]); ++i)
		if (s_fortuneBoxes983[i].tid == typeId)
		{
			name = K2L_Compatibility(s_fortuneBoxes983[i].name);
			open = K2L_Compatibility(s_fortuneBoxes983[i].open);
			img = s_fortuneBoxes983[i].img;
		}
	for (size_t i = 0; i < sizeof(s_fortuneNamed983) / sizeof(s_fortuneNamed983[0]); ++i)
		if (s_fortuneNamed983[i].tid == typeId && itemName != NULL)
		{
			name = MakeStr(K2L_Compatibility("\xc0\xcc\xb8\xa7 : %s"), itemName);
			open = MakeStr(K2L_Compatibility("\xbc\xd2\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 %s\xc0\xbb \xbf\xad\xbe\xee\xba\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"), itemName);
			img = s_fortuneNamed983[i].img;
		}
	for (size_t i = 0; i < sizeof(s_fortuneBlank983) / sizeof(s_fortuneBlank983[0]); ++i)
		if (s_fortuneBlank983[i] == typeId)
			img = "";
	if (img == NULL || m_pTextName983 == NULL || m_pTextOpen983 == NULL || m_pCaptionImg983 == NULL)
		return;
	m_pTextName983->SetCaption(name.c_str());
	m_pTextOpen983->SetCaption(open.c_str());
	m_pCaptionImg983->SetBgImg(img);
}
