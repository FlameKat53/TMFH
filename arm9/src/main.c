#include "main.h"
#include "menu.h"
#include "message.h"
#include <time.h>

#define APP_VERSION "Bruh"

enum {
	MAIN_MENU_TITLES
};

void setFontTop() {
	PrintConsole *console = consoleInit(NULL, 2, BgType_Text4bpp, BgSize_T_256x256, 2, 0, true, true);
	ConsoleFont font;
	font.gfx = (u16*)fontTiles;
	font.pal = (u16*)fontPal;
	font.numChars = 95;
	font.numColors =  fontPalLen / 2;
	font.bpp = 4;
	font.asciiOffset = 32;
	font.convertSingleColor = false;
	consoleSetFont(console, &font);
}

void setFontSub() {
	PrintConsole *console = consoleInit(NULL, 2, BgType_Text4bpp, BgSize_T_256x256, 0, 15, false, true);
	ConsoleFont font;
	font.gfx = (u16*)fontTiles;
	font.pal = (u16*)fontPal;
	font.numChars = 95;
	font.numColors =  fontPalLen / 2;
	font.bpp = 4;
	font.asciiOffset = 32;
	font.convertSingleColor = false;
	consoleSetFont(console, &font);
}

static void _setupScreens() {
	REG_DISPCNT = MODE_FB0;
	VRAM_A_CR = VRAM_ENABLE;

	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	setFontTop();
	setFontSub();

	clearScreen(&bottomScreen);

	VRAM_A[100] = 0xFFFF;
}

static int _mainMenu(int cursor) {
	//top screen
	clearScreen(&topScreen);

	iprintf("\tRelaunch.nds %s", APP_VERSION);

	//menu
	Menu* m = newMenu();
	setMenuHeader(m, "Everyone is legal");

	addMenuItem(m, "Titles", NULL, 0);

	m->cursor = cursor;

	//bottom screen
	printMenu(m);

	while (1) {
		swiWaitForVBlank();
		scanKeys();

		if (moveCursor(m))
			printMenu(m);

		if (keysDown() & KEY_A)
			break;
	}

	int result = m->cursor;
	freeMenu(m);

	return result;
}

int main(int argc, char **argv) {
	srand(time(0));
	_setupScreens();

	//setup sd card access
	if (!fatInitDefault()) {
		messageBox("fatInitDefault()...\x1B[31mFailed\n\x1B[47m");
		return 0;
	}

	//main menu
	bool programEnd = false;
	int cursor = 0;

	while (!programEnd) {
		cursor = _mainMenu(cursor);

		switch (cursor) {
			case MAIN_MENU_TITLES:
				titleMenu();
				break;
		}
	}

	return 0;
}

void clearScreen(PrintConsole* screen) {
	consoleSelect(screen);
	consoleClear();
}
