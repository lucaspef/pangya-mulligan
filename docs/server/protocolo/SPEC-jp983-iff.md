# Layout das tabelas do pangya_jp.iff (cliente JP 983) para o servidor

Fonte: `data/pangya_jp.iff` (zip). Cada tabela: `u16 n, u16 bind, u32 versão (=13 em todas)`, depois `n` registros de tamanho fixo.
Todos os offsets abaixo são **hex, relativos ao início do registro**, little-endian. Conferidos com python sobre o arquivo real
(scripts em scripts de conferência da pesquisa) e cruzados com `data/pangya.iff` (KR 645) nos typeids que existem nos dois.
Referência de nomes: PangyaAPI.IFF.JP (Models/General, Models/Data). `[?]` = incerto / não confirmado por código do cliente.

Observações gerais:
- O texto no arquivo é misto: a maioria dos nomes de item está **em inglês** (arquivo traduzido); Match.iff, GrandPrixData,
  GrandPrixConditionEquip e Achievement (parte) estão em **Shift-JIS (cp932)**. Ler sempre como cp932 (ASCII é subconjunto).
  Alguns nomes viraram literal `???` (0x3F) na tradução (ex.: TimeLimitItem, Achievement 0x4c000001).
- Campos `bool` do JP são `u8 + 3 de pad` (ou `u32` 0/1) — ler o byte 0 é equivalente ao `Final` do KR.
- Faltam no zip (o servidor JP da comunidade lê de arquivos .sff separados): Title.iff, ChildItem.iff, MemorialShopNormalItem.sff,
  ClubSetWorkShopChange*.sff, AttendancePrize*.sff, BongdariShop*, ScratchShop*, RandomBox.sff. `ScratchRewardSetting.iff` vem com n=0.

## 0. Tamanhos de registro (conferido: (len-8)/n)

| tabela | n | bind | registro JP | KR 645 (Structs.g.cs) | tsv 983 |
|---|---|---|---|---|---|
| Character.iff | 14 | 0 | 0x1A4 (420) | sChar 372 | 0x1a4 ok |
| Part.iff | 9587 | 0 | 0x238 (568) | sPart 516 | 0x238 ok |
| Club.iff | 488 | 0 | 0xF4 (244) | sClub 196 | ok |
| ClubSet.iff | 126 | 0 | 0x104 (260) | sClubSet 180 | ok |
| Ball.iff | 150 | 0 | 0x330 (816) | sBall 764 | ok |
| Item.iff | 643 | 0 | 0xF8 (248) | sItem 200 | ok |
| Caddie.iff | 43 | 0 | 0xF8 (248) | sCaddie 200 | ok |
| CaddieItem.iff | 110 | 0 | 0x11C (284) | sCadItem 236 | ok |
| SetItem.iff | 1088 | 0 | 0x10C (268) | sSetItem 220 | ok |
| Course.iff | 22 | 0 | 0x1D0 (464) | sCourse 312 | ok |
| Match.iff | 424 | 0 | 0x14C (332) | sMatch 332 (igual) | ok |
| Enchant.iff | 175 | 0 | 0x10 (16) | sEnchant 16 (igual) | ok |
| Skin.iff | 1476 | 0 | 0xF4 (244) | sSkin 196 | ok |
| HairStyle.iff | 111 | 0 | 0xC4 (196) | sHairStyle 148 | ok |
| Mascot.iff | 69 | 0 | 0x130 (304) | sMascot 252 | ok |
| AuxPart.iff | 42 | 0 | 0xE4 (228) | sAuxPart 176 | ok |
| Card.iff | 264 | 124 | 0x180 (384) | sCard 328 | ok |
| Furniture.iff | 55 | 0 | 0x200 (512) | sFurniture 464 | ok |
| Desc.iff | 9851 | 0 | 0x204 (516) | sDesc 516 (igual) | ok |
| CadieMagicBox.iff | 2056 | 28749 | **0x8C (140)** | sCadieMagicBox 104 | 0x8c (nosso loader 0x70: DIFERE) |
| CadieMagicBoxRandom.iff | 179 | 57 | 0x10 (16) | sRandomRecycle 16 (igual) | não medido -> 0x10 |
| SpecialPrizeItem.iff | 42 | 57 | 0x0C (12) | sSpecialPrizeItem 12 (igual) | ok |
| NonVisibleItemTable.iff | 104 | 57 | 0x2C (44) | sNonVisibleItem 44 (igual) | ok |
| SubscriptionItemTable.iff | 2 | 57 | 0x2C (44) | sSubscriptionItem 44 | ok |
| CounterItem.iff | 196 | 0 | 0x118 (280) | — | ok |
| Achievement.iff | 201 | 0 | 0x600 (1536) | — | ok |
| QuestStuff.iff | 1862 | 0 | 0x10C (268) | — | ok |
| QuestItem.iff | 257 | 0 | 0x110 (272) | — | ok |
| LevelUpPrizeItem.iff | 71 | 24 | 0xC0 (192) | — | (tsv: não medido) |
| MemorialShopRareItem.iff | 705 | 68 | 0x5C (92) | — | ok |
| MemorialShopCoinItem.sff (no zip) | 14 | 65 | 0x40 (64) | — | ok |
| GrandPrixData.iff | 741 | 0 | 0x330 (816) | — | ok |
| GrandPrixSpecialHole.iff | 84 | 57 | 0x14 (20) | — | ok |
| GrandPrixConditionEquip.iff | 2 | 57 | 0x210 (528) | — | ok |
| GrandPrixRankReward.iff | 781 | 24 | 0x4C (76) | — | ok |
| GrandPrixAIOptionalData.sff (no zip) | 990 | 57 | 0x25C (604) | — | ok |
| ClubSetWorkShopLevelUpProb.iff | 5 | 57 | 0x18 (24) | — | ok |
| ClubSetWorkShopLevelUpLimit.iff | 30 | 57 | 0x14 (20) | — | ok |
| ClubSetWorkShopRankUpExp.iff | 4 | 57 | 0x1C (28) | — | ok |
| CharacterMastery.iff | 140 | 57 | 0x3C (60) | — | ok |
| ShopLimitItem.iff | 3 | 57 | 0x3C (60) | — | ok |
| PointShop.iff | 12 | 57 | 0x14 (20) | — | ok |
| TimeLimitItem.iff | 6 | 57 | 0x64 (100) | — | (não está no tsv) |
| SetEffectTable.iff | 46 | 57 | 0x4C (76) | — | ok |
| TwinsItemTable.iff | 505 | 57 | 0x1C (28) | — | ok |
| HoleCupDropItem.iff | 87 | 57 | 0x2C (44) | — | ok |
| AddonPart.iff | 64 | 57 | 0x120 | — | ok |
| Ability.iff | 920 | 57 | 0x50 | — | ok |
| ErrorCodeInfo.iff | 1111 | 65 | 0x110 | — | ok |
| ArtifactManaInfo.iff | 27 | 65 | 0x98 | — | ok |
| CutinInfomation.iff | 641 | 57 | 0xD0 | — | ok |
| CaddieVoiceTable.iff | 550 | 57 | 0x86 | — | ok |
| Tiki*.iff / FurnitureAbility.iff | — | 57 | 0x34/0x30/0x3C/0x3C | iguais ao KR | ok |

