#pragma once

#define ENCRYPTION_KEY "{782AE110-2EEF-4c61-B030-A53F17634F7D}"
#define ENCRYPTION_KEY_LEN (sizeof(ENCRYPTION_KEY) - 1)

template <typename T>
static unsigned int Decrypt(T* src, unsigned int srclen)
{
	unsigned char* data = (unsigned char*)src;

	unsigned int i = 0;
	if (srclen < ENCRYPTION_KEY_LEN)
	{
		for (unsigned int j = 0; j < ENCRYPTION_KEY_LEN; j++)
		{
			data[i++] ^= ENCRYPTION_KEY[j];
			if (i >= srclen)
				i = 0;
		}
		return ENCRYPTION_KEY_LEN;
	}

	for (unsigned int j = 0; j < srclen; j++)
	{
		data[j] ^= ENCRYPTION_KEY[i++];
		if (i >= ENCRYPTION_KEY_LEN)
			i = 0;
	}
	return srclen;
}
