/* updateLcd.h
* Provided file to update LCD screen
*/
#ifndef _UPDATELCD_H_
#define _UPDATELCD_H_

//Initialize and clean up the LCD screen.
void UpdateLcd_init();
void UpdateLcd_cleanup();

void UpdateLcd_withPage(int page);

#endif