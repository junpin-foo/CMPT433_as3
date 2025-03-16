#ifndef _TERMINAL_OUTPUT_H_
#define _TERMINAL_OUTPUT_H_
#include "periodTimer.h"

void TerminalOutput_init(void);
void TerminalOutput_cleanup(void);
Period_statistics_t TerminalOutput_getAccelStats();

Period_statistics_t TerminalOutput_getAudioStats();
#endif
