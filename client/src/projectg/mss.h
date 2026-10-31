#pragma once

// Subset of the Miles Sound System 7 API (mss.h) used by miles.cpp.

typedef int S32;
typedef unsigned int U32;
typedef float F32;

#define AILCALL __stdcall
#define AILCALLBACK __stdcall
#define DXDEC extern "C" __declspec(dllimport)

struct _DIG_DRIVER;
struct _SAMPLE;
struct _STREAM;
typedef _DIG_DRIVER* HDIGDRIVER;
typedef _SAMPLE* HSAMPLE;
typedef _STREAM* HSTREAM;

typedef S32 MSS_MC_SPEC;
struct MSS_VECTOR3D
{
	F32 x;
	F32 y;
	F32 z;
};
typedef MSS_VECTOR3D MSS_SPEAKER;

struct AILSOUNDINFO
{
	S32 format;
	void const* data_ptr;
	U32 data_len;
	U32 rate;
	S32 bits;
	S32 channels;
	U32 channel_mask;
	U32 samples;
	U32 block_size;
	void const* initial_ptr;
};

typedef U32(AILCALLBACK* AIL_file_open_callback)(char const* filename,
	U32* file_handle);
typedef void(AILCALLBACK* AIL_file_close_callback)(U32 file_handle);
typedef S32(
	AILCALLBACK* AIL_file_seek_callback)(U32 file_handle, S32 offset, U32 type);
typedef U32(AILCALLBACK* AIL_file_read_callback)(U32 file_handle, void* buffer,
	U32 bytes);
typedef S32(AILCALLBACK* AILLENGTHYCB)(U32 state, U32 user);

#define SMP_DONE 0x0002
#define SMP_PLAYING 0x0004
#define SMP_STOPPED 0x0008

#define AIL_FILE_SEEK_BEGIN 0
#define AIL_FILE_SEEK_CURRENT 1
#define AIL_FILE_SEEK_END 2

#define AILFILETYPE_ADPCM_WAV 2
#define AILFILETYPE_MPEG_L3_AUDIO 13

#define MSS_MC_STEREO 2
#define MSS_MC_USE_SYSTEM_CONFIG 0x10
#define MSS_MC_HEADPHONES 0x20
#define MSS_MC_DOLBY_SURROUND 0x30
#define MSS_MC_40_DISCRETE 0x50
#define MSS_MC_51_DISCRETE 0x60
#define MSS_MC_71_DISCRETE 0x80

#define AIL_OPEN_DIGITAL_FORCE_PREFERENCE 1

DXDEC S32 AILCALL AIL_startup(void);
DXDEC void AILCALL AIL_shutdown(void);
DXDEC char* AILCALL AIL_last_error(void);
DXDEC char* AILCALL AIL_set_redist_directory(char const* dir);
DXDEC void AILCALL AIL_mem_free_lock(void* ptr);
DXDEC void AILCALL AIL_set_file_callbacks(AIL_file_open_callback opencb,
	AIL_file_close_callback closecb, AIL_file_seek_callback seekcb,
	AIL_file_read_callback readcb);
// 983 (Miles novo): AIL_file_size/AIL_file_read sao macros para as _info
// (o Mss32.dll do cliente JP exporta as duas formas; o 983 importa as _info).
DXDEC S32 AILCALL AIL_file_size_info(char const* filename,
	char const* caller, U32 line);
DXDEC void* AILCALL AIL_file_read_info(char const* filename, void* dest,
	char const* caller, U32 line);
#define AIL_file_size(f) AIL_file_size_info(f, __FILE__, __LINE__)
#define AIL_file_read(f, d) AIL_file_read_info(f, d, __FILE__, __LINE__)
DXDEC S32 AILCALL AIL_file_type(void const* data, U32 size);

DXDEC HDIGDRIVER AILCALL AIL_open_digital_driver(U32 frequency, S32 bits,
	S32 channel, U32 flags);
