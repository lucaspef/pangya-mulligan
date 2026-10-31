#pragma once

class NetworkState
{
public:
	NetworkState();
	virtual ~NetworkState();

	// So Process() recuperado da vtable do 983 (decisao F5: nada mais e chamado)
	virtual void Process() = 0;
};