## 1. IFF_ITEM_COMMON JP (0xC0 = 192 bytes; KR = 144)

Diferença para o KR: nome 40 -> **64** bytes (+24) e o `u32 Point` do KR virou um bloco de 24 bytes ("tiki shop") + `u32` antes das datas (+24).
Total +48. (O tsv compara 983 x 839, que já tinha o nome de 64; daí "+0x18".)

| offset | tipo | campo | KR equivalente (offset KR) | conferido (exemplo) |
|---|---|---|---|---|
| 0x00 | u8 (+3 pad) | Final / Active | Final (0) | 1 em todos os registros ativos |
| 0x04 | u32 | TypeId | TypeId (4) | Part 0x08000400 "Basic Head (N)" |
| 0x08 | char[64] cp932 | Name | Name char[40] (8) | Item 0x18000000 "Spin Mastery"; Match "アマチュア6級" |
| 0x48 | u8 | Level (bits 0-6) / IsUnderLvl (bit 7) | Bits_Level (48) | Caddie Pippin = 6; Part 0x8F (bit7 + 15) |
| 0x49 | char[40] | Icon | Icon (49) | Card "card_icon_char_00"; Item "item0_00" |
| 0x71 | 3 bytes | pad (sempre 0) | pad (89) | 000000 em todos |
| 0x74 | u32 | Price | Price (92) | Cowboy Hat 0x8000800 = 1100 (igual KR); Spin Mastery = 120 |
| 0x78 | u32 | SalePrice (desconto) | SalePrice (96) | 0 na maioria |
| 0x7C | u32 | UsedPrice | UsedPrice (100) | 0 |
| 0x80 | u8 | bit0 IsCash, bits1-4 IsSalable, bits5-7 InStock | Bits_IsCash (104), **mesmo bitfield** | 0x22 = pang, salable 1, em loja (Cowboy Hat, igual KR); 0x21 = cash (Blue Cowboy Hat 42 cookies); 0x26 = salable 3 (Blank Top Dupe); 0x60/0xC0 = InStock 3/6 |
| 0x81 | u8 | bit0 New, bit1 Hit, bit2 Reserve (resto [?]) | Bits_New (105) | 0x01 Pumpkin Head; 0x02 Soccer Shoes; 0x15 em Premium Ticket [?] |
| 0x82 | u8 | TimeFlag | TimeFlag (106) | Skin 1, Mascot 1, CaddieItem 1 |
| 0x83 | u8 | Time (dias; 365 vira 109 = 0x16D&0xFF) | Time (107) | Skin 7 / 30 / 109 |
| 0x84 | u32 | Tiki_Qnt_Pts [?] | — (KR: Point u32 em 108 [?]) | Item 0/1/42; Card 1/2 |
| 0x88 | u32 | Tiki_Pts [?] | — | Card 1/2/5; Part 0/3/4 |
| 0x8C | u16 | Mileage_Pts [?] | — | Item Spin Mastery 2; Card Nuri N 500 |
| 0x8E | u16 | Bonus_Prob [?] | — | 100 |
| 0x90 | i16[2] | Bonus min/max [?] | — | 1/5 (item), 250/500 (card) |
| 0x94 | u32 | Type_TikiShop (0..3) [?] | — | Part: 3/2/0/1 |
| 0x98 | u32 | Tiki_Pang [?] | — | 200 ou 0 |
| 0x9C | u32 | flag de data ativa [?] | — | **0 em todas** as tabelas, mesmo com datas preenchidas |
| 0xA0 | SYSTEMTIME | saleStart | saleStart (112) | Part 0x80440b6 "Mitakihara School Uniform" 2013-12-12 11:00 |
| 0xB0 | SYSTEMTIME | saleEnd | saleEnd (128) | idem 2014-02-13 11:00 |

