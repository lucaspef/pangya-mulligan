#pragma once
// LZO1X: codificador e decodificador próprios do PangYa Mulligan, escritos a partir da descrição pública do
// formato (fluxo compatível com o lzo1x_decompress usado pelos dados e pacotes do jogo). Licença: AGPL-3.0.
#include <stddef.h>

enum
{
	LZO1X_OK = 0,
	LZO1X_ERROR_INPUT = -1,   // fluxo truncado ou sem o marcador de fim
	LZO1X_ERROR_OUTPUT = -2,  // saída maior que a capacidade informada
	LZO1X_ERROR_MATCH = -3    // match aponta para antes do início da saída
};

// Pior caso de saída de Lzo1xCompress para `size` bytes de entrada.
inline size_t Lzo1xCompressBound(size_t size)
{
	return size + size / 16 + 64 + 3;
}

// Devolvem LZO1X_OK e gravam em *outSize o total escrito.
int Lzo1xCompress(const unsigned char* src, size_t srcSize, unsigned char* dst, size_t dstCapacity, size_t* outSize);
int Lzo1xDecompress(const unsigned char* src, size_t srcSize, unsigned char* dst, size_t dstCapacity, size_t* outSize);
