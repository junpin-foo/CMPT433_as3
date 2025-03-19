/* beat_box.c
* 
* This file contains the main function for the beat box application. 
* It initializes all the necessary components (audio mixer, joystick, accelerometer) which allows the user to create and play beats.
*
*/

#include "hal/audioMixer.h"
#include "hal/joystick.h"
#include "hal/lcd.h"
#include "beatPlayer.h"
#include "hal/accelerometer.h"
#include "terminalOutput.h"
#include "udp_listener.h"
#include "sleep_timer_helper.h"
#include <stdio.h>
#include <unistd.h>


int main(void)
{
    Period_init();
    BeatPlayer_init();
    TerminalOutput_init();
    Lcd_init();
    UdpListener_init();

    // UDP clean up will block until UDP server stop
    UdpListener_cleanup();
    Lcd_cleanup();
    TerminalOutput_cleanup();
    BeatPlayer_cleanup();
    Period_cleanup();
	return 0;
}