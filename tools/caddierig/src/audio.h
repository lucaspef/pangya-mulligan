// SPDX-License-Identifier: MIT
// audio.h - plays the game's .wav effects through waveOut, one device handle per
// voice so Windows does the mixing and ACM decodes any compressed files.
#ifndef AUDIO_H
#define AUDIO_H

#include <string>

// False when the file cannot be read or the system has no decoder for its format.
bool AudioPlay(const std::string& utf8Path, float volume);
// The same checks without opening the device for playback.
bool AudioCanPlay(const std::string& utf8Path);
int AudioVoices();
void AudioUpdate();			// call now and then; frees voices that have finished
void AudioStopAll();
void AudioSetMuted(bool muted);

#endif // AUDIO_H
