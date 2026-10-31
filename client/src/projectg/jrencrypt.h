#pragma once

void SimpleStreamEncrypt_Alpha(const char* src, char* tar, unsigned int len,
	unsigned int key);
void SimpleStreamDecrypt_Alpha(const char* src, char* tar, unsigned int len,
	unsigned int key);
void SimpleStreamEncrypt_Delta(const char* src, char* tar, unsigned int len,
	unsigned int key);
void SimpleStreamDecrypt_Delta(const char* src, char* tar, unsigned int len,
	unsigned int key);
