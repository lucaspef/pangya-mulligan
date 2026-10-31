#pragma once

#include "msg983.h"

class IActor;

#ifndef MSGSEL983_DEFINED
#define MSGSEL983_DEFINED
template<class T> struct MsgSel983 { };
#endif
class MsgObject
{
public:
	MsgObject() { }
	MsgObject(const IActor* _sender, int _message, int _param1, int _param2,
		int _param3, int _param4, int _param5)
		: sender(_sender),
		  message(_message),
		  param1(_param1),
		  param2(_param2),
		  param3(_param3),
		  param4(_param4),
		  param5(_param5)
	{
		time = 0;
	}

	// JP 983: leitores de parametro por tipo, fora de linha (004bb1d0...004bb300 chamam 004bad80...004baee0 com um seletor
	// vazio de 1 byte na pilha) - pedido B14/B15. Nomes nossos.
	template<class T> T Param2() const;
	template<class T> T Param2(MsgSel983<T>) const;
	template<class T> T Param3() const;
	template<class T> T Param3(MsgSel983<T>) const;
	template<class T> T Param4() const;
	template<class T> T Param4(MsgSel983<T>) const;

	const IActor* sender;
	int message;
	int param1;
	int param2;
	int param3;
	int param4;
	int param5;
	unsigned long time;
};
template<class T> T MsgObject::Param2() const { return Param2(MsgSel983<T>()); }
template<class T> T MsgObject::Param2(MsgSel983<T>) const { return (T)param2; }
template<class T> T MsgObject::Param3() const { return Param3(MsgSel983<T>()); }
template<class T> T MsgObject::Param3(MsgSel983<T>) const { return (T)param3; }
template<class T> T MsgObject::Param4() const { return Param4(MsgSel983<T>()); }
template<class T> T MsgObject::Param4(MsgSel983<T>) const { return (T)param4; }
