#include "main.h"
#include "rom.h"
#include "install.h"
#include "menu.h"
#include "storage.h"
#include "message.h"
#include <dirent.h>

enum {
	INSTALL_MENU_INSTALL,
	INSTALL_MENU_00,
	INSTALL_MENU_01,
	INSTALL_MENU_04,
	INSTALL_MENU_05,
	INSTALL_MENU_11,
	INSTALL_MENU_15,
	INSTALL_MENU_17,
	INSTALL_MENU_DELETE,
	INSTALL_MENU_BACK
};

static char currentDir[512] = "";

static void generateList(Menu* m);
static void printItem(Menu* m);
static int subMenu();
static bool delete(Menu* m);

static void _setHeader(Menu* m) {
	if (!m) return;
	if (currentDir[0] == '\0')
		setMenuHeader(m, "/");
	else
		setMenuHeader(m, currentDir);
}

void installMenu() {
	Menu* m = newMenu();
	_setHeader(m);
	generateList(m);
	{
		while (1) {
			swiWaitForVBlank();
			scanKeys();

			if (moveCursor(m)) {
				if (m->changePage != 0)
					generateList(m);

				printMenu(m);
				printItem(m);
			}

			//back
			if (keysDown() & KEY_B) {
				char* ptr = strrchr(currentDir, '/');

				if (ptr) {
					*ptr = '\0';
					_setHeader(m);
					resetMenu(m);
					generateList(m);
					printMenu(m);
				} else {
					break;
				}
			}

			else if (keysDown() & KEY_X)
				break;

			//selection
			else if (keysDown() & KEY_A) {
				if (m->itemCount > 0) {
					if (m->items[m->cursor].directory == false) {
						//nds file
						switch (subMenu()) {
							case INSTALL_MENU_INSTALL:
								install(m->items[m->cursor].value, 0);
								break;
							case INSTALL_MENU_00:
								install(m->items[m->cursor].value, 1);
								break;
							case INSTALL_MENU_01:
								install(m->items[m->cursor].value, 2);
								break;
							case INSTALL_MENU_04:
								install(m->items[m->cursor].value, 3);
								break;
							case INSTALL_MENU_05:
								install(m->items[m->cursor].value, 4);
								break;
							case INSTALL_MENU_11:
								install(m->items[m->cursor].value, 5);
								break;
							case INSTALL_MENU_15:
								install(m->items[m->cursor].value, 6);
								break;
							case INSTALL_MENU_17:
								install(m->items[m->cursor].value, 7);
								break;

							case INSTALL_MENU_DELETE:
							{
								if (delete(m)) {
									resetMenu(m);
									generateList(m);
								}
							}
							break;

							case INSTALL_MENU_BACK:					
								break;
						}
					} else {
						//directory
						sprintf(currentDir, "%s", m->items[m->cursor].value);
						_setHeader(m);
						resetMenu(m);
						generateList(m);
					}

					printMenu(m);
				}
			}
		}
	}

	freeMenu(m);
}

static void generateList(Menu* m) {
	if (!m) return;

	//reset menu
	clearMenu(m);

	m->page += sign(m->changePage);
	m->changePage = 0;

	bool done = false;

	struct dirent* ent;
	DIR* dir = NULL;

	if (currentDir[0] == '\0')
		dir = opendir("/");
	else
		dir = opendir(currentDir);	

	if (dir) {
		int count = 0;

		//scan directory
		while ( (ent = readdir(dir)) && !done) {
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type == DT_DIR) {
				if (count < m->page * ITEMS_PER_PAGE)
						count += 1;

				else {
					if (m->itemCount >= ITEMS_PER_PAGE)
						done = true;
					
					else {
						char* fpath = (char*)malloc(strlen(currentDir) + strlen(ent->d_name) + 8);
						sprintf(fpath, "%s/%s", currentDir, ent->d_name);

						addMenuItem(m, ent->d_name, fpath, 1);
					}
				}
			} else {
				if (strstr(ent->d_name, ".nds") != NULL ||
					strstr(ent->d_name, ".app") != NULL ||
					strstr(ent->d_name, ".dsi") != NULL ||
					strstr(ent->d_name, ".cia") != NULL ||
					strstr(ent->d_name, ".srl") != NULL ||
					strstr(ent->d_name, ".NDS") != NULL ||
					strstr(ent->d_name, ".APP") != NULL ||
					strstr(ent->d_name, ".DSI") != NULL ||
					strstr(ent->d_name, ".CIA") != NULL ||
					strstr(ent->d_name, ".SRL") != NULL)
				{
					if (count < m->page * ITEMS_PER_PAGE)
						count += 1;
					
					else {
						if (m->itemCount >= ITEMS_PER_PAGE)
							done = true;
						
						else {
							char* fpath = (char*)malloc(strlen(currentDir) + strlen(ent->d_name) + 8);
							sprintf(fpath, "%s/%s", currentDir, ent->d_name);

							addMenuItem(m, ent->d_name, fpath, 0);

							free(fpath);
						}
					}
				}
			}
		}
	}

	closedir(dir);

	m->nextPage = done;

	if (m->cursor >= m->itemCount)
		m->cursor = m->itemCount - 1;

	printItem(m);
	printMenu(m);
}

static void printItem(Menu* m) {
	if (!m) return;
	if (m->itemCount <= 0) return;

	if (m->items[m->cursor].directory)
		clearScreen(&topScreen);
	else
		printRomInfo(m->items[m->cursor].value);
}

static int subMenu() {
	int result = -1;

	Menu* m = newMenu();

	addMenuItem(m, "Install to default", NULL, 0);
	addMenuItem(m, "Install to 00", NULL, 0);
	addMenuItem(m, "Install to 01", NULL, 0);
	addMenuItem(m, "Install to 04", NULL, 0);
	addMenuItem(m, "Install to 05", NULL, 0);
	addMenuItem(m, "Install to 11", NULL, 0);
	addMenuItem(m, "Install to 15", NULL, 0);
	addMenuItem(m, "Install to 17", NULL, 0);
	addMenuItem(m, "Delete", NULL, 0);
	addMenuItem(m, "Back - [B]", NULL, 0);

	printMenu(m);

	while (1) {
		swiWaitForVBlank();
		scanKeys();

		if (moveCursor(m))
			printMenu(m);

		if (keysDown() & KEY_B) {
			result = -1;
			break;
		}

		else if (keysDown() & KEY_A) {
			result = m->cursor;
			break;
		}
	}

	freeMenu(m);
	return result;
}

static bool delete(Menu* m) {
	if (!m) return false;
	
	char* fpath = m->items[m->cursor].value;

	bool result = false;
	bool choice = NO;
	{
		char str[] = "Are you sure you want to delete\n";
		char* msg = (char*)malloc(strlen(str) + strlen(fpath) + 1);
		sprintf(msg, "%s%s", str, fpath);

		choice = choiceBox(msg);

		free(msg);
	}

	if (choice == YES) {
		if (!fpath) {
			messageBox("\x1B[31mCould not delete file.\x1B[47m");
		} else {
			if (remove(fpath) == 0) {
				result = true;
				messageBox("\x1B[42mFile deleted.\x1B[47m");
			} else {
				messageBox("\x1B[31mCould not delete file.\x1B[47m");
			}
		}		
	}

	return result;
}
