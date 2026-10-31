#pragma once

struct sRoomInfo;

namespace S5
{
	namespace CLASSICSRV
	{
		enum eWINDPOWERSTEP
		{
			WINDPOWERSTEP_WEAK,
			WINDPOWERSTEP_MIDDLE,
			WINDPOWERSTEP_STRONG
		};

		int IsClassicServer(unsigned long serverFlag);
		int IsClassicRoom983(const sRoomInfo* room); // 00b30560
		int IsShortGameRoom983(const sRoomInfo* room); // 00b30570
		int IsClassicGame983(); // 00b30580
		int IsShortGame983(); // 00b305b0
		int GetClassicExp983(int star, int hole, int kind, int col); // 00b305d0
		// 983: inline em quem chama (FrGuiItem/gui_control); sem copia propria no exe.
		inline eWINDPOWERSTEP GetWindPowerStep(float windPower)
		{
			if (windPower < 4.0f)
				return WINDPOWERSTEP_WEAK;
			if (windPower < 7.0f)
				return WINDPOWERSTEP_MIDDLE;
			return WINDPOWERSTEP_STRONG;
		}
	}
}
