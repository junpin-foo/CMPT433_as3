/* beat_box.c
*/

#include "hal/audioMixer.h"
#include "hal/joystick.h"
#include "hal/lcd.h"
#include "beatPlayer.h"
#include <stdio.h>
#include <unistd.h>
int main(void)
{
    BeatPlayer_init();
    Joystick_initialize();
    Lcd_init();
    // BeatPlayer_setVolume(100);
    // BeatPlayer_setBpm(300);
    while(1) {
        // BeatPlayer_playSnare();
        Joystick_getReading();   
        sleep(1);
    }
	return 0;
}