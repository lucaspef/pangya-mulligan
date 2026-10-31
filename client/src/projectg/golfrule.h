#pragma once

#ifdef REBANG_GOLFRULE_PCH
#include "taskmain.h"
#include <stack>
#else
#include "actor.h"
#endif

#if _MSC_VER > 1600
#define GOLFRULE_OFS_SHOTDATA 0x344 // A727: medido no cl 14.44 (D859): std::string do BaseObject e a STL do CTaskMain/CGolfRule menores
#define GOLFRULE_OFS_AUTOSHOT983 0x238  // B1463: m_autoShotCount/m_modeDuration/m_turnStartTime medidos no cl 14.44 (out/D859/ofs.py)
#define GOLFRULE_OFS_MODEDUR983 0x2bc
#define GOLFRULE_OFS_TURNSTART983 0x630
#define GOLFRULE_OFS_SHOTINFLIGHT983 0x2b4 // B1537: m_bShotInFlight no cl 14.44 (0xc antes de m_modeDuration 0x2bc)
#else
#define GOLFRULE_OFS_SHOTDATA 0x354 // 983 LoadShot 0067c240
#define GOLFRULE_OFS_AUTOSHOT983 0x244  // B1463: unidades fora do pch escrevem estes campos do CGolfRule cru (983 inline)
#define GOLFRULE_OFS_MODEDUR983 0x2c8
#define GOLFRULE_OFS_TURNSTART983 0x648
#define GOLFRULE_OFS_SHOTINFLIGHT983 0x2c0 // B1537: m_bShotInFlight (983 +0x2c0; CGolfRuleJp::SetFlag2c0_983)
#endif
#ifdef REBANG_GOLFRULE_PCH
class FrReplayControlDlg;
class FrEmoticonDlg;
class FrApproachResultDlg;
class FrApproachEndDlg;
class FrOnelineReqDlg;
class CGolfRuleBase;
class CSubGolfRuleBase; // 983: include/jp/subgolfrule.h (frente B)
class FrComboBox;
class CSimpleUI;

