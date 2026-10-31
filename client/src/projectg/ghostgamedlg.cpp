#include "minatl.h"
#include "ghostgamedlg.h"
#include "../shared/globalgamedefine.h"
#include "shareddoc.h"
#include "frgraphicinterface.h"
#include "frwndmanager.h"
#include "frlistbox.h"
#include "frstatic.h"
#include "frarea.h"
#include "frbutton.h"
#include "frviewer.h"
#include "fresh.h"
#include "wfont.h"
#include "frgaugebar.h"
#include "golfdoc.h"
#include "taskmanager.h"
#include "actor.h"
#include "../shared/ghostshared.h"
#include "../shared/sharedtables.h"
#include "jp/levelnametable.h"
#include "vc10_layout.h"
#include "s5/classicserver/classicserver.h" // 983: IsClassicGame983/GetClassicExp983 (FrGhostAward::Init)

static CLevelNameTable983 s_levelTable983; // 00e7bc80 (nivel -> nome/exp; Get 008329d0)
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.stat.Level) == 0x91d); // D894: 983 00643fe0 [this+0x91d]

// 983 00643fe0 e um metodo de CSharedDoc (D): este molde thiscall local o replica ate a D ter o dela
struct sLevelUpDesc983
{
	__declspec(noinline) std::string Build();
};
__declspec(noinline) std::string sLevelUpDesc983::Build()
{
	// D894: era *((unsigned char*)this + 0x91d) cru (VC10 = m_myInfo.stat.Level); no cl 14.44 m_myInfo sai de 0x570 (Level 0x6ed)
	unsigned char level = ((CSharedDoc*)this)->m_myInfo.stat.Level;
	IFF983::sLevelUpPrizeItem* prize = ItemManager()->FindLevelUpPrizeItem983(level);
	std::ostringstream os;
	os << K2L_Compatibility(s_levelTable983.Get(level + 1).name)
	   << K2L_Compatibility("\xb7\xce ") << (prize ? prize->szNextPrize : "");
	return os.str();
}






// Reward the player gets for levelling up: gift descriptions of the previous level and the notice
// for the next one.
struct sLevelUpDesc
{
	const char* gift;
	const char* giftDesc;
	const char* notice;
};

#define RB_DYNAMIC_INIT(s) (0, (s))

