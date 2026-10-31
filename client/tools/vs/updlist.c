#define _CRT_SECURE_NO_WARNINGS
/* updlist.c: pós-build do cliente, refaz fcrc/fsize de arquivos no updatelist cifrado do jogo (XTEA de 16 rodadas e CRC do IntegrityPakCRC).
   Uso: updlist <updatelist> <arquivo>...   (cada arquivo casa a linha <fileinfo ... fname="<nome do arquivo>" ...>)
   Compilado pelo projeto proj/vs2022/updlist.vcxproj. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef unsigned int u32;
static const u32 K[4] = { 0x020a5fd4, 0x01eebdff, 0x02b3c6a0, 0x04f6a3e1 };
#define DLT 0x9e3779b9u

static void dec(u32 *v)
{
    u32 v0 = v[0], v1 = v[1], s = DLT * 16; int i;
    for (i = 0; i < 16; i++) {
        v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (s + K[(s >> 11) & 3]);
        s -= DLT;
        v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (s + K[s & 3]);
    }
    v[0] = v0; v[1] = v1;
}

static void enc(u32 *v)
{
    u32 v0 = v[0], v1 = v[1], s = 0; int i;
    for (i = 0; i < 16; i++) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (s + K[s & 3]);
        s += DLT;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (s + K[(s >> 11) & 3]);
    }
    v[0] = v0; v[1] = v1;
}

static u32 T[256];

static u32 refl(u32 v, int n)
{
    u32 r = 0; int i;
    for (i = 1; i <= n; i++) { if (v & 1) r |= 1u << (n - i); v >>= 1; }
    return r;
}

static unsigned char *ler(const char *p, long *n)
{
    FILE *f = fopen(p, "rb"); unsigned char *b;
    if (!f) return 0;
    fseek(f, 0, SEEK_END); *n = ftell(f); fseek(f, 0, SEEK_SET);
    b = (unsigned char *)malloc(*n + 16);
    if (fread(b, 1, *n, f) != (size_t)*n) { fclose(f); free(b); return 0; }
    fclose(f);
    return b;
}

/* troca o valor do atributo at="..." dentro de [ini, *fim) por val; ajusta *fim e *len */
static int troca_attr(char *buf, long *len, long ini, long *fim, const char *at, const char *val)
{
    char chave[16]; long i, a, b, d; size_t nk;
    sprintf(chave, "%s=\"", at); nk = strlen(chave);
    for (i = ini; i + (long)nk < *fim; i++)
        if (!memcmp(buf + i, chave, nk)) break;
    if (i + (long)nk >= *fim) return 0;
    a = i + nk; for (b = a; b < *fim && buf[b] != '"'; b++) ;
    d = (long)strlen(val) - (b - a);
    memmove(buf + b + d, buf + b, *len - b);
    memcpy(buf + a, val, strlen(val));
    *len += d; *fim += d;
    return 1;
}

int main(int argc, char **argv)
{
    long n, len, i, k; unsigned char *cif; char *xml; int a, erro = 0; FILE *f;
    if (argc < 3) { fprintf(stderr, "uso: updlist <updatelist> <arquivo>...\n"); return 2; }
    for (i = 0; i < 256; i++) {
        u32 x = refl((u32)i, 8) << 24; int j;
        for (j = 0; j < 8; j++) x = (x << 1) ^ ((x & 0x80000000u) ? 0xedb88320u : 0);
        T[i] = refl(x, 32);
    }
    cif = ler(argv[1], &n);
    if (!cif || n % 8) { fprintf(stderr, "updlist: %s ausente ou tamanho nao multiplo de 8\n", argv[1]); return 1; }
    for (i = 0; i < n; i += 8) dec((u32 *)(cif + i));
    len = n; while (len && !cif[len - 1]) len--;
    xml = (char *)malloc(len + 64 * (argc - 2) + 16);
    memcpy(xml, cif, len);
    for (a = 2; a < argc; a++) {
        long tam, ini, fim; unsigned char *d = ler(argv[a], &tam); u32 c = 0xffffffffu;
        const char *nome = argv[a] + strlen(argv[a]); char pad[300], v[16];
        while (nome > argv[a] && nome[-1] != '\\' && nome[-1] != '/') nome--;
        if (!d) { fprintf(stderr, "updlist: %s nao lido\n", argv[a]); erro = 1; continue; }
        for (i = 0; i < tam; i++) c = (c >> 8) ^ T[(c ^ d[i]) & 0xff];
        c ^= 0xffffffffu; free(d);
        sprintf(pad, "fname=\"%s\"", nome);
        for (ini = 0; ini + (long)strlen(pad) <= len; ini++) {
            for (k = 0; pad[k] && tolower((unsigned char)xml[ini + k]) == tolower((unsigned char)pad[k]); k++) ;
            if (!pad[k]) break;
        }
        if (ini + (long)strlen(pad) > len) { fprintf(stderr, "updlist: linha %s nao achada\n", pad); erro = 1; continue; }
        while (ini > 0 && xml[ini] != '<') ini--;
        for (fim = ini; fim < len && xml[fim] != '>'; fim++) ;
        sprintf(v, "%d", (int)c);       /* atoi do 983: int32 com sinal */
        k = troca_attr(xml, &len, ini, &fim, "fcrc", v);
        sprintf(v, "%ld", tam);
        k &= troca_attr(xml, &len, ini, &fim, "fsize", v);
        if (!k) { fprintf(stderr, "updlist: fcrc/fsize faltando em %s\n", pad); erro = 1; continue; }
        printf("updlist: %s fsize=%ld fcrc=%d (0x%08x)\n", nome, tam, (int)c, c);
    }
    if (erro) return 1;
    /* = encrypt do updatelist.py: ao menos um \0 depois do texto, completar a 8 */
    n = len + 1; n += (8 - n % 8) % 8;
    cif = (unsigned char *)realloc(cif, n);
    memcpy(cif, xml, len); memset(cif + len, 0, n - len);
    for (i = 0; i < n; i += 8) enc((u32 *)(cif + i));
    f = fopen(argv[1], "wb");
    if (!f || fwrite(cif, 1, n, f) != (size_t)n) { fprintf(stderr, "updlist: erro ao gravar %s\n", argv[1]); return 1; }
    fclose(f);
    printf("updlist: %s gravado (%ld B)\n", argv[1], n);
    return 0;
}
