#pragma once

#include "s5/sharedutilities.h"

struct sLevelTable
{
	char name[16];
	unsigned long exp;
};

// B1589: valores do 983 (008331a0, mapa global 00e78cf0 nivel -> {nome, exp}); o 645 tinha outra curva a partir do nivel 5
static const sLevelTable g_LevelTable[71] = {
	{ "\xb7\xe7\xc5\xb0"
	  "F", 30      },
	{ "\xb7\xe7\xc5\xb0"
	  "E", 40      },
	{ "\xb7\xe7\xc5\xb0"
	  "D", 50      },
	{ "\xb7\xe7\xc5\xb0"
	  "C", 60      },
	{ "\xb7\xe7\xc5\xb0"
	  "B", 70      },
	{ "\xb7\xe7\xc5\xb0"
	  "A", 140     },
	{ "\xba\xf1\xb1\xe2\xb3\xca"
	  "E", 105     },
	{ "\xba\xf1\xb1\xe2\xb3\xca"
	  "D", 125     },
	{ "\xba\xf1\xb1\xe2\xb3\xca"
	  "C", 145     },
	{ "\xba\xf1\xb1\xe2\xb3\xca"
	  "B", 165     },
	{ "\xba\xf1\xb1\xe2\xb3\xca"
	  "A", 330     },
	{ "\xc1\xd6\xb4\xcf\xbe\xee"
	  "E", 248     },
	{ "\xc1\xd6\xb4\xcf\xbe\xee"
	  "D", 278     },
	{ "\xc1\xd6\xb4\xcf\xbe\xee"
	  "C", 308     },
	{ "\xc1\xd6\xb4\xcf\xbe\xee"
	  "B", 338     },
	{ "\xc1\xd6\xb4\xcf\xbe\xee"
	  "A", 675     },
	{ "\xbd\xc3\xb4\xcf\xbe\xee"
	  "E", 506     },
	{ "\xbd\xc3\xb4\xcf\xbe\xee"
	  "D", 546     },
	{ "\xbd\xc3\xb4\xcf\xbe\xee"
	  "C", 586     },
	{ "\xbd\xc3\xb4\xcf\xbe\xee"
	  "B", 626     },
	{ "\xbd\xc3\xb4\xcf\xbe\xee"
	  "A", 1253    },
	{ "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
	  "E", 1002    },
	{ "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
	  "D", 1052    },
	{ "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
	  "C", 1102    },
	{ "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
	  "B", 1152    },
	{ "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
	  "A", 2304    },
	{ "\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce"
	  "E", 1843    },
	{ "\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce"
	  "D", 1903    },
	{ "\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce"
	  "C", 1963    },
	{ "\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce"
	  "B", 2023    },
	{ "\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce"
	  "A", 4046    },
	{ "\xc7\xc1\xb7\xce"
	  "E", 3237    },
	{ "\xc7\xc1\xb7\xce"
	  "D", 3307    },
	{ "\xc7\xc1\xb7\xce"
	  "C", 3377    },
	{ "\xc7\xc1\xb7\xce"
	  "B", 3447    },
	{ "\xc7\xc1\xb7\xce"
	  "A", 6894   },
	{ "\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce"
	  "E", 5515    },
	{ "\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce"
	  "D", 5595    },
	{ "\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce"
	  "C", 5675    },
	{ "\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce"
	  "B", 5755    },
	{ "\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce"
	  "A", 11511   },
	{ "\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce"
	  "E", 8058    },
	{ "\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce"
	  "D", 8148    },
	{ "\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce"
	  "C", 8238    },
	{ "\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce"
	  "B", 8328    },
	{ "\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce"
	  "A", 16655   },
	{ "\xb8\xb6\xbd\xba\xc5\xcd"
	  "E", 8328    },
	{ "\xb8\xb6\xbd\xba\xc5\xcd"
	  "D", 8428   },
	{ "\xb8\xb6\xbd\xba\xc5\xcd"
	  "C", 8528   },
	{ "\xb8\xb6\xbd\xba\xc5\xcd"
	  "B", 8628   },
	{ "\xb8\xb6\xbd\xba\xc5\xcd"
	  "A", 17255   },
	{ "\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd"
	  "E", 9490   },
	{ "\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd"
	  "D", 9690   },
	{ "\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd"
	  "C", 9890   },
	{ "\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd"
	  "B", 10090   },
	{ "\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd"
	  "A", 20181   },
	{ "\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd"
	  "E", 20181   },
	{ "\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd"
	  "D", 20481   },
	{ "\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd"
	  "C", 20781   },
	{ "\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd"
	  "B", 21081   },
	{ "\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd"
	  "A", 42161   },
	{ "\xb7\xb9\xc0\xfc\xb5\xe5"
	  "E", 37945  },
	{ "\xb7\xb9\xc0\xfc\xb5\xe5"
	  "D", 68301  },
	{ "\xb7\xb9\xc0\xfc\xb5\xe5"
	  "C", 122942  },
	{ "\xb7\xb9\xc0\xfc\xb5\xe5"
	  "B", 221296  },
	{ "\xb7\xb9\xc0\xfc\xb5\xe5"
	  "A", 442592 },
	{ "\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5"
	  "E", 663887 },
	{ "\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5"
	  "D", 995831 },
	{ "\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5"
	  "C", 1493747 },
	{ "\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5"
	  "B", 2240620 },
	{ "\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5"
	  "A", 0xffffffff },
};

