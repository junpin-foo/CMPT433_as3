/* beat_box.c
*/

#include "hal/audioMixer.h"
#include "hal/joystick.h"
#include "hal/lcd.h"
#include "beatPlayer.h"
#include "hal/accelerometer.h"
#include "terminalOutput.h"
#include "udp_listener.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
int main(void)
{
    Period_init();
    BeatPlayer_init();
    Joystick_initialize();
    Accelerometer_initialize();
    TerminalOutput_init();
    // Lcd_init();
    UdpListener_init();
    while(UdpListener_isRunning()) {
        Joystick_getReading();   
        // printf("still inside the loop");
        struct timespec reqDelay = {0, 1000000000};
        nanosleep(&reqDelay, (struct timespec *) NULL);
    }
    UdpListener_cleanup();
    // Lcd_cleanup();
    TerminalOutput_cleanup();
    Accelerometer_cleanUp();
    Joystick_cleanUp();
    BeatPlayer_cleanup();
    Period_cleanup();
	return 0;
}