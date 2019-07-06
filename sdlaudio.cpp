#include "sdlaudio.h"
#include <algorithm> // remove_if()
using namespace std;


void AudioData::load(const std::string& fileName){
    if (SDL_LoadWAV(fileName.c_str(), &wav_spec, &wav_buffer, &wav_length) == NULL) {
        fprintf(stderr, "Could not open test.wav: %s\n", SDL_GetError());
        wav_length = 0;
        pos = 0;
    }
}


void Audio::init(){
    want.freq = 22000;
    want.format = AUDIO_U8; // AUDIO_S8, AUDIO_S16LSB, AUDIO_U16LSB
    want.channels = 1;
    want.samples = 4096;
    want.userdata = this;
    want.callback = [](void* userdata, Uint8 *stream, int len) { static_cast<Audio*>(userdata)->mix(stream, len); };

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if ( dev < 0 ){
        fprintf(stderr, "Can't open audio: %s\n", SDL_GetError());
    } else {
        SDL_PauseAudioDevice(dev,0);
    }
}


void Audio::mix(Uint8 *stream, int len){
    for(std::shared_ptr<AudioData>& data: audioData){
        Uint32 len2 = std::min((Uint32)len, data->wav_length - data->pos);
        SDL_MixAudioFormat(stream, data->wav_buffer+data->pos, want.format, len2, SDL_MIX_MAXVOLUME);
        data->pos+=len2;
    }
    auto end = std::remove_if(audioData.begin(), audioData.end(), [](shared_ptr<AudioData>& ad){ return ad->donePlaying(); } );
    audioData.erase(end, audioData.end()); // delete what's donePlaying()
}


void Audio::play(const std::shared_ptr<AudioData>& data){
    SDL_LockAudioDevice(dev);
    audioData.push_back(data); // this data will be mixed in during the next callback
    SDL_UnlockAudioDevice(dev);
}


/*
class AudioLock {
    SDL_AudioDeviceID dev;
public:
    AudioLock(SDL_AudioDeviceID device): dev(device) { SDL_LockAudioDevice(dev); }
    ~AudioLock(){ SDL_UnlockAudioDevice(dev); }
};
*/
