#include <SDL2/SDL.h>
#include <string>
#include <memory>
#include <vector>


class AudioData {
    double balance; // [0.0 through 1.0]
    size_t pos;
    size_t len;
public:
    AudioData(): balance(0.5) {}
    AudioData(double balancE): balance(balancE) {}
    void load(const std::string& fileName);
    bool donePlaying(){ return 0==len; }
};


class Audio{
    SDL_AudioDeviceID dev;
    SDL_AudioSpec want, have;
    std::vector<std::shared_ptr<AudioData> > audioData;
    void init();
public:
    Audio(): dev(0) { init(); }
    ~Audio(){ SDL_CloseAudioDevice(dev); }
    void mix(Uint8 *stream, int len);
    void play(const std::shared_ptr<AudioData>& data);
};


void audioCallback(void *userdata, Uint8 *stream, int len){
    static_cast<Audio*>(userdata)->mix(stream, len);
}


void Audio::init(){
    want.freq = 22000;
    want.format = AUDIO_U8; // AUDIO_S8, AUDIO_S16LSB, AUDIO_U16LSB
    want.channels = 2;
    want.samples = 4096;
    want.callback = [](void* userdata, Uint8 *stream, int len) { static_cast<Audio*>(userdata)->mix(stream, len); };
//    want.callback = audioCallback; // TODO: use lambda instead???
    want.userdata = this;
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if ( dev < 0 ){
        fprintf(stderr, "Can't open audio: %s\n", SDL_GetError());
    } else {
        SDL_PauseAudioDevice(dev,0);
    }
}


void Audio::mix(Uint8 *stream, int len){
    for(std::shared_ptr<AudioData>& data: audioData){
//        SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    }
    // TODO: delete what's donePlaying() here
}


//class AudioLock(){
//    SDL_AudioDeviceID dev;
//public:
//    AudioLock(SDL_AudioDeviceID device): dev(device) { SDL_LockAudioDevice(dev); }
//    ~AudioLock(){ SDL_UnlockAudioDevice(dev); }
//};


void Audio::play(const std::shared_ptr<AudioData>& data){
    SDL_LockAudioDevice(dev);
    audioData.push_back(data); // this data will be mixed in during the next callback
    SDL_UnlockAudioDevice(dev);
}
