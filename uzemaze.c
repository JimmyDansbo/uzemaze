/*
 *  Uzebox video mode 40 simple demo
 *  Copyright (C) 2017 Sandor Zsuga (Jubatian)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <uzebox.h>

#include "levels.h"

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

#define LEFT	0
#define RIGHT	1
#define UP	2
#define DOWN	3

#define WALLCOL		LIGHTGRAY

static const char COLORS[]={
	BLACK,WHITE,RED,CYAN,
	PURPLE,GREEN,BLUE,YELLOW,
	ORANGE,BROWN,PINK,DARKGRAY,
	MIDGRAY,LIGHTGREEN,LIGHTBLUE,LIGHTGRAY };
static const char HEXTBL[] = {
	'0','1','2','3','4','5','6','7','8','9',1,2,3,4,5,6};

unsigned char cursorx, cursory, bgcolor, curlvl;
unsigned int lvlindex, remflds;

unsigned int ramaddr(char x, char y) {
	return ((x*2)+(y*(SCREEN_WIDTH*2)));
}

void printhex(unsigned char x, unsigned char y, char c) {
	PrintChar(x, y, HEXTBL[c>>4]);
	PrintChar(x+1,y,HEXTBL[c&0x0F]);
}

void printstr(unsigned char x, unsigned char y, char *str) {
	unsigned int cnt=0;
	unsigned char ch;

	while (str[cnt]!=0) {
		ch = str[cnt++];
		if (ch > 0x3F) ch=ch-0x40;
		PrintChar(x++, y, ch);
	}
}
void printstrfg(unsigned char x, unsigned char y, char *str, char fgcol) {
	unsigned int cnt=0;
	unsigned char ch;

	while (str[cnt]!=0) {
		ch = str[cnt++];
		if (ch > 0x3F) ch=ch-0x40;
		aram[ramaddr(x,y)+1]=fgcol;
		PrintChar(x++, y, ch);
	}
}
void printstrcol(unsigned char x, unsigned char y, char *str, char fgc, char bgc) {
	unsigned int cnt=0;
	unsigned char ch;

	while (str[cnt]!=0) {
		ch = str[cnt++];
		if (ch > 0x3F) ch=ch-0x40;
		aram[ramaddr(x,y)+1]=fgc;
		aram[ramaddr(x,y)]=bgc;
		PrintChar(x++, y, ch);
	}
}

void resetPlayfield() {
	unsigned char x, y;

	for (y=0; y<25; y++)
		for (x=0; x<40; x++) {
			PrintChar(x, y, 0x20);
			aram[ramaddr(x, y)]=WHITE;
		}
	for (y=1; y<24; y++)
		for (x=1; x<39; x++) {
			PrintChar(x, y, 0x20);
			aram[ramaddr(x, y)]=WALLCOL;
			aram[ramaddr(x, y)+1]=BLACK;
		}

	printstrfg((SCREEN_WIDTH/2)-(4),0,"UZEMAZE", RED);
	printstrfg((SCREEN_WIDTH-8),0,"LVL:   ", RED);
	PrintByte(SCREEN_WIDTH-2, 0, curlvl, true);
	printstrfg((SCREEN_WIDTH/2)-15,24,"DPAD=MOVE B=NEXT SELECT=RESET", RED);
}

void nextbgcolor() {
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

void splashscreen() {
	unsigned char x, y;
	unsigned int btn=0;

	for (y=0;y<25;y++)
		for (x=0;x<40;x++) {
			PrintChar(x, y, ' ');
			aram[ramaddr(x, y)]=BLACK;
		}
	
	printstrfg(3, 5,  "*  * **** **** *  *  **  **** ****", ORANGE);
	printstrfg(3, 6,  "*  *   ** *    **** *  *   ** *", ORANGE);
	printstrfg(3, 7,  "*  *  **  ***  *  * ****  **  **", ORANGE);
	printstrfg(3, 8,  "*  * **   *    *  * *  * **   *", ORANGE);
	printstrfg(3, 9,  " **  **** **** *  * *  * **** ****", ORANGE);
	printstrfg((SCREEN_WIDTH/2)-12, 13, "PRESS START TO BEGIN GAME", WHITE);
	printstrfg((SCREEN_WIDTH/2)-11, 17, "CREATED BY JIMMY DANSBO", GREEN);
	printstrfg((SCREEN_WIDTH/2)-11, 19, "(JIMMY@DANSBO.DK)  2023", GREEN);
	printstrfg((SCREEN_WIDTH/2)-15, 21, "HTTPS://GITHUB.COM/JIMMYDANSBO/", LIGHTGREEN);

	while (btn != BTN_START) {
		WaitVsync(1);
		nextbgcolor();
		btn=ReadJoypad(0);
	}
}

unsigned char levelbyte(unsigned int index) {
	return (pgm_read_byte(&(levels[index])));
}

void seeklevel() {
	unsigned char lvl=1;

	lvlindex = 0;

	while (lvl++ != curlvl) {
		if (levelbyte(lvlindex)==0) {
			curlvl=1;
			lvlindex=0;
			return;
		}
		lvlindex += levelbyte(lvlindex);
	}
	if (levelbyte(lvlindex)==0) {
		curlvl=1;
		lvlindex=0;
		return;
	}
}

void drawlevel() {
	unsigned char ch, bitcnt;
	unsigned char offsetx, offsety, datacnt;
	unsigned char curx, cury;

	remflds=0;

	SetBorderColor(bgcolor);

	offsetx = (SCREEN_WIDTH/2)-(levelbyte(lvlindex+1)/2);
	offsety = (SCREEN_HEIGHT/2)-(levelbyte (lvlindex+2)/2);

	datacnt=0;
	bitcnt=0;
	ch=levelbyte(lvlindex+5+datacnt++);
	for (cury=offsety; cury<offsety+levelbyte(lvlindex+2); cury++) {
		if (bitcnt!=0) {
			ch = levelbyte(lvlindex+5+datacnt++);
			bitcnt=0;
		}
		for (curx=offsetx; curx<offsetx+levelbyte(lvlindex+1); curx++) {
			if ((ch & 0x80) == 0) {
				aram[ramaddr(curx, cury)]=BLACK;
				remflds++;
			}
			ch = ch<<1;
			if (++bitcnt == 8) {
				ch = levelbyte(lvlindex+5+datacnt++);
				bitcnt=0;
			}
		}
	}

	cursorx = offsetx+levelbyte(lvlindex+3);
	cursory = offsety+levelbyte(lvlindex+4);

	PrintChar(cursorx, cursory, 0x57);
	aram[ramaddr(cursorx, cursory)]=bgcolor;
	remflds--;
}

void do_move(char dir) {
 switch (dir) {
	case RIGHT:
		while (aram[ramaddr(cursorx+1, cursory)]!=WALLCOL) {
			cursorx++;
			PrintChar(cursorx-1, cursory, ' ');
			if (aram[ramaddr(cursorx, cursory)]==BLACK) remflds--;
			PrintChar(cursorx, cursory, 0x57);
			aram[ramaddr(cursorx, cursory)]=bgcolor;
			WaitVsync(1);
		}
		break;
	case LEFT:
		while (aram[ramaddr(cursorx-1, cursory)]!=WALLCOL) {
			cursorx--;
			PrintChar(cursorx+1, cursory, ' ');
			if (aram[ramaddr(cursorx, cursory)]==BLACK) remflds--;
			PrintChar(cursorx, cursory, 0x57);
			aram[ramaddr(cursorx, cursory)]=bgcolor;
			WaitVsync(1);
		}
		break;
	case UP:
		while (aram[ramaddr(cursorx, cursory-1)]!=WALLCOL) {
			cursory--;
			PrintChar(cursorx, cursory+1, ' ');
			if (aram[ramaddr(cursorx, cursory)]==BLACK) remflds--;
			PrintChar(cursorx, cursory, 0x57);
			aram[ramaddr(cursorx, cursory)]=bgcolor;
			WaitVsync(1);
		}
		break;
	case DOWN:
		while (aram[ramaddr(cursorx, cursory+1)]!=WALLCOL) {
			cursory++;
			PrintChar(cursorx, cursory-1, ' ');
			if (aram[ramaddr(cursorx, cursory)]==BLACK) remflds--;
			PrintChar(cursorx, cursory, 0x57);
			aram[ramaddr(cursorx, cursory)]=bgcolor;
			WaitVsync(1);
		}
		break;
 }
}

void show_win() {
	unsigned int btn=0;

	printstrcol(10, 10, "*******************", ORANGE, BLACK);
	printstrcol(10, 11, "*                 *", ORANGE, BLACK);
	printstrcol(10, 12, "* LEVEL COMPLETED *", ORANGE, BLACK);
	printstrcol(10, 13, "*                 *", ORANGE, BLACK);
	printstrcol(10, 14, "*******************", ORANGE, BLACK);

	while (btn != BTN_B) {
		WaitVsync(10);
		nextbgcolor();
		SetBorderColor(bgcolor);
		btn=ReadJoypad(0);
	}
}

int main(){
	unsigned int btn;
	curlvl=1;
	bgcolor=WHITE;

	ClearVram();
	SetBorderColor(LIGHTGRAY);
	splashscreen();

	while (1) {
		seeklevel();
		resetPlayfield();
		drawlevel();
		btn=0;
		while (btn != BTN_SELECT) {
			WaitVsync(1);
			btn=ReadJoypad(0);
			if (btn & BTN_RIGHT) 
				do_move(RIGHT);
			else if (btn & BTN_LEFT)
				do_move(LEFT);
			else if (btn & BTN_UP)
				do_move(UP);
			else if (btn & BTN_DOWN)
				do_move(DOWN);
			if (remflds==0) {
				show_win();
				curlvl++;
				btn=BTN_SELECT;
			}
		}
	}
}
