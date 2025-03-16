/* terminalOutput.h
 * This module is responsible for outputting periodic statistics to the terminal.
 * 
 * It record and printout the following information every second via a thread, including:
 * - Beat mode (as "M0", "M1", etc.)
 * - Current tempo (in format "90bpm")
 * - Current volume (in format "vol:80")
 * - Time between refilling the audio playback buffer, with statistics like minimum, maximum, average times, and the number of samples.
 * - Time between samples of the accelerometer, with similar statistics.
 * 
 * The periodic output format is as follows:
 * M0 90bpm vol:80 Audio[16.283, 16.942] avg 16.667/61 Accel[12.276, 13.965] avg 12.998/77
 */

#ifndef _TERMINAL_OUTPUT_H_
#define _TERMINAL_OUTPUT_H_
#include "periodTimer.h"

// Initialize/Clean up the moduule
void TerminalOutput_init(void);
void TerminalOutput_cleanup(void);

// The helper function for other module to access the colleced stat about audioMixer and Accelerometer
Period_statistics_t TerminalOutput_getAccelStats();
Period_statistics_t TerminalOutput_getAudioStats();
#endif
