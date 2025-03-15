#ifndef BEAT_HELPER_H
#define BEAT_HELPER_H

#define BASE_DRUM_FILE "wave-files/100051__menegass__gui-drum-bd-hard.wav"
#define HI_HAT_FILE "wave-files/100053__menegass__gui-drum-cc.wav"
#define SNARE_FILE "wave-files/100059__menegass__gui-drum-snare-soft.wav"

void BeatPlayer_init();
void BeatPlayer_playHiHat();
void BeatPlayer_playBaseDrum();
void BeatPlayer_playSnare();
int BeatPlayer_getBpm();
void BeatPlayer_setBpm(int newBpm);
int BeatPlayer_getVolume();
void BeatPlayer_setVolume(int newVolume);
void BeatPlayer_cleanup();
#endif