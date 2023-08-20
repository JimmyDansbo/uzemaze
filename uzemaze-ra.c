#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>
#include <string.h>
#include "levels-ra.h"

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 25

// Color are in normal Uzebox colors: BBGGGRRR
#define BLACK		0x00
#define WHITE		0xFF
#define RED		0x07
#define CYAN		0xFB
#define PURPLE		0xC4
#define GREEN		0x38
#define BLUE		0xC0
#define YELLOW		0x3E
#define ORANGE		0x2E
#define BROWN		0x0A
#define PINK		0x46
#define DARKGRAY	0x08
#define MIDGRAY		0x5A
#define LIGHTGREEN	0x7C
#define LIGHTBLUE	0xDA
#define LIGHTGRAY	0xAD
#define RA_BLUE 0xD8
#define RA_GOLD 0x26
#define LEFT	0
#define RIGHT	1
#define UP	2
#define DOWN	3

#define WALLCOL		LIGHTGRAY

u8 cursorx, cursory, bgcolor, curlvl;
u16 lvlindex, remflds, myTimer, MoveCnt;

static u16 ramaddr(u8 x, u8 y) {
	return ((x*2)+(y*(SCREEN_WIDTH*2)));
}

static void printstr(u8 x, u8 y, char *str) {
	u16 cnt=0;
	u8 ch;

	while (str[cnt]!=0) {
		ch = str[cnt++];
		if (ch > 0x3F) ch=ch-0x40;
		PrintChar(x++, y, ch);
	}
}
static void printstrfg(u8 x, u8 y, char *str, u8 fgcol) {
	u16 cnt=0;
	u8 ch;

	while (str[cnt]!=0) {
		ch = str[cnt++];
		if (ch > 0x3F) ch=ch-0x40;
		aram[ramaddr(x,y)+1]=fgcol;
		PrintChar(x++, y, ch);
	}
}
static void printstrcol(u8 x, u8 y, char *str, u8 fgc, u8 bgc) {
	u16 cnt=0;
	u8 ch;

	while (str[cnt]!=0) {
		ch = str[cnt++];
		if (ch > 0x3F) ch=ch-0x40;
		aram[ramaddr(x,y)+1]=fgc;
		aram[ramaddr(x,y)]=bgc;
		PrintChar(x++, y, ch);
	}
}
char* names[15] ={
	"HEXADIGITAL",
	"STINGX2",
	"DAROACHIE",
	"XALERZYX",
	"SUTARION",
	"BRYAN1150",
	"ELDRIDGECRIMSON",
	"1STPRIZE",
	"MARIOKNESS",
	"NEOMAR",
	"ZEERA",
	"LABUFF",
	"TRIFORCE",
	"LADYLUMINA",
	"PINGUUPINGUU"
	
};
static void GetUser(u8 target){
	printstrfg(SCREEN_WIDTH - strlen(names[target - 1]) - 1,0, names[target - 1], RED);		
}
static void resetPlayfield() {
	u8 x, y;	
	for (y=0; y<SCREEN_HEIGHT; y++)
		for (x=0; x<SCREEN_WIDTH; x++) {
			PrintChar(x, y, 0x20);
			aram[ramaddr(x, y)]=WHITE;
		}
	for (y=1; y<24; y++)
		for (x=1; x<39; x++) {
			PrintChar(x, y, 0x20);
			aram[ramaddr(x, y)]=WALLCOL;
			aram[ramaddr(x, y)+1]=BLACK;
		}
	printstrfg(1,0,"FLDS:      ", RED); 
	PrintByte(8,0,0,true);
	GetUser(curlvl);
	printstrfg((SCREEN_WIDTH/2)-15,24,"DPAD=MOVE B=NEXT SELECT=RESET", RED);
}
static void nextbgcolor() {
	switch (bgcolor) {
		case WHITE:	bgcolor=RED;	break;
		case RED:	bgcolor=CYAN;	break;
		case CYAN:	bgcolor=PURPLE;	break;
		case PURPLE:	bgcolor=GREEN;	break;
		case GREEN:	bgcolor=BLUE;	break;
		case BLUE:	bgcolor=YELLOW;	break;
		case YELLOW:	bgcolor=ORANGE; break;
		case ORANGE:	bgcolor=PINK;	break;
		case PINK:	bgcolor=LIGHTGREEN; break;
		case LIGHTGREEN:bgcolor=LIGHTBLUE; break;
		case LIGHTBLUE:	bgcolor=WHITE;	break;
	}
}
static void splashscreen() {
	u8 x, y;
	u16 btn=0;

	for (y=0;y<SCREEN_HEIGHT;y++)
		for (x=0;x<SCREEN_WIDTH;x++) {
			PrintChar(x, y, ' ');
			aram[ramaddr(x, y)]=BLACK;
		}
	printstrfg(3, 5,  "*  * **** **** *  *  **  **** ****", ORANGE);
	printstrfg(3, 6,  "*  *   ** *    **** *  *   ** *", ORANGE);
	printstrfg(3, 7,  "*  *  **  ***  *  * ****  **  **", ORANGE);
	printstrfg(3, 8,  "*  * **   *    *  * *  * **   *", ORANGE);
	printstrfg(3, 9,  " **  **** **** *  * *  * **** ****", ORANGE);
	printstrfg((SCREEN_WIDTH/2)-12, 13, "PRESS START TO BEGIN GAME", WHITE);
	printstrfg((SCREEN_WIDTH/2)-11, 16, "HACK BY PINGUUPINGUU", WHITE);
	printstrfg((SCREEN_WIDTH/2)-11, 19, "MAPS MADE BY    USERS", WHITE);
	printstrfg((SCREEN_WIDTH/2)+2, 19, "R", RA_BLUE);
	printstrfg((SCREEN_WIDTH/2)+3, 19, "A", RA_GOLD);
	printstrfg((SCREEN_WIDTH/2)-15, 23, "HTTPS://RETRO", RA_BLUE);
	printstrfg((SCREEN_WIDTH/2)-2, 23, "ACHIEVEMENTS.ORG", RA_GOLD);
	while (btn != BTN_START) {
		WaitVsync(1);
		nextbgcolor();
		btn=ReadJoypad(0);
	}
}
static void seeklevel() {
	u8 lvl=1;

	lvlindex = 0;

	while (lvl++ != curlvl) {
		if (levels[lvlindex]==0) {
			curlvl=1;
			lvlindex=0;
			return;
		}
		lvlindex += levels[lvlindex];
	}
	if (levels[lvlindex]==0) {
		curlvl=1;
		lvlindex=0;
		return;
	}
}
static void drawlevel() {
	u8 ch, bitcnt;
	u8 offsetx, offsety, datacnt;
	u8 curx, cury;
	remflds=0;
	SetBorderColor(bgcolor);
	offsetx = (SCREEN_WIDTH/2)-(levels[lvlindex+1]/2);
	offsety = (SCREEN_HEIGHT/2)-(levels[lvlindex+2]/2);
	datacnt=0;
	bitcnt=0;
	ch=levels[lvlindex+5+datacnt++];
	for (cury=offsety; cury<offsety+levels[lvlindex+2]; cury++) {
		if (bitcnt!=0) {
			ch = levels[lvlindex+5+datacnt++];
			bitcnt=0;
		}
		for (curx=offsetx; curx<offsetx+levels[lvlindex+1]; curx++) {
			if ((ch & 0x80) == 0) {
				aram[ramaddr(curx, cury)]=BLACK;
				remflds++;
			}
			ch = ch<<1;
			if (++bitcnt == 8) {
				ch = levels[lvlindex+5+datacnt++];
				bitcnt=0;
			}
		}
	}
	cursorx = offsetx+levels[lvlindex+3];
	cursory = offsety+levels[lvlindex+4];
	PrintChar(cursorx, cursory, 0x57);
	aram[ramaddr(cursorx, cursory)]=bgcolor;
	remflds--;
	PrintByte(8, 0, remflds, true);
	myTimer=0;
	MoveCnt=0;
}

