// JP 983: RFPicture.cpp da Refresh (B156, F3.7.13 item 5): a figura (0041e400–0041ef9d). Estática = pincel (rfbrush.cpp) do
// ImageSet da <Picture>; animada = RFDirector com a cena "ANIMPICTURE" do arquivo (o RFDirector é do item (6)). ToString/
// GetTypeName/GetDirector em linha no rfcontrols.h; TypeName no rfcontrols.cpp.
#include <string>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfbrush.h"
#include "refresh/lib/rfdirector.h"
#include "refresh/rfimageset.h"
#include "refresh/rfwndstruct.h"

extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

// posição na tela somando a de todos os pais (o 983 repete o laço em cada método)
static __forceinline RFVector2 RFPictureScreenPos983(RFWnd* wnd)
{
	RFVector2 pos(0.0f, 0.0f);
	for (; wnd; wnd = wnd->_super)
	{
		RFVector2 p = wnd->GetPos();
		pos.x += p.x;
		pos.y += p.y;
	}
	return pos;
}

// 0041e400
RFPicture::RFPicture()
{
}

// 0041e420
RFPicture::~RFPicture()
{
}

// 0041e430
void RFPicture::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	_imageSet983 = static_cast<ImageSet*>(static_cast<PictureBase*>(base)->_image);
	_static983 = static_cast<PictureBase*>(base)->_static;
}

// 0041e470 (o RFWnd::PreInitialize embutido)
void RFPicture::PreInitialize()
{
	RFWnd::PreInitialize();
	_brush983 = 0;
	_director = 0;
}

// 0041e4d0: a imagem só é apagada aqui se não veio da estrutura do XML (SetPicture/SetPatchPicture)
void RFPicture::Destroy()
{
	if (_director)
	{
		delete _director;
		_director = 0;
	}
	if (!static_cast<PictureBase*>(_base)->_image && _imageSet983)
	{
		delete _imageSet983;
		_imageSet983 = 0;
	}
	RFWnd::Destroy();
}

// 0041e520: animação = o caminho do ImageSet é o arquivo de animação
void RFPicture::LoadResource()
{
	if (!_static983)
	{
		LoadAnimation_(std::wstring(L"ANIMPICTURE"), static_cast<SingleImageSet*>(_imageSet983)->_path);
		_director->PlayIndex983(0);
	}
	else
		_brush983 = RFBrush::Create(_imageSet983);
}

// 0041e630
void RFPicture::OnDestroy()
{
	if (_brush983)
	{
		delete _brush983;
		_brush983 = 0;
	}
}

// 0041e660 (sem chamar o RFWnd::OnLayoutChanged)
void RFPicture::OnLayoutChanged(unsigned int flags)
{
	if (flags & 2)
		ApplyAnchor();
	if ((flags & 1) && _director)
		_director->SetPosition(GetScreenPos());
}

// 0041e6c0
void RFPicture::LoadAnimation(const std::wstring& name)
{
	if (_director)
	{
		delete _director;
		_director = 0;
	}
	_static983 = false;
	LoadAnimation_(std::wstring(L"ANIMPICTURE"), name);
	_director->PlayIndex983(0);
}

// 0041e7d0
void RFPicture::UnloadAnimation()
{
	if (_director)
	{
		delete _director;
		_director = 0;
	}
}

// 0041e800
R_ERRORTYPE RFPicture::SetAnimation(RFDirector* director)
{
	if (!director)
		return R_FAIL983;
	_static983 = false;
	if (_director)
	{
		delete _director;
		_director = 0;
	}
	_director = director;
	_director->SetPosition(RFPictureScreenPos983(this));
	_director->PlayIndex983(0);
	return R_OK983;
}

// 0041e900 (o nome da cena não é usado)
void RFPicture::LoadAnimation_(const std::wstring&, const std::wstring& file)
{
	_director = new RFDirector();
	_director->LoadAnimation(file);
	_director->SetPosition(RFPictureScreenPos983(this));
}

// 0041e9d0
void RFPicture::SetAnimPos(float x, float y)
{
	_director->SetPosition(RFVector2(x, y));
}

