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
#define PURPPLE		0xC4
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

#define RAMADDR(x,y) ((x*2)+(y*(SCREEN_WIDTH*2)))

static const unsigned char COLORS[]={
	BLACK,WHITE,RED,CYAN,
	PURPPLE,GREEN,BLUE,YELLOW,
	ORANGE,BROWN,PINK,DARKGRAY,
	MIDGRAY,LIGHTGREEN,LIGHTBLUE,LIGHTGRAY };
static const unsigned char HEXTBL[] = {
	'0','1','2','3','4','5','6','7','8','9',1,2,3,4,5,6};



void printhex(unsigned char x, unsigned char y, unsigned char c) {
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
void printstrfg(unsigned char x, unsigned char y, char *str, unsigned char fgcol) {
	unsigned int cnt=0;
	unsigned char ch;

	while (str[cnt]!=0) {
		ch = str[cnt++];
		if (ch > 0x3F) ch=ch-0x40;
		aram[RAMADDR(x,y)+1]=fgcol;
		PrintChar(x++, y, ch);
	}
}

void resetPlayfield(unsigned char CurrLvl) {
	unsigned char x, y;

	for (y=0; y<25; y++)
		for (x=0; x<40; x++) {
			PrintChar(x, y, 0x20);
			aram[RAMADDR(x, y)]=WHITE;
		}
	for (y=1; y<24; y++)
		for (x=1; x<39; x++) {
			PrintChar(x, y, 0x20);
			aram[RAMADDR(x, y)]=LIGHTGRAY;
			aram[RAMADDR(x, y)+1]=BLACK;
		}

	printstrfg((SCREEN_WIDTH/2)-(4),0,"UZEMAZE", RED);
	printstrfg((SCREEN_WIDTH-8),0,"LVL:   ", RED);
	PrintByte(SCREEN_WIDTH-2, 0, CurrLvl, true);
	printstrfg((SCREEN_WIDTH/2)-15,24,"DPAD=MOVE B=NEXT SELECT=RESET", RED);
}

void splashscreen() {
	unsigned char x, y;
	unsigned int btn=0;

	for (y=0;y<25;y++)
		for (x=0;x<40;x++) {
			PrintChar(x, y, ' ');
			aram[RAMADDR(x, y)]=BLACK;
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

	while (btn != BTN_SELECT) {
		WaitVsync(1);
		btn=ReadJoypad(0);
	}

}

int seeklevel(unsigned char level) {
	struct level *lvlptr;
	int lvlindex=0;
	unsigned char lvl=1;

	lvlptr = levels + lvlindex;
	while (level != lvl++) {
		if (lvlptr->size == 0) return (-1);
		lvlindex = lvlindex + (int)lvlptr->size;
		lvlptr = levels + lvlindex;
	}
	if (lvlptr->size==0) return (-1);
	return (lvlindex);
}

void drawlevel(int lvlindex) {
	struct level *lvlptr;
	unsigned char *data;

	lvlptr = levels + lvlindex;

	PrintByte(10, 10, (char)lvlptr->size, true);
	PrintByte(10, 11, (char)lvlptr->width, true);
	PrintByte(10, 12, (char)lvlptr->height, true);
	PrintByte(10, 13, (char)lvlptr->startx, true);
	PrintByte(10, 14, (char)lvlptr->starty, true);


}

int main(){
	unsigned int btn;
	unsigned char CurrLvl=1;
	unsigned int lvlindex;


	ClearVram();
	SetBorderColor(LIGHTGRAY);
	splashscreen();
	SetBorderColor(DARKGRAY);
	resetPlayfield(CurrLvl);
	
	lvlindex = seeklevel(1);
	if (lvlindex != -1)
		drawlevel(lvlindex);

	while(1) {
		WaitVsync(1);
/*		btn = ReadJoypad(0);
		if (btn & BTN_RIGHT) PrintChar(10, 10, 'R'-0x40); 
		else PrintChar(10, 10, ' ');
		if (btn & BTN_LEFT)  PrintChar(8, 10, 'L'-0x40);
		else PrintChar(8,10, ' ');
		if (btn & BTN_UP)    PrintChar(9, 9, 'U'-0x40);
		else PrintChar(9, 9, ' ');
		if (btn & BTN_DOWN)  PrintChar(9, 11, 'D'-0x40);
		else PrintChar(9, 11, ' ');
		if (btn & BTN_Y)     PrintChar(16, 10, 'Y'-0x40);
		else PrintChar(16,10, ' ');
		if (btn & BTN_A)     PrintChar(18, 10, 'A'-0x40);
		else PrintChar(18, 10, ' ');
		if (btn & BTN_X)     PrintChar(17, 9, 'X'-0x40);
		else PrintChar(17, 9, ' ');
		if (btn & BTN_B)     PrintChar(17, 11, 'B'-0x40);
		else PrintChar(17, 11, ' ');
		if (btn & BTN_SELECT) PrintChar(12, 10, 'L'-0x40);
		else PrintChar(12, 10, ' ');
		if (btn & BTN_START) PrintChar(14, 10, 'S'-0x40);
		else PrintChar(14, 10, ' ');*/
	}


}