Uso pelo servidor (`Common()`/`CanTrade`): Price 0x74, SalePrice 0x78, IsCash = `b[0x80]&1`, IsSalable = `(b[0x80]>>1)&15` (1 ou 3 = troca), InStock = `b[0x80]>>5`.
Comparação KR x JP nos typeids comuns: Part 1282/4840 com preço+flags idênticos (o resto mudou de valor, não de layout).

## 2. Tabelas que o Kr645GameData usa

Base = 0xC0 (fim do common). Coluna "KR" = campo/offset de Structs.g.cs.

### Character.iff (0x1A4) — PlayStats, Exists, Shop

| offset | tipo | campo | KR equivalente | conferido |
|---|---|---|---|---|
| 0x00 | common | | c | Nuri 0x04000000 |
| 0xC0 | char[40] | Data (modelo) | Data (144) | "m_def" |
| 0xE8 | char[40] | HairTex | HairTex (184) | "2#m_hair_01-01.jpg" |
| 0x110 | char[40] | ShirtsTex | ShirtsTex (224) | "m_shirts_00-01.jpg" |
| 0x138 | char[40] | FaceTex | FaceTex (264) | "m_skin_01-01.jpg" |
| 0x160 | i16[5] | Attr (teto P/C/I/S/Cv) | Attr (304) | Nuri [9,11,6,2,2] — 10/10 iguais ao KR |
| 0x16A | u8 | nParts | nParts (314) | 9 |
| 0x16B | u8 | nAcsries | nAcsries (315) | 12 |
| 0x16C | u8 (+3 pad) | ClubType | ClubType (316) | 1 |
| 0x170 | f32 | ClubScale | ClubScale (320) | 1.0 |
| 0x174 | u8[5] | PCL (nível base) | PCL (324) | [9,11,6,2,2] igual KR |
| 0x179 | char[43] | Camera / Mtn30sWinner [?] | Mtn30sWinner char[40] (329) | texto cp932 |

### Part.iff (0x238) — Exists, DefaultParts/IsDefaultPart, UccPart, PlayStats, CanTrade, Shop

| offset | tipo | campo | KR equivalente | conferido |
|---|---|---|---|---|
| 0xC0 | char[40] | Data (MPet) | Data (144) | "m_ha_00_!hb" |
| 0xE8 | **u32** | Category (tipo) | Category **u8** (184) | dist.: 5:3587, 2:1648, 0:1625, 4:1309, 3:701, 1:594, **9:70, 8:35**, 6:18 (nenhum 7) |
| 0xEC | u32 | PosMask | PosMask (188) | 0x3 basic head; 0x8 camisa |
| 0xF0 | u32 | HideMask | HideMask (192) | 0 |
| 0xF4 | char[40]×3 | Tex[0..2] (0xF4, 0x11C, 0x144) | Tex (196) | Blank Top 1 0x800602f: "M_TS_u01f-01.jpg","M_TS_u01b-01.jpg","" |
| 0x16C | char[40]×3 | OrgTex[0..2] (0x16C, 0x194, 0x1BC) | OrgTex (316) | idem |
| 0x1E4 | i16[5] | Attr | Attr (436) | Blank Top 1 [1,0,0,0,0] |
| 0x1EE | i16[5] | Slot | Slot (446) | Cat Tee [2,0,0,0,0] — 680/955 iguais ao KR |
| 0x1F8 | char[40] | EquippableWith | EquippableWith (456) | "" |
| 0x220 | u32[2] | SubPart | SubPart (496) | 0 |
| 0x228 | u16 | CharacterSlot (cards) | CharacterSlot i16 (504) | 2 registros com 1 |
| 0x22A | u16 | CaddieSlot | CaddieSlot (506) | |
| 0x22C | u16 | NPCSlot (novo) | — | 0 em todos |
| 0x22E | u16 | Point [?] | Point (514) [?] | 0 em todos |
| 0x230 | u32 | RentalPrice | RentalPrice (508) | Blank Top 1 = 12000 |
| 0x234 | u32 | desconhecido [?] (KR tinha Beginner u8 em 512) | Beginner? [?] | 0 (9497) / 1 (90) |

- UCC: o KR usa categoria 7/8/9; no JP só aparecem **8 e 9** (8 = "Blank Top 1", 9 = "Blank Top 1 Dupe"). Chave de UccInfo = Data(0xC0)+Tex(0xF4..)+OrgTex(0x16C..), mesmo esquema.
- DefaultParts: `0x08000400 | idx<<18 | slot<<13` existe (0x08000400 "Basic Head (N)", 0x08000600 "Basic Assisting Head (N)").

### Club.iff (0xF4) — Exists, Shop

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0xC0 | char[40] | Data | Data (144) | "club01_01_w" (0x0C000000 Air Knight Wood) |
| 0xE8 | u16 | Type | Type u8 (184) | 0 |
| 0xEA | i16[5] | COM / stats | COM (186) | [0,0,0,0,0] |

### ClubSet.iff (0x104) — PlayStats, Shop (UpgradeSlots), CanTrade

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0xC0 | u32[4] | ClubIds (W/I/We/P) | ClubIds (144) | Air Knight Set [0xc000000..0xc000003] igual KR |
| 0xD0 | i16[5] | Attr | Attr (160) | [6,6,4,3,3] igual KR |
| 0xDA | i16[5] | Slot | Slot (170) | [8,9,8,3,3] igual KR |
| 0xE4 | i32 | WorkShop.tipo (-1 = não evolui) | — | -1 (Air Knight), 0/1 |
| 0xE8 | u32 | WorkShop.rank_s_stat [?] | — | 0 |
| 0xEC | u32 | WorkShop.total_recovery [?] | — | 0 / 5 |
| 0xF0 | f32 | WorkShop.rate | — | 1.5 / 1.0 |
| 0xF4 | u32 | WorkShop.tipo_rank_s [?] | — | 0 |
| 0xF8 | u32 | WorkShop.flag_transformar [?] | — | 0 / 1 |
| 0xFC | u32 | desconhecido [?] | — | 0 |
| 0x100 | u32 | text_pangya [?] | — | 0 |

