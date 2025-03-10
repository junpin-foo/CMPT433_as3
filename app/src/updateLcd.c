/* updateLcd.c
* Provided file to update LCD screen, updated DrawStuff_updateScreen to diplay name, frequency, dips, and max ms.
*/
#include "updateLcd.h"

#include "DEV_Config.h"
#include "LCD_1in54.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include <stdio.h>		//printf()
#include <stdlib.h>		//exit()
#include <signal.h>     //signal()
#include <stdbool.h>
#include <assert.h>

#define DELAY_MS 2000
#define BACKLIGHT 1023
#define INITIAL_X 5
#define INITIAL_Y 40
#define NEXTLINE_Y 40
#define FREQUENCY_X 140
#define DIPS_X 120
#define MAX_MS_X 160
#define VALUE_OFFSET 40

static UWORD *s_fb;
static bool isInitialized = false;

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
    isInitialized = true;
}
void UpdateLcd_cleanup()
{
    assert(isInitialized);
    LCD_1IN54_Clear(WHITE);
    LCD_SetBacklight(0);
    // Module Exit
    free(s_fb);
    s_fb = NULL;
	DEV_ModuleExit();
    isInitialized = false;
}

void UpdateLcd_withPage(int page)
{
    assert(isInitialized);

    const int x = INITIAL_X;
    int y = INITIAL_Y;

    // Initialize the RAM frame buffer to be blank (white)
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);
    

   switch (page)
    {
        case 1: // Status Screen
            Paint_DrawString_EN(x, y, "Current Beat:", &Font20, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "currentBeat", &Font24, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, LCD_1IN54_HEIGHT - 40, "Vol:", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + 40, LCD_1IN54_HEIGHT - 40, "volume", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(LCD_1IN54_WIDTH - 80, LCD_1IN54_HEIGHT - 40, "BPM:", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(LCD_1IN54_WIDTH - 40, LCD_1IN54_HEIGHT - 40, "bpm", &Font16, WHITE, BLACK);
            break;

        case 2: // Audio Timing Summary
            Paint_DrawString_EN(x, y, "Audio Timing", &Font20, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Min: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, "minAudioMs", &Font16, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Max: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, "maxAudioMs", &Font16, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Avg: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, "avgAudioMs", &Font16, WHITE, BLACK);
            break;

        case 3: // Accelerometer Timing Summary
            Paint_DrawString_EN(x, y, "Accel. Timing", &Font20, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Min: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, "minAccelMs", &Font16, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Max: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, "maxAccelMs", &Font16, WHITE, BLACK);
            y += NEXTLINE_Y;
            Paint_DrawString_EN(x, y, "Avg: ", &Font16, WHITE, BLACK);
            Paint_DrawString_EN(x + VALUE_OFFSET, y, "avgAccelMs", &Font16, WHITE, BLACK);
            break;

        default:
            Paint_DrawString_EN(x, y, "Invalid Page", &Font20, WHITE, BLACK);
            break;
    }


    // Send the RAM frame buffer to the LCD (actually display it)
    LCD_1IN54_Display(s_fb);
}
