// JP 983: as strings estáticas dos controles da Refresh (B155, F3.7.13 item (5)): TypeName (nome do tipo no XML e na fábrica,
// RFManager::GetCompatibleControlName 004199f0) e os nomes de evento. No 983 cada uma vem do .cpp do controle (inicializadores
// 00cc7710–00cc7a00, texto pelo push de cada um); juntas aqui até os controles ganharem unidade própria.
#include <string>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfdirector.h"

const std::wstring RFPicture::TypeName = L"Picture"; // 00e46cf0
const std::wstring RFLabel::TypeName = L"Label"; // 00e46d0c
const std::wstring RFButton::EventClicked = L"Clicked"; // 00e46d28
const std::wstring RFButton::TypeName = L"Button"; // 00e46d44
const std::wstring RFList::EventSelChanged = L"SelChanged"; // 00e46d60
const std::wstring RFList::EventBeginDrawItem = L"BeginDrawItem"; // 00e46d7c
const std::wstring RFList::EventEndDrawItem = L"EndDrawItem"; // 00e46d98
const std::wstring RFList::EventDrawItem = L"DrawItem"; // 00e46db4
const std::wstring RFListItem::TypeName = L"UserItemType"; // 00e46dd0
const std::wstring RFListItemWnd::TypeName = L"WindowItemType"; // 00e46dec
const std::wstring RFCheck::EventChanged = L"Changed"; // 00e46e08
const std::wstring RFCheck::TypeName = L"CheckBox"; // 00e46e24
const std::wstring RFLabelButton::TypeName = L"LabelButton"; // 00e46e40
const std::wstring RFTextArea::EventHyperLinkClicked = L"HyperLinkClicked"; // 00e46e5c
const std::wstring RFTextArea::EventHyperLinkEnter = L"HyperLinkEnter"; // 00e46e78
const std::wstring RFTextArea::EventHyperLinkLeave = L"HyperLinkLeave"; // 00e46e94
const std::wstring RFTextArea::EventRichTextDrawImage = L"RichTextDrawImage"; // 00e46eb0
const std::wstring RFScrollBar::EventPosChanged = L"PosChanged"; // 00e46ecc
const std::wstring RFScrollBar::TypeName = L"ScrollBar"; // 00e46ee8
const std::wstring RFAnimButton::TypeName = L"AnimButton"; // 00e46fe4
const std::wstring RFEdit::EventEnterKeyDown = L"EnterKeyDown"; // 00e47000
const std::wstring RFEdit::TypeName = L"Edit"; // 00e4701c
const std::wstring RFProgress::TypeName = L"ProgressBar"; // 00e47038
const std::wstring RFTextAreaSC::TypeName = L"TextArea"; // 00e47054
const std::wstring RFStackPanel::TypeName = L"StackPanel"; // 00e47070
const std::wstring RFSlider::EventPosChanged = L"PosChanged"; // 00e4708c
const std::wstring RFSlider::TypeName = L"Slider"; // 00e470a8
const std::wstring RFListSC::TypeName = L"ListBox"; // 00e470c4
const std::wstring RFComboBox::EventSelChanged = L"SelChanged"; // 00e470e0
const std::wstring RFComboBox::TypeName = L"ComboBox"; // 00e470fc
const std::wstring RFDirector::EventAnimFinished = L"AnimFinished"; // 00e46f20 (RFDirector.cpp; aqui até a unidade dele)