### Ball.iff (0x330) — Exists, CanTrade

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0xC0 | u32 | desconhecido [?] | — (novo) | 0 |
| 0xC4 | char[40] | Data | Data (144) | "ball_b" (0x14000001 Bomber Comet) |
| 0xEC | u32 | Bound | Bound (184) | 1 |
| 0xF0 | u32 | Roll | Roll (188) | 1 |
| 0xF4 | char[40]×14 | Fx[7] + FxBone[7] | Fx/FxBone (192) | "BFX_B0_FX01.seq" |
| 0x324 | i16[5] | COM | COM (752) | [100,0,0,0,0] |
| 0x32E | u16 | Point [?] | — | 0 |

### Item.iff (0xF8) — Exists, CanTrade, Shop (PackSize)

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0xC0 | u32 | Bits_RandomBox / ItemType | Bits_RandomBox (144) | 0 |
| 0xC4 | char[40] | Data | Data (148) | "item0_00" |
| 0xEC | i16[5] | COM (COM[0] = PackSize) | COM (188) | Silent Wind 0x18000006 COM[0]=10 (igual KR) — 82/100 iguais |
| 0xF6 | i16 | Point | Point (198) | 0 |

### Caddie.iff (0xF8) — PlayStats (nível + Attr), Shop

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0x48 | u8 | Level (no common) | c.Level | Pippin 6, Titan Boo 18 (igual KR) |
| 0xC0 | u32 | MonthlyFee | MonthlyFee (144) | Pippin 4100 (igual KR) |
| 0xC4 | char[40] | Data | Data (148) | "pipin" |
| 0xEC | i16[5] | Attr | Attr (188) | Titan Boo [2,2,0,0,0] — 21/24 iguais |
| 0xF6 | u16 | Point [?] | — | 0 |

### CaddieItem.iff (0x11C) — Shop (PeriodPrices)

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0xC0 | char[40] | FaceTex | FaceTex (144) | "" |
| 0xE8 | char[40] | BodyTex | BodyTex (184) | "Caddie_Gift_Bon_001.jpg" |
| 0x110 | u16[4] | COM[0..3] | COM[0..3] (224) | **JP = [1,7,15,30]** (dias?) ; KR = [2,0,12,22] (preços por período) |
| 0x118 | u32 | COM[4] / unit_power_gauge_start [?] | COM[4] (232) | 23/26/29/33/36/38 (KR 33) |

**Atenção [?]**: no JP o COM[0..3] de CaddieItem e Mascot traz `1,7,15,30` em todos os itens à venda — parecem ser as durações
oferecidas e não os preços por período do KR. Antes de usar como `PeriodPrices`, conferir no 983 o equivalente de
`CItemManager::GetItemSalePrice`.

### SetItem.iff (0x10C) — Exists, Shop (SetElements)

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0xC0 | **u32** | nElems | nElems **u8** (144) | "Greenline Swimset (N)" 3; "Beach set (with Nuri)" 4 |
| 0xC4 | u32[10] | ElemIds | ElemIds (148) | [0x8006010,0x801000a,0x800080a] |
| 0xEC | u16[10] | ElemNum (0 = 1) | ElemNum i16 (188) | 0x24dfe004: [18,18,18] |
| 0x100 | i16[5] | COM | COM (208) | 0 |
| 0x10A | u16 | Point [?] | — | 0 |

### Course.iff (0x1D0) — Courses, CourseStars

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0x00 | u8 | Final | c.Final | 1 nos 22 registros |
| 0x04 | u32 | TypeId = 0x28000000 \| mapa | c.TypeId | 0x28000000..0x15, 0x28000040 (GRAND ZODIAC); falta 0x0C |
| 0xC0 | char[40] | Data | Data (144) | "blue" |
| 0xE8 | char[40] | AmbientSnd | AmbientSnd (184) | "amb_sea" |
| 0x110 | u8 | bits0-3 Difficulty (estrelas), bits4-7 DiffFlag | Bits_Difficulty (224) | Blue Lagoon 0x11 (1★), Blue Water 0x23 (3★), Deep Inferno 0x15 (5★) |
| 0x111 | char[43] | TexProp (xml) | TexProp char[40] (225) | "blue_property.xml" |
| 0x13C | f32 | Slope / RatePang | Slope (268) | 1.1 (Wind Hill 1.3) |
| 0x140 | char[40] | SkyFx / Seq | SkyFx (272) | |
| 0x168 | u32[12] | desconhecido [?] | — | |
| 0x198 | i8[18] | Par por buraco | — | Blue Lagoon [4,3,4,4,5,3,5,4,4,3,4,5,4,4,5,3,4,4] |
| 0x1AA | u8[18] | Min score por buraco | — | |
| 0x1BC | u8[18] | Max score por buraco | — | |
| 0x1CE | u16 | desconhecido [?] | — | |

