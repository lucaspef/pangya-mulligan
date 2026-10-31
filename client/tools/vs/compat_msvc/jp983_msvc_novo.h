// Forçado (/FI) só no toolset novo pela tools/vs/sonda_msvc.py (projeto do VS, D734 P3/P4). O build VS2010 não vê isto.
// P3: o STL do VC10 trazia <functional> (std::binary_function) pelo <map>; o novo não.
// P4: std::tr1 saiu do STL novo; o fonte usa tr1::shared_ptr/function/array/unordered_set (os mesmos do std).
#pragma once
#ifdef __cplusplus
#include <functional>
#include <memory>
#include <array>
#include <unordered_set>
namespace std { namespace tr1 {
	using std::shared_ptr; using std::weak_ptr; using std::function; using std::array; using std::unordered_set;
	using std::enable_shared_from_this; using std::bind;
} }
// D737: internos do STL do VC10 chamados direto (itemcursor983.cpp, _Buynode do 983 004795d0): _Allocate(n, T*) e
// _Cons_val(alloc, p, v). Mesma semântica do VC10 (operator new de n*sizeof(T); construção por cópia no lugar).
#include <new>
namespace std {
	template <class _Ty> inline _Ty* _Allocate(size_t _Count, _Ty*)
	{ return static_cast<_Ty*>(::operator new(_Count * sizeof(_Ty))); }
	template <class _Alloc, class _Ty1, class _Ty2> inline void _Cons_val(_Alloc&, _Ty1* _Pdest, const _Ty2& _Src)
	{ ::new (static_cast<void*>(_Pdest)) _Ty1(_Src); }
}
#endif
