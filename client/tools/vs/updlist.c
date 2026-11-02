#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <io.h>
#include <stddef.h>
#include <stdarg.h>
#include <windows.h>
#include <errno.h>

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

typedef enum {
    LOG_INFO,
    LOG_SUCCESS,
    LOG_WARNING,
    LOG_ERROR
} LogType;

static HANDLE g_console = NULL;
static WORD g_default_color = 0;
static FILE* g_log = NULL;


typedef uint32_t u32;
typedef uint64_t u64;

static const u32 K[4] = {
    0x020a5fd4u,
    0x01eebdffu,
    0x02b3c6a0u,
    0x04f6a3e1u
};

#define DLT 0x9e3779b9u
#define CRC_POLY 0xedb88320u
#define CRC_BUFFER_SIZE (1024u * 1024u)

static u32 T[256];

/* Prototipos */

static void crc_init(void);

static void dec(u32* v);
static void enc(u32* v);

static void dec_block(unsigned char* p);
static void enc_block(unsigned char* p);

static unsigned char* ler(const char* path, u64* size);

static int ler_ini(
    const char* filename,
    char* game_dir,
    size_t game_max,
    char* update_dir,
    size_t update_max,
    char* output_name,
    size_t output_max
);

static int write_cipher(
    const char* path,
    const char* xml,
    size_t len
);

static int file_crc(
    const char* path,
    u32* crc,
    u64* size
);

static int troca_attr(
    char** buf,
    size_t* len,
    size_t* capacity,
    size_t ini,
    size_t* fim,
    const char* at,
    const char* val
);

static int generate(
    const char* dir,
    const char* out
);

static int by_name(
    const void* a,
    const void* b
);

static int make_path(
    char* output,
    size_t capacity,
    const char* directory,
    const char* filename
);

static int reserve_buffer(
    char** buffer,
    size_t* capacity,
    size_t required
);

/* ============================================================
   CRC32
   ============================================================ */

static void crc_init(void)
{
    u32 i;
    int j;

    for (i = 0; i < 256; i++) {
        u32 crc = i;

        for (j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ CRC_POLY;
            else
                crc >>= 1;
        }

        T[i] = crc;
    }
}

/* ============================================================
   XTEA
   ============================================================ */

static void dec(u32* v)
{
    u32 v0 = v[0];
    u32 v1 = v[1];
    u32 sum = 16u * DLT;

    int i;

    for (i = 0; i < 16; i++) {

        v1 -= (
            (((v0 << 4) ^ (v0 >> 5)) + v0)
            ^ (sum + K[(sum >> 11) & 3])
            );

        sum -= DLT;

        v0 -= (
            (((v1 << 4) ^ (v1 >> 5)) + v1)
            ^ (sum + K[sum & 3])
            );
    }

    v[0] = v0;
    v[1] = v1;
}

static void enc(u32* v)
{
    u32 v0 = v[0];
    u32 v1 = v[1];
    u32 sum = 0;

    int i;

    for (i = 0; i < 16; i++) {

        v0 += (
            (((v1 << 4) ^ (v1 >> 5)) + v1)
            ^ (sum + K[sum & 3])
            );

        sum += DLT;

        v1 += (
            (((v0 << 4) ^ (v0 >> 5)) + v0)
            ^ (sum + K[(sum >> 11) & 3])
            );
    }

    v[0] = v0;
    v[1] = v1;
}

/* ============================================================
   XTEA - acesso seguro a blocos
   ============================================================ */

static u32 load_u32(const unsigned char* p)
{
    return
        ((u32)p[0]) |
        ((u32)p[1] << 8) |
        ((u32)p[2] << 16) |
        ((u32)p[3] << 24);
}

static void store_u32(unsigned char* p, u32 value)
{
    p[0] = (unsigned char)value;
    p[1] = (unsigned char)(value >> 8);
    p[2] = (unsigned char)(value >> 16);
    p[3] = (unsigned char)(value >> 24);
}

static void dec_block(unsigned char* p)
{
    u32 v[2];

    v[0] = load_u32(p);
    v[1] = load_u32(p + 4);

    dec(v);

    store_u32(p, v[0]);
    store_u32(p + 4, v[1]);
}