// 0041ea00: sem nome = a 1ª cena (RFDirector::Play embutido)
void RFPicture::Play(const std::wstring& name)
{
	RFDirector* director = _director;
	if (director)
	{
		if (name.empty())
			director->PlayIndex983(0);
		else
		{
			RFAnimation* animation = director->GetAnimation(name);
			if (animation)
			{
				director->_playing = animation;
				animation->Stop983();
				animation->Play983();
			}
		}
	}
}

// 0041ea80 (RFDirector::Stop embutido)
void RFPicture::Stop()
{
	if (_director && _director->_playing)
		_director->_playing->Stop983();
}

// 0041eaa0 (RFDirector::IsPlaying embutido)
bool RFPicture::IsPlaying() const
{
	if (_director && _director->_playing)
		return _director->_playing->_playing983;
	return false;
}

// 0041eac0: a animação anda só com a janela (e os pais) visível
void RFPicture::OnUpdate(UpdateEventArgs& e)
{
	if (!_hiding && _visible && (!_super || _super->IsWindowVisible(false)))
	{
		if (!_static983 && _director->_playing)
			_director->_playing->FrameMove983(e.delta);
	}
	RFWnd::OnUpdate(e);
}

// 0041eb20
void RFPicture::OnDraw()
{
	RFRect clip;
	clip.IntersectRect(_rc._clipRect, ClientToScreen());
	if (_rc._clip)
		g_pRFRenderer983->PushClip983(clip);
	if (!_static983)
	{
		_director->SetPosition(RFPictureScreenPos983(this));
		float alpha = GetAlpha(false);
		if (_director->_playing)
			_director->_playing->Draw983(alpha);
	}
	else if (_brush983)
		_brush983->Draw(GetScreenPos(), GetSize(), GetAlpha(false), _rc._depth);
	RFRect rect;
	rect.IntersectRect(_rc._clipRect, ClientToScreen());
	_rc._clipRect = rect;
	if (_rc._clip)
		g_pRFRenderer983->PopClip983();
}

// solta a imagem atual: a da estrutura do XML por referência, a própria com delete (embutido no SetPicture/SetPatchPicture)
static __forceinline void RFPictureReleaseImage983(RFPicture* picture)
{
	PictureBase* base = static_cast<PictureBase*>(picture->_base);
	if (base->_image)
	{
		RF_SAFE_RELEASE983(base->_image);
		picture->_imageSet983 = 0;
	}
	else if (picture->_imageSet983)
	{
		delete picture->_imageSet983;
		picture->_imageSet983 = 0;
	}
	if (picture->_brush983)
	{
		delete picture->_brush983;
		picture->_brush983 = 0;
	}
}

// 0041ed00
void RFPicture::SetPicture(const std::wstring& path)
{
	_static983 = true;
	RFPictureReleaseImage983(this);
	if (!path.empty())
	{
		SingleImageSet* imageSet = new SingleImageSet;
		_imageSet983 = imageSet;
		imageSet->_path.assign(path, 0, std::wstring::npos);
		_brush983 = RFBrush::Create(_imageSet983);
	}
}

// 0041ede0
void RFPicture::SetPatchPicture(const std::wstring& path, float left, float top, float right, float bottom)
{
	_static983 = true;
	RFPictureReleaseImage983(this);
	if (!path.empty())
	{
		NinePatchImageSet* imageSet = new NinePatchImageSet;
		_imageSet983 = imageSet;
		imageSet->_path.assign(path, 0, std::wstring::npos);
		static_cast<NinePatchImageSet*>(_imageSet983)->_left = left;
		static_cast<NinePatchImageSet*>(_imageSet983)->_top = top;
		static_cast<NinePatchImageSet*>(_imageSet983)->_right = right;
		static_cast<NinePatchImageSet*>(_imageSet983)->_bottom = bottom;
		_brush983 = RFBrush::Create(_imageSet983);
	}
}

// 0041eef0
RFSize RFPicture::GetTextureSize() const
{
	if (_static983 && _brush983)
		return _brush983->GetImageSize();
	return RFSize(0.0f, 0.0f);
}

// 0041ef30 (2 = tamanho)
void RFPicture::FitToContent()
{
	if (_static983 && _brush983)
	{
		RFSize size = _brush983->GetImageSize();
		if (size.width != _localSize.width || size.height != _localSize.height)
		{
			_localSize = size;
			OnLayoutChanged(2);
		}
	}
}
