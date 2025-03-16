/* updateLcd.h
 * 
 * This module handles the LCD screen output, it start a thread and repeately display periodic statistics.
 * It has three screens:
 * 
 * Screen 1: Displays the current beat name, volume (bottom-left), and BPM (bottom-right).
 * Screen 2: Displays audio timing statistics, including min, max, and avg ms for buffer refills.
 * Screen 3: Displays accelerometer timing stats, including min, max, and avg ms between samples.
 * 
 * The LCD screens can be cycled through by pressing the center button in joystick.
 */
#ifndef _UPDATELCD_H_
#define _UPDATELCD_H_

//Initialize and clean up the LCD screen.
void UpdateLcd_init();
void UpdateLcd_cleanup();
void UpdateLcd_withPage(int page);

#endif