#ifdef REBANG_GOLFRULE_PCH
#ifndef RB_HOLEINBONUS_LINKAGE
#define RB_HOLEINBONUS_LINKAGE static
#endif
// HACK: golfrule.cpp exports it (extern "C"): voiceitem.obj still references the unit's data blob by this name
RB_HOLEINBONUS_LINKAGE sHoleInBonus g_HoleInBonusTable[20] = {
#else
static sHoleInBonus g_HoleInBonusTable[20] = {
#endif
	{ 50,  50,  30,  12,  3,  0 },
	{ 100, 100, 60,  30,  11, 0 },
	{ 110, 110, 65,  32,  12, 0 },
	{ 180, 180, 98,  59,  20, 0 },
	{ 150, 150, 85,  43,  16, 0 },
	{ 55,  55,  36,  17,  5,  0 },
	{ 100, 100, 60,  30,  11, 0 },
	{ 150, 150, 85,  43,  16, 0 },
	{ 80,  80,  52,  26,  9,  0 },
	{ 110, 110, 65,  32,  12, 0 },
	{ 80,  80,  52,  26,  9,  0 },
	{ 50,  50,  30,  12,  3,  0 },
	{ 0,   0,   0,   0,   0,  0 },
	{ 180, 180, 98,  59,  20, 0 },
	{ 50,  50,  30,  12,  3,  0 },
	{ 50,  50,  30,  12,  3,  0 },
	{ 80,  80,  52,  26,  9,  0 },
	{ 800, 800, 520, 260, 90, 0 },
	{ 0,   0,   0,   0,   0,  0 },
	{ 80,  80,  52,  26,  9,  0 },
};

inline bool IsLicenseLimitedItem(unsigned long tid)
{
	// 983 00b349e0: 42 bolas sem reposicao no Supply Pack (o 645 tinha so 0x14000004)
	const int LICENSEITEMSIZE = 42;

	static const unsigned long aLicenseLimitedItem[LICENSEITEMSIZE] = {
		0x14000004, 0x1400000b, 0x1400001c, 0x1400001d, 0x14000023, 0x14000024,
		0x14000026, 0x14000027, 0x14000028, 0x14000029, 0x1400002a, 0x1400002b,
		0x14000044, 0x14000045, 0x14000046, 0x14000053, 0x14000054, 0x14000050,
		0x1400008f, 0x14000090, 0x140000d6, 0x140000b7, 0x140000ba, 0x140000bb,
		0x140000bd, 0x140000be, 0x140000c0, 0x140000c1, 0x140000c2, 0x140000c3,
		0x140000c4, 0x140000c6, 0x140000c7, 0x140000c8, 0x140000cb, 0x140000cd,
		0x140000ce, 0x140000cf, 0x140000d0, 0x140000d1, 0x140000d8, 0x140000dd,
	};

	for (int i = 0; i < LICENSEITEMSIZE; i++)
	{
		if (aLicenseLimitedItem[i] == tid)
			return true;
	}

	return false;
}
