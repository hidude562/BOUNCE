////////////////////////////////////////
// { PROGRAM NAME } { VERSION }
// Author:
// License:
// Description:
////////////////////////////////////////

/*
* The comments in this file are here to guide you initially. Note that you shouldn't actually
* write comments that are pointless or obvious in your own code, write useful ones instead!
* See this for more details: https://ce-programming.github.io/toolchain/static/coding-guidelines.html
*
* Have fun!
*/

/* You probably want to keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

/* Here are some standard headers. Take a look at the toolchain for more. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphx.h>
#include <keypadc.h>
#include "gfx/gfx.h"

#define SCREEN_X    320
#define SCREEN_Y    240
#define LAUNCH_VELOCITY 90
int16_t PHYSICS_PRECISION = 50;
#define GRAVITY 1
#define numLevelBalls 20


void initLevel();
void drawGUI();


uint8_t  background_color = 52;
const uint16_t SPITTER_SIZE = 32;

int16_t cos_cache[360];
int16_t sin_cache[360];

int16_t spitterAngle = 90;
bool inPlay = false;
bool displayingLines = false;
uint16_t numGuides = 5;
uint16_t levelsCompleted = 0;

uint32_t score = 0;
int numBalls = 20;
uint32_t streak = 0;
uint32_t highestStreak = 0;


typedef struct PlayerBall {
    int16_t x;
    int16_t y;

    int16_t previousDrawX;
    int16_t previousDrawY;

    int16_t vX;
    int16_t vY;

    int16_t size;
} PlayerBall;

PlayerBall playerBall;

typedef struct {
    bool isOn;
    int16_t x;
    int16_t y;

    int16_t size;
    int16_t pointValue;
} LevelBall;

LevelBall levelBalls[numLevelBalls];

struct BackupState {
    PlayerBall player;
    LevelBall levelBalls[numLevelBalls];

    int16_t spitterAngle;
    bool inPlay;

    uint32_t score;
    int numBalls;
    uint32_t streak;
    uint8_t backgroundColor;
    uint16_t numGuides;
} backupState;

void initPlayer() {
    playerBall.x = 0;
    playerBall.y = 0;

    playerBall.vX = 0;
    playerBall.vY = 0;

    playerBall.size = 4;
}
void drawLevel() {
    gfx_FillScreen(background_color);

    for(int i = 0; i < numLevelBalls; i++) {
        if(levelBalls[i].isOn) {
            gfx_SetColor(background_color+1);
            gfx_FillCircle(levelBalls[i].x/PHYSICS_PRECISION, levelBalls[i].y/PHYSICS_PRECISION, levelBalls[i].size);
            gfx_SetColor(2);
            gfx_Circle(levelBalls[i].x/PHYSICS_PRECISION, levelBalls[i].y/PHYSICS_PRECISION, levelBalls[i].size);

        }
    }
}
void drawPlayer() {
    gfx_SetColor(background_color);
    gfx_FillCircle(playerBall.previousDrawX/PHYSICS_PRECISION, playerBall.previousDrawY/PHYSICS_PRECISION, playerBall.size);

    //gfx_SetColor(1);
    //gfx_FillCircle(playerBall.x/PHYSICS_PRECISION, playerBall.y/PHYSICS_PRECISION, playerBall.size);

    gfx_TransparentSprite(ball, playerBall.x/PHYSICS_PRECISION-4, playerBall.y/PHYSICS_PRECISION-4);

    playerBall.previousDrawX = playerBall.x;
    playerBall.previousDrawY = playerBall.y;
}
uint_fast8_t bit_width(int_fast64_t x) {
    return x == 0 ? 1 : 64 - __builtin_clzll(x);
}
int_fast64_t fastestSqrt64(const int_fast64_t n)
{
    // This will be probably up to 1% off or something
    int_fast64_t log2floor = bit_width(n) - 1;
    int_fast64_t a = (int_fast64_t) (n != 0) << (log2floor >> 1);
    int_fast64_t b;

    b = n / a; a = (a+b) / 2;
    b = n / a; a = (a+b) / 2;
    return a;
}
void drawMovableObjects() {
    // playerBall

    drawPlayer();
}
bool checkGameOver() {
    return numBalls<0;
}
void drawAll() {
    drawLevel();
    drawMovableObjects();
    drawGUI();
}
void drawFast() {
    drawMovableObjects();
}
void normalizeAngle(int16_t* angle) {
    if(*angle < 0)
        *angle+=360;
    if(*angle > 359)
        *angle-=360;
}
void applyVelocityToPlayer() {
    playerBall.x += playerBall.vX;
    playerBall.y += playerBall.vY;
}
void clearBall(uint8_t i) {
    gfx_SetColor(background_color);
    gfx_FillCircle(levelBalls[i].x/PHYSICS_PRECISION, levelBalls[i].y/PHYSICS_PRECISION, levelBalls[i].size);
}
void drawSpitter() {
    // Base of spitter
    gfx_SetColor(background_color);
    gfx_FillCircle(SCREEN_X/2,0,SPITTER_SIZE+20);



    // Cannon of spitter
    // TODO:



    gfx_SetColor(25);

    int16_t tmpAngle = spitterAngle-10;
    normalizeAngle(&tmpAngle);

    gfx_Line(
        SCREEN_X/2,0,
        SCREEN_X/2 + (cos_cache[tmpAngle] * 2 / (150/SPITTER_SIZE)),
        0  + (sin_cache[tmpAngle] * 2 / (150/SPITTER_SIZE))
    );

    int16_t tmpAngle2 = spitterAngle+10;
    normalizeAngle(&tmpAngle);



    gfx_Line(
        SCREEN_X/2,0,
        SCREEN_X/2 + (cos_cache[tmpAngle2] * 2 / (150/SPITTER_SIZE)),
        0  + (sin_cache[tmpAngle2] * 2 / (150/SPITTER_SIZE))
    );

    gfx_Line(
        SCREEN_X/2 + (cos_cache[tmpAngle] * 2 / (150/SPITTER_SIZE)),
        0  + (sin_cache[tmpAngle] * 2 / (150/SPITTER_SIZE)),
        SCREEN_X/2 + (cos_cache[tmpAngle2] * 2 / (150/SPITTER_SIZE)),
        0  + (sin_cache[tmpAngle2] * 2 / (150/SPITTER_SIZE))
    );

    gfx_SetColor(3);
    gfx_FillCircle(SCREEN_X/2,0,SPITTER_SIZE);

    gfx_SetColor(2);
    gfx_Circle(SCREEN_X/2,0,SPITTER_SIZE);


}
void drawGUI() {
    drawSpitter();

    gfx_SetColor(2);
    gfx_FillRectangle(0,0,SCREEN_X,10);
    gfx_FillRectangle(0,SCREEN_Y-9,24,9);
    gfx_FillRectangle(SCREEN_X-68,SCREEN_Y-9,68,9);

    gfx_PrintStringXY("P1", 1, SCREEN_Y-8);

    gfx_SetTextFGColor(1);
    gfx_SetTextXY(0,1);
    gfx_PrintString("Streak:");
    gfx_PrintUInt(streak,1);

    gfx_SetTextXY(80,1);
    gfx_PrintString("Score:");
    gfx_PrintUInt(score,1);

    gfx_SetTextXY(210,1);
    gfx_PrintString("Pinballs:");
    gfx_PrintInt(numBalls,1);

    gfx_SetTextXY(SCREEN_X-68, SCREEN_Y-8);
    gfx_PrintString("Guides: ");
    gfx_PrintInt(numGuides,2);
}
bool getIfLevelCompleted() {
    bool hasPegs = false;
    for(int i = 0; i < numLevelBalls; i++) {
        if(levelBalls[i].isOn)
            hasPegs=true;
    }

    return !hasPegs;
}
void playerPhysics() {
    if(inPlay) {
        playerBall.vY += GRAVITY;
        applyVelocityToPlayer();

        // Does collision for balls
        for(int i = 0; i < numLevelBalls; i++) {
            // TODO: distance formula

            if(levelBalls[i].isOn) {
                if(abs(playerBall.x - levelBalls[i].x) +
                   abs(playerBall.y - levelBalls[i].y) < playerBall.size * PHYSICS_PRECISION * 4) {

                    // TODO: faster calculate atan2
                    int16_t direction = (int) (atan2(playerBall.x - levelBalls[i].x, playerBall.y - levelBalls[i].y) * 57.296);
                    normalizeAngle(&direction);

                    int totalVelocity = fastestSqrt64(playerBall.vX * playerBall.vX + playerBall.vY * playerBall.vY);


                    playerBall.vX=sin_cache[direction]*totalVelocity;
                    playerBall.vX/=100;
                    playerBall.vY=cos_cache[direction]*totalVelocity;
                    playerBall.vY/=100;

                    levelBalls[i].isOn = false;

                    clearBall(i);

                    score += levelBalls[i].pointValue + streak*10;
                    streak+=1;
                    if(streak >= 5)
                        numGuides+=1;

                    drawGUI();
                }
            }
        }

        // Checks and fixes if out of bounds
        if(playerBall.x < 0) {
            playerBall.x+=SCREEN_X*PHYSICS_PRECISION;
        } else if(playerBall.x > SCREEN_X*PHYSICS_PRECISION) {
            playerBall.x-=SCREEN_X*PHYSICS_PRECISION;
        }

        if(playerBall.y > SCREEN_Y*PHYSICS_PRECISION) {
            inPlay = false;
            streak=0;
            initPlayer();
            bool complete = getIfLevelCompleted();
            numBalls--;
            drawGUI();

            if(complete) {
                initLevel();
                background_color+=1;
                numBalls+=3;
                levelsCompleted++;
                drawAll();
            }
        }
    } else {
        playerBall.x = (SCREEN_X/2 + (cos_cache[spitterAngle] * 2 / (150/SPITTER_SIZE))) * PHYSICS_PRECISION;
        playerBall.y = ((sin_cache[spitterAngle] * 2 / (150/SPITTER_SIZE))) * PHYSICS_PRECISION;
    }
}
void initLevel() {
    int i = 0;

    // wow this is inneficient but it works
    while(true) {
        for(int y = 80; y < 240; y+=16) {
            for(int x = 0; x < 320; x+=16) {
                if(rand() % 35 == 0) {
                    if(i > numLevelBalls-2) {
                        return;
                    }

                    levelBalls[i].x = x*PHYSICS_PRECISION;
                    levelBalls[i].y = y*PHYSICS_PRECISION;
                    levelBalls[i].size = 8;
                    levelBalls[i].isOn = true;
                    levelBalls[i].pointValue = 50;
                    i++;
                }
            }
        }
    }
}
void saveState() {
    backupState.player.x = playerBall.x;
    backupState.player.y = playerBall.y;
    backupState.player.previousDrawX = playerBall.previousDrawX;
    backupState.player.previousDrawY = playerBall.previousDrawY;
    backupState.player.vX = playerBall.vX;
    backupState.player.vY = playerBall.vY;
    backupState.player.size = playerBall.size;

    for(uint16_t i = 0; i < numLevelBalls; i++) {
        backupState.levelBalls[i].isOn = levelBalls[i].isOn;
        backupState.levelBalls[i].x = levelBalls[i].x;
        backupState.levelBalls[i].y = levelBalls[i].y;
        backupState.levelBalls[i].size = levelBalls[i].size;
        backupState.levelBalls[i].pointValue = levelBalls[i].pointValue;
    }

    backupState.spitterAngle = spitterAngle;
    backupState.inPlay = inPlay;
    backupState.score = score;
    backupState.numBalls = numBalls;
    backupState.streak = streak;
    backupState.backgroundColor = background_color;
    backupState.numGuides = numGuides;
}
void revertState() {
    playerBall.x = backupState.player.x;
    playerBall.y = backupState.player.y;
    playerBall.previousDrawX = backupState.player.previousDrawX;
    playerBall.previousDrawY = backupState.player.previousDrawY;
    playerBall.vX = backupState.player.vX;
    playerBall.vY = backupState.player.vY;
    playerBall.size = backupState.player.size;

    for(uint16_t i = 0; i < numLevelBalls; i++) {
        levelBalls[i].isOn = backupState.levelBalls[i].isOn;
        levelBalls[i].x = backupState.levelBalls[i].x;
        levelBalls[i].y = backupState.levelBalls[i].y;
        levelBalls[i].size = backupState.levelBalls[i].size;
        levelBalls[i].pointValue = backupState.levelBalls[i].pointValue;
    }

    spitterAngle = backupState.spitterAngle;
    inPlay = backupState.inPlay;
    score = backupState.score;
    numBalls = backupState.numBalls;
    streak = backupState.streak;
    background_color = backupState.backgroundColor;
    numGuides = backupState.numGuides;
}
void launchBall() {
    playerBall.vY = LAUNCH_VELOCITY * sin_cache[spitterAngle] / 100;
    playerBall.vX = LAUNCH_VELOCITY * cos_cache[spitterAngle] / 100;
    inPlay=true;
}
void predictBall() {
    numGuides--;
    saveState();

    launchBall();
    displayingLines = true;

    uint16_t i = 0;

    int16_t previousX = playerBall.x/PHYSICS_PRECISION-4;
    int16_t previousY = playerBall.y/PHYSICS_PRECISION-4;
    gfx_SetColor(2);

    while(inPlay) {
        playerPhysics();

        gfx_Line(playerBall.x/PHYSICS_PRECISION-4, playerBall.y/PHYSICS_PRECISION-4, previousX, previousY);

        previousX = playerBall.x/PHYSICS_PRECISION-4;
        previousY = playerBall.y/PHYSICS_PRECISION-4;

        i++;
    }

    revertState();
    //drawLevel();
    //drawGUI();
}
void rerenderIfDisplayingLines() {
    if(displayingLines) {
        displayingLines = false;
        drawAll();
    }
}
void renderGameOver() {
    gfx_SetTextFGColor(254);

    gfx_SetColor(2);
    gfx_FillRectangle(20, 96, SCREEN_X - 40, 63);

    gfx_SetColor(255);
    gfx_Rectangle(20, 96, SCREEN_X - 40, 63);

    gfx_Rectangle(25, 110, SCREEN_X - 50, 45);
    gfx_SetTextXY(27, 114);
    gfx_PrintString("Final score: ");
    gfx_PrintInt(score, 1);

    gfx_SetTextXY(27, 124);
    gfx_PrintString("Highest Streak: ");
    gfx_PrintInt(highestStreak, 1);

    gfx_SetTextXY(27, 134);
    gfx_PrintString("Levels Completed: ");
    gfx_PrintInt(levelsCompleted, 1);


    char* rank;
    if(score < 2000)
        rank="Are you even trying?";
    else if(score < 3000)
        rank="Ehh..";
    else if(score < 4000)
        rank="Novice";
    else if(score < 5000)
        rank="Almost average";
    else if(score < 6000)
        rank="Just Ok";
    else if(score < 7000)
        rank="Average";
    else if(score < 8000)
        rank="Above Average";
    else if(score < 9000)
        rank="Good";
    else if(score < 10000)
        rank="Great!";
    else if(score < 11000)
        rank="Better than me";
    else if(score < 12000)
        rank="Amazing!";
    else if(score < 13000)
        rank="Whaaat";
    else if(score < 14000)
        rank="How are you doing this?";
    else if(score < 15000)
        rank="Whaaat?!?";
    else if(score < 16000)
        rank="You are insane.";
    else if(score < 17000)
        rank="Pay attention in class!";
    else if(score < 18000)
        rank="Beast";
    else if(score < 18000)
        rank="I dont even know...";
    else if(score < 19000)
        rank="HOWWWW";
    else if(score < 20000)
        rank="Ok. this is out-of-hand";
    else if(score < 21000)
        rank="God Tier";
    else if(score < 22000)
        rank="Better than God Tier";

    gfx_SetTextXY(27, 144);
    gfx_PrintString("Rank: ");
    gfx_PrintString(rank);


    gfx_SetTextXY(27, 100);
    gfx_PrintString("Game over! Press ON to quit");
}

/* This is the entry point of your program. */
/* argc and argv can be there if you need to use arguments, see the toolchain example. */
int main() {

    boot_Set48MHzMode();

    uint8_t backc = 0x19;
    os_ClrHomeFull();
    gfx_Begin(gfx_8bpp);
    //gfx_SetDrawBuffer(); Use this in a loop or something

    gfx_SetTextFGColor(0xFE);
    gfx_SetPalette(global_palette, 256, 0);


    float float_i = 0;

    for(int i=0; i < 360; i++){
        cos_cache[i] = (int)(cos(float_i*0.01745)*100);//*(r_quality)); // all int16_ting point ints are scaled by 1k, also stored in degrees
        float_i+=1;
    }

    float_i = 0;
    for(int i=0; i < 360; i++){
        sin_cache[i] = (int)(sin(float_i*0.01745)*100);//*(r_quality)); // all int16_ting point ints are scaled by 1k, also stored in degrees
        float_i+=1;
    }

    initPlayer();
    initLevel();

    gfx_BlitScreen();
    drawAll();
    gfx_SwapDraw();

    while (true){ // Refer to https://ce-programming.github.io/toolchain/libraries/keypadc.html for key register info
        gfx_BlitScreen();

        kb_Scan();
        if(!inPlay) {
            if(kb_Data[7] & kb_Right){
                spitterAngle--;
                rerenderIfDisplayingLines();
                drawSpitter();
            }
            if(kb_Data[7] & kb_Left){
                spitterAngle++;
                rerenderIfDisplayingLines();
                drawSpitter();
            }
            if ((kb_Data[1] & kb_2nd)) {
                launchBall();
                rerenderIfDisplayingLines();
            }
            if ((kb_Data[1] & kb_Mode) && numGuides > 0 && !displayingLines){
                predictBall();
            }
        }

        normalizeAngle(&spitterAngle);

        drawFast();

        for(int i = 0; i < 3; i++)
        playerPhysics();

        if(kb_On){
            break; //Stops program
        }

        if(numBalls < 0)
            break;

        if(streak > highestStreak)
            highestStreak = streak;

        gfx_SwapDraw();
    }

    while (true) {
        gfx_BlitScreen();
        renderGameOver();
        gfx_SwapDraw();

        if(kb_On)
            break;
    }

        //A for loop that copies temp_life to lifez
    gfx_End();
    return 0;
}