static void enc_block(unsigned char* p)
{
    u32 v[2];

    v[0] = load_u32(p);
    v[1] = load_u32(p + 4);

    enc(v);

    store_u32(p, v[0]);
    store_u32(p + 4, v[1]);
}


static void log_init(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    g_console = GetStdHandle(STD_OUTPUT_HANDLE);

    if (g_console != INVALID_HANDLE_VALUE &&
        g_console != NULL &&
        GetConsoleScreenBufferInfo(g_console, &info)) {

        g_default_color = info.wAttributes;
    }
    else {
        g_console = NULL;
    }
}

static void log_color(LogType type)
{
    WORD color;

    if (!g_console)
        return;

    switch (type) {

    case LOG_SUCCESS:
        color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;

    case LOG_WARNING:
        color = FOREGROUND_RED |
            FOREGROUND_GREEN |
            FOREGROUND_INTENSITY;
        break;

    case LOG_ERROR:
        color = FOREGROUND_RED | FOREGROUND_INTENSITY;
        break;

    case LOG_INFO:
    default:
        color = FOREGROUND_GREEN |
            FOREGROUND_BLUE |
            FOREGROUND_INTENSITY;
        break;
    }

    SetConsoleTextAttribute(g_console, color);
}

static void log_reset_color(void)
{
    if (g_console)
        SetConsoleTextAttribute(g_console, g_default_color);
}

static void log_msg(LogType type, const char* format, ...)
{
    va_list args;
    va_list copy;
    const char* prefix;

    switch (type) {

    case LOG_SUCCESS:
        prefix = "[SUCESSO] ";
        break;

    case LOG_WARNING:
        prefix = "[ATENCAO] ";
        break;

    case LOG_ERROR:
        prefix = "[ERRO] ";
        break;

    case LOG_INFO:
    default:
        prefix = "[INFO] ";
        break;
    }

    va_start(args, format);
    va_copy(copy, args);

    log_color(type);

    printf("%s", prefix);
    vprintf(format, args);

    if (g_log) {
        fprintf(g_log, "%s", prefix);
        vfprintf(g_log, format, copy);
        fflush(g_log);
    }

    va_end(copy);
    va_end(args);

    log_reset_color();
    fflush(stdout);
}
/* ============================================================
   Leitura de arquivos
   ============================================================ */

