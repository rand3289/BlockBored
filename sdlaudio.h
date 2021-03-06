#ifndef SDLAUDIO_H_INCLUDED
#define SDLAUDIO_H_INCLUDED

#include <SDL2/SDL.h>
#include <string>
#include <memory>
#include <vector>


class Audio;

class AudioData {
    friend Audio;
    SDL_AudioSpec wav_spec;
    Uint32 pos = 0;
    Uint32 wav_length = 0;
    Uint8 *wav_buffer = nullptr; // shared_ptr<Uint8> buffer; would be nice
public:
    AudioData() {}
    AudioData(const std::string& fileName){ load(fileName); }
    ~AudioData(){ SDL_FreeWAV(wav_buffer); }
    bool donePlaying() const { return pos == wav_length; }
    void load(const std::string& fileName);
};


class Audio{
    SDL_AudioDeviceID dev;
    SDL_AudioSpec want, have;
    std::vector<std::shared_ptr<AudioData> > audioData;
    void init();
public:
    Audio(){ init(); }
    ~Audio(){ SDL_CloseAudioDevice(dev); }
    void mix(Uint8 *stream, int len); // callback
    void play(std::shared_ptr<AudioData>& data);
};


#endif // SDLAUDIO_H_INCLUDED