static void do_move(signed char x, signed char y) {
	u8 moved=false;
	while (aram[ramaddr(cursorx+x, cursory+y)]!=WALLCOL) {
		PrintChar(cursorx, cursory, ' ');
		cursorx+=x;
		cursory+=y;
		moved=true;
		if (aram[ramaddr(cursorx, cursory)]==BLACK) {
			remflds--;
			PrintByte(8, 0, remflds, true);
		}
		PrintChar(cursorx, cursory, 0x57);
		aram[ramaddr(cursorx, cursory)]=bgcolor;
		WaitVsync(1);
	}
	if (moved) MoveCnt++;
}
static void show_win() {
	u16 btn=0;
	u8 hour, minute, second;
	u16 msec, curtimer;
	char str[40];

	curtimer=myTimer;

	hour = curtimer/60/60/60;
	curtimer=curtimer-(hour*60*60*60);
	minute = curtimer/60/60;
	curtimer=curtimer-(minute*60*60);
	second = curtimer/60;
	curtimer=curtimer-(second*60);
	msec=curtimer*(1000/60);

	sprintf(str, "* %02d HR %02d MIN %02d SEC %03d MSEC *",hour,minute,second,msec);

	printstrcol(4, 6,  "********************************", ORANGE, BLACK);
	printstrcol(4, 7,  "*                              *", ORANGE, BLACK);
	printstrcol(4, 8,  "*       LEVEL COMPLETED        *", ORANGE, BLACK);
	printstrcol(4, 9, "*                              *", ORANGE, BLACK);
	printstrcol(4, 10, str, ORANGE, BLACK);
	printstrcol(4, 11, "*                              *", ORANGE, BLACK);
	printstrcol(4, 13, "*                              *", ORANGE, BLACK);
	printstrcol(4, 14, "********************************", ORANGE, BLACK);
	sprintf(str,"*          %04d MOVES          *",MoveCnt);
	printstrcol(4, 12, str, ORANGE,BLACK);

	while (btn != BTN_B) {
		WaitVsync(13);
		nextbgcolor();
		SetBorderColor(bgcolor);
		btn=ReadJoypad(0);
	}
}
static void select_level() {
	u16 btnPressed=0;
	u16 btnHeld=0;
	u16 btnPrev=0;
	char str[6];
	printstrcol(12, 9, "*****************", ORANGE, BLACK);
	printstrcol(12,10, "*               *", ORANGE, BLACK);
	printstrcol(12,11, "* SELECT LEVEL: *", ORANGE, BLACK);
	printstrcol(12,12, "*               *", ORANGE, BLACK);
	printstrcol(12,13, "*               *", ORANGE, BLACK);
	printstrcol(12,14, "*               *", ORANGE, BLACK);
	printstrcol(12,15, "*****************", ORANGE, BLACK);
	sprintf(str, "%03d", curlvl);
	printstr(19, 13, str);
	while (btnPressed != BTN_SELECT) {
		WaitVsync(1);
		btnHeld = ReadJoypad(0);
		btnPressed = btnHeld & (btnHeld ^ btnPrev);

		if (btnPressed & BTN_DOWN) {
			if (--curlvl == 0) curlvl=MAX_LEVELS;
			sprintf(str, "%03d", curlvl);
			printstr(19, 13, str);
		} else if (btnPressed & BTN_UP) {
			if (++curlvl > MAX_LEVELS) curlvl=1;
			sprintf(str, "%03d", curlvl);
			printstr(19, 13, str);
		}
		btnPrev = btnHeld;
	}
}
void myCallbackFunc(void) {
	myTimer++;
}
int main(){
	u16 btnPressed;
	u16 btnHeld=0;
	u16 btnPrev=0;
	curlvl=1;
	bgcolor=WHITE;
	ClearVram();
	SetBorderColor(LIGHTGRAY);
	SetUserPreVsyncCallback(&myCallbackFunc);
	splashscreen();
	while (1) {
		seeklevel();
		resetPlayfield();
		drawlevel();
		btnPressed=0;
		while (btnPressed != BTN_SELECT) {
			WaitVsync(1);
			btnHeld = ReadJoypad(0);
			btnPressed = btnHeld & (btnHeld ^ btnPrev);
			if (btnPressed & BTN_RIGHT) 
				do_move(1, 0);
			else if (btnPressed & BTN_LEFT)
				do_move(-1, 0);
			else if (btnPressed & BTN_UP)
				do_move(0, -1);
			else if (btnPressed & BTN_DOWN)
				do_move(0, 1);
			else if (btnPressed & BTN_X) {
				select_level();
				btnPressed=BTN_SELECT;
			}
			btnPrev = btnHeld;
			if (remflds==0) {
				show_win();
				curlvl++;
				btnPressed=BTN_SELECT;
			}
		}
	}
}