static unsigned char* ler(const char* path, u64* size)
{
    FILE* f;
    __int64 file_size;
    unsigned char* buffer;

    if (!path || !size)
        return NULL;

    f = fopen(path, "rb");

    if (!f)
        return NULL;

    if (_fseeki64(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }

    file_size = _ftelli64(f);

    if (file_size < 0) {
        fclose(f);
        return NULL;
    }

    if ((u64)file_size > (u64)SIZE_MAX - 1) {
        fclose(f);
        return NULL;
    }

    if (_fseeki64(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }

    buffer = (unsigned char*)malloc((size_t)file_size + 1);

    if (!buffer) {
        fclose(f);
        return NULL;
    }

    if (fread(buffer, 1, (size_t)file_size, f) !=
        (size_t)file_size) {

        free(buffer);
        fclose(f);
        return NULL;
    }

    buffer[file_size] = 0;

    fclose(f);

    *size = (u64)file_size;

    return buffer;
}

/* ============================================================
   CRC de arquivos - leitura em blocos
   ============================================================ */

static int file_crc(
    const char* path,
    u32* crc,
    u64* size)
{
    static unsigned char buffer[CRC_BUFFER_SIZE];

    FILE* f;
    size_t read_size;
    size_t i;

    u32 current = 0xffffffffu;
    u64 total = 0;

    if (!path || !crc || !size)
        return 0;

    f = fopen(path, "rb");

    if (!f)
        return 0;

    while ((read_size = fread(
        buffer,
        1,
        sizeof(buffer),
        f)) > 0) {

        for (i = 0; i < read_size; i++) {

            current =
                (current >> 8) ^
                T[(current ^ buffer[i]) & 0xff];
        }

        total += (u64)read_size;
    }

    if (ferror(f)) {
        fclose(f);
        return 0;
    }

    fclose(f);

    *crc = current ^ 0xffffffffu;
    *size = total;

    return 1;
}

/* ============================================================
   Gerenciamento de buffer
   ============================================================ */

static int reserve_buffer(
    char** buffer,
    size_t* capacity,
    size_t required)
{
    char* new_buffer;
    size_t new_capacity;

    if (required <= *capacity)
        return 1;

    new_capacity = *capacity ? *capacity : 256;

    while (new_capacity < required) {

        if (new_capacity > SIZE_MAX / 2) {
            new_capacity = required;
            break;
        }

        new_capacity *= 2;
    }

    new_buffer = (char*)realloc(
        *buffer,
        new_capacity
    );

    if (!new_buffer)
        return 0;

    *buffer = new_buffer;
    *capacity = new_capacity;

    return 1;
}

/* ============================================================
   Substituição de atributos XML
   ============================================================ */

static int troca_attr(
    char** buf,
    size_t* len,
    size_t* capacity,
    size_t ini,
    size_t* fim,
    const char* at,
    const char* val)
{
    char chave[32];

    size_t i;
    size_t a;
    size_t b;
    size_t old_size;
    size_t new_size;
    size_t new_len;
    size_t nk;

    int diff;

    if (!buf || !len || !capacity || !fim || !at || !val)
        return 0;

    if (snprintf(chave, sizeof(chave), "%s=\"", at) >=
        (int)sizeof(chave))
        return 0;

    nk = strlen(chave);

    if (*fim > *len || ini > *fim)
        return 0;

    for (i = ini; i + nk <= *fim; i++) {

        if (memcmp(*buf + i, chave, nk) == 0)
            break;
    }

    if (i + nk > *fim)
        return 0;

    a = i + nk;

    for (b = a; b < *fim; b++) {

        if ((*buf)[b] == '"')
            break;
    }

    if (b >= *fim)
        return 0;

    old_size = b - a;
    new_size = strlen(val);

    if (new_size > SIZE_MAX - (*len - old_size))
        return 0;

    new_len = *len - old_size + new_size;

    if (new_len == SIZE_MAX)
        return 0;

    if (!reserve_buffer(
        buf,
        capacity,
        new_len + 1))
        return 0;

    diff = (int)0;

    if (new_size >= old_size) {

        if (new_size - old_size > (size_t)INT_MAX)
            return 0;

        diff = (int)(new_size - old_size);

    }
    else {

        if (old_size - new_size > (size_t)INT_MAX)
            return 0;

        diff = -(int)(old_size - new_size);
    }

    memmove(
        *buf + b + diff,
        *buf + b,
        *len - b + 1
    );

    memcpy(
        *buf + a,
        val,
        new_size
    );

    *len = new_len;
    *fim = (size_t)((ptrdiff_t)*fim + diff);

    return 1;
}

/* ============================================================
   Gravação do Updatelist criptografado
   ============================================================ */

static int write_cipher(
    const char* path,
    const char* xml,
    size_t len)
{
    size_t n;
    size_t i;

    unsigned char* cipher;
    FILE* f;

    if (!path || !xml)
        return 1;

    if (len > SIZE_MAX - 8)
        return 1;

    n = len + 1;

    n += (8 - (n % 8)) % 8;

    cipher = (unsigned char*)calloc(n, 1);

    if (!cipher) {
        fprintf(stderr,
            "updlist: memoria insuficiente para criptografia\n");

        return 1;
    }

    memcpy(cipher, xml, len);

    for (i = 0; i < n; i += 8)
        enc_block(cipher + i);

    f = fopen(path, "wb");
     
    if (!f) {
        fprintf(
            stderr,
            "[ERRO] Nao foi possivel criar o arquivo:\n"
            "       Caminho: %s\n"
            "       Codigo do Windows: %lu\n"
            "       Erro: %s\n",
            path,
            (unsigned long)GetLastError(),
            strerror(errno)
        );

        free(cipher);
        return 1;
    }

    if (fwrite(cipher, 1, n, f) != n) {

        fprintf(stderr,
            "updlist: erro ao gravar %s\n",
            path);

        fclose(f);
        free(cipher);

        return 1;
    }

    if (fclose(f) != 0) {

        fprintf(stderr,
            "updlist: erro ao fechar %s\n",
            path);

        free(cipher);
        return 1;
    }

    free(cipher);

    printf(
        "updlist: %s gravado (%zu B)\n",
        path,
        n
    );

    return 0;
}

/* ============================================================
   Comparação de nomes
   ============================================================ */

static int by_name(
    const void* a,
    const void* b)
{
    return _stricmp(
        *(char* const*)a,
        *(char* const*)b
    );
}

/* ============================================================
   Construção de caminhos
   ============================================================ */
static int make_path(
    char* output,
    size_t capacity,
    const char* directory,
    const char* filename)
{
    size_t len;
    int written;

    if (!output || !directory || !filename || capacity == 0)
        return 0;

    len = strlen(directory);

    if (len > 0 &&
        (directory[len - 1] == '\\' ||
            directory[len - 1] == '/')) {

        written = snprintf(
            output, capacity, "%s%s", directory, filename
        );
    }
    else {
        written = snprintf(
            output, capacity, "%s\\%s", directory, filename
        );
    }

    if (written < 0 || (size_t)written >= capacity) {
        output[0] = '\0';
        return 0;
    }

    return 1;
}

/* ============================================================
   Gerar Updatelist
   ============================================================ */

static int generate(
    const char* dir,
    const char* out)
{
    static const char* const required[] = {
        "ProjectG.exe",
        "wangreal.dll",
        "ijl15.dll"
    };

    enum {
        MAX_FILES = 4096
    };

    static char* names[MAX_FILES];

    struct __finddata64_t fd;

    intptr_t handle;

    char search_path[1024];
    char file_path[1024];
    char line[1024];

    char* xml = NULL;

    size_t capacity;
    size_t len;

    int count = 0;
    int i;
    int result = 1;

    u32 crc;
    u64 size;

    if (!dir || !out)
        return 1;

    memset(names, 0, sizeof(names));

    for (i = 0; i < 3; i++) {

        names[count] = _strdup(required[i]);

        if (!names[count])
            goto cleanup;

        count++;
    }

    if (!make_path(
        search_path,
        sizeof(search_path),
        dir,
        "*.pak")) {

        fprintf(stderr,
            "updlist: caminho muito grande\n");

        goto cleanup;
    }

    handle = _findfirst64(search_path, &fd);

    if (handle != -1) {

        do {

            size_t name_len = strlen(fd.name);

            if (fd.attrib & _A_SUBDIR)
                continue;

            if (name_len <= 4)
                continue;

            if (_stricmp(
                fd.name + name_len - 4,
                ".pak") != 0)
                continue;

            if (count >= MAX_FILES) {

                fprintf(stderr,
                    "updlist: limite de arquivos excedido\n");

                _findclose(handle);
                goto cleanup;
            }

            names[count] = _strdup(fd.name);

            if (!names[count]) {

                _findclose(handle);
                goto cleanup;
            }

            count++;

        } while (_findnext64(handle, &fd) == 0);

        _findclose(handle);
    }

    if (count == 3) {

        fprintf(stderr,
            "updlist: nenhum .pak encontrado em %s\n",
            dir);

        goto cleanup;
    }

    qsort(
        names,
        count,
        sizeof(names[0]),
        by_name
    );

    capacity = 256 + (size_t)count * 512;

    xml = (char*)malloc(capacity);

    if (!xml)
        goto cleanup;

    len = (size_t)snprintf(
        xml,
        capacity,
        "<?xml version=\"1.0\" encoding=\"euc-kr\" standalone=\"yes\" ?>\r\n"
        "<patchVer value=\"JP.R7.983.00\" />\r\n"
        "<patchNum value=\"1\" />\r\n"
        "<updatelistVer value=\"1\" />\r\n"
        "<updatefiles count=\"%d\">\r\n",
        count
    );

    if (len >= capacity)
        goto cleanup;

    for (i = 0; i < count; i++) {

        size_t name_len = strlen(names[i]);

        if (!make_path(
            file_path,
            sizeof(file_path),
            dir,
            names[i])) {

            fprintf(stderr,
                "updlist: caminho muito grande: %s\n",
                names[i]);

            goto cleanup;
        }
        log_msg(LOG_WARNING,
    " %d/%d - %s\n",
    i + 1,
    count,
    names[i]
);

        if (!file_crc(file_path, &crc, &size)) {

            fprintf(stderr,
                "updlist: arquivo nao lido: %s\n",
                file_path);

            goto cleanup;
        }

        log_msg(LOG_SUCCESS,
            " %s | Tamanho: %" PRIu64
            " bytes | CRC: 0x%08x\n",
            names[i],
            size,
            (unsigned int)crc
        );

        {
            int written = snprintf(
                line,
                sizeof(line),
                "\t<fileinfo fname=\"%s\" fdir=\"\\\" fsize=\"%" PRIu64 "\" fcrc=\"%" PRId32 "\" />\r\n",
                names[i],
                size,
                (int32_t)crc
            );

            if (written < 0 ||
                (size_t)written >= sizeof(line)) {

                fprintf(stderr,
                    "updlist: linha XML muito grande\n");

                goto cleanup;
            }

            if (!reserve_buffer(
                &xml,
                &capacity,
                len + (size_t)written + 32))
                goto cleanup;

            memcpy(
                xml + len,
                line,
                (size_t)written
            );

            len += (size_t)written;
            xml[len] = '\0';
        }

        if (name_len < 4 ||
            _stricmp(
                names[i] + name_len - 4,
                ".pak") != 0) {

			log_msg(LOG_INFO,
				"%s fsize=%" PRIu64
				" fcrc=%" PRId32 " (0x%08x)\n",
				names[i],
				size,
				(int32_t)crc,
				(unsigned int)crc
			); 
        }
    }

    {
        const char* end_xml = "</updatefiles>";

        size_t end_len = strlen(end_xml);

        if (!reserve_buffer(
            &xml,
            &capacity,
            len + end_len + 1))
            goto cleanup;

        memcpy(
            xml + len,
            end_xml,
            end_len
        );

        len += end_len;
        xml[len] = '\0';
    }
     
	log_msg(LOG_SUCCESS,
		"%d arquivos processados com sucesso\n",
		count
	);

    result = write_cipher(out, xml, len);

cleanup:

    if (xml)
        free(xml);

    for (i = 0; i < count; i++) {

        if (names[i])
            free(names[i]);

        names[i] = NULL;
    }

    return result;
}

/* ============================================================
   Atualização automática de configurações
   ============================================================ */

static void trim(char* text)
{
    char* start = text;
    char* end;

    while (*start && isspace((unsigned char)*start))
        start++;

    if (start != text)
        memmove(text, start, strlen(start) + 1);

    end = text + strlen(text);

    while (end > text && isspace((unsigned char)end[-1]))
        *--end = '\0';
}

static int ler_ini(
    const char* filename,
    char* game_dir,
    size_t game_max,
    char* update_dir,
    size_t update_max,
    char* output_name,
    size_t output_max)
{
    FILE* f;
    char linha[1024];
    int config_section = 0;

    if (!filename || !game_dir || !update_dir || !output_name)
        return 0;

    game_dir[0] = '\0';
    update_dir[0] = '\0';
    output_name[0] = '\0';

    f = fopen(filename, "r");

    if (!f) {
        fprintf(stderr,
            "[ERRO] Nao foi possivel abrir %s\n", filename);
        return 0;
    }

    while (fgets(linha, sizeof(linha), f)) {
        char* p;
        char* eq;
        char* key;
        char* value;

        trim(linha);
        p = linha;

        if (*p == '\0' || *p == ';' || *p == '#')
            continue;

        /* Identificar seção */

        if (*p == '[') {
            char* close = strchr(p, ']');

            if (!close)
                continue;

            *close = '\0';
            config_section = (_stricmp(p + 1, "Config") == 0);
            continue;
        }

        if (!config_section)
            continue;

        eq = strchr(p, '=');

        if (!eq)
            continue;

        *eq = '\0';

        key = p;
        value = eq + 1;

        trim(key);
        trim(value);

        if (_stricmp(key, "GameDirectory") == 0) {
            if (strlen(value) >= game_max)
                goto invalid;

            strcpy_s(game_dir, game_max, value);
        }
        else if (_stricmp(key, "UpdatelistDirectory") == 0) {
            if (strlen(value) >= update_max)
                goto invalid;

            strcpy_s(update_dir, update_max, value);
        }
        else if (_stricmp(key, "OutputUpdatelist") == 0) {
            if (strlen(value) >= output_max)
                goto invalid;

            strcpy_s(output_name, output_max, value);
        }
    }

    if (ferror(f)) {
        fclose(f);
        return 0;
    }

    fclose(f);

    if (!game_dir[0] || !update_dir[0] || !output_name[0]) {
        fprintf(stderr,
            "[ERRO] Configuracao incompleta na secao [Config].\n");
        return 0;
    }

    return 1;

invalid:

    fprintf(stderr,
        "[ERRO] Valor de configuracao excede o tamanho permitido.\n");

    fclose(f);
    return 0;
}
/* ============================================================
   Atualizar arquivos existentes no Updatelist
   ============================================================ */

static int update_existing(
    int argc,
    char** argv)
{
    u64 cipher_size;

    unsigned char* cipher = NULL;

    char* xml = NULL;

    size_t len;
    size_t capacity;

    int a;
    int error = 0;

    cipher = ler(argv[1], &cipher_size);

    if (!cipher || cipher_size == 0 ||
        cipher_size % 8 != 0 ||
        cipher_size > SIZE_MAX) {

        fprintf(stderr,
            "updlist: %s ausente ou tamanho invalido\n",
            argv[1]);

        free(cipher);
        return 1;
    }

    for (a = 0; a < (int)cipher_size; a += 8)
        dec_block(cipher + a);

    len = (size_t)cipher_size;

    while (len > 0 && cipher[len - 1] == 0)
        len--;

    if (len == 0) {

        fprintf(stderr,
            "updlist: XML vazio ou invalido\n");

        free(cipher);
        return 1;
    }

    if (len > SIZE_MAX - 1) {

        free(cipher);
        return 1;
    }

    capacity = len + 1;

    xml = (char*)malloc(capacity);

    if (!xml) {

        free(cipher);
        return 1;
    }

    memcpy(xml, cipher, len);

    xml[len] = '\0';

    free(cipher);
    cipher = NULL;

    for (a = 2; a < argc; a++) {

        u32 crc;
        u64 size;

        size_t ini;
        size_t fim;

        char* nome;
        char* p;

        char fname[1024];

        char crc_value[32];
        char size_value[32];

        int written;

        p = argv[a] + strlen(argv[a]);

        while (p > argv[a] &&
            p[-1] != '\\' &&
            p[-1] != '/') {

            p--;
        }

        nome = p;

        if (*nome == '\0') {

            fprintf(stderr,
                "updlist: nome de arquivo invalido\n");

            error = 1;
            continue;
        }

        if (!file_crc(argv[a], &crc, &size)) {

            fprintf(stderr,
                "updlist: %s nao lido\n",
                argv[a]);

            error = 1;
            continue;
        }

        written = snprintf(
            fname,
            sizeof(fname),
            "fname=\"%s\"",
            nome
        );

        if (written < 0 ||
            (size_t)written >= sizeof(fname)) {

            fprintf(stderr,
                "updlist: nome muito grande\n");

            error = 1;
            continue;
        }

        {
            size_t search_len = strlen(fname);
            size_t i;
            int found = 0;

            for (i = 0; i + search_len <= len; i++) {

                size_t k;

                for (k = 0; k < search_len; k++) {

                    if (tolower((unsigned char)xml[i + k]) !=
                        tolower((unsigned char)fname[k]))
                        break;
                }

                if (k == search_len) {

                    ini = i;

                    found = 1;
                    break;
                }
            }

            if (!found) {

                fprintf(stderr,
                    "updlist: linha fname=\"%s\" nao encontrada\n",
                    nome);

                error = 1;
                continue;
            }
        }

        /* Localizar o início do elemento XML. */

        while (ini > 0 && xml[ini] != '<')
            ini--;

        if (xml[ini] != '<' ||
            ini + 9 > len ||
            memcmp(xml + ini, "<fileinfo", 9) != 0) {

            fprintf(stderr,
                "updlist: elemento fileinfo invalido para %s\n",
                nome);

            error = 1;
            continue;
        }

        fim = ini;

        while (fim < len && xml[fim] != '>')
            fim++;

        if (fim >= len) {

            fprintf(stderr,
                "updlist: elemento XML incompleto para %s\n",
                nome);

            error = 1;
            continue;
        }

        snprintf(
            crc_value,
            sizeof(crc_value),
            "%" PRId32,
            (int32_t)crc
        );

        snprintf(
            size_value,
            sizeof(size_value),
            "%" PRIu64,
            size
        );

        if (!troca_attr(
            &xml,
            &len,
            &capacity,
            ini,
            &fim,
            "fcrc",
            crc_value)) {

            fprintf(stderr,
                "updlist: atributo fcrc nao encontrado em %s\n",
                nome);

            error = 1;
            continue;
        }

        if (!troca_attr(
            &xml,
            &len,
            &capacity,
            ini,
            &fim,
            "fsize",
            size_value)) {

            fprintf(stderr,
                "updlist: atributo fsize nao encontrado em %s\n",
                nome);

            error = 1;
            continue;
        }

        printf(
            "updlist: %s fsize=%" PRIu64
            " fcrc=%" PRId32 " (0x%08x)\n",
            nome,
            size,
            (int32_t)crc,
            (unsigned int)crc
        );
    }

    if (error) {

        fprintf(stderr,
            "updlist: operacao cancelada. Arquivo original preservado.\n");

        free(xml);
        return 1;
    }

    {
        int result = write_cipher(
            argv[1],
            xml,
            len
        );

        free(xml);

        return result;
    }
}

/* ============================================================
   MAIN
   ============================================================ */

int main(int argc, char** argv)
{
    log_init();

    char game_dir[1024] = { 0 };
    char out_file[1024] = { 0 };

    crc_init();

    /* Modo automático */
    if (argc < 2) {

        int result;

        char game_dir[1024] = { 0 };
        char update_dir[1024] = { 0 };
        char output_name[512] = { 0 };
        char output_path[2048] = { 0 };

        g_log = fopen("updlist.log", "a");

        log_msg(LOG_INFO, "\n====================================\n");
        log_msg(LOG_INFO, "PangYa Updatelist Generator\n");
        log_msg(LOG_INFO, "====================================\n");

        if (!ler_ini(
            "config.ini",
            game_dir,
            sizeof(game_dir),
            update_dir,
            sizeof(update_dir),
            output_name,
            sizeof(output_name))) {

            log_msg(LOG_ERROR, "Falha ao carregar config.ini\n");
            result = 1;
        }
        else if (!make_path(
            output_path,
            sizeof(output_path),
            update_dir,
            output_name)) {

            log_msg(LOG_ERROR, "Caminho de saida muito grande.\n");
            result = 1;
        }
        else {

            log_msg(LOG_INFO, "Configuracoes carregadas.\n");

            log_msg(LOG_INFO, "GameDirectory: %s\n", game_dir);
             log_msg(LOG_INFO, "UpdatelistDirectory: %s\n", update_dir);
             log_msg(LOG_INFO, "OutputUpdatelist: %s\n", output_name);

             log_msg(LOG_INFO, "Arquivo final: %s\n", output_path);

             log_msg(LOG_INFO, "\nIniciando processamento...\n");

            result = generate(game_dir, output_path);
        }

        if (result == 0)
            log_msg(LOG_SUCCESS, "\nUpdatelist gerado corretamente!\n");
        else
            log_msg(LOG_ERROR, "\nOperacao encerrada com erro: %d\n", result);

        log_msg(LOG_INFO, "====================================\n");

        if (g_log) {
            fclose(g_log);
            g_log = NULL;
        }

        printf("\nPressione ENTER para fechar...");
        getchar();

        return result;
    }
    /* Modo de geração */

    if (strcmp(argv[1], "--generate") == 0) {

        if (argc != 4) {

            fprintf(stderr,
                "Uso: updlist --generate <pasta do jogo> <updatelist>\n"
            );

            return 2;
        }

        return generate(
            argv[2],
            argv[3]
        );
    }

    /* Modo de atualização */

    if (argc < 3) {

        fprintf(stderr,
            "Uso: updlist <updatelist> <arquivo>...\n"
        );

        return 2;
    }

    return update_existing(argc, argv);
}