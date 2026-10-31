#include "lzo1x.h"
#include <string.h>
#include <vector>

namespace
{
const size_t kMinMatch = 3, kMaxDistance = 49151, kM2Distance = 2048, kM3Distance = 16384;
const int kHashBits = 15;

inline unsigned Hash3(const unsigned char* p)
{
	const unsigned v = p[0] | (p[1] << 8) | (p[2] << 16);
	return (v * 0x9E3779B1u) >> (32 - kHashBits);
}

// Comprimento estendido: cada zero soma 255 e o primeiro byte diferente de zero fecha a conta.
inline unsigned char* WriteExtension(unsigned char* op, size_t rest)
{
	for (; rest > 255; rest -= 255)
		*op++ = 0;
	*op++ = (unsigned char)rest;
	return op;
}

inline bool ReadExtension(const unsigned char*& ip, const unsigned char* end, size_t& value)
{
	for (; ip < end && *ip == 0; ++ip)
		value += 255;
	if (ip >= end)
		return false;
	value += *ip++;
	return true;
}

unsigned char* WriteLiterals(unsigned char* op, const unsigned char* start, const unsigned char* src, size_t count)
{
	if (count == 0)
		return op;
	if (op == start && count <= 238)
		*op++ = (unsigned char)(17 + count);
	else if (count <= 3)
		op[-2] |= (unsigned char)count;  // os 2 bits baixos do penúltimo byte do match anterior contam os literais
	else if (count <= 18)
		*op++ = (unsigned char)(count - 3);
	else
	{
		*op++ = 0;
		op = WriteExtension(op, count - 18);
	}
	memcpy(op, src, count);
	return op + count;
}

unsigned char* WriteMatch(unsigned char* op, size_t length, size_t distance)
{
	if (length <= 8 && distance <= kM2Distance)
	{
		*op++ = (unsigned char)(((length - 1) << 5) | (((distance - 1) & 7) << 2));
		*op++ = (unsigned char)((distance - 1) >> 3);
		return op;
	}
	size_t tail;
	if (distance <= kM3Distance)
	{
		tail = (distance - 1) << 2;
		if (length <= 33)
			*op++ = (unsigned char)(32 | (length - 2));
		else
		{
			*op++ = 32;
			op = WriteExtension(op, length - 33);
		}
	}
	else
	{
		const size_t far = distance - kM3Distance;
		tail = (far & 0x3FFF) << 2;
		const unsigned char high = (unsigned char)(16 | ((far >> 11) & 8));
		if (length <= 9)
			*op++ = (unsigned char)(high | (length - 2));
		else
		{
			*op++ = high;
			op = WriteExtension(op, length - 9);
		}
	}
	*op++ = (unsigned char)tail;
	*op++ = (unsigned char)(tail >> 8);
	return op;
}
}

int Lzo1xCompress(const unsigned char* src, size_t srcSize, unsigned char* dst, size_t dstCapacity, size_t* outSize)
{
	if (dstCapacity < Lzo1xCompressBound(srcSize))
		return LZO1X_ERROR_OUTPUT;

	std::vector<unsigned> table((size_t)1 << kHashBits);  // posição + 1; 0 = vazio
	unsigned char* op = dst;
	size_t ip = 0, literalStart = 0;

	while (ip + kMinMatch < srcSize)
	{
		const unsigned hash = Hash3(src + ip);
		const size_t candidate = table[hash];
		table[hash] = (unsigned)(ip + 1);

		size_t length = 0;
		if (candidate != 0 && ip - (candidate - 1) <= kMaxDistance)
		{
			const unsigned char* a = src + candidate - 1;
			const unsigned char* b = src + ip;
			const size_t limit = srcSize - ip;
			while (length < limit && a[length] == b[length])
				++length;
		}
		if (length < kMinMatch)
		{
			++ip;
			continue;
		}

		op = WriteLiterals(op, dst, src + literalStart, ip - literalStart);
		op = WriteMatch(op, length, ip - (candidate - 1));
		const size_t next = ip + length;
		for (++ip; ip < next && ip + kMinMatch <= srcSize; ++ip)
			table[Hash3(src + ip)] = (unsigned)(ip + 1);
		ip = literalStart = next;
	}

	op = WriteLiterals(op, dst, src + literalStart, srcSize - literalStart);
	*op++ = 0x11;
	*op++ = 0;
	*op++ = 0;
	*outSize = (size_t)(op - dst);
	return LZO1X_OK;
}

