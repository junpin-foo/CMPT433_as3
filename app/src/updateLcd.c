/* updateLcd.c
* 
* This file contains the implementation of the LCD update module. 
* The module is responsible for updating the LCD display with the current beat mode, volume, and BPM.
* The module also displays the audio and accelerometer timing statistics.
*
*/

#include "DEV_Config.h"
#include "LCD_1in54.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include <stdio.h>		//printf()
#include <stdlib.h>		//exit()
#include <signal.h>     //signal()
#include <stdbool.h>
#include <assert.h>
#include "pthread.h"
#include "updateLcd.h"
#include "beatPlayer.h"
#include "periodTimer.h"
#include "terminalOutput.h"
#include "hal/joystick.h"
#include "sleep_timer_helper.h"

#define DELAY_MS 2000
#define SLEEP_MS 10
#define BACKLIGHT 1023
#define INITIAL_X 5
#define INITIAL_Y 40
#define NEXTLINE_Y 40
#define FREQUENCY_X 140
#define DIPS_X 120
#define MAX_MS_X 160
#define VALUE_OFFSET 40
#define statBufferSize 12

static UWORD *s_fb;
static bool isInitialized = false;
static char volume[statBufferSize];
static char beatMode[statBufferSize];
static char bpm[statBufferSize];
static char minAudioMs[statBufferSize];
static char maxAudioMs[statBufferSize];
static char avgAudioMs[statBufferSize]; 

static char minAccelMs[statBufferSize];
static char maxAccelMs[statBufferSize];
static char avgAccelMs[statBufferSize];
static pthread_t outputThread;
static bool isRunning = false;
static void* UpdateLcdThread(void* args);
void UpdateLcd_init()
{
    assert(!isInitialized);

    // Exception handling:ctrl + c
    // signal(SIGINT, Handler_1IN54_LCD);
    
    // Module Init
	if(DEV_ModuleInit() != 0){
        DEV_ModuleExit();
        exit(0);
    }
	
    // LCD Init
    DEV_Delay_ms(DELAY_MS);
	LCD_1IN54_Init(HORIZONTAL);
	LCD_1IN54_Clear(WHITE);
	LCD_SetBacklight(BACKLIGHT);

    UDOUBLE Imagesize = LCD_1IN54_HEIGHT*LCD_1IN54_WIDTH*2;
    if((s_fb = (UWORD *)malloc(Imagesize)) == NULL) {
        perror("Failed to apply for black memory");
        exit(0);
    }
    isRunning = true;
    isInitialized = true;
    pthread_create(&outputThread, NULL, &UpdateLcdThread, NULL);
}
void UpdateLcd_cleanup()
{
    assert(isInitialized);
    isRunning = false;
    pthread_join(outputThread, NULL);
    LCD_1IN54_Clear(WHITE);
    LCD_SetBacklight(0);
    // Module Exit
    free(s_fb);
    s_fb = NULL;
	DEV_ModuleExit();
    isInitialized = false;
}

static void* UpdateLcdThread(void* args) {
    (void) args;
    assert(isInitialized);
    while (isRunning) {
        UpdateLcd_withPage(Joystick_getPageCount());
        sleepForMs(SLEEP_MS);
    }
    return NULL;
}


void UpdateLcd_withPage(int page)
{
    assert(isInitialized);

    const int x = INITIAL_X;
    int y = INITIAL_Y;

    // Initialize the RAM frame buffer to be blank (white)
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);
    Period_statistics_t audioStat = TerminalOutput_getAudioStats();
    Period_statistics_t accelStat = TerminalOutput_getAccelStats();
    int beatModeNum = BeatPlayer_getBeatMode();
    switch (page)
    {
        case 1: // Status Screen
            if (beatModeNum == NONE_MODE) {
                sprintf(beatMode, "%s", "None");
            } else if (beatModeNum == ROCK_MODE) {
                sprintf(beatMode, "%s", "Rock");
            } else {
                sprintf(beatMode, "%s", "Custom");
            }
            sprintf(volume, "%d", BeatPlayer_getVolume());
            sprintf(bpm, "%d", BeatPlayer_getBpm());
            Paint_DrawString_EN(x, y, "Current Beat:", &Font20, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, beatMode, &Font24, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, LCD_1IN54_HEIGHT - VALUE_OFFSET, "Vol:", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, LCD_1IN54_HEIGHT - VALUE_OFFSET, volume, &Font16, WHITE, BLACK);
            Paint_DrawString_EN(LCD_1IN54_WIDTH - (VALUE_OFFSET * 2), LCD_1IN54_HEIGHT - VALUE_OFFSET, "BPM:", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(LCD_1IN54_WIDTH - VALUE_OFFSET, LCD_1IN54_HEIGHT - VALUE_OFFSET, bpm, &Font16, WHITE, BLACK);
            break;

        case 2: // Audio Timing Summary
            sprintf(minAudioMs, "%.3f ms", audioStat.minPeriodInMs);
            sprintf(maxAudioMs, "%.3f ms", audioStat.maxPeriodInMs);
            sprintf(avgAudioMs, "%.3f ms", audioStat.avgPeriodInMs);
            Paint_DrawString_EN(x, y, "Audio Timing", &Font20, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Min: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, minAudioMs, &Font16, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Max: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, maxAudioMs, &Font16, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Avg: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, avgAudioMs, &Font16, WHITE, BLACK);
            break;

        case 3: // Accelerometer Timing Summary
            sprintf(minAccelMs, "%.3f ms", accelStat.minPeriodInMs);
            sprintf(maxAccelMs, "%.3f ms", accelStat.maxPeriodInMs);
            sprintf(avgAccelMs, "%.3f ms", accelStat.avgPeriodInMs);
            Paint_DrawString_EN(x, y, "Accel. Timing", &Font20, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Min: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, minAccelMs, &Font16, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Max: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, maxAccelMs, &Font16, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Avg: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, avgAccelMs, &Font16, WHITE, BLACK);
            break;

        default:
            Paint_DrawString_EN(x, y, "Invalid Page", &Font20, WHITE, BLACK);
            break;
    }

    // Send the RAM frame buffer to the LCD (actually display it)
    LCD_1IN54_Display(s_fb);
}
