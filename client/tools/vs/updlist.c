#define _CRT_SECURE_NO_WARNINGS
/* updlist.c: pós-build do cliente, refaz fcrc/fsize de arquivos no updatelist cifrado do jogo (XTEA de 16 rodadas e CRC do IntegrityPakCRC).
   Uso: updlist <updatelist> <arquivo>...   (cada arquivo casa a linha <fileinfo ... fname="<nome do arquivo>" ...>)
        updlist --generate <pasta do jogo> <updatelist>   (cria a lista do zero: todos os *.pak, ProjectG.exe, wangreal.dll e ijl15.dll)
   Compilado pelo projeto proj/vs2022/updlist.vcxproj. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>

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

/* = encrypt do updatelist.py: ao menos um \0 depois do texto, completar a 8 */
static int write_cipher(const char *path, const char *xml, long len)
{
    long n = len + 1, i; unsigned char *cif; FILE *f;
    n += (8 - n % 8) % 8;
    cif = (unsigned char *)calloc(n, 1);
    memcpy(cif, xml, len);
    for (i = 0; i < n; i += 8) enc((u32 *)(cif + i));
    f = fopen(path, "wb");
    if (!f || fwrite(cif, 1, n, f) != (size_t)n) { fprintf(stderr, "updlist: erro ao gravar %s\n", path); return 1; }
    fclose(f);
    free(cif);
    printf("updlist: %s gravado (%ld B)\n", path, n);
    return 0;
}

/* CRC em blocos: os paks passam de 1 GB e o processo tem 32 bits */
static int file_crc(const char *path, u32 *crc, unsigned long long *size)
{
    static unsigned char b[1 << 20]; size_t r, i; u32 c = 0xffffffffu; FILE *f = fopen(path, "rb");
    if (!f) return 0;
    for (*size = 0; (r = fread(b, 1, sizeof b, f)) > 0; *size += r)
        for (i = 0; i < r; i++) c = (c >> 8) ^ T[(c ^ b[i]) & 0xff];
    fclose(f);
    *crc = c ^ 0xffffffffu;
    return 1;
}

static int by_name(const void *a, const void *b) { return _stricmp(*(char *const *)a, *(char *const *)b); }

/* lista nova da pasta do jogo. A ordem importa: o cliente ordena por numero com sort estavel, e 838 vem antes de 838a */
static int generate(const char *dir, const char *out)
{
    static const char *const required[] = { "ProjectG.exe", "wangreal.dll", "ijl15.dll" };
    enum { MAX_FILES = 4096, LINE = 400 };
    static char *names[MAX_FILES];
    char path[1024], *xml; struct __finddata64_t fd; intptr_t h; int n = 0, i; long len;
    for (i = 0; i < 3; i++) names[n++] = _strdup(required[i]);
    sprintf(path, "%s\\*.pak", dir);
    if ((h = _findfirst64(path, &fd)) != -1) {
        do {
            size_t l = strlen(fd.name);
            if (l > 4 && !_stricmp(fd.name + l - 4, ".pak") && n < MAX_FILES) names[n++] = _strdup(fd.name);
        } while (!_findnext64(h, &fd));
        _findclose(h);
    }
    if (n == 3) { fprintf(stderr, "updlist: nenhum .pak em %s\n", dir); return 1; }
    qsort(names, n, sizeof *names, by_name);
    xml = (char *)malloc(256 + (size_t)n * LINE);
    len = sprintf(xml, "<?xml version=\"1.0\" encoding=\"euc-kr\" standalone=\"yes\" ?>\r\n<patchVer value=\"JP.R7.983.00\" />\r\n"
        "<patchNum value=\"1\" />\r\n<updatelistVer value=\"1\" />\r\n<updatefiles count=\"%d\">\r\n", n);
    for (i = 0; i < n; i++) {
        u32 c; unsigned long long size; size_t l = strlen(names[i]);
        sprintf(path, "%s\\%s", dir, names[i]);
        if (!file_crc(path, &c, &size)) { fprintf(stderr, "updlist: %s nao lido\n", path); return 1; }
        len += sprintf(xml + len, "\t<fileinfo fname=\"%s\" fdir=\"\\\" fsize=\"%llu\" fcrc=\"%d\" />\r\n", names[i], size, (int)c);
        if (_stricmp(names[i] + l - 4, ".pak"))
            printf("updlist: %s fsize=%llu fcrc=%d (0x%08x)\n", names[i], size, (int)c, c);
    }
    len += sprintf(xml + len, "</updatefiles>");
    printf("updlist: %d arquivos\n", n);
    return write_cipher(out, xml, len);
}

int main(int argc, char **argv)
{
    long n, len, i, k; unsigned char *cif; char *xml; int a, erro = 0;
    if (argc < 3) { fprintf(stderr, "uso: updlist <updatelist> <arquivo>...\n     updlist --generate <pasta do jogo> <updatelist>\n"); return 2; }
    for (i = 0; i < 256; i++) {
        u32 x = refl((u32)i, 8) << 24; int j;
        for (j = 0; j < 8; j++) x = (x << 1) ^ ((x & 0x80000000u) ? 0xedb88320u : 0);
        T[i] = refl(x, 32);
    }
    if (!strcmp(argv[1], "--generate")) return argc == 4 ? generate(argv[2], argv[3]) : 2;
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
    return write_cipher(argv[1], xml, len);
}