DXDEC void AILCALL AIL_close_digital_driver(HDIGDRIVER dig);
DXDEC S32 AILCALL AIL_digital_handle_release(HDIGDRIVER drvr);
DXDEC S32 AILCALL AIL_digital_CPU_percent(HDIGDRIVER dig);
DXDEC MSS_SPEAKER* AILCALL AIL_speaker_configuration(HDIGDRIVER dig,
	S32* nchannels, S32* nspeakers, F32* falloff_power,
	MSS_MC_SPEC* channel_spec);
DXDEC void AILCALL AIL_set_speaker_configuration(HDIGDRIVER dig,
	MSS_SPEAKER* array, S32 n_channels, F32 falloff_power);

DXDEC HSAMPLE AILCALL AIL_allocate_sample_handle(HDIGDRIVER dig);
DXDEC void AILCALL AIL_release_sample_handle(HSAMPLE S);
DXDEC S32 AILCALL AIL_set_sample_file(HSAMPLE S, void const* file_image,
	S32 block);
DXDEC S32 AILCALL AIL_set_named_sample_file(HSAMPLE S,
	char const* file_type_suffix, void const* file_image, U32 file_size,
	S32 block);
DXDEC void AILCALL AIL_start_sample(HSAMPLE S);
DXDEC void AILCALL AIL_stop_sample(HSAMPLE S);
DXDEC U32 AILCALL AIL_sample_status(HSAMPLE S);
DXDEC U32 AILCALL AIL_sample_position(HSAMPLE S);
DXDEC S32 AILCALL AIL_sample_playback_rate(HSAMPLE S);
DXDEC void AILCALL AIL_set_sample_playback_rate(HSAMPLE S, S32 playback_rate);
DXDEC void AILCALL AIL_set_sample_volume_pan(HSAMPLE S, F32 volume, F32 pan);
DXDEC void AILCALL AIL_set_sample_loop_count(HSAMPLE S, S32 loop_count);
DXDEC void AILCALL AIL_set_sample_3D_distances(HSAMPLE S, F32 max_dist,
	F32 min_dist, S32 auto_3D_wet_atten);
DXDEC void AILCALL AIL_set_sample_3D_position(HSAMPLE obj, F32 X, F32 Y, F32 Z);

DXDEC HSTREAM AILCALL AIL_open_stream(HDIGDRIVER dig, char const* filename,
	S32 stream_mem);
DXDEC void AILCALL AIL_close_stream(HSTREAM stream);
DXDEC HSAMPLE AILCALL AIL_stream_sample_handle(HSTREAM stream);
DXDEC void AILCALL AIL_start_stream(HSTREAM stream);
DXDEC void AILCALL AIL_pause_stream(HSTREAM stream, S32 onoff);
DXDEC S32 AILCALL AIL_stream_status(HSTREAM stream);
DXDEC void AILCALL AIL_set_stream_loop_count(HSTREAM stream, S32 count);
DXDEC void AILCALL AIL_set_stream_position(HSTREAM stream, S32 offset);
DXDEC S32 AILCALL AIL_stream_position(HSTREAM stream);
DXDEC void AILCALL AIL_stream_info(HSTREAM stream, S32* datarate, S32* sndtype,
	S32* length, S32* memory);
DXDEC void AILCALL AIL_stream_ms_position(HSTREAM S, S32* total_milliseconds,
	S32* current_milliseconds);
DXDEC void AILCALL AIL_set_stream_user_data(HSTREAM S, U32 index, S32 value);

DXDEC S32 AILCALL AIL_WAV_info(void const* data, AILSOUNDINFO* info);
DXDEC S32 AILCALL AIL_decompress_ADPCM(AILSOUNDINFO const* info, void** outdata,
	U32* outsize);
DXDEC S32 AILCALL AIL_decompress_ASI(void const* indata, U32 insize,
	char const* filename_ext, void** outdata, U32* outsize,
	AILLENGTHYCB callback);