static sLevelUpDesc
	g_LevelUpDesc
		[22] = {
			{ "",																																														  "",
             RB_DYNAMIC_INIT(
					"\xb7\xe7\xc5\xb0"
					"E\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xbd\xc5\xb0\xe6 \xbe\xc8\xc1\xa4 \xba\xb8\xc1\xb6\xc1\xa6 10\xb0\xb3, \xb7\xb0\xc5\xb0 \xc6\xce\xbe\xdf \xba\xb8\xc1\xb6\xc1\xa6 10\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!") },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc6\xce\xbe\xdf\xbc\xa6\xc0\xbb \xbd\xb1\xb0\xd4 \xc4\xa5 \xbc\xf6 \xc0\xd6\xb4\xc2\n\xbe\xc6\xc0\xcc\xc5\xdb\xb5\xe9\xc0\xbb \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xc6\xce\xbe\xdf\xbc\xa6\xc0\xcc \xbe\xee\xb7\xc1\xbf\xef \xb6\xa7\n\xbb\xe7\xbf\xeb\xc7\xcf\xbc\xbc\xbf\xe4!",
             "\xb7\xe7\xc5\xb0"
				"D\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xb9\xcc\xb6\xf3\xc5\xac \xbb\xe7\xc0\xce 10\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                              },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xb9\xcc\xb6\xf3\xc5\xac \xbb\xe7\xc0\xce 10\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xbe\xee\xb7\xc1\xbf\xee \xc6\xdb\xc6\xc3\xbf\xa1 \xbb\xe7\xbf\xeb\xc7\xcf\xb8\xe9\n\xc1\xc1\xbe\xc6\xbf\xe4!",
             "\xb7\xe7\xc5\xb0"
				"C\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xc3\xbc\xb7\xc2\xba\xb8\xc1\xb6\xc1\xa6 10\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                               },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc5\xb8\xc0\xd3\xba\xce\xbd\xba\xc5\xcd 20\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xba\xfc\xb8\xa5 \xc1\xf8\xc7\xe0\xc0\xbb \xbf\xf8\xc7\xcf\xbd\xc5\xb4\xd9\xb8\xe9 \xc7\xca\xbc\xf6 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc\xc1\xd2!",
             "\xb7\xe7\xc5\xb0"
				"B\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xc3\xbc\xb7\xc2\xba\xb8\xc1\xb6\xc1\xa6 10\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                               },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc3\xbc\xb7\xc2 \xba\xb8\xc1\xb6\xc1\xa6 10\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xc6\xaf\xbc\xf6\xbc\xa6 \xbf\xac\xbd\xc0\xbf\xa1 \xb5\xb5\xbf\xf2\xc0\xcc\n\xb5\xc7\xb1\xe6 \xba\xf4\xbe\xee\xbf\xe4!",
             "\xb7\xe7\xc5\xb0"
				"A\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xc3\xb5\xbb\xe7\xc0\xc7\xbb\xe7\xc5\xc1 5\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                                },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc3\xb5\xbb\xe7\xc0\xc7 \xbb\xe7\xc5\xc1 5\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xba\xf1\xb1\xe2\xb3\xca\xb1\xee\xc1\xf6\n\xbe\xf3\xb8\xb6 \xbe\xc8 \xb3\xb2\xbe\xd2\xbe\xee\xbf\xe4!",
             "\xba\xf1\xb1\xe2\xb3\xca"
				"E\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 3,000\xc6\xce\xc0\xbb \xb5\xe5\xb7\xc1\xbf\xe4!"																																	 },
			{ "\xbc\xb1\xb9\xb0\xb7\xce 3,000\xc6\xce\xc0\xbb \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xba\xf1\xb1\xe2\xb3\xca \xb5\xee\xb1\xde\xc0\xc7 \xbb\xf5\xb7\xce\xbf\xee\n\xbe\xc6\xc0\xcc\xc5\xdb\xb5\xe9\xc0\xbb \xc3\xbc\xc7\xe8\xc7\xd8 \xba\xb8\xbc\xbc\xbf\xe4",
             "\xba\xf1\xb1\xe2\xb3\xca"
				"D\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xc3\xbc\xb7\xc2\xbe\xc8\xc1\xa4\xc1\xa6 5\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                                },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc3\xbc\xb7\xc2\xbe\xc8\xc1\xa4\xc1\xa6 5\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xb5\xce\xb0\xa1\xc1\xf6 \xb4\xc9\xb7\xc2\xc0\xcc \xc7\xd5\xc1\xae\xc1\xf8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbb\xe7\xbf\xeb\xc7\xd8 \xba\xb8\xbc\xbc\xbf\xe4!",
             "\xba\xf1\xb1\xe2\xb3\xca"
				"C\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xc1\xa4\xc8\xae\xb5\xb5\xc0\xc7 \xb9\xdd\xc1\xf6\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                                 },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc1\xa4\xc8\xae\xb5\xb5 \xb9\xdd\xc1\xf6\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xc6\xce\xbe\xdf\xbc\xa6\xc0\xbb \xbd\xc7\xc6\xd0\xc7\xd8\xb5\xb5 \xb9\xae\xc1\xa6\xbe\xf8\xbe\xee\xbf\xe4!",
             "\xba\xf1\xb1\xe2\xb3\xca"
				"B\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xba\xc0\xb4\xd9\xb8\xae\xbc\xa5\xc0\xcc\xbf\xeb\xb1\xc7 5\xc0\xe5\xc0\xbb \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xba\xc0\xb4\xd9\xb8\xae\xbc\xa5 \xc0\xcc\xbf\xeb\xb1\xc7\xc0\xbb\n5\xc0\xe5 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xb7\xb9\xbe\xee \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb\n\xbe\xf2\xc0\xbb \xbc\xf6 \xc0\xd6\xb4\xc2 \xb1\xe2\xc8\xb8!",
             "\xba\xf1\xb1\xe2\xb3\xca"
				"A\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xc6\xce \xb8\xb6\xbd\xba\xc5\xcd\xb8\xae 5\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                               },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc6\xce \xb8\xb6\xbd\xba\xc5\xcd\xb8\xae 5\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xc1\xd6\xb4\xcf\xbe\xee\xb1\xee\xc1\xf6 \xc8\xfb \xb3\xbb\xbc\xbc\xbf\xe4!",
             "\xc1\xd6\xb4\xcf\xbe\xee"
				"E\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 5,000\xc6\xce\xc0\xbb \xb5\xe5\xb7\xc1\xbf\xe4!"																																	 },
			{ "\xbc\xb1\xb9\xb0\xb7\xce 5,000\xc6\xce\xc0\xbb \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xc1\xd6\xb4\xcf\xbe\xee \xb5\xee\xb1\xde\xc0\xc7 \xbb\xf5\xb7\xce\xbf\xee\n\xbe\xc6\xc0\xcc\xc5\xdb\xb5\xe9\xc0\xbb \xc3\xbc\xc7\xe8\xc7\xd8 \xba\xb8\xbc\xbc\xbf\xe4!",
             "\xc1\xd6\xb4\xcf\xbe\xee"
				"D\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xbd\xba\xc7\xc9\xbe\xc8\xc1\xa4\xc1\xa6 10\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                               },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xbd\xba\xc7\xc9\xbe\xc8\xc1\xa4\xc1\xa6 10\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xb5\xce\xb0\xa1\xc1\xf6 \xb4\xc9\xb7\xc2\xc0\xcc \xc7\xd5\xc1\xae\xc1\xf8 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbb\xe7\xbf\xeb\xc7\xd8 \xba\xb8\xbc\xbc\xbf\xe4!",
             "\xc1\xd6\xb4\xcf\xbe\xee"
				"C\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xbd\xba\xc7\xc9\xc0\xc7 \xb9\xdd\xc1\xf6\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                                         },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xbd\xba\xc7\xc9\xc0\xc7 \xb9\xdd\xc1\xf6\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xb4\xf5\xbf\xed \xb1\xe4 \xc6\xc4\xbf\xf6 \xbd\xba\xc7\xc9\xbc\xa6\xc0\xbb\n\xc4\xa5 \xbc\xf6 \xc0\xd6\xbe\xee\xbf\xe4!",
             "\xc1\xd6\xb4\xcf\xbe\xee"
				"B\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xc3\xbc\xb7\xc2\xba\xb8\xc3\xe6\xc1\xa6 10\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                               },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc3\xbc\xb7\xc2 \xba\xb8\xc3\xe6\xc1\xa6 10\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xc6\xc4\xbf\xf6 \xc6\xaf\xbc\xf6\xbc\xa6\xc0\xbb\n\xbd\xb1\xb0\xd4 \xbb\xe7\xbf\xeb\xc7\xd8 \xba\xb8\xbc\xbc\xbf\xe4!",
             "\xc1\xd6\xb4\xcf\xbe\xee"
				"A\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xc6\xce \xb8\xb6\xbd\xba\xc5\xcd\xb8\xae 5\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                               },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc6\xce \xb8\xb6\xbd\xba\xc5\xcd\xb8\xae 5\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xb8\xd6\xb0\xed\xb5\xb5 \xc7\xe8\xc7\xd1 \xbd\xc3\xb4\xcf\xbe\xee\xc0\xc7\n\xb1\xe6\xb7\xce \xc0\xfc\xc1\xf8!",
             "\xbd\xc3\xb4\xcf\xbe\xee"
				"E\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 10,000\xc6\xce\xc0\xbb \xb5\xe5\xb7\xc1\xbf\xe4!"																																	},
			{ "\xbc\xb1\xb9\xb0\xb7\xce 10,000\xc6\xce\xc0\xbb \xb5\xe5\xb7\xc1\xbf\xe4!",
             "\xbd\xc3\xb4\xcf\xbe\xee \xb5\xee\xb1\xde\xc0\xc7 \xbb\xf5\xb7\xce\xbf\xee\n\xbe\xc6\xc0\xcc\xc5\xdb\xb5\xe9\xc0\xbb \xc3\xbc\xc7\xe8\xc7\xd8 \xba\xb8\xbc\xbc\xbf\xe4!",
             "\xbd\xc3\xb4\xcf\xbe\xee"
				"D\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xba\xea\xb7\xd0\xc1\xee\xc4\xab\xb5\xe5\xc6\xbc\xc4\xcf\xc0\xbb \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                          },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xba\xea\xb7\xd0\xc1\xee\xc4\xab\xb5\xe5\xc6\xbc\xc4\xcf\xc0\xbb \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xc4\xab\xb5\xe5\xc8\xa6\xb8\xaf \xbd\xc3\xbd\xba\xc5\xdb\xc0\xbb \xc3\xbc\xc7\xe8\xc7\xd8 \xba\xb8\xbc\xbc\xbf\xe4!",
             "\xbd\xc3\xb4\xcf\xbe\xee"
				"C\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xbd\xba\xc5\xa9\xb7\xa1\xc4\xa1 \xba\xb8\xc1\xb6\xb1\xc7 3\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                               },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xbd\xba\xc5\xa9\xb7\xa1\xc4\xa1 \xba\xb8\xc1\xb6\xb1\xc7\n3\xc0\xe5\xc0\xbb \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",                                                   "10\xc0\xe5\xc0\xbb \xb8\xf0\xc0\xb8\xb8\xe9 \xbd\xba\xc5\xa9\xb7\xa1\xc4\xa1 \xc4\xab\xb5\xe5\xb8\xa6 \xb8\xb8\xb5\xe9 \xbc\xf6 \xc0\xd6\xbe\xee\xbf\xe4",
             "\xbd\xc3\xb4\xcf\xbe\xee"
				"B\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xc3\xbc\xb7\xc2\xba\xb8\xc3\xe6\xc1\xa6 20\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                               },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc3\xbc\xb7\xc2 \xba\xb8\xc3\xe6\xc1\xa6 20\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xc6\xc4\xbf\xf6 \xc6\xaf\xbc\xf6\xbc\xa6\xc0\xbb\n\xbb\xe7\xbf\xeb\xc7\xd8 \xba\xb8\xbc\xbc\xbf\xe4!",
             "\xbd\xc3\xb4\xcf\xbe\xee"
				"A\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xc6\xce \xb8\xb6\xbd\xba\xc5\xcd\xb8\xae 5\xb0\xb3\xb8\xa6 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                               },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xc6\xce \xb8\xb6\xbd\xba\xc5\xcd\xb8\xae 5\xb0\xb3\xb8\xa6 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xb0\xed\xbc\xf6\xc0\xc7 \xb1\xe6\xc0\xcc \xb4\xab\xbe\xd5\xbf\xa1!",
             "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
				"E\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xbd\xba\xc5\xa9\xb7\xa1\xc4\xa1 \xc4\xab\xb5\xe5\xb8\xa6 1\xc0\xe5 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                       },
			{ "\xbc\xb1\xb9\xb0\xb7\xce \xbd\xba\xc5\xa9\xb7\xa1\xc4\xa1 \xc4\xab\xb5\xe5\xb8\xa6\n1\xc0\xe5 \xb5\xe5\xb8\xb3\xb4\xcf\xb4\xd9.",
             "\xb7\xb9\xbe\xee \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc\n\xb3\xaa\xbf\xc0\xb1\xe6 \xba\xf4\xbe\xee\xbf\xe4!",
             "\xb7\xce \xbd\xc2\xb1\xde\xc7\xcf\xbd\xc3\xb8\xe9 \xbd\xba\xc5\xa9\xb7\xa1\xc4\xa1 \xc4\xab\xb5\xe5\xb8\xa6 1\xc0\xe5 \xb5\xe5\xb7\xc1\xbf\xe4!"                                                                                           },
};