### Match.iff (0x14C) — troféus (o servidor usa typeids fixos em Trophy.cs; layout idêntico ao KR)

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0x00 | u8/u32 | Final | Final (0) | 1 |
| 0x04 | u32 | TypeId | TypeId (4) | 0x2C000000, 0x2C010000 |
| 0x08 | char[80] cp932 | Name | Name (8) | "アマチュア6級", "アマチュア5級" |
| 0x58 | u8 | Level | Level (88) | 10 |
| 0x59 | char[40]×6 | Icon[6] | Icon (89) | "a6_1" |

### Enchant.iff (0x10) — UpgradePrice (idêntico ao KR)

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0x00 | u8/u32 | Final | Final | 1 |
| 0x04 | u32 | TypeId = 0x34000000 \| stat<<20 \| nível | TypeId | 0x34000000 |
| 0x08 | u32 | ReqdPang | ReqdPang | 0x34000000 = 2100; 0x34000001 = 4200; 0x3440001E (curva nv 30) = 69600 |
| 0x0C | u8 | ReqdLevel | ReqdLevel | 0 |

### Skin.iff (0xF4) — Shop (PeriodPrices, UnitPricedSkin)

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0xC0 | char[40] | Tex | Tex (144) | "skin_001" |
| 0xE8 | u8 | HScroll | HScroll (184) | 0 |
| 0xE9 | u8 | VScroll | VScroll (185) | 0 |
| 0xEA | i16[5] | COM (preços por período) | COM (186) | 0x38000003 JP [0,0,0,22000,30000] / KR [0,0,0,20000,30000] — 87/164 iguais |

### HairStyle.iff (0xC4) — Shop

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0xC0 | u8 | cHairID (cor) | cHairID (144) | 0x3C000002 "Raven Black Dye(N)" = 1 (63/70 iguais) |
| 0xC1 | u8 | cCharID | cCharID (145) | 0 |
| 0xC2 | u16 | pad | — | 0 |

### Mascot.iff (0x130) — Shop

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0xC0 | char[40] | Data | Data (144) | "03devil_mozzi" |
| 0xE8 | char[40] | MsgTex | MsgTex (184) | |
| 0x110 | u8[5] | COM | COM (224) | **JP [1,7,15,30,0]** (dias? ver CaddieItem) / KR [6,19,0,39,0] |
| 0x115 | u8[5] | Attr | Attr (229) | 0 |
| 0x11A | i8 (+pad) | DriveUp | DriveUp (234) | Wikky (P) 0x40000014 = 2 |
| 0x11C | u16 | ItemDropUp | (236) | |
| 0x11E | u16 | ComboUp | (238) | |
| 0x120 | u16 | PangUp | (240) | Cocoa 0x40000002 = 120 (igual KR) |
| 0x122 | u16 | ExpUp | (242) | Puff = 120 (igual KR) |
| 0x124 | u8 | ItemSlot | (244) | Billy = 1 (igual KR) |
| 0x125 | u8 | ShowMsg | (245) | |
| 0x128 | u32 | MsgFee | (248) | |
| 0x12C | u16 | BonusPang (novo) [?] | — | 10 |
| 0x12E | u16 | BonusFlag (novo) [?] | — | 0 |

### AuxPart.iff (0xE4) — PlayStats (anéis), Shop

| offset | tipo | campo | KR | conferido (22/22 iguais ao KR) |
|---|---|---|---|---|
| 0xC0 | i16[5] | COM | COM (144) | [100,0,0,0,0] |
| 0xCA | u8[5] | Attr | Attr (154) | Impact Ring 0x70000002 [0,0,2,0,0] |
| 0xCF | u8[5] | Slot | Slot (159) | 0 |
| 0xD4 | i8 (+pad) | DriveUp | DriveUp (164) | 0 |
| 0xD6 | u16 | ItemDropUp | (166) | 0x70000005 = 120 |
| 0xD8 | u16 | ComboUp | (168) | 0x70000006 = 110 |
| 0xDA | u16 | PangUp | (170) | |
| 0xDC | u16 | ExpUp | (172) | |
| 0xDE | u16 | ItemSlot (novo) [?] | — | |
| 0xE0 | u16 | BonusPang (novo) [?] | — | |
| 0xE2 | u16 | BonusFlag (novo) [?] | — | |

### Card.iff (0x180) — Cards, Shop

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0x00 | u8 | Final | c.Final | 1 |
| 0xC0 | u8 | RareType (0 N, 1 R, 2 SR, 3 SC) | RareType (144) | Nuri (SC) 0x7C000002 = 3; dist. 0:109 1:66 2:46 3:42 |
| 0xC1 | char[40] | Image | Image (145) | "card_char_00" |
| 0xEA | i16[5] | COM (stats do card de personagem) | COM (186) | Nuri (SR) [0,2,0,1,0] igual KR |
| 0xF4 | u16 | Avility (efeito) | Avility (196) | Nuri (SR) 1 (igual KR); N/R do JP vêm 0 (KR 2..5) |
| 0xF6 | u16 | AvilityValue | AvilityValue (198) | Nuri (SR) 2 |
| 0xF8 | char[40] | SubIcon | SubIcon (200) | "card_sub_char_04" |
| 0x120 | char[40] | SlotImg | SlotImg (240) | "card_char_slot_04" |
| 0x148 | char[40] | BuffImg | BuffImg (280) | |
| 0x170 | u16 | UseTime (min) | UseTime (320) | 0x7C800003 = 120 |
| 0x172 | u16 | Volume | Volume (322) | 1..5 |
| 0x174 | u32 | CardIndex / Position | CardIndex u16 (324) | Nuri (SR) 4 (igual KR) |
| 0x178 | u32 | flag1 [?] | — | 2, 10, 14 |
| 0x17C | u32 | flag2 [?] | — | 1 |

### Furniture.iff (0x200) — Shop (Position)

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0xC0 | char[40] | Data | Data (144) | "wardrobe_a" |
| 0xE8 | u16 | Num | Num (184) | 1 |
| 0xEA | u16 | IsOwn | (186) | 0 |
| 0xEC | u16 | IsMove | (188) | 2 |
| 0xEE | u16 | IsFunction | (190) | 1 / 0 |
| 0xF0 | i16 (+pad) / i32 | Etc | Etc (192) | 0 |
| 0xF4 | f32[4] | Pos (x,y,z,r) | Pos (196) | **só x vem preenchido no JP** (0x48001817 x=20.36, y=z=r=0; KR 20.36,1.0,4.81,270) [?] |
| 0x104 | char[40]×3 | Tex | Tex (212) | vazio |
| 0x17C | char[40]×3 | OrgTex | OrgTex (332) | vazio |
| 0x1F4 | i16[5] | COM | COM (452) | |
| 0x1FE | u16 | UseTime | UseTime (462) | |

### SpecialPrizeItem.iff (0x0C) — SpItems (idêntico ao KR)

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0x00 | u32 | typeId | typeId | 0x08000848 |
| 0x04 | u32 | ability (0..4) | ability | 1, 4, 2 |
| 0x08 | f32 | rate | rate | 2.0 |

### CadieMagicBox.iff (0x8C) — MagicBox (DIFERE do KR 104)

| offset | tipo | campo | KR equivalente | conferido |
|---|---|---|---|---|
| 0x00 | u32 | uiNumber (1..2056, contíguo) | uiNumber (0) | 1..2056 sem buracos |
| 0x04 | u32 | bFinal | bFinal u8 (4) | 1 em todos |
| 0x08 | u32 | uiCategory | uiCategory (8) | 3:1986, 4:31, 0:26 |
| 0x0C | i32 | iCharacter (-1/-2 = todos [?]) | iCharacter (12) | -2, -1, 1, 3, 6, 0xC |
| 0x10 | i32 | iLevel | iLevel (16) | 0 em 2050; 15, 6, 21, 18 |
| 0x14 | u32 | **novo** [?] (1/2 nas trocas de par) | — | 0 (2052), 1, 2 ("Kat Sea Hair (A)" = 1, "Ket Sea's ear (A)" = 2) |
| 0x18 | u32 | uiOutput | uiOutput (20) | 0x1A000133 "Wheel of Time"; 0x0819C086 |
| 0x1C | u32 | uiOutputCount | uiOutputCount (24) | 1 (2042), 10, 2, 5 |
| 0x20 | u32[4] | uiElem | uiElem (28) | [0x1A00005B,0x1A00005C,0x1A00005D,0x1A00005E] |
| 0x30 | u32[4] | uiElemCount | uiElemCount (44) | [1,1,1,1]; Lirim Horn 50 |
| 0x40 | u32 | uiRandSeq | uiRandSeq (60) | **0 nos 2056** (CadieMagicBoxRandom tem 17 grupos: 2..22, 101, não usados aqui) [?] |
| 0x44 | char[40] | szRandName / nome | szRandName (64) | "Wheel of Time", "Bot Ticket" |
| 0x6C | SYSTEMTIME | início (novo) | — | todo zero |
| 0x7C | SYSTEMTIME | fim (novo) | — | todo zero |

### CadieMagicBoxRandom.iff (0x10) — MagicBoxRandom (idêntico ao KR)

| offset | tipo | campo | KR | conferido |
|---|---|---|---|---|
| 0x00 | u32 | uiRandSeq | uiRandSeq | 2 |
| 0x04 | u32 | uiTypeId | uiTypeId | 0x08010032 |
| 0x08 | u32 | uiCount | uiCount | 1 |
| 0x0C | u32 | uiProbs | uiProbs | 112, 111 |

### Exists — tabelas que entram no conjunto de typeids
Character, Part, Club, ClubSet, Ball, Item, Caddie, CaddieItem, SetItem, Skin, HairStyle, Mascot, AuxPart, Card, Furniture:
todas com TypeId em 0x04 (common). Para o JP vale incluir também CounterItem (0x6C......), QuestItem (0x78......),
QuestStuff (0x74......), Achievement (0x4C......), que têm o mesmo common.

## 3. Tabelas novas do JP (lojas e sistemas)

### CounterItem.iff (0x118) — contadores de conquista
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | common (0xC0) | TypeId 0x6C...... | 0x6C000001 "Achievement Point", 0x6C400001 "Game Counter", 0x6C400065 "HI by Back spin Counter" |
| 0xC0 | u8[88] | desconhecido [?] | zero em todos os registros conferidos |

### QuestStuff.iff (0x10C) — condição de quest (contadores)
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | common | TypeId 0x74...... | 0x74800001 "Achieved -5 + excellent records" |
| 0xC0 | u32[5] | counter typeid | [0x6C400004, 0x6C400002, 0x6C40000C] |
| 0xD4 | i32[5] | quantidade alvo | [1, 1, -5] |
| 0xE8 | u32[3] | reward typeid | 0 |
| 0xF4 | u32[3] | reward qtd | 0 |
| 0x100 | u32[3] | reward tempo [?] | 0 |

### QuestItem.iff (0x110) — quest (agrupa QuestStuff e dá prêmio)
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | common | TypeId 0x78...... | 0x78000004 |
| 0xC0 | u32 | desconhecido [?] | 0 |
| 0xC4 | u32 | type / sequência [?] | 1, 2 |
| 0xC8 | u32 | qtd de quests | 1 |
| 0xCC | u32[10] | QuestStuff typeid | [0x74800001] |
| 0xF4 | u32[2] | reward typeid | [0x1A000011] |
| 0xFC | u32[2] | reward qtd | [50] / [100] |
| 0x104 | u32[2] | reward tempo [?] | 0 |
| 0x10C | u32 | desconhecido [?] | 0 |

### Achievement.iff (0x600)
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | common | TypeId 0x4C...... | 0x4C000001 |
| 0xC0 | u32 | QuestStuff/índice principal | 0x748002BC |
| 0xC4 | u32 | Achievement_Type | 0 |
| 0xC8 | char[129]×10 | nomes dos níveis | "Starting the Competition I", "... II", "... III" |
| 0x5D2 | i16 | Tipo [?] | 0 |
| 0x5D4 | u32[10] | QuestStuff typeids | [0x748002C4 ... 0x748002BC, 0] |
| 0x5FC | u32 | Option [?] | 0 |

### LevelUpPrizeItem.iff (0xC0)
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | u8 | Active | 1 |
| 0x01 | char[33] | nome do nível | "Rookie F", "Rookie E" |
| 0x22 | u16 | Level (índice) | 0, 1 |
| 0x24 | u32[2] | reward typeid | [0x18000008, 0x18000007] |
| 0x2C | u32[2] | qtd | [10, 10] |
| 0x34 | u32[2] | tempo | 0 |
| 0x3C | char[132] | descrição | "This will help you hit Pangya!" |

### MemorialShopRareItem.iff (0x5C)
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | i32 | Active (-1 / 1) | -1, 1 |
| 0x04 | u32 | Gacha.Number | 1, 2 |
| 0x08 | u32 | Gacha.Count | 136 |
| 0x0C | u32 | typeid | 0x10000080, 0x70010080 |
| 0x10 | u32 | probabilidade | 999 |
| 0x14 | i32 | RareType | 2 |
| 0x18 | i32[10] | filtro | [5], [10] |
| 0x40 | char[28] | código [?] (ref: bytes nulos) | "S01", "S02" |

### MemorialShopCoinItem.sff (0x40) — está dentro do zip
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | u32 | Active | 1 |
| 0x04 | u32 | typeid da moeda | 0x1A000273, 0x1A000272 |
| 0x08 | u32 | type (0 normal) | 0, 1 |
| 0x0C | u32 | probabilidade | 0, 100 |
| 0x10 | u32 | gacha min | |
| 0x14 | u32 | gacha max | |
| 0x18 | i32[10] | filtro | |

### GrandPrixData.iff (0x330)
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | u32 | Active | 1 |
| 0x04 | u32 | ID (typeid do GP) | 0x100, 0x80415 |
| 0x08 | u32 | TypeID_Link | 0x100, 0x80400 |
| 0x0C | u32 | TypeGP | 0 |
| 0x10 | u16 | TimeHole (s) | 0, 120 |
| 0x12 | char[66] cp932 | Name | "Control Practice Tournament", "リベラビーチカップ" |
| 0x54 | u32 | ticket typeid | 0x1A000264 |
| 0x58 | u32 | ticket qtd | 1 |
| 0x5C | char[41] | Event_Icon | "" |
| 0x85 | u8[3] | flags: Natural, Shot, HoleCup x2 | [0,0,1] |
| 0x88 | u32 | rule (typeid) | 0x1A000265 |
| 0x8C | u32 | course | 0, 1 |
| 0x90 | u32 | modo | 0 |
| 0x94 | u8 | buracos | 3, 18 |
| 0x95 | u8 | MinLevel | 0 |
| 0x96 | u8 | MaxLevel | 0 |
| 0x97 | u8 | desconhecido | 0 |
| 0x98 | u32[2] | condition | 0 |
| 0xA0 | i32[3] | score bot max/med/min | [0,5,10], [-8,-3,2] |
| 0xAC | u32 | class | 1, 2 |
| 0xB0 | u32 | pang | 120, 600 |
| 0xB4 | u32[5] | reward typeid | [0x1A000010, 0x140000A2] |
| 0xC8 | u32[5] | reward qtd | [120, 3] |
| 0xDC | u32[5] | reward tempo | 0 |
| 0xF0 | SYSTEMTIME | Open (só hora/min) | 20:15 (wMilliseconds vem 12/13 [?]) |
| 0x100 | SYSTEMTIME | Start | 20:25 |
| 0x110 | SYSTEMTIME | End | 21:25 |
| 0x120 | u32 | desconhecido | 0 |
| 0x124 | u32 | Clear_GP_TypeID | 0 |
| 0x128 | u32 | Lock_YN | 0 |
| 0x12C | char[516] | Info | "Perfect your technique to measure your power..." |

### GrandPrixSpecialHole.iff (0x14)
| 0x00 u32 Enable (1) | 0x04 u32 TypeID GP (0x100) | 0x08 u32 HolePOS (1,2) | 0x0C u32 Map (11, 16) | 0x10 u32 Hole (2, 6) |
|---|---|---|---|---|

### GrandPrixConditionEquip.iff (0x210)
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | u32 | active | 1 |
| 0x04 | u32 | GP typeid | 0x180500, 0x3180200 |
| 0x08 | u32 | item exigido | 0x10000000 (Air Knight Set), 0x04000000 (Nuri) [nome diz "ケン"] |
| 0x0C | char[516] cp932 | texto | "参加条件 : エアーナイトセット装備必須" |

