/* reaction_timer.c
    This program implements a reaction timer game using a joystick and LEDs.
    The player must quickly press the joystick in the correct direction when prompted.
    The program records the player's best reaction time and provides feedback via LED.
 */

#include "reaction_timer.h"

static long long bestRecordTime = 0;

//Prototypes
static void initializeGame(void);
static void exitGameCleanUp(void);
static JoystickDirection getJoystickDirection(void);
static long long getTimeInMs(void);
static void sleepForMs(long long delayInMs);
static void initialFlashLed(void);
static void waitForJoystickRelease(void);
static void incorrectResponse(void);
static void correctResponse(long long currentTime);
static void startGameWithDirection(bool topSide);

/*
This function is used to get the  direction of the joystick by reading the x and y values of the joystick.
    @Return: the direction of the joystick.
*/
static JoystickDirection getJoystickDirection(void) {
    struct JoystickData data = Joystick_getReading();
    if (data.x > 0.7) return JOYSTICK_RIGHT;
    if (data.x < -0.7) return JOYSTICK_LEFT;
    if (data.y > 0.7) return JOYSTICK_UP;
    if (data.y < -0.7) return JOYSTICK_DOWN;
    return JOYSTICK_CENTER;
}

static long long getTimeInMs(void){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000 + nanoSeconds / 1000000;
    return milliSeconds;
}

static void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

/*
This function is used to flash the LEDs to indicate the start of the game.
*/
static void initialFlashLed(void) {
    printf("Get ready...\n");
    Led_setBrightness(GREEN_LED, 0);
    Led_setBrightness(RED_LED, 0);

    for (int i = 0; i < 4; i++) {
        Led_setBrightness(GREEN_LED, 1);
        sleepForMs(FLASH_DURATION_MS);
        Led_setBrightness(GREEN_LED, 0);
        
        Led_setBrightness(RED_LED, 1);
        sleepForMs(FLASH_DURATION_MS);
        Led_setBrightness(RED_LED, 0);
    }
}

/*
This function is used to wait for the joystick to be released before starting the game.
*/
static void waitForJoystickRelease(void) {
    bool messagePrinted = false;
    while (1) {
        JoystickDirection data = getJoystickDirection();
        if (data == JOYSTICK_DOWN || data == JOYSTICK_UP) {
            if (!messagePrinted) {
                printf("Please release the joystick...\n");
                messagePrinted = true;
            }
        } else {
            sleepForMs(100);
            return;
        }
        sleepForMs(300);
    }
}

/*
This function is used to indicate that the player has responded incorrectly.
*/
static void incorrectResponse(void) {
    printf("Incorrect.\n");
    Led_setBrightness(GREEN_LED, 0);
    //Flash red LED 5 times within 1 second
    Led_setTrigger(RED_LED, "timer");
    sleepForMs(100);
    Led_setDelayOn(RED_LED, 50);
    Led_setDelayOff(RED_LED, 150);

    sleepForMs(1000);
    Led_setBrightness(RED_LED, 0);
}

/*
This function is used to indicate that the player has responded correctly.
    @Param currentTime: the current response time taken.
*/
static void correctResponse(long long currentTime) {
    printf("Correct!\n");
    if(bestRecordTime == 0 || bestRecordTime > currentTime){
        printf("New best time!\n");
        bestRecordTime = currentTime;
    }
    printf("Your reaction time was %llums; best so far in game is %llums.!\n", currentTime, bestRecordTime);
    Led_setBrightness(RED_LED, 0);
    //Flash green LED 5 times within 1 second
    Led_setTrigger(GREEN_LED, "timer");
    sleepForMs(50);
    Led_setDelayOn(GREEN_LED, 50);
    Led_setDelayOff(GREEN_LED, 150);

    sleepForMs(1000);
    Led_setBrightness(GREEN_LED, 0);
}

/*
This function is used to start the game with the specified direction.
    @Param topSide: true if the game is to be started with the positive y-axis side, false if the game is to be started with the negative y-axis side.
*/
static void startGameWithDirection(bool topSide) {
    if(topSide) {
        printf("Press UP now!\n");
        long long start = getTimeInMs();
        Led_setBrightness(GREEN_LED, 1);
        while(1){
            JoystickDirection data = getJoystickDirection();
            if(data == JOYSTICK_UP){ //CORRECT
                long long end = getTimeInMs();
                correctResponse(end - start);
                break;
            } 

            if((getTimeInMs() - start) >= MAX_RESPONSE_TIME_MS){ //NO RESPONSE
                printf("No input within 5000ms; quitting!\n");
                exitGameCleanUp();
            }

            if(data == JOYSTICK_DOWN) {//WRONG
                incorrectResponse();
                break;
            }

            if(data == JOYSTICK_LEFT || data == JOYSTICK_RIGHT){ //Quit
                printf("User selected to quit.\n");
                exitGameCleanUp();
            }            
        }
    }
    else {
        printf("Press DOWN now!\n");
        long long start = getTimeInMs();
        Led_setBrightness(RED_LED, 1);
        while(1){
            JoystickDirection data = getJoystickDirection();
            if(data == JOYSTICK_DOWN){ //CORRECT
                long long end = getTimeInMs();
                correctResponse(end - start);
                break;
            } 

            if((getTimeInMs() - start) >= MAX_RESPONSE_TIME_MS){ //NO RESPONSE
                printf("No input within 5000ms; quitting!\n");
                exitGameCleanUp();
            }

            if(data == JOYSTICK_UP) {//WRONG
                incorrectResponse();
                break;
            }

            if(data == JOYSTICK_LEFT || data == JOYSTICK_RIGHT){ //Quit
                printf("User selected to quit.\n");
                exitGameCleanUp();
            }            
        }

    }
}

/*
This function is used to initialize the game by randomizing the seed for the random number generator and initializing the joystick and LEDs.
*/
static void initializeGame(void){
    srand(time(NULL));
    Joystick_initialize();
    Led_initialize();
}

/*
This function is used to clean up the game and exit.
*/
static void exitGameCleanUp(void){
    Led_setBrightness(GREEN_LED, 0);
    Led_setBrightness(RED_LED, 0);
    Led_cleanUp();
    Joystick_cleanUp();
    exit(0);
}

int main() {
    initializeGame();

    printf("Hello embedded world, from JunPin Foo!\n\n");
    printf("When the LEDs light up, press the joystick in that direction!\n");
    printf("(Press left or right to exit)\n");

    while(1){
        initialFlashLed();
        waitForJoystickRelease();
        int randomNumber = rand()%((3000+1)-500) + 500; //between 3000 and 500 ms

        //GAME STARTED
        sleepForMs(randomNumber);
        JoystickDirection data = getJoystickDirection();
        if(data == JOYSTICK_LEFT || data == JOYSTICK_RIGHT){ //player choose to Quit
            printf("User selected to quit.\n");
            exitGameCleanUp();
        }  
        if(data != JOYSTICK_CENTER) { //player moved joystick
            printf("too soon\n");
            continue; // Restart the game loop
        }

        if(randomNumber % 2 == 0) {
            startGameWithDirection(true); //Up game state
        } 
        else {
            startGameWithDirection(false); //Down game state
        }

    }

}