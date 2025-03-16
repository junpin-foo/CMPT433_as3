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
static void sleepForMs(long long delayInMs) { 
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000; 
    long long delayNs = delayInMs * NS_PER_MS;  
    int seconds = delayNs / NS_PER_SECOND;  
    int nanoseconds = delayNs % NS_PER_SECOND;  
    struct timespec reqDelay = {seconds, nanoseconds}; 
    nanosleep(&reqDelay, (struct timespec *) NULL); 
}
int main(void)
{
    Period_init();
    BeatPlayer_init();
    Joystick_initialize();
    Accelerometer_initialize();
    TerminalOutput_init();
    Lcd_init();
    UdpListener_init();
    while(UdpListener_isRunning()) {
        Joystick_getReading();   
        sleepForMs(1000);
    }
    UdpListener_cleanup();
    Lcd_cleanup();
    TerminalOutput_cleanup();
    Accelerometer_cleanUp();
    Joystick_cleanUp();
    BeatPlayer_cleanup();
    Period_cleanup();
	return 0;
}