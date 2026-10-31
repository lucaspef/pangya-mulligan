// V3: layout do 983 das classes vistas por src/projectg/taskmain.cpp (gerado por tools/v3/gen.py; não editar à mão).
// unidade: src/projectg/taskmain.cpp
V3_OFF(CSharedDoc, m_bBackgroundVideo, 0x10b26) // 983: TurnOn3DBackground 00657690 (006576b3/006576c1) e TurnOff3DBackground 006576d0 (006576d5) gravam byte [doc+0x10b26] (C-PKT lote 31)
V3_OFF(CSharedDoc, m_bReturnLobbyNotice, 0x10b28) // 983: OnPacketCommon 0xf5 grava [doc+0x10b28]=1 e [doc+0x10b29]=0 (009c8d2a/009c8d37; o nosso = m_bReturnLobbyNotice=true, m_bRebuildUnderBar=false); OnUnderBar_BackUp 0065c23c le +0x10b28; byte novo do 983 em +0x10b27 (00566390, 005f7080, 00ab5be0) (C-PKT lote 31)
V3_OFF(CSharedDoc, m_bRebuildUnderBar, 0x10b29) // 983: OnPacketCommon 0xf5 009c8d37 / 009c8ec2 / 009d6149 gravam byte [doc+0x10b29]=0 logo depois do +0x10b28 (C-PKT lote 31)
V3_OFF(CSharedDoc, m_selChannel, 0x10b2a) // 983: lea esi,[doc+0x10b2a] em HandleMsgCommon 0065e4ff e FrServerDlg::OnChannelListBtnDown 00af6196 (copia do canal escolhido) (C-PKT lote 31)
V3_SIZE(CTaskMain, 0x218) // 983: push 0x218; call operator new em 0065d3c3, construtor 0065d1f0 (mapa=1; 1 new); unidade por new
V3_OFF(CTaskMain, m_pCardComposeDlg983, 0x18c) // 983: 00659490 grava [esi+0x18c] em 006594d5 (A s100; C-PKT lote 24)
V3_ANOT(CTaskMain, m_pAttendanceDlg983, 0x190) // anot: anotação do cabeçalho src/projectg/taskmain.h:362
V3_ANOT(CTaskMain, m_pBongdariShop983, 0x194) // anot: anotação do cabeçalho src/projectg/taskmain.h:363
V3_ANOT(CTaskMain, m_pPointShopDlg983, 0x19c) // anot: anotação do cabeçalho src/projectg/taskmain.h:365
V3_OFF(CTaskMain, m_pPostReciveDlg983, 0x1a0) // 983: OnUnderBar_GiftUp 00658480 (cmp [esi+0x1a0]) (A s100; C-PKT lote 24)
V3_ANOT(CTaskMain, m_pPostSendDlg983, 0x1a4) // anot: anotação do cabeçalho src/projectg/taskmain.h:367
V3_ANOT(CTaskMain, m_pAlchemyDlg983, 0x1a8) // anot: anotação do cabeçalho src/projectg/taskmain.h:368
V3_OFF(CTaskMain, m_pMaketDlg, 0x1ac) // 983: CreateMaketDlg 00655d10 (00655d1d/00655d42 [esi+0x1ac]) (A s100; C-PKT lote 24)