class CGolfRule : public CTaskMain
#else
class CGolfRule : public IActor
#endif
{
#ifdef REBANG_GOLFRULE_PCH
	DECLARE_OBJECT(CGolfRule)

#endif
public:
	friend class CGolfRuleBase;
	friend class CGolfRuleQuickrace;
	friend class CGolfRuleTimeattack;
	friend class CGolfRuleTutorial;
	friend class CGolfRuleGuildMatch; // 983: RequestShotReview (CheckNetMsgWait_Send)
	friend class CGolfRule30Battle;   // 983: RequestShotReview (CheckNetMsgWait_Send)
	friend class CGolfRuleNewApproach; // 983: [10] 006a9790 (SendShotResult, ApproachTimeOut983)
#ifdef REBANG_GOLFRULE_PCH

	struct sBackUp
	{
		// estado do jogador antes da tacada; o desfazer (983 00687050) volta a ele
		WVector pos;              // 0x00
		float cameraAngle;        // 0x0c: volta em m_cameraData.angle
		unsigned char bOB;        // 0x10: m_bOB da bola (so gravado)
		unsigned char bunker;     // 0x11: sPlayerData::bunker
		float gauge;              // 0x14: sPlayerData::gauge
		__int64 totalPang;        // 0x18: sPlayerData::totalPang
		__int64 bonusPang;        // 0x20: sPlayerData::bonusPang
		__int64 holePang;         // 0x28: sPlayerData::pang[buraco]
		unsigned long reserved30[(0x58 - 0x30) / 4]; // 0x30: o desfazer (00687050) nao le
		char club[4];             // 0x58: nome do taco (SetClub)
		unsigned char holeStroke; // 0x5c: sPlayerData::stroke[buraco]
		unsigned char flag;       // 0x5d: sPlayerData::flag (+0x4e8)
	};
#endif

	struct sShotData
	{
		unsigned char hole;
		WVector wind;
		unsigned char weather;
		unsigned char stroke;
		sBar bar;
		WVector pos;
		float angle;
		float curve;
		float spin;
		int pvsFrame;
		unsigned long special;
		unsigned long clubPower;
		char club[4];
		unsigned char bunker;
		unsigned long item;
		float deviation;
		unsigned char holeStroke;
		int pang;
		int bonusPang;
		WCrypticValue<float> gauge;
	};

	void AddPlayer(unsigned char index, const char* name, unsigned long uid,
		unsigned long oid, unsigned long caddie);
	void SendHoleData();
	void LoadShot(const char* filename);

#ifdef REBANG_GOLFRULE_PCH
	CGolfRule();
	virtual ~CGolfRule();

#endif
	sShotData* GetShotData() { return &m_shotData; }
	void SendMsg983(const char* target, const MsgObject& msg); // JP 983: 00671330
	bool IsModeTimeOver983(); // JP 983: 0066e8c0 (m_modeTime >= 3.0f)
	bool IsNetWaiting983();   // JP 983: 0066e8a0 (m_bContinueDlg || m_bNetMsgWait)
	void SetModeDuration983(float time); // 983: a base grava +0x2c8 direto (006a8817); aqui ela so ve a parte comum
protected:
	void ReturnLobby(bool); // 0067cc70 (visivel fora do pch para o CGolfRule30Battle)
public:
#ifdef REBANG_GOLFRULE_PCH
	unsigned char GetCurTeam() { return m_curTeam; }
	virtual void OnLoad();
	void SetFlagCutin(bool);
	bool GetFlagCutin();
	virtual void OnInit();
	virtual void OnDisplay();
	void SetCurrentGolfRule();
	virtual void OnDestroy();
	virtual void OnReload();
	void SaveShot(char const*);
	virtual void OnPreLoadInit();
	virtual void HandleMsg(MsgObject const&);
	virtual void OnProcess(float);
#endif

protected:
	unsigned char GetFirstTurn();
	void ResetData(const WVector& pos);
	void ChangeGameMode(unsigned long mode);
	void SetNetMsgWait(bool, int, char*, float);
	void CheckNetMsgWait_Send(char*);
	void SendShotResult();       // fora do bloco PCH: a CGolfRuleNewApproach [10] chama
	void ApproachTimeOut983(); // 00695270
#ifdef REBANG_GOLFRULE_PCH
	void OnChatTargetBtnDown(int);
	void OnSpecialCardBtnUp();
	void OnCaddieBookBtnUp();
	void OnChatRecordBtnUp();
	void SetGrade();
	void SetVisibleUpperBtns(bool);
	bool CheckGiveUp();
	void ToggleCommandBtns(bool);
	void CheckNetMsgWait_Recv(char*);
	void SetAngle(float);
	bool OnPlayerInfoDlgResult(int, FrForm*);
	void OnEmoticonInit(int);
	void OnLanguageInit(int);
	void OnReportInit(int);
	bool OnChatTargetEnterKey(int);
	void OnChatInputInit(int);
	void OnSpecialCardInit(int);
	void OnMyInfoInit(int);
	void OnOptionInit(int);
	void OnOptionBtnUp();
	void OnUnplayableInit(int);
	void OnPauseInit(int);
	void OnChatRecordInit(int);
	void OnOnelineViewInit(int);
	void OnOnelineViewOwnerDraw(int);
	void OnOnelineBtnInit(int);
	bool OnOnelineReqChkOutDlgResult(int, FrForm*);
	void OnRadioPredictNormalShotInit(int);
	void OnRadioPredictTomahwakShotInit(int);
	void OnRadioPredictCobraShotInit(int);
	void OnRadioPredictSpikeShotInit(int);
	void OnRadioPredictNormalShotDown();
	void OnRadioPredictNormalShotDraw(int);
	void SetPlayer();
	void IntergrateFile(char const*);
	void SendReportError(unsigned char);
	void ResetUiPos_Board();
	void OnMessengerBtnUp();
	void OnMessengerBubbleEditInit(int);
	void OnMessenger_AlarmBtnUp();
	void OnMessenger_GiftAlarmBtnInit(int);
	void OnReplayStatusInit(int);
	void OnMoveTeeInit(int);
	void OnMoveTeeBtnUp();
	void OnShotReviewPreInit(int);
	void OnShotReviewNextInit(int);
	void OnShotReviewPreBtnUp();
	void OnShotReviewNextBtnUp();
	void OnMainFinish();
	void OnPremiumServiceEndNoticeInit(int);
	void OnPremiumServiceEndNoticeOwnerDraw(int);
	void OnChatViewInit(int);
	void OnChatTargetInit(int);
	bool OnChatInputEnterKey(int);
	void OnCaddieBookInit(int);
	void OnUnplayableBtnUp();
	void OnPauseBtnUp();
	bool OnOnelineReqDlgResult(int, FrForm*);
	void OnOnelineBtnUp();
	void OnTopBarHoverOn(int);
	void OnTopBarHoverOff(int);
	void OnRadioPredictTomahwakShotDown();
	void OnRadioPredictCobraShotDown();
	void OnRadioPredictSpikeShotDown();
	void OnRadioPredictTomahwakShotDraw(int);
	void OnRadioPredictCobraShotDraw(int);
	void OnRadioPredictSpikeShotDraw(int);
	unsigned long GetHolePang(unsigned char);
	void DoDebugCamera(float);
	bool GetBarCondition();
	void SetSpecialShot();
	void CheckSpecialShotSpinAndCurve();
	WMatrix BankCamera(float);
	void ReactionForShot();
	bool PauseDlgResult(int, FrForm*);
	bool CancelDropDlgResult(int, FrForm*);
	bool GoStopDlgResult(int, FrForm*);
	bool OnEmoticonResult(int, FrForm*);
	bool OnReportResult(int, FrForm*);
	bool CanCancelShot();
	void OnMessengerBtnInit(int);
	void OnMessenger_AlarmBtnInit(int);
	void OnMessenger_GiftAlarmBtnUp();
	bool IsMaxWithHGParts(unsigned char);
	void OnEmoticonBtnUp();
	void OnReportBtnUp();
	void OnMyInfoBtnUp();
	void RecordPlayerList();
	bool CheckDropOut();
	bool HaveTeeShots();
	void RecordPlayerData(unsigned char);
	void RecordTeamData(unsigned char);
	void RecordGiveUp(unsigned char);
	void DispatchMatchItems();
	void SetHoleInPose(bool);
	__int64 CalcWinnersPang(unsigned long);
	void DoFlyBall_Flying(float);
	void CaptureProcess();
	void DoApproachNextHole(float);
	void UpdateShotResultStat();
	void CorrectShotResult();
	void SetEnvironmentVariableA();
	void ForcedMoveBall(WVector const&);
	unsigned char GetNextTurn();
	void AddPrize(int);
	bool CheckAllHoleOut();
	bool ContinueDlgResult(int, FrForm*);
	bool OnReturnLobbyResult(int, FrForm*);
	bool OnGPAwardNotifyResult983(int, FrForm*); // 00671310 (B72): retorno do "notify_ok2" da premiacao do Grand Prix
	void DoGameOver(float);
	void CalcInputCommand();
	bool SetTurnChangeEvent();
	void Replay(bool);
	void AddExtPrize();
	void CancelShot();
	bool OnForgetDlgResult(int, FrForm*);
	void PlayBGM();
	void CommandBox();
	void ChangeTurn();
	void GetWinningPrize(int);
	bool SetHoleInPose_Skins();
	void SendHoleStat();
	bool ReturnToLobbyDlgResult(int, FrForm*);
	void CheckUserBGM(char const*);
	void EncodePacket(enumClientPacket, WSendPacket&);
	void ResetRank();
	bool CanMoveToNextHole();
	void RecordScoreByPlayer(unsigned char);
	void RecordScoreByTeam(unsigned char);
	bool SetHoleInPose_Team();
	void CalcSkinsHolePang();
	void DoFlyBall_Stopped(float);
	void DoCoursePreview(float);
	void DoGetReadyToCaptureBG(float);
	void DoNavigateCamera(float);
	void DoTopViewCamera(float);
	void Skip();
	void HitShot();
	bool OnApproachEndDlgResult(int, FrForm*);
	bool OnApproachDlgResult(int, FrForm*);
	void SetReady();
	void DoFixedCamera(float);
	void DoSpotCamera(float);
	void DoDespairPose(float);
	void DoScoreBoard(float);
	void DoHoleInPose(float);
	void DoWaitForStart(float);
	void DoCloseupCaddie(float);
	void ReadyForNextTurn();
	void DoDrive(float);
	void DoFlyBall(float);
	void DoStand(float);
	void DriveGauge(float, bool);
	void PuttingGauge(float, bool);
	void DoToDefaultCamera(float);
	void DoToDefaultCameraNext983();
	void Drop(WVector const&);
	void Cancel();
	void Warp(WVector const&);
	void DoTimeOut(float);
	void DoPlayerPreview(float);
	bool IsWaiting983(); // JP 983: 0066e870
	void DoPowerGauge(float);

private:
	void SetRadioPredictShotPress(FrButton*);
	void UpdateRadioPredictShotButtonStatus();
	bool StandViewClick();
	int HoleReplay2();
	void ReplayBoxClose();
	bool IsLastShotRelplay();
	void InitRadioPredictShotButton(FrButton**, int, float);
	void ToggleGameUI();
	void _ImeReset();
	char* GetPangyaInstallDic();
	void HoleReplay();
	void SwingReplay();
	void AutoReplay();
	bool OnReplayModeDlgResult(int, FrForm*);
	bool OnRecordAviResult(int, FrForm*);
	bool GetShotCntData();
	void SaveGame(int);
	void LoadGame(int);
	void QuickSave(char const*);
	void ReplayBoxOpen();
	void OnRecordAviBtnUp();
	void AddSomeInfo(WSendPacket&);
	void HoleComboGaugeSetting();
	bool TopViewClick();
	void FlyBy(float);
	void AllReplay();
	bool ShotDataLoad(char const*);
	int ShotDataSave(char const*); // 983: sem o modo anonimo (006884f0)
	void SaveErrorFile();
	void ReplayTapeCheckAndUse();
	void CollectBallStopData();
	void QuickLoad(char*);
	void ProcessObserver();

	DECLARE_FRESH_MSGMAP()
#endif

public:
#ifdef REBANG_GOLFRULE_PCH
	// offsets do 983 (dump); CTaskMain termina em 0x1e0 (dsize: os membros reaproveitam o padding final)
	unsigned long m_inputCmdHead; // 0x1e0
	unsigned long m_inputCmdTail; // 0x1e4
	unsigned long m_specialShotGroup; // 0x1e8
	std::deque<void*> m_cmdQueue; // 0x224 (983 0068aee3): comandos de tacada pendentes (FlushCommandQueue983 passa 1 a cada 0.1 s para o doc +0x8c)
	float m_cmdQueueTime; // 0x23c: tempo acumulado da fila m_cmdQueue (00685b90)
	unsigned char m_screenLockMode; // 0x240: trava de tela da auto-tacada (0x10; msgs 0x329/0x32a/0x32b -> Screen 0x330)
	bool m_bAutoShot; // 0x241: auto-tacada ligada (item 0x1a00019f / modo 0xb; msg 0x32c devolve, 0x32d/0x32e)
	unsigned char m_pad983_242[2];
	int m_autoShotCount; // 0x244: usos restantes (983 so zera e decrementa no 0x32e; comparacao com sinal)
	WVector m_dbgEye983; // 983 +0x248 (645 0x1ec): olho da camera livre de depuracao (DBG_EYE)
	WVector m_camPivot; // 0x1f8
	float m_modeTime;   // 0x204
	float m_goStopTime983; // 983 +0x264: tempo da janela Go/Stop aberta (PM_NET_ASK_GOSTOP; OnProcess soma, abrir zera)
	unsigned long m_savedGameMode983; // 983 +0x268: modo de jogo restaurado ao sair da camera livre de depuracao
	WMatrix m_savedCamera983; // 983 +0x26c: g_camera guardada ao entrar na camera livre (0x400)
	unsigned char m_unused983_29c[0x2a4 - 0x29c];
	bool m_bDriveCamStarted983; // 983 +0x2a4: DoDrive ja passou de 1 s (msg Player 0x10e; camera de tacada especial)
	bool m_bUnplayable983; // 983 +0x2a5: msg 0x77 (param1 != 0): bola em area injogavel -> ChangeGameMode(0x10)
	unsigned char m_pad983_2a6[2];
	WVector m_targetPos; // 0x24c
	WVector m_holePos983; // 983 +0x2b4 (645 0x258; lea [esi+0x2b4] em 00687c91): posicao do jogador no buraco (HP_POS)
	bool m_bShotInFlight; // 983 +0x2c0: tacada saiu (0x77) e a bola ainda nao parou; msg 0x83 so pula (Skip) com ele; DoFlyBall_Stopped consome
	bool m_bTimeOutSent; // 0x265
	bool m_bReplayReady;      // 983 +0x2c2: bola parou, tecla REPLAY liberada na vez (006924c4)
	bool m_bTurnStarted983;   // 983 +0x2c3: vez comecou; o 983 so grava (ninguem le em 00670000..006b0000)
	bool m_bFastForwardReady; // 983 +0x2c4: tecla FASTFORWARD liberada (tacada fora do green: groundType != 2)
	unsigned char m_pad983_2c5[3];
	float m_modeDuration; // 0x26c
	void* m_pCamArea983; // 983 +0x2cc: WPolySoupEx2::sArea da camera instalada atual (troca "*cam" da area da bola)
	bool m_bSpecialCamDone983; // 983 +0x2d0: camera da tacada especial (Tomahawk/Spike) ja trocada
	unsigned char m_pad983_2d1[3];
	float m_shotExtra983; // 983 +0x2d4: sHitShotInfo983.extra (+0x2a) do pacote da tacada; BallSpeedFactor e Spot 0xfa
	std::stack<sBackUp> m_backUpStack; // 0x27c
	sBackUp m_backUp;                  // 0x290
	unsigned char m_nextTurn;          // 0x2f0; 983 +0x350 (byte): proximo turno do servidor (GetNextTurn 0067bc90)
	unsigned char m_unused2f1[3];
	sShotData m_shotData;              // 0x2f4
	char m_sfxName[0x20]; // 983 0x3dc (645: m_wind 0x37c + 0x388 nome do som); o 983 não tem m_wind: LoadGame 0067c5a5 lê o vento numa
	                      // local; o nome do som do voo fica em +0x3dc (ChangeGameMode 0068fc2e/0068fc4d, DoFlyBall_Flying 0067f7ad); s96
	std::vector<WVector> m_bgPosList; // 0x3a8
	unsigned char m_curTeam;          // 0x3b8
	unsigned char m_unused3b9[3];
	unsigned long m_gaugeTick;      // 983 +0x410: ultimo timeGetTime do DoPowerGauge (passo = agora - este); 0x20 zera; 0xa2 retoma = m_gaugePauseTick
	unsigned long m_gaugeStartTick; // 983 +0x414: 1o tick do medidor no turno (so grava; 0x20 zera)
	unsigned long m_gaugePauseTick; // 983 +0x418: DoPowerGauge com m_bGaugePaused grava o tick (tutorial)
	unsigned char m_bGaugePaused;   // 983 +0x41c: msg 0xa2 alterna (tutorial pausa o medidor)
	unsigned char m_unused983_41d;  // 983 +0x41d: sem leitor; so OnPreLoadInit 0068bf1e zera (word com +0x41e)
	bool m_bContinueDlg;            // 0x3ca
	bool m_bNetMsgWait;             // 0x3cb
	int m_netMsgWaitType;           // 0x3cc
	char* m_pNetMsgWait;            // 0x3d0
	char* m_pNetMsgRecv;            // 0x3d4
	unsigned long m_netMsgSendTick; // 0x3d8
	unsigned long m_netMsgRecvTick; // 0x3dc
	float m_netMsgWaitTime;         // 0x3e0
	unsigned char m_unused983_438;
	bool m_bTutorialShot983;        // 983 +0x439: msg 0x87 tipo 1/2 liga (0 desliga); hit so com a tecla de espaco e medidor dentro de m_tutorialGauge983
	unsigned char m_unused983_43a[2];
	float m_tutorialGauge983[2];    // 983 +0x43c/+0x440: limite do medidor na fase 1/2 (msg 0x87 param2/param3); o Tutorial recebe o ponteiro do da fase
	bool m_bPuttTutorial983;        // 983 +0x444: msg 0x9a liga, inicio da vez zera; bola parada manda putt sucesso/falha ao Player
	unsigned char m_unused983_445[3];
	unsigned long m_pendingInitSpecial;   // 0x3f4
	unsigned char m_bgmIndex; // 0x3f8
	unsigned char m_unused3f9[0x3fc - 0x3f9];
	unsigned long m_cmdTick; // 983 +0x450: tick da 1a tecla de direcao (repete apos 100 ms); 0 = sem tecla
	unsigned char m_unused983_454;  // 983 +0x454: so no layout
	bool m_bServerHold983;          // 983 +0x455: pacote 0xbe -> M983(0x9e) 1 liga, 0x9e 0 desliga; ligado: sem CClubEx::Process nem som unplayable; solta sozinho apos 5 s
	unsigned char m_unused983_456[2];
	float m_serverHoldTime983;      // 983 +0x458: tempo desde o 0x9e 1 (OnProcess soma time; > 5.0 solta)
	bool m_bRemoteHitPending; // 0x408; 983 +0x45c: 0x7a na fase 1 (0069724c) -> a vez de outro bate (0069b9da)
	unsigned char m_unused409[3];
	unsigned long m_observerReqTick; // 983 +0x460: ultimo pedido 0x3f do observador (00694c30, 3 s)
	bool m_bChatIconSent983;        // 983 +0x464: OnProcess com linha no chat online manda CHeadIcon::SendIcon(1) uma vez; volta a 0 quando a linha esvazia
	unsigned char m_unused983_465[3];
	FrForm* m_pPauseDlg;                       // 0x414
	FrForm* m_pGoStopDlg;                      // 0x418
	FrForm* m_pCancelDropDlg;                  // 0x41c
	unsigned long m_unused983_474;             // 983 +0x474: sem leitor; ctor 0068b07a e OnPlayerInfoDlgResult 0066e840 (sem referencia) zeram
	FrApproachResultDlg* m_pApproachResultDlg; // 0x424
	FrApproachEndDlg* m_pApproachEndDlg;       // 0x428
	bool m_bTrajectoryMsgSent;                             // 0x42c
	unsigned char m_unused42d[3];
	float m_dbgCam[2];    // 983 0x484/0x488 (645 0x430/0x434): câmera de depuração (DoDebugCamera; OnPreLoadInit -1.0f/100.0f); s96
	float m_unused983_48c; // 0x48c: 983 so zera (OnPreLoadInit 0068bf4f); ninguem le em 00670000..006b0000
	WVector m_dbgCamPos;  // 983 0x490 (645 0x438..0x440; DoDebugCamera: 490 494 498 = 438 43c 440); s96
	bool m_bCancelShotAsk;  // 0x444; 983 +0x49c: bola parou com cancelamento possivel -> janela flowerofforgetting, sem SendShotResult
	float m_forgetDlgTime; // 0x448; 983: float (fldz/fstp [esi+0x4a0] em 0068af83)
	int m_cancelItemSlot;   // 0x44c
	bool m_bSkinsNextHole;          // 983 +0x4a8: Skins, PM_NET_NEXT_HOLE chegou (HandleMsg 00697e56; ChangeGameMode/DoHoleInPose testam)
	unsigned char m_unused4a9[3];
	float m_holeInCoinFxTime;       // 983 +0x4ac: intervalo do efeito de moedas (DoFlyBall_Stopped 00692946; OnProcess 0069cb7b)
	int m_holeInCoinFxCount;        // 983 +0x4b0: moedas que faltam (Skins hole-in)
	unsigned long m_holeInCoinFxId; // 983 +0x4b4: parametro do "Effect" M983(0x17d) (0x1e)
	std::vector<std::string> m_bgmList;      // 0x460
	FrComboBox* m_pChatTarget;               // 0x470
	FrEdit* m_pChatEdit;                     // 0x474
	FrButton* m_pEmoticonBtn;                // 0x478
	FrArea* m_pLanguage;                     // 0x47c
	FrButton* m_pReportBtn;                  // 0x480
	FrEdit* m_pChatView;                     // 0x484
	FrEmoticonDlg* m_pEmoticonDlg;           // 0x488
	FrForm* m_pReportDlg;                    // 0x48c
	FrWnd* m_pForgetDlg;                        // 0x490
	FrOnelineReqDlg* m_pOnelineReqDlg;       // 0x494
	FrReplayControlDlg* m_pReplayControlDlg; // 0x498
	FrForm* m_pRecordAviDlg;                 // 0x49c
	FrButton* m_pReplayBox;                  // 0x4a0; 983 +0x4f8: FrButton (00671190)
	FrButton* m_pMyInfoBtn;                  // 0x4a4
	FrButton* m_pOptionBtn;                  // 0x4a8
	FrButton* m_pCaddieBookBtn;              // 0x4ac
	FrButton* m_pUnplayableBtn;              // 0x4b0
	FrButton* m_pPauseBtn;                   // 0x4b4
	FrButton* m_pChatRecordBtn;              // 0x4b8
	FrArea* m_pOnelineView;                  // 0x4bc
	FrButton* m_pOnelineBtn;                 // 0x4c0
	FrButton* m_pMoveTeeBtn;        // 983 +0x51c: MAIN.move_tee (006770d0)
	FrButton* m_pShotReviewNextBtn; // 983 +0x520: MAIN.shot_review_button_next (00670830)
	FrButton* m_pShotReviewPreBtn;  // 983 +0x524: MAIN.shot_review_button_pre (006707c0)
	bool m_bClosedBonusBallPending;                           // 0x4c4
	char m_closedBonusBall[100];                      // 0x4c5
	FrArea* m_pPremiumNotice;  // 983 +0x590: MAIN.premium_service_end_notice (00672e10)
	float m_premiumNoticeAlpha;         // 983: OnPreLoadInit/OnProcess 0069cc0a
	float m_premiumNoticeFadeRate;
	bool m_b983_59c;            // 983: ligado no DoPlayerPreview (00696021) quando CSharedDoc+0x1160a != 0
	const Bitmap* m_pPremiumBmp[4]; // 983 +0x5a0: "Premium service_close_00/01/02", "Premium service_base"
	unsigned long m_pvsFrame[3]; // 983 0x5b0 (645 0x52c): quadros do PVS no tiro (ChangeGameMode 0068d911..0068d943); s96
	FrButton* m_pRadioPredictNormalShot;   // 0x538
	FrButton* m_pRadioPredictTomahwakShot; // 0x53c
	FrButton* m_pRadioPredictCobraShot;    // 0x540
	FrButton* m_pRadioPredictSpikeShot;    // 0x544
	CSimpleUI* m_pPredictShotUI;           // 0x548
	int m_captureTiles;                    // 0x54c
	int m_captureIndex;                    // 0x550
	int m_captureMode;                     // 0x554
	bool m_bGameUI;                        // 0x558
	char m_saveFileName[0x59c - 0x559];    // 0x559
	unsigned long m_replayBasePang;        // 0x59c; 983 +0x620: totalPang no inicio do replay (sShotData.pang = atual - base)
	unsigned long m_replayBaseBonusPang;   // 0x5a0; 983 +0x624: idem bonusPang
	float m_replayGauge;                   // 0x5a4; 983 +0x628 (fstp 0068cbd3) -> sShotData.gauge
	unsigned long m_replayFlags;           // 0x5a8
	int m_replayState;                     // 0x5ac
	bool m_bNotifyPending983;       // 983 +0x634: OnProcess abre o "notify" e solta m_bNetMsgWait (983 so liga via 0066e850, que e return 0)
	unsigned char m_bRainbowFx983; // 983 +0x635: rainbow_002.seq da bola 0x20 ja aberto nesta tacada (DoFlyBall_Flying 0067fd4b; zera no 0x80 0068f9a9)
	bool m_bFlagCutin; // 0x5b2
	unsigned char m_unused5b3[0x5b4 - 0x5b3];
	CGolfRuleBase* m_pRuleBase; // 0x5b4; 983: 0x638
	CSubGolfRuleBase* m_pSubRule983; // 983 +0x63c: sub-regra da frente B (include/jp/subgolfrule.h): CSubGolfRuleGame (m_playingMode 1) ou CSubGolfRuleReplayTape (2), criada no SetCurrentGolfRule 00677290 (s101)
	bool m_bHoleIntroEntered;              // 0x5b8; 983: 0x640
	bool m_bWaitForStart;       // 0x5b9; 983: 0x641
	int m_topViewMode;               // 0x5bc; 983: 0x644
	unsigned long m_turnStartTime; // 0x648: g_CurrentTime no inicio da vez (0067f4cc, 0068cb5a, 30battle 006a5312); lido pelo Grand Prix 00684a81
	unsigned long m_pad983_64c; // 0x64c: 983 nao usa; sizeof 0x650
#else
	// fora do pch: visao so ate m_shotData (os outros membros so sao usados em golfrule.cpp)
	unsigned char m_unused1c[GOLFRULE_OFS_SHOTDATA - sizeof(IActor)]; // D192: m_shotData em 0x354 no 983 (LoadShot 0067c240); era 0x2f4 (645) fora do pch
	sShotData m_shotData;
#endif
};

#ifdef REBANG_GOLFRULE_PCH
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: inline bodies the original PCH has here (emission order)
inline void CGolfRule::SetFlagCutin(bool flag)
{
	m_bFlagCutin = flag;
}

inline bool CGolfRule::GetFlagCutin()
{
	return m_bFlagCutin;
}
#endif

#endif