int Lzo1xDecompress(const unsigned char* src, size_t srcSize, unsigned char* dst, size_t dstCapacity, size_t* outSize)
{
	const unsigned char* ip = src;
	const unsigned char* const end = src + srcSize;
	unsigned char* op = dst;
	unsigned char* const opEnd = dst + dstCapacity;
	size_t state = 0;  // literais da instrução anterior: 0, 1..3, ou 4 depois de uma série longa

#define NEED_IN(n) do { if ((size_t)(end - ip) < (size_t)(n)) return LZO1X_ERROR_INPUT; } while (0)
#define NEED_OUT(n) do { if ((size_t)(opEnd - op) < (size_t)(n)) return LZO1X_ERROR_OUTPUT; } while (0)

	NEED_IN(1);
	if (*ip > 17)
	{
		const size_t count = *ip++ - 17u;
		NEED_IN(count);
		NEED_OUT(count);
		memcpy(op, ip, count);
		ip += count;
		op += count;
		state = count < 4 ? count : 4;
	}

	for (;;)
	{
		NEED_IN(1);
		const unsigned op0 = *ip++;
		size_t length, distance, tail;

		if (op0 < 16)
		{
			if (state == 0)
			{
				length = 3u + op0;
				if (op0 == 0)
				{
					length = 18;
					if (!ReadExtension(ip, end, length))
						return LZO1X_ERROR_INPUT;
				}
				NEED_IN(length);
				NEED_OUT(length);
				memcpy(op, ip, length);
				ip += length;
				op += length;
				state = 4;
				continue;
			}
			NEED_IN(1);
			distance = ((size_t)*ip++ << 2) + (op0 >> 2) + (state == 4 ? 2049 : 1);
			length = state == 4 ? 3 : 2;
			tail = op0 & 3;
		}
		else if (op0 >= 64)
		{
			NEED_IN(1);
			distance = ((size_t)*ip++ << 3) + ((op0 >> 2) & 7) + 1;
			length = op0 >= 128 ? 5u + ((op0 >> 5) & 3) : 3u + ((op0 >> 5) & 1);
			tail = op0 & 3;
		}
		else
		{
			const bool nearForm = op0 >= 32;
			const unsigned mask = nearForm ? 31 : 7;
			length = 2u + (op0 & mask);
			if ((op0 & mask) == 0)
			{
				length = 2u + mask;
				if (!ReadExtension(ip, end, length))
					return LZO1X_ERROR_INPUT;
			}
			NEED_IN(2);
			const size_t word = ip[0] | ((size_t)ip[1] << 8);
			ip += 2;
			distance = (word >> 2) + 1;
			tail = word & 3;
			if (!nearForm)
			{
				distance = 16384 + ((size_t)(op0 & 8) << 11) + (word >> 2);
				if (distance == 16384)  // marcador de fim
				{
					*outSize = (size_t)(op - dst);
					return length == 3 && ip == end ? LZO1X_OK : LZO1X_ERROR_INPUT;
				}
			}
		}

		if ((size_t)(op - dst) < distance)
			return LZO1X_ERROR_MATCH;
		NEED_OUT(length);
		for (const unsigned char* from = op - distance; length != 0; --length)
			*op++ = *from++;  // byte a byte: origem e destino podem se sobrepor

		NEED_IN(tail);
		NEED_OUT(tail);
		memcpy(op, ip, tail);
		ip += tail;
		op += tail;
		state = tail;
	}
#undef NEED_IN
#undef NEED_OUT
}