### GrandPrixRankReward.iff (0x4C)
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | u32 | Active | 1 |
| 0x04 | u32 | GP typeid | 0x00080100 |
| 0x08 | u32 | Rank (posição) | 1, 2 |
| 0x0C | u32[5] | reward typeid | [0x1A000010, 0x1A000273] |
| 0x20 | u32[5] | qtd | [400, 1], [300, 1] |
| 0x34 | u32[5] | tempo | 0 |
| 0x48 | u32 | troféu typeid | 0x2F010101 |

### ClubSetWorkShopLevelUpProb.iff (0x18)
`0x00 u32 tipo` (0..4) · `0x04 u32[5] prob por stat` (tipo 0: 500,200,800,500,600). Conferido.

### ClubSetWorkShopLevelUpLimit.iff (0x14)
`0x00 u32 tipo` · `0x04 u32 rank` · `0x08 u16[5] limite por stat` (0/0: 14,12,12,5,5) · `0x12 u16 option`. Conferido.

### ClubSetWorkShopRankUpExp.iff (0x1C)
`0x00 u32 tipo` · `0x04 u32[6] exp por rank F..S` (tipo 0: 0, 900, 2800, 11000, 20200, 68000). Conferido (o ref lê 5; são 6).

### CharacterMastery.iff (0x3C)
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | u32 | active | 1 |
| 0x04 | u32 | personagem typeid | 0x04000000 |
| 0x08 | u32 | seq | 1, 2 |
| 0x0C | u32 | stats | 1 |
| 0x10 | u32 | level | 0 |
| 0x14 | u32[5] | item exigido | [0x1A0002A6, 0x74800330] |
| 0x28 | u32[5] | qtd | [2], [5,1] |

### ShopLimitItem.iff (0x3C)
| offset | tipo | campo | conferido |
|---|---|---|---|
| 0x00 | u32 | active | 1 |
| 0x04 | u32 | desconhecido (1) [?] | 1 |
| 0x08 | u32 | typeid | 0x24DFE055 (SetItem) |
| 0x0C | u32 | limite / estoque [?] | 50 |
| 0x10 | u32 | desconhecido [?] | 2 |
| 0x14 | u32 | desconhecido [?] | 24 |
| 0x18 | u32 | desconhecido [?] | 14 |
| 0x1C | SYSTEMTIME | início | 2012-09-20 11:00 |
| 0x2C | SYSTEMTIME | fim | 2012-10-04 11:00 |

### PointShop.iff (0x14)
`0x00 i32 Active (-1)` · `0x04 u32 typeid (0x1800002C)` · `0x08 u32 pontos (30)` · `0x0C u32 qtd (5)` · `0x10 u32 flag (0/1)`. Conferido.

### TimeLimitItem.iff (0x64)
`0x00 u32 active` · `0x04 u32 typeid (0x1A0000B3)` · `0x08 char[40] nome ("???")` · `0x30 char[40] ícone ("Tli_Buff_01")` ·
`0x58 u32 type (1)` · `0x5C u32 percent (5, 50)` · `0x60 u32 time (120 min)`. Conferido.

### NonVisibleItemTable.iff / SubscriptionItemTable.iff (0x2C) — iguais ao KR sNonVisibleItem
`0x00 u32 active (1)` · `0x04 u32 type (15)` · `0x08 u32 typeid (0x1A0000BE)` · `0x0C SYSTEMTIME start` · `0x1C SYSTEMTIME end`. Conferido (datas zero).

### SetEffectTable.iff (0x4C) — efeito de conjunto (KR 839 tinha 0x38)
| offset | tipo | campo | conferido (registro 1) |
|---|---|---|---|
| 0x00 | u32 | Index | 1 (registro 0 vazio) |
| 0x04 | u32[3] | effect | [1, 1, 3] |
| 0x10 | u32[3] | effect type (2 game, 4 sala, 8 lounge) | [8, 4, 2] |
| 0x1C | u32[5] | itens do conjunto | [0x0800609A, 0x0800A041] |
| 0x30 | u8[5] | item ativo | [1,1,0,0,0] |
| 0x35 | u8[11] | desconhecido [?] | 0 |
| 0x40 | i16[5] | Slot | 0 |
| 0x4A | i16 | Effect_Add_Power | 0 |

### TwinsItemTable.iff (0x1C) / HoleCupDropItem.iff (0x2C)
- Twins: `0x00 u32 active` · `0x04 u32 type (15)` · `0x08 u32[5] typeids` (0x1C00000C, 0x1C00000D, 0x1C00000E).
- HoleCupDrop: `0x00 u32 typeid (0x1A000044)` · `0x04 char[40] sprite ("aeboat-01.spr")`.

## 4. Resumo para o leitor JP do servidor
1. Common: Final/TypeId/Name no mesmo lugar; Level, Icon, Price..Time = offset KR + 0x18; saleStart/saleEnd = KR + 0x30; o resto do registro começa em 0xC0 (KR 0x90), ou seja, campos específicos = offset KR + 0x30 até onde o layout muda.
2. Mudou tipo: Part.Category u8->u32; SetItem.nElems u8->u32; Card.CardIndex u16->u32; CadieMagicBox ganhou u32 em 0x14 e datas no fim.
3. Campos novos no fim: Part (NPCSlot, u32), Ball (u32 no início), Mascot/AuxPart (BonusPang/Flag), ClubSet (WorkShop 0xE4..0x103), Card (flag1/flag2), Course (par/min/max por buraco).
4. Semântica a confirmar no exe 983: COM de CaddieItem/Mascot (dias x preços), tiki block 0x84..0x9B, Furniture.Pos (só x), CadieMagicBox.uiRandSeq sempre 0.