// Experience of a ghost game by course difficulty (5) and number of holes (19); columns 2 and 3 are
// the experience for a game played as user and as ghost. JP 983: valores do 983 (00d2dc10; o 645 tinha outra tabela).
static const int g_GhostGameExpTable[95][11] = {
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
	{ 2,  1,  2,  1,  3,  2,  2,  4,  4,  3,  2  },
	{ 3,  2,  3,  2,  4,  3,  3,  5,  5,  4,  3  },
	{ 7,  6,  7,  6,  9,  8,  7,  11, 10, 9,  8  },
	{ 10, 7,  10, 7,  12, 10, 9,  15, 13, 12, 10 },
	{ 12, 9,  12, 9,  15, 13, 11, 18, 16, 14, 12 },
	{ 14, 10, 14, 10, 17, 15, 13, 21, 19, 17, 15 },
	{ 16, 12, 16, 12, 20, 17, 14, 25, 23, 20, 17 },
	{ 18, 13, 18, 13, 23, 20, 16, 28, 26, 22, 19 },
	{ 20, 15, 20, 15, 25, 22, 18, 31, 29, 25, 22 },
	{ 23, 17, 23, 17, 28, 24, 20, 35, 32, 28, 24 },
	{ 25, 18, 25, 18, 31, 27, 22, 38, 35, 30, 26 },
	{ 27, 20, 27, 20, 34, 29, 24, 42, 38, 33, 28 },
	{ 29, 21, 29, 21, 36, 31, 26, 45, 41, 36, 31 },
	{ 31, 23, 31, 23, 39, 33, 28, 48, 44, 39, 33 },
	{ 33, 24, 33, 24, 42, 36, 30, 52, 47, 41, 35 },
	{ 36, 26, 36, 26, 45, 38, 32, 55, 50, 44, 37 },
	{ 38, 28, 38, 28, 47, 40, 34, 58, 53, 47, 40 },
	{ 40, 29, 40, 29, 50, 43, 36, 62, 56, 49, 42 },
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
	{ 2,  1,  2,  1,  3,  3,  2,  4,  4,  3,  3  },
	{ 3,  2,  3,  2,  4,  4,  3,  5,  5,  4,  4  },
	{ 8,  6,  8,  6,  9,  8,  7,  12, 11, 9,  8  },
	{ 10, 8,  10, 8,  12, 11, 9,  15, 14, 12, 11 },
	{ 12, 9,  12, 9,  15, 13, 11, 19, 17, 15, 13 },
	{ 14, 11, 14, 11, 18, 16, 13, 22, 20, 18, 16 },
	{ 17, 12, 17, 12, 21, 18, 15, 26, 24, 21, 18 },
	{ 19, 14, 19, 14, 24, 20, 17, 29, 27, 24, 20 },
	{ 21, 16, 21, 16, 26, 23, 19, 33, 30, 26, 23 },
	{ 23, 17, 23, 17, 29, 25, 21, 36, 33, 29, 25 },
	{ 26, 19, 26, 19, 32, 28, 23, 40, 37, 32, 28 },
	{ 28, 21, 28, 21, 35, 30, 25, 43, 40, 35, 30 },
	{ 30, 22, 30, 22, 38, 33, 27, 47, 43, 38, 33 },
	{ 32, 24, 32, 24, 41, 35, 29, 50, 46, 41, 35 },
	{ 35, 26, 35, 26, 43, 37, 31, 54, 49, 43, 37 },
	{ 37, 27, 37, 27, 46, 40, 34, 57, 53, 46, 40 },
	{ 39, 29, 39, 29, 49, 42, 36, 61, 56, 49, 42 },
	{ 41, 30, 41, 30, 52, 45, 38, 65, 59, 52, 45 },
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
	{ 2,  2,  2,  2,  3,  3,  2,  5,  4,  3,  3  },
	{ 3,  3,  3,  3,  4,  4,  3,  6,  5,  4,  4  },
	{ 8,  6,  8,  6,  10, 9,  7,  12, 11, 10, 9  },
	{ 10, 8,  10, 8,  13, 11, 10, 16, 15, 13, 11 },
	{ 13, 10, 13, 10, 16, 14, 12, 19, 18, 16, 14 },
	{ 15, 11, 15, 11, 19, 16, 14, 23, 21, 19, 16 },
	{ 17, 13, 17, 13, 22, 19, 16, 27, 25, 22, 19 },
	{ 19, 15, 19, 15, 25, 21, 18, 30, 28, 25, 22 },
	{ 22, 16, 22, 16, 27, 24, 20, 34, 31, 28, 24 },
	{ 24, 18, 24, 18, 30, 26, 22, 38, 35, 31, 27 },
	{ 26, 20, 26, 20, 33, 29, 25, 41, 38, 34, 29 },
	{ 29, 22, 29, 22, 36, 32, 27, 45, 42, 37, 32 },
	{ 31, 23, 31, 23, 39, 34, 29, 49, 45, 40, 34 },
	{ 33, 25, 33, 25, 42, 37, 31, 52, 48, 43, 37 },
	{ 36, 27, 36, 27, 45, 39, 33, 56, 52, 46, 40 },
	{ 38, 28, 38, 28, 48, 42, 35, 60, 55, 49, 42 },
	{ 40, 30, 40, 30, 51, 44, 37, 63, 58, 52, 45 },
	{ 43, 32, 43, 32, 54, 47, 40, 67, 62, 55, 47 },
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
	{ 3,  2,  3,  2,  4,  3,  3,  5,  5,  4,  3  },
	{ 4,  3,  4,  3,  5,  4,  4,  6,  6,  5,  4  },
	{ 8,  7,  8,  7,  11, 9,  8,  13, 12, 11, 10 },
	{ 11, 8,  11, 8,  14, 12, 10, 17, 16, 14, 12 },
	{ 13, 10, 13, 10, 17, 15, 13, 21, 19, 17, 15 },
	{ 16, 12, 16, 12, 20, 18, 15, 25, 23, 21, 18 },
	{ 18, 14, 18, 14, 23, 20, 18, 29, 27, 24, 21 },
	{ 21, 16, 21, 16, 26, 23, 20, 33, 30, 27, 24 },
	{ 23, 18, 23, 18, 30, 26, 22, 37, 34, 30, 27 },
	{ 26, 20, 26, 20, 33, 29, 25, 41, 38, 34, 30 },
	{ 28, 22, 28, 22, 36, 31, 27, 45, 41, 37, 33 },
	{ 31, 23, 31, 23, 39, 34, 29, 49, 45, 40, 35 },
	{ 33, 25, 33, 25, 42, 37, 32, 53, 49, 44, 38 },
	{ 35, 27, 35, 27, 45, 40, 34, 57, 52, 47, 41 },
	{ 38, 29, 38, 29, 49, 43, 37, 61, 56, 50, 44 },
	{ 40, 31, 40, 31, 52, 45, 39, 65, 60, 53, 47 },
	{ 43, 33, 43, 33, 55, 48, 41, 69, 63, 57, 50 },
	{ 45, 35, 45, 35, 58, 51, 44, 72, 67, 60, 53 },
	{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
	{ 3,  2,  3,  2,  4,  4,  3,  6,  5,  5,  4  },
	{ 4,  3,  4,  3,  5,  5,  4,  7,  6,  6,  5  },
	{ 9,  7,  9,  7,  12, 10, 9,  14, 13, 12, 11 },
	{ 12, 9,  12, 9,  15, 13, 12, 19, 17, 16, 14 },
	{ 14, 11, 14, 11, 19, 17, 15, 23, 22, 20, 18 },
	{ 17, 14, 17, 14, 22, 20, 17, 27, 26, 23, 21 },
	{ 20, 16, 20, 16, 26, 23, 20, 32, 30, 27, 24 },
	{ 23, 18, 23, 18, 29, 26, 23, 36, 34, 31, 28 },
	{ 25, 20, 25, 20, 33, 29, 25, 41, 38, 34, 31 },
	{ 28, 22, 28, 22, 36, 32, 28, 45, 42, 38, 34 },
	{ 31, 24, 31, 24, 40, 35, 31, 50, 46, 42, 37 },
	{ 33, 26, 33, 26, 43, 38, 33, 54, 50, 46, 41 },
	{ 36, 28, 36, 28, 47, 41, 36, 58, 55, 49, 44 },
	{ 39, 30, 39, 30, 50, 44, 39, 63, 59, 53, 47 },
	{ 41, 32, 41, 32, 54, 48, 42, 67, 63, 57, 51 },
	{ 44, 34, 44, 34, 57, 51, 44, 72, 67, 60, 54 },
	{ 47, 37, 47, 37, 61, 54, 47, 76, 71, 64, 57 },
	{ 49, 39, 49, 39, 64, 57, 50, 80, 75, 68, 61 },
};

extern Fresh* g_pFresh;

// ---------------------------------------------------------------- FrRecentMatch

IMPLEMENT_OBJECT(FrRecentMatch, FrForm)
BEGIN_FRESH_MSGMAP(FrRecentMatch, FrForm)

ON_FRESH_VI("bg_area", FRCMD_INIT, FrRecentMatch::OnInitBgArea)
ON_FRESH_VV("bg_area", FRCMD_OWNERDRAW, FrRecentMatch::OnDrawBgArea)
ON_FRESH_VI("match_list", FRCMD_INIT, FrRecentMatch::OnInitHistoryList)
ON_FRESH_VI("match_list", FRCMD_OWNERDRAW, FrRecentMatch::OnDrawHistoryList)
ON_FRESH_VI("text1", FRCMD_INIT, FrRecentMatch::OnInitStaticText1)

END_FRESH_MSGMAP()

FrRecentMatch::FrRecentMatch()
	: m_pCourseImg(NULL),
	  m_pBallImg(NULL),
	  m_pang(0),
	  m_score(0),
	  m_pHistoryList(NULL),
	  m_pBgArea(NULL),
	  m_pText(NULL),
	  m_pFont(NULL)
{
	static char* s_font[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_font;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ymP-.?/:";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

FrRecentMatch::~FrRecentMatch()
{
	std::for_each(m_history.begin(), m_history.end(), Delete_Object());

	if (g_resrcmng != NULL && m_pFont != NULL)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

void FrRecentMatch::Init(const std::string& name, int course, int ball,
	int pang, int score)
{
	m_name = name;

	IFF_STRUCT::sCourse* pCourse = ItemManager()->FindCourse(0x28000000 |
		((course > 0x7f && course != 0xfd) ? course - 0x80 : course));
	IFF_ITEM_COMMON* pBall = ItemManager()->FindCommonItem(ball | 0x4000000);

	if (pCourse != NULL && pBall != NULL)
	{
		m_pCourseImg =
			g_pFresh->GetBitmap(MakeStr("ghost_mid_%s", pCourse->c.Icon));
		m_pBallImg = g_pFresh->GetBitmap(MakeStr("%s", pBall->Icon));
		m_pPangImg = g_pFresh->GetBitmap("ghost_pang");
		m_pang = pang;
		m_score = score;
	}
}

void FrRecentMatch::AddHistory(const sRecentMatchForm& form)
{
	sRecentMatchForm* item = new sRecentMatchForm;
	*item = form;
	m_history.push_back(item);
	m_pHistoryList->AddItem(item);
}

void FrRecentMatch::AddFinish()
{
	m_pText->SetCaption("");

	int count = m_pHistoryList->GetCurrentItemSize(true);
	for (int i = count; i < 5; ++i)
		m_pHistoryList->AddItem(NULL);
}

void FrRecentMatch::OnInitBgArea(int param)
{
	m_pBgArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrRecentMatch::OnDrawBgArea()
{
	WRect rect;
	rect.x = m_pBgArea->GetRect().x;
	rect.y = m_pBgArea->GetRect().y;
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device && m_pCourseImg && m_pBallImg && m_pPangImg)
	{
		rect.x = rect.x + 20.0f;
		rect.y = rect.y + 7.0f;
		rect.w = (float)m_pCourseImg->Width();
		rect.h = (float)m_pCourseImg->Height();
		float y = rect.y;
		float x = rect.x;
		device->DrawTexture(m_pCourseImg, rect, 0xffffffff, 0);

		x = (float)m_pCourseImg->Width() + x + 20.0f;
		device->SetTextStyle(4);
		device->SetTextColor(0xff000000, 0xffc8c8c8);
		g_pFresh->GetManager()->PrintText(WPoint(x + 60.0f, y + 5.0f), 0,
			m_name.c_str(), -1.0f, 0xffffffff);
		device->SetTextStyle(0);
		device->SetTextColor(0xff000000, 0xffffffff);

		rect.x = x - 5.0f;
		rect.y = y + 25.0f;
		rect.w = (float)m_pBallImg->Width();
		rect.h = (float)m_pBallImg->Height();
		device->DrawTexture(m_pBallImg, rect, 0xffffffff, 0);

		x = x + 115.0f;
		m_pFont->Print(g_view, x, y + 35.0f, MakeStr("-%d", m_pang), 0,
			0xffffffff, NULL);
		y = y + 55.0f;
		float w = m_pFont->Print(g_view, x, y, MakeStr("%d", m_score), 0,
			0xffffffff, NULL);
		rect.x = w + x + 5.0f;
		rect.y = y;
		rect.w = (float)m_pPangImg->Width();
		rect.h = (float)m_pPangImg->Height();
		device->DrawTexture(m_pPangImg, rect, 0xffffffff, 0);
	}
}

void FrRecentMatch::OnInitHistoryList(int param)
{
	m_pHistoryList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrRecentMatch::OnDrawHistoryList(int param)
{
	FrListItem* item = (FrListItem*)param;
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (item && device)
	{
		device->SetTextStyle(0);
		device->SetTextColor(0xff000000, 0xffffffff);
		float x = item->pos.x;
		float y = item->pos.y + 8.0f;
		sRecentMatchForm* form = (sRecentMatchForm*)item->pData;
		if (form)
		{
			const Bitmap* gender;
			if (form->gender)
				gender = g_pFresh->RegisterBitmap("i_female");
			else
				gender = g_pFresh->RegisterBitmap("i_male");
			if (gender)
			{
				WRect rect(x + 3.0f, item->pos.y, (float)gender->Width(),
					(float)gender->Height());
				device->DrawTexture(gender, rect, 0xffffffff, 0);
				x = x + (float)gender->Width() + 10.0f;
			}
			g_pFresh->GetManager()->PrintText11(WPoint(x, y), 0,
				form->name.c_str(), -1.0f, 0xffffffff);
			// HACK: 130.0f; carddlg.cpp owns __real@43020000 as a function-local static (naked asm), the
			// literal here would emit a competing COMDAT (the volatile read keeps the x87 operand order)
			x += 130.0f;	// 983/VS2010: literal direto
			if (form->win)
				device->SetTextColor(0xff1c1c1c, 0xffffffff);
			else
				device->SetTextColor(0xff999999, 0xffffffff);
			const char* result = K2L_Compatibility("\xbd\xc2");
			if (form->win == 0)
				result = K2L_Compatibility("\xc6\xd0");
			x = g_pFresh->GetManager()->PrintText11(WPoint(x, y), 0, result,
					-1.0f, 0xffffffff) +
				x + 30.0f;
			device->SetTextColor(0xff000000, 0xffffffff);
			const Bitmap* grade = g_pFresh->GetBitmap(
				MakeStr("ghost_grade_%d", (unsigned int)form->grade));
			if (grade)
			{
				WRect rect(x - 10.0f, y - 2.0f, (float)grade->Width(),
					(float)grade->Height());
				device->DrawTexture(grade, rect, 0xffffffff, 0);
			}
			x = x + 35.0f;
			{
				const Bitmap* pang = m_pPangImg;
				WRect rect(x, y - 1.0f, (float)pang->Width(),
					(float)pang->Height());
				device->DrawTexture(pang, rect, 0xffffffff, 0);
			}
			x = (float)m_pPangImg->Width() + x + 5.0f;
			device->Print11(WPoint(x, y), 0, "%d", form->score);
			g_pFresh->GetManager()->PrintText11(WPoint(x + 50.0f, y), 0,
				MakeStr("%02d.%02d.%02d", form->time.wYear, form->time.wMonth,
					form->time.wDay),
				-1.0f, 0xffffffff);
		}
		else
		{
			device->SetTextStyle(4);
			device->SetTextColor(0xffc8c8c8, 0xff000000);
			x = x + 55.0f;
			device->Print(WPoint(x, y), 1, K2L_Compatibility("\xb1\xe2\xb7\xcf \xbe\xf8\xc0\xbd"));
			x = x + 105.0f;
			m_pFont->Print(g_view, x, y, "-", 0, 0xffffffff, NULL);
			x = x + 40.0f;
			m_pFont->Print(g_view, x, y, "-", 0, 0xffffffff, NULL);
			x = x + 65.0f;
			m_pFont->Print(g_view, x, y, "-", 0, 0xffffffff, NULL);
			m_pFont->Print(g_view, x + 70.0f, y, "-", 0, 0xffffffff, NULL);
		}
		device->SetTextStyle(0);
		device->SetTextColor(0xff000000, 0xffffffff);
	}
}

void FrRecentMatch::OnInitStaticText1(int param)
{
	m_pText = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	m_pText->SetCaption(
		K2L_Compatibility("\xbc\xad\xb9\xf6\xbf\xa1 \xc1\xa4\xba\xb8\xb8\xa6 \xbf\xe4\xc3\xbb\xc7\xcf\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."));
}

// ---------------------------------------------------------------- FrGhostGameMapDlg

IMPLEMENT_OBJECT(FrGhostGameMapDlg, FrForm)
BEGIN_FRESH_MSGMAP(FrGhostGameMapDlg, FrForm)

ON_FRESH_VV("no", FRCMD_LBUTTONDOWN, FrGhostGameMapDlg::OnDownCancel)
ON_FRESH_VV("yes", FRCMD_LBUTTONDOWN, FrGhostGameMapDlg::OnDownOK)
ON_FRESH_VI("recent", FRCMD_INIT, FrGhostGameMapDlg::OnInitRecentMatch)
ON_FRESH_VV("recent", FRCMD_LBUTTONDOWN, FrGhostGameMapDlg::OnDownRecentMatch)
ON_FRESH_VI("bg_area", FRCMD_INIT, FrGhostGameMapDlg::OnInitAreaBg)
ON_FRESH_VI("bg_area", FRCMD_OWNERDRAW, FrGhostGameMapDlg::OnDrawAreaBg)
ON_FRESH_VI("map_select", FRCMD_INIT, FrGhostGameMapDlg::OnInitSelMapList)
ON_FRESH_VI("map_select", FRCMD_OWNERDRAW, FrGhostGameMapDlg::OnDrawSelMapList)
ON_FRESH_VV("map_select", FRCMD_LBUTTONDOWN,
	FrGhostGameMapDlg::OnDownSelMapList)
ON_FRESH_VI("choice_text", FRCMD_INIT, FrGhostGameMapDlg::OnInitAreaSelecText)
ON_FRESH_VV("choice_text", FRCMD_OWNERDRAW, FrGhostGameMapDlg::OnDrawSelecText)
ON_FRESH_VI("choice_map", FRCMD_INIT, FrGhostGameMapDlg::OnInitAreaMap)
ON_FRESH_VI("choice_map", FRCMD_OWNERDRAW, FrGhostGameMapDlg::OnDrawChoiceMap)

END_FRESH_MSGMAP()

void FrGhostGameMapDlg::sImgTag::Animate(float deltaTime)
{
	if (alpha < 255.0f)
	{
		alpha = deltaTime * 204.0f + alpha;
		if (alpha > 255.0f)
			alpha = 255.0f;
	}
	if (glow < 255.0f && offset < width + destX)
	{
		glow = deltaTime * 357.0f + glow;
		if (glow > 255.0f)
			glow = 255.0f;
	}
	if (destX < offset)
	{
		offset = offset - (800.0f - destX) * deltaTime * 1.5f;
		if (offset < destX)
			offset = destX;
	}
}

void FrGhostGameMapDlg::sImgTag::Reset()
{
	glow = 0.0f;
	alpha = 0.0f;
	offset = 800.0f;
}

void FrGhostGameMapDlg::sImgInfo::AnimateLock(float deltaTime)
{
	if (bLocked && !bPlayable && lockAlpha < 200.0f)
	{
		lockAlpha = deltaTime * 400.0f + lockAlpha;
		if (lockAlpha > 200.0f)
			lockAlpha = 200.0f;
	}
}

FrGhostGameMapDlg::FrGhostGameMapDlg()
	: m_pSelMapList(NULL),
	  m_selected(0),
	  m_pAreaMap(NULL),
	  m_tipAlpha(0.0f),
	  m_pRecentBtn(NULL)
{
	m_imgInfo.reserve(22); // 983 0081fa20: 22 cursos (645: 20)
	m_imgTag.reserve(22);
	for (int i = 22; i != 0; --i)
	{
		sImgInfo info(0.0f, 0.0f);
		info.courseIndex = 22;
		m_imgInfo.push_back(info);

		sImgTag tag;
		memset(&tag, 0, sizeof(tag));
		m_imgTag.push_back(tag);
	}
}

FrGhostGameMapDlg::~FrGhostGameMapDlg()
{
}

void* FrGhostGameMapDlg::GetSelectItem()
{
	std::list<GHOST_GAME_BRIEF>::iterator it =
		std::find(m_briefList.begin(), m_briefList.end(), m_selected);
	if (it != m_briefList.end())
		return &*it;
	return NULL;
}

void FrGhostGameMapDlg::Init(unsigned long uid, const std::string& name,
	const std::list<GHOST_GAME_BRIEF>& briefList)
{
	if (uid == CSharedDoc::Instance()->m_myInfo.info.dwUID)
	{
		m_pRecentBtn->SetVisible(true);
		m_pRecentBtn->Enable(true);
	}
	m_briefList.assign(briefList.begin(), briefList.end());
	m_ownerUid = uid;
	m_name = name;
	if (!m_pSelMapList)
		return;

	std::list<int> dataList;
	std::list<int> emptyList;
	for (int i = 21; i >= 0; --i)
	{
		if (i == 17 || i == 12)
			continue;
		int index = i;
		if (i > 0x7f && i != 0xfd)
			index = i - 0x80;
		IFF_STRUCT::sCourse* course =
			ItemManager()->FindCourse(index | 0x28000000);
		if (!course)
			continue;

		sImgInfo info(-100.0f, 0.0f);
		info.courseIndex = i;
		std::list<GHOST_GAME_BRIEF>::iterator it =
			std::find(m_briefList.begin(), m_briefList.end(), i);
		if (it != m_briefList.end() && (*it).active != 0)
		{
			dataList.push_back(i);
			info.bPlayable = true;
		}
		else
		{
			if (it != m_briefList.end())
				info.bHasRecord = true;
			emptyList.push_back(i);
			info.bPlayable = false;
		}
		m_imgInfo[i] = info;

		sImgTag tag;
		tag.pBg = g_pFresh->GetBitmap(MakeStr("30in_back_%s", course->Data));
		if (tag.pBg)
		{
			tag.rect.x = m_areaRect.x;
			tag.rect.y = m_areaRect.y;
			tag.rect.w = (float)tag.pBg->Width();
			tag.rect.h = (float)tag.pBg->Height();
		}
		tag.pImg = g_pFresh->GetBitmap("ghost_mapdlg_info");
		if (tag.pImg)
		{
			tag.destX = m_areaRect.x + 1.0f;
			tag.y = m_areaRect.y + 6.0f;
			tag.width = (float)tag.pImg->Width();
			tag.height = (float)tag.pImg->Height();
		}
		tag.offset = 800.0f;
		tag.alpha = 0.0f;
		tag.glow = 0.0f;
		m_imgTag[i] = tag;
	}

	if (dataList.empty())
		m_selected = 21;
	else
		m_selected = *dataList.begin();

	for (std::list<int>::iterator it = dataList.begin(); it != dataList.end();
		++it)
	{
		int index = *it;
		if (index > 0x7f && index != 0xfd)
			index -= 0x80;
		IFF_STRUCT::sCourse* course =
			ItemManager()->FindCourse(index | 0x28000000);
		if (course)
			m_pSelMapList->AddItem(course);
	}
	for (std::list<int>::iterator it = emptyList.begin(); it != emptyList.end();
		++it)
	{
		int index = *it;
		if (index > 0x7f && index != 0xfd)
			index -= 0x80;
		IFF_STRUCT::sCourse* course =
			ItemManager()->FindCourse(index | 0x28000000);
		if (course)
			m_pSelMapList->AddItem(course);
	}
}

void FrGhostGameMapDlg::OnProc(const float deltaTime)
{
	for (std::vector<sImgInfo>::iterator it = m_imgInfo.begin();
		it != m_imgInfo.end(); ++it)
	{
		if ((*it).courseIndex == 22 || !(*it).bDrawn)
			continue;
		if ((*it).y > (*it).x)
		{
			(*it).x = ((*it).y + 100.0f) * deltaTime * 1.3f + (*it).x;
			if ((*it).x > (*it).y)
				(*it).x = (*it).y;
		}
		if ((*it).alpha < 255.0f && (*it).x > 0.0f)
		{
			(*it).alpha = deltaTime * 331.5f + (*it).alpha;
			if ((*it).alpha > 255.0f)
				(*it).alpha = 255.0f;
		}
		if ((*it).y == (*it).x)
			(*it).bLocked = true;
		(*it).AnimateLock(deltaTime);
	}
	sImgTag& tag = m_imgTag[m_selected];
	tag.Animate(deltaTime);
	if (m_tipAlpha < 255.0f)
	{
		m_tipAlpha = deltaTime * 127.5f + m_tipAlpha;
		if (m_tipAlpha >= 255.0f)
			m_tipAlpha = 255.0f;
	}
	FrForm::OnProc(deltaTime);
}

void FrGhostGameMapDlg::OnDownOK()
{
}

void FrGhostGameMapDlg::OnDownCancel()
{
	if (IsLocalContent(S4_MATCHING_SYSTEM))
	{
		WSendPacket packet((enumClientPacket)0xc1);
		packet.Encode1(1);
		packet.Send(TO_GAME);
	}
	FrForm::Close(true);
}

void FrGhostGameMapDlg::OnDownRecentMatch()
{
	FrRecentMatch* form = CreateForm<FrRecentMatch>(g_pFresh->GetManager(),
		NULL, "GHOST_HISTORY", NULL);
	std::list<GHOST_GAME_BRIEF>::iterator it =
		std::find(m_briefList.begin(), m_briefList.end(), m_selected);
	if (it != m_briefList.end())
	{
		form->Init(m_name, m_selected, (*it).ball, (*it).pang, (*it).score);
		form->SetTopmost(true);
		form->FindNextTopFocus(false);
		form->Open(NULL, 1);

		WSendPacket packet((enumClientPacket)0xc2);
		packet.Encode1(9);
		packet.Encode1((unsigned char)m_selected);
		packet.Encode4((*it).recordId);
		packet.Encode4(m_ownerUid);
		packet.Send(TO_GAME);
	}
}

void FrGhostGameMapDlg::OnInitRecentMatch(int param)
{
	m_pRecentBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pRecentBtn->SetVisible(false);
}

void FrGhostGameMapDlg::OnInitSelMapList(int param)
{
	m_pSelMapList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrGhostGameMapDlg::OnDownSelMapList()
{
	FrListItem* item = m_pSelMapList->GetSelected();
	if (item && item->pData)
	{
		unsigned int id = ((unsigned int*)item->pData)[1] & 0x3ffffff;
		if (m_imgInfo[id].bPlayable)
		{
			m_selected = id;
			m_imgTag[id].Reset();
		}
	}
}

void FrGhostGameMapDlg::OnDrawSelMapList(int param)
{
	FrListItem* item = (FrListItem*)param;
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (!item || !device)
		return;
	IFF_ITEM_COMMON* course = (IFF_ITEM_COMMON*)item->pData;
	if (!course)
		return;
	const Bitmap* img =
		g_pFresh->GetBitmap(MakeStr("ghost_mid_%s", course->Icon));
	if (!img)
		return;
	unsigned int idx = course->TypeId & 0x3ffffff;
	sImgInfo& info = m_imgInfo[idx];
	WRect rect;
	rect.x = info.x;
	info.bDrawn = true;
	info.y = item->pos.x;
	rect.y = item->pos.y + 6.0f;
	rect.w = (float)img->Width();
	rect.h = (float)img->Height();
	device->DrawTexture(img, rect, ((unsigned char)info.alpha << 24) | 0xffffff,
		0);

	if (m_selected == idx)
	{
		const Bitmap* sel = g_pFresh->GetBitmap("ghost_mid_select_o");
		if (sel)
		{
			rect.y = item->pos.y + 6.0f;
			rect.x = item->pos.x;
			rect.w = (float)sel->Width();
			rect.h = (float)sel->Height();
			device->DrawTexture(sel, rect,
				((unsigned char)info.alpha << 24) | 0xffffff, 0);
		}
	}
	if (!info.bPlayable)
	{
		const Bitmap* lock = g_pFresh->GetBitmap(
			info.bHasRecord ? "ghost_mid_map_lock" : "ghost_mid_map_nodata");
		if (lock)
		{
			rect.y = item->pos.y + 6.0f;
			rect.x = info.x;
			rect.w = (float)lock->Width();
			rect.h = (float)lock->Height();
			device->DrawTexture(lock, rect,
				((unsigned char)info.lockAlpha << 24) | 0xffffff, 0);
		}
	}
}

void FrGhostGameMapDlg::OnInitAreaSelecText(int param)
{
	m_pAreaMap = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrGhostGameMapDlg::OnDrawSelecText()
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device)
	{
		const Bitmap* tip = g_pFresh->GetBitmap("ghost_map_select_tip");
		if (tip)
		{
			WRect rect = m_pAreaMap->GetRect();
			rect.w = (float)tip->Width();
			rect.h = (float)tip->Height();
			device->DrawTexture(tip, rect,
				((unsigned char)m_tipAlpha << 24) | 0xffffff, 0);
		}
	}
}

void FrGhostGameMapDlg::OnInitAreaMap(int param)
{
	FrArea* area = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (area)
		m_areaRect = area->GetRect();
}

void FrGhostGameMapDlg::OnDrawChoiceMap(int param)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (!device)
		return;
	sImgTag& tag = m_imgTag[m_selected];
	if (tag.pBg)
		device->DrawTexture(tag.pBg, (const WRect&)tag.rect,
			((unsigned char)tag.alpha << 24) | 0xffffff, 0);
	if (!tag.pImg)
		return;

	WRect pos;
	pos.x = tag.offset;
	pos.y = tag.y;
	pos.h = tag.height;
	float right = tag.width + tag.rect.x + 1.0f;
	if (!(pos.x < right))
		return;
	pos.w = right - pos.x;
	WRect src(0.0f, 0.0f, pos.w, pos.h);
	device->DrawTexture(tag.pImg, src, pos,
		((unsigned char)tag.glow << 24) | 0xffffff, 0);

	std::list<GHOST_GAME_BRIEF>::iterator it =
		std::find(m_briefList.begin(), m_briefList.end(), m_selected);
	if (!(it != m_briefList.end()))
		return;

	IFF_ITEM_COMMON* ball =
		ItemManager()->FindCommonItem((*it).ball | 0x4000000);
	const Bitmap* ballImg = g_pFresh->GetBitmap(MakeStr("%s", ball->Icon));
	if (ballImg)
	{
		WRect rect;
		rect.x = pos.x + 7.0f;
		rect.y = pos.y + 6.0f;
		rect.w = (float)ballImg->Width();
		rect.h = (float)ballImg->Height();
		device->DrawTexture(ballImg, rect,
			((unsigned char)tag.glow << 24) | 0xffffff, 0);
	}
	if (tag.glow >= 184.0f)
	{
		device->SetTextStyle(2);
		device->SetTextColor(0xffffffff, 0xff000000);
		g_pFresh->GetManager()->PrintText(
			WPoint(tag.rect.x + 108.0f, tag.rect.y + 19.0f), 0, m_name.c_str(),
			-1.0f, 0xffffffff);
		device->Print(WPoint(tag.rect.x + 108.0f, tag.rect.y + 37.0f), 0, "%d",
			-(*it).pang);
		float w = device->Print(WPoint(tag.rect.x + 108.0f, tag.rect.y + 52.0f),
			0, "%d", (*it).score);
		const Bitmap* pangImg = g_pFresh->GetBitmap("ghost_pang");
		if (pangImg)
		{
			WRect rect;
			rect.x = w + tag.rect.x + 110.0f;
			rect.y = tag.rect.y + 52.0f;
			rect.w = (float)pangImg->Width();
			rect.h = (float)pangImg->Height();
			device->DrawTexture(pangImg, rect, 0xffffffff, 0);
		}
		device->SetTextStyle(0);
		device->SetTextColor(0xff000000, 0xffffffff);
	}
}

void FrGhostGameMapDlg::OnDrawAreaBg(int param)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device)
	{
		WRect rect = GetRect();
		device->SetTextStyle(2);
		device->SetTextColor(0xffffffff, 0xff000000);
		device->PrintText(WPoint(rect.x + 45.0f, rect.y + rect.h - 40.0f), 0,
			K2L_Compatibility("\xb0\xed\xbd\xba\xc6\xae \xb5\xa5\xc0\xcc\xc5\xcd\xb4\xc2 \xc0\xd3\xc0\xc7\xb7\xce \xbb\xe8\xc1\xa6 \xb5\xc9 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
			NULL);
		device->SetTextStyle(0);
		device->SetTextColor(0xff000000, 0xff000000);
	}
}

void FrGhostGameMapDlg::OnInitAreaBg(int param)
{
	FrViewer* viewer = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (viewer)
	{
		viewer->Open("ghost_mapdlg_bg.jpg");
		viewer->SetAlpha(0.8f);
	}
}

// ---------------------------------------------------------------- FrGhostTimeBar

IMPLEMENT_OBJECT(FrGhostTimeBar, FrFormBar)
BEGIN_FRESH_MSGMAP(FrGhostTimeBar, FrFormBar)

ON_FRESH_VI("gauge_area", FRCMD_INIT, FrFormBar::OnInitGaugeArea)
ON_FRESH_VV("gauge_area", FRCMD_OWNERDRAW, FrFormBar::OnDrawGaugeArea)
ON_FRESH_VI("mouse_area", FRCMD_INIT, FrGhostTimeBar::OnInitMouseArea)
ON_FRESH_VV("mouse_area", FRCMD_OWNERDRAW, FrGhostTimeBar::OnDrawMouseArea)

END_FRESH_MSGMAP()

FrGhostTimeBar::FrGhostTimeBar()
	: m_gauge(0.0f), m_bTimeOver(false), m_pMouseArea(NULL), m_pMouseImg(NULL)
{
	m_time = 4.0f;
	m_closeTime = 1.0f;
}

FrGhostTimeBar::~FrGhostTimeBar()
{
}

void FrGhostTimeBar::OnInitMouseArea(int param)
{
	m_pMouseArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrGhostTimeBar::OnDrawMouseArea()
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device && m_pMouseImg)
	{
		WRect rect;
		rect.x = m_rect.x + 256.0f;
		rect.y = m_rect.y + 18.0f;
		rect.w = (float)m_pMouseImg->Width();
		rect.h = (float)m_pMouseImg->Height();
		device->DrawTexture(m_pMouseImg, rect, 0xffffffff, 0);
	}
}

void FrGhostTimeBar::OnProc(const float deltaTime)
{
	FrFormBar::OnProc(deltaTime);

	if (m_bTimeOver)
	{
		m_closeTime -= deltaTime;
		if (m_closeTime < 0.0f)
			OnCancel();
		return;
	}

	m_time -= deltaTime;
	if (m_time < 0.0f)
	{
		m_time = 0.0f;
		m_bTimeOver = true;
		m_pMouseImg = g_pFresh->GetBitmap("ghost_bonus_nclick");
		SetPos(0.0f);
		return;
	}

	m_gauge = m_time * 25.0f;
	SetPos(m_gauge);

	unsigned long state = 0;
	if (g_input->GetButton(LEFT_BUTTON) == 3)
		state = 4;
	if (g_input->GetButton(LEFT_BUTTON) == 1)
		state |= 1;
	if (g_input->GetButton(RIGHT_BUTTON) == 3)
		state |= 8;
	if (g_input->GetButton(RIGHT_BUTTON) == 1)
		state |= 2;

	if (state != 0)
	{
		if ((state & 3) == 0)
		{
			SetGaugeImg("formbar_gauge_n");
		}
		else
		{
			g_audio->PlaySfx(MakeStr("pang_coin_%d", rand() % 3 + 1));
			SetGaugeImg("formbar_gauge_o");
		}
		if (state & 4)
		{
			if (state & 8)
				m_pMouseImg = g_pFresh->GetBitmap("ghost_bonus_aclick");
			else
				m_pMouseImg = g_pFresh->GetBitmap("ghost_bonus_lclick");
			return;
		}
		m_pMouseImg = g_pFresh->GetBitmap("ghost_bonus_rclick");
	}
	else
	{
		SetGaugeImg("formbar_gauge_n");
		m_pMouseImg = g_pFresh->GetBitmap("ghost_bonus_nclick");
	}
}

bool FrGhostTimeBar::Open(FRESH_PFN_RESULT pFnResult, unsigned long flag)
{
	SetFixedWnd();
	bool result = FrFormBar::Open(pFnResult, flag);
	WPoint pos(GetGaugeRect().x, GetGaugeRect().y);
	MoveBar(WPoint(pos.x - 47.0f, pos.y - 26.0f));
	return result;
}

IMPLEMENT_OBJECT(FrGhostAward, FrForm)
BEGIN_FRESH_MSGMAP(FrGhostAward, FrForm)

ON_FRESH_VI("text_desc", FRCMD_INIT, FrGhostAward::OnInitDescText)
ON_FRESH_VI("gauge_exp", FRCMD_INIT, FrGhostAward::OnInitGaugeBar)
ON_FRESH_VI("gauge_text", FRCMD_INIT, FrGhostAward::OnInitGaugeText)
ON_FRESH_VI("bg_pouch", FRCMD_INIT, FrGhostAward::OnInitPouchArea)
ON_FRESH_VI("bg_score", FRCMD_OWNERDRAW, FrGhostAward::OnDrawScore)
ON_FRESH_VI("bg_pouch", FRCMD_OWNERDRAW, FrGhostAward::OnDrawPouch)

END_FRESH_MSGMAP()

inline FrGhostAward::cPangFx::cPangFx()
{
}

FrGhostAward::cPangFx::~cPangFx()
{
}

inline bool FrGhostAward::cPangFx::IsAlive()
{
	return m_rect.y <= m_destY;
}

void FrGhostAward::cPangFx::Create(const WRect& rect)
{
	m_pImg = g_pFresh->GetBitmap(MakeStr("ghost_pang_0%d", rand() % 5 + 1));

	float offset =
		(float)(rand() % (int)(rect.w - (float)m_pImg->Width() * 2.5f));

	unsigned long w = m_pImg->Width();
	unsigned long h = m_pImg->Height();
	m_rect = WRect((float)w * 0.8f + rect.x + offset,
		(float)(rand() % 5 - 5) + rect.y, (float)w, (float)h);

	m_speed = (float)(rand() % 50 + 200) * 0.01f;
	m_alpha = 125.0f;
	m_destY = rect.y + 30.0f;
}

inline void FrGhostAward::cPangFx::Proc(float deltaTime)
{
	if (m_alpha < 255.0f)
	{
		m_alpha += deltaTime * 255.0 * 4.0;
		if (m_alpha >= 255.0f)
			m_alpha = 255.0f;
	}

	if (m_rect.y < m_destY)
		m_rect.y += m_speed * (deltaTime * 30.0);
}

inline void FrGhostAward::cPangFx::Render(FrGraphicInterface* device)
{
	if (m_pImg)
		device->DrawTexture(m_pImg, m_rect,
			((unsigned char)m_alpha << 24) | 0xffffff, 0);
}

FrGhostAward::FrGhostAward()
	: m_pTimeBar(NULL),
	  m_pPouchArea(NULL),
	  m_pExpGauge(NULL),
	  m_pDescText(NULL),
	  m_pGaugeText(NULL),
	  m_calcBonusTime(0.0f),
	  m_clickCount(0),
	  m_autoBonus(0),
	  m_clickBonus(10),
	  m_basePang(0),
	  m_shownBasePang(0.0f),
	  m_gainExp(0),
	  m_remainExp(0)
{
	static char* s_font[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_font;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:,";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

FrGhostAward::~FrGhostAward()
{
	if (g_resrcmng != NULL && m_pFont != NULL)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
	std::for_each(m_pangFxList.begin(), m_pangFxList.end(), Delete_Object());
}

void FrGhostAward::Init(bool bUser)
{
	m_dwStyle.Enable(0x100);
	EnableDrag(false);
	SetCaption(K2L_Compatibility("\xb0\xed\xbd\xba\xc6\xae \xb0\xd4\xc0\xd3\xb0\xe1\xb0\xfa"));

	int ghostPang = GOLFDOC()->GetPlayer(1)->totalPang;
	int userPang = GOLFDOC()->GetPlayer(0)->totalPang;

	CGhostGameBonusUtil util;
	bool bWin = util.IsUserWin(GOLFDOC()->GetPlayer(0)->score, userPang,
		GOLFDOC()->GetPlayer(1)->score, ghostPang);

	int difficulty = 1;
	IFF_STRUCT::sCourse* course =
		ItemManager()->FindCourse(GetCurMap() | 0x28000000);
	if (course)
		difficulty = course->Difficulty;

	// number (1..18) of the current hole in the playing order; 19 when it is not found
	unsigned char curHole = 19;
	for (unsigned char i = 0; i < 18; i++)
	{
		if (CSharedDoc::Instance()->m_holeOrder[i] == GOLFDOC()->m_currentHole)
		{
			curHole = i + 1;
			break;
		}
	}
	int bonus =
		util.GetOneClickBonus(curHole, bWin, GOLFDOC()->GetPlayer(0)->score,
			difficulty, CSharedDoc::Instance()->m_userInfo[1].stat.Level,
			CSharedDoc::Instance()->m_userInfo[0].stat.Level);
	m_clickBonus = bonus >= 200 ? 200 : bonus;
	m_basePang = userPang <= ghostPang ? userPang : ghostPang;

	unsigned char level = CSharedDoc::Instance()->m_myInfo.stat.Level;
	std::string desc(K2L_Compatibility(K2L_Compatibility("\xbe\xcb \xbc\xf6 \xbe\xf8\xb4\xc2 \xb7\xb9\xba\xa7")));
	desc = ((sLevelUpDesc983*)CSharedDoc::Instance())->Build();
	m_pDescText->SetCaption(desc.c_str());

	unsigned char hole = 19;
	for (unsigned char j = 0; j < 18; j++)
	{
		if (CSharedDoc::Instance()->m_holeOrder[j] == GOLFDOC()->m_currentHole)
		{
			hole = j + 1;
			break;
		}
	}
	hole--;
	if (hole == 0)
		hole = 1;
	else if (hole > 18)
		hole = 18;
	unsigned char star = CSharedDoc::Instance()->m_golfGame.pCourse
		? CSharedDoc::Instance()->m_golfGame.pCourse->Difficulty
		: 1;
	int index = (unsigned char)(star - 1) * 19 + hole;
	if (S5::CLASSICSRV::IsClassicGame983()) // 983 (0081ff8f): tabela clássica (00b305d0)
		m_gainExp = S5::CLASSICSRV::GetClassicExp983((unsigned char)(star - 1), hole, 2, bUser ? 0 : 1);
	else if (bUser)
		m_gainExp = g_GhostGameExpTable[index][2];
	else
		m_gainExp = g_GhostGameExpTable[index][3];
	m_remainExp = m_gainExp;

	int maxExp = INT_MAX;
	if (level < 71)
		maxExp = s_levelTable983.Get(level).value;
	m_pExpGauge->SetRange(0, maxExp, 0);
	m_pExpGauge->SetPos(CSharedDoc::Instance()->m_myInfo.stat.dwExp);
	m_pGaugeText->SetCaption(MakeStr("%d / %d",
		CSharedDoc::Instance()->m_myInfo.stat.dwExp, maxExp));

	WPoint pos(GetRect().x, g_view->GetHeight() - GetRect().h);
	WPoint base = pos;
	MoveWindow(pos);

	float x = base.x + 180.0f;
	m_ghostResult.pos.x = x;
	m_myResult.pos.x = x;
	m_myResult.pos.y = base.y + 58.0f;
	m_ghostResult.pos.y = base.y + 86.0f;

	if (bUser)
	{
		SetResultInfo(m_myResult, 0);
		SetResultInfo(m_ghostResult, 1);
	}
	else
	{
		SetResultInfo(m_myResult, 1);
		SetResultInfo(m_ghostResult, 0);

		WRect gaugeRect = m_pExpGauge->GetRect();
		gaugeRect.y += 29.0f;
		m_pExpGauge->SetRect(gaugeRect);

		WRect textRect = m_pGaugeText->GetRect();
		textRect.y += 29.0f;
		m_pGaugeText->SetRect(textRect);
	}

	m_pTimeBar = CreateForm<FrGhostTimeBar>(g_pFresh->GetManager(), NULL,
		"FORM_BAR_GHOST", NULL);
	if (m_pTimeBar)
		m_pTimeBar->Open(NULL, 4);
}

void FrGhostAward::SetResultInfo(sResultInfo& info, int index)
{
	info.bGhost = index == 1;
	info.exp = CSharedDoc::Instance()->m_userInfo[index].stat.dwExp;
	info.name = CSharedDoc::Instance()->m_userInfo[index].info.sNick;
	int level = CSharedDoc::Instance()->m_userInfo[index].stat.Level + 1;
	info.levelImg = MakeStr("level_%03d", level < 71 ? level : 71);
	info.score = GOLFDOC()->GetPlayer(index)->score;
}

bool FrGhostAward::IsFinishCalcBonus()
{
	return m_calcBonusTime > 4.0f;
}

void FrGhostAward::OnProc(const float deltaTime)
{
	static float s_pangTime = 0.0f;
	static float s_pangInterval = 0.3f;

	s_pangTime += deltaTime;
	m_calcBonusTime += deltaTime;

	float basePang = (float)m_basePang;
	if (basePang > m_shownBasePang)
	{
		m_shownBasePang += deltaTime * basePang * 0.2f;
		if (m_shownBasePang > basePang)
			m_shownBasePang = basePang;
	}

	if (s_pangTime > s_pangInterval)
	{
		s_pangTime = 0.0f;
		s_pangInterval = (float)(rand() % 2 + 2) * 0.1f;

		if (m_calcBonusTime < 4.0f)
		{
			cPangFx* fx = new cPangFx;
			fx->Create(m_pPouchArea->GetRect());
			m_pangFxList.push_back(fx);
			m_autoBonus += 10;
			g_audio->PlaySfx(MakeStr("pang_coin_%d", rand() % 3 + 1));
		}
	}

	if (m_calcBonusTime < 4.0f)
	{
		if (g_input->GetButton(LEFT_BUTTON) == 1 ||
			g_input->GetButton(RIGHT_BUTTON) == 1)
		{
			m_clickCount++;
			cPangFx* fx = new cPangFx;
			fx->Create(m_pPouchArea->GetRect());
			m_pangFxList.push_back(fx);
		}
	}

	std::list<cPangFx*>::iterator it = m_pangFxList.begin();
	while (it != m_pangFxList.end())
	{
		cPangFx* fx = *it;
		fx->Proc(deltaTime);
		if (!fx->IsAlive())
		{
			delete fx;
			m_pangFxList.erase(it++);
		}
		else
		{
			++it;
		}
	}

	AwardProc(deltaTime);
}

void FrGhostAward::AwardProc(float deltaTime)
{
	if (m_calcBonusTime < 1.0f)
		return;

	if (m_gainExp <= 0)
		return;

	sUserInfo& user = CSharedDoc::Instance()->m_userInfo[0];
	if (user.stat.Level < 65)
	{
		if (user.stat.dwExp < s_levelTable983.Get(user.stat.Level).value)
		{
			if (m_remainExp > 0)
			{
				user.stat.dwExp++;
				CSharedDoc::Instance()->m_myInfo.stat.dwExp++;
				m_remainExp--;
			}
		}
		else
		{
			user.stat.dwExp = 0;
			user.stat.Level++;

			CSharedDoc::Instance()->m_myInfo.stat.dwExp = 0;
			CSharedDoc::Instance()->m_myInfo.stat.Level++;

			m_pExpGauge->SetRange(0, s_levelTable983.Get(user.stat.Level).value, 0);

			if (m_myResult.bGhost)
				m_ghostResult.levelImg = MakeStr("level_%03d",
					CSharedDoc::Instance()->m_myInfo.stat.Level);
			else
				m_myResult.levelImg = MakeStr("level_%03d",
					CSharedDoc::Instance()->m_myInfo.stat.Level);

			AfxGetTask()->GetActor("Player")
				<< MsgObject(NULL, M983(0x1f8), 0, 0, 0, 0, 0);
		}

		m_pExpGauge->SetPos(user.stat.dwExp);
		m_pGaugeText->SetCaption(MakeStr("%d / %d", user.stat.dwExp,
			s_levelTable983.Get(user.stat.Level).value));
	}
	else
	{
		m_gainExp = 0;
	}
}

void FrGhostAward::OnInitDescText(int param)
{
	m_pDescText = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrGhostAward::OnInitGaugeText(int param)
{
	m_pGaugeText = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrGhostAward::OnInitGaugeBar(int param)
{
	m_pExpGauge = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
}

void FrGhostAward::OnInitPouchArea(int param)
{
	m_pPouchArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrGhostAward::OnDrawScore(int param)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device)
	{
		const WRect& rect = GetRect();
		float x = rect.x + 2.0f;
		float y = rect.y + 49.0f;

		device->SetTextStyle(1);

		const WRect& pouchRect = m_pPouchArea->GetRect();
		if (!m_myResult.bGhost)
		{
			WRect box(x, y, 498.0f, 30.0f);
			WOverlay::DrawBox(g_view, box, 0, 0x999b9dff, 0.0f);
			m_pFont->Print(g_view, pouchRect.x + 170.0f, pouchRect.y - 25.0f,
				"-", 0, 0xffffffff, NULL);
		}
		else
		{
			WRect box(x, y + 27.0f, 498.0f, 30.0f);
			WOverlay::DrawBox(g_view, box, 0, 0x999b9dff, 0.0f);
			m_pFont->Print(g_view, pouchRect.x + 170.0f, pouchRect.y - 55.0f,
				"-", 0, 0xffffffff, NULL);
		}

		const Bitmap* myLevel =
			g_pFresh->GetBitmap(m_myResult.levelImg.c_str());
		WRect myLevelRect(m_myResult.pos.x - (float)myLevel->Width() - 90.0f,
			m_myResult.pos.y - 2.0f, (float)myLevel->Width(),
			(float)myLevel->Height());
		device->DrawTexture(myLevel, myLevelRect, 0xffffffff, 0);
		g_pFresh->GetManager()->PrintText(m_myResult.pos, 1,
			m_myResult.name.c_str(), -1.0f, 0xffffffff);
		WPoint myScorePos(m_myResult.pos.x + 100.0f, m_myResult.pos.y);
		g_pFresh->GetManager()->PrintText(myScorePos, 1,
			MakeStr("%d", m_myResult.score), -1.0f, 0xffffffff);

		const Bitmap* ghostLevel =
			g_pFresh->GetBitmap(m_ghostResult.levelImg.c_str());
		WRect ghostLevelRect(m_ghostResult.pos.x - (float)ghostLevel->Width() -
				90.0f,
			m_ghostResult.pos.y - 2.0f, (float)ghostLevel->Width(),
			(float)ghostLevel->Height());
		device->DrawTexture(ghostLevel, ghostLevelRect, 0xffffffff, 0);
		g_pFresh->GetManager()->PrintText(m_ghostResult.pos, 1,
			m_ghostResult.name.c_str(), -1.0f, 0xffffffff);
		WPoint ghostScorePos(m_ghostResult.pos.x + 100.0f, m_ghostResult.pos.y);
		g_pFresh->GetManager()->PrintText(ghostScorePos, 1,
			MakeStr("%d", m_ghostResult.score), -1.0f, 0xffffffff);

		device->SetTextStyle(0);
	}
}

void FrGhostAward::OnDrawPouch(int param)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device)
	{
		const Bitmap* bag2 = g_pFresh->GetBitmap("pang_bag2");
		const Bitmap* bag1 = g_pFresh->GetBitmap("pang_bag1");

		const WRect& rect = m_pPouchArea->GetRect();
		if (bag2)
			device->DrawTexture(bag2, rect, 0xffffffff, 0);

		for (std::list<cPangFx*>::iterator it = m_pangFxList.begin();
			it != m_pangFxList.end(); ++it)
			(*it)->Render(device);

		if (bag1)
			device->DrawTexture(bag1, rect, 0xffffffff, 0);

		std::string basePang = MakeStr("%05d", (int)m_shownBasePang);
		int autoBonus = m_autoBonus < 200 ? m_autoBonus : 200;
		int clickBonus = m_clickBonus < 200 ? m_clickBonus : 200;
		unsigned char clickCount = m_clickCount < 40 ? m_clickCount : 40;
		std::string total = MakeStr("%07d",
			(int)m_shownBasePang + (clickCount * clickBonus + autoBonus));

		m_pFont->Print(g_view, rect.x - 130.0f, rect.y + 24.0f,
			basePang.c_str(), 0, 0xffffffff, NULL);
		m_pFont->Print(g_view, rect.x + 150.0f, rect.y + 24.0f, total.c_str(),
			0, 0xffffffff, NULL);
	}
}
