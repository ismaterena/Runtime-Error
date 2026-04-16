#define _CRT_SECURE_NO_WARNINGS
#include "iGraphics.h"
#include "Globals.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "Level4.h"
#include "Level5.h"
#include "Level6.h"
#include <stdio.h>

// Global Variables
GameState currentState = MENU; // Boots to the Main Menu now so you can choose!
int unlockedLevel = 6;
int menuCooldown = 0;

// --- SAVE/LOAD SYSTEM ---
void saveProgress(int levelToUnlock) {
	if (levelToUnlock > unlockedLevel) unlockedLevel = levelToUnlock;
	FILE* fp = fopen("savegame.txt", "w");
	if (fp) {
		fprintf(fp, "%d", unlockedLevel);
		fclose(fp);
	}
}

void loadProgress() {
	FILE* fp = fopen("savegame.txt", "r");
	if (fp) {
		fscanf(fp, "%d", &unlockedLevel);
		fclose(fp);
	}
}

// --- THE STATE MACHINE ---
void iDraw() {
	iClear();
	if (currentState == MENU) {
		iSetColor(0, 0, 0); iFilledRectangle(0, 0, 1280, 720);

		iSetColor(0, 255, 255);
		iText(420, 500, "RUNTIME ERROR", GLUT_BITMAP_TIMES_ROMAN_24);

		iSetColor(255, 255, 255);
		iText(430, 400, "PRESS [ENTER] TO RESTART FROM LEVEL 1", GLUT_BITMAP_HELVETICA_18);

		iSetColor(200, 200, 200);
		iText(430, 350, "PRESS [L] TO CONTINUE HIGHEST LEVEL", GLUT_BITMAP_HELVETICA_18);

		// --- LEVEL SELECT UI ---
		iSetColor(0, 255, 100);
		iText(530, 280, "--- LEVEL SELECT ---", GLUT_BITMAP_HELVETICA_18);

		char selectText[100];
		sprintf(selectText, "PRESS [1] TO [%d] TO JUMP DIRECTLY TO A LEVEL", unlockedLevel);
		iSetColor(150, 255, 150);
		iText(410, 240, selectText, GLUT_BITMAP_HELVETICA_18);

		char levelTxt[50];
		sprintf(levelTxt, "Highest Level Unlocked: %d/6", unlockedLevel);
		iSetColor(255, 150, 50);
		iText(530, 150, levelTxt, GLUT_BITMAP_HELVETICA_12);
	}
	else if (currentState == LEVEL_1) drawLevel1();
	else if (currentState == LEVEL_2) drawLevel2();
	else if (currentState == LEVEL_3) drawLevel3();
	else if (currentState == LEVEL_4) drawLevel4();
	else if (currentState == LEVEL_5) drawLevel5();
	else if (currentState == LEVEL_6) drawLevel6();
}

void fixedUpdate() {
	if (menuCooldown > 0) menuCooldown--;

	if (currentState == MENU) {
		// New Game (Restart from Level 1)
		if ((GetAsyncKeyState(VK_RETURN) & 0x8000) && menuCooldown == 0) {
			initLevel1(); currentState = LEVEL_1; menuCooldown = 20;
		}
		// Load Highest Level
		else if ((GetAsyncKeyState('L') & 0x8000) && menuCooldown == 0) {
			loadProgress(); menuCooldown = 20;
			if (unlockedLevel == 2) { initLevel2(); currentState = LEVEL_2; }
			else if (unlockedLevel == 3) { initLevel3(); currentState = LEVEL_3; }
			else if (unlockedLevel == 4) { initLevel4(); currentState = LEVEL_4; }
			else if (unlockedLevel == 5) { initLevel5(); currentState = LEVEL_5; }
			else if (unlockedLevel == 6) { initLevel6(); currentState = LEVEL_6; }
			else { initLevel1(); currentState = LEVEL_1; }
		}
		// --- LEVEL SELECT CONTROLS ---
		else if ((GetAsyncKeyState('1') & 0x8000) && menuCooldown == 0) {
			initLevel1(); currentState = LEVEL_1; menuCooldown = 20;
		}
		else if ((GetAsyncKeyState('2') & 0x8000) && menuCooldown == 0 && unlockedLevel >= 2) {
			initLevel2(); currentState = LEVEL_2; menuCooldown = 20;
		}
		else if ((GetAsyncKeyState('3') & 0x8000) && menuCooldown == 0 && unlockedLevel >= 3) {
			initLevel3(); currentState = LEVEL_3; menuCooldown = 20;
		}
		else if ((GetAsyncKeyState('4') & 0x8000) && menuCooldown == 0 && unlockedLevel >= 4) {
			initLevel4(); currentState = LEVEL_4; menuCooldown = 20;
		}
		else if ((GetAsyncKeyState('5') & 0x8000) && menuCooldown == 0 && unlockedLevel >= 5) {
			initLevel5(); currentState = LEVEL_5; menuCooldown = 20;
		}
		else if ((GetAsyncKeyState('6') & 0x8000) && menuCooldown == 0 && unlockedLevel >= 6) {
			initLevel6(); currentState = LEVEL_6; menuCooldown = 20;
		}
	}
	else if (currentState == LEVEL_1) updateLevel1();
	else if (currentState == LEVEL_2) updateLevel2();
	else if (currentState == LEVEL_3) updateLevel3();
	else if (currentState == LEVEL_4) updateLevel4();
	else if (currentState == LEVEL_5) updateLevel5();
	else if (currentState == LEVEL_6) updateLevel6();
}

void iKeyboard(unsigned char key) {
		// --- GLOBAL SOUND EFFECT ---
		// Plays on any standard key press EXCEPT during Levels 5 and 6
		if (currentState != LEVEL_5 && currentState != LEVEL_6) {
			// SND_ASYNC ensures the game doesn't freeze while the sound plays
			PlaySound(TEXT("click.wav"), NULL, SND_ASYNC);
		}

		// --- LEVEL ROUTING ---
		if (currentState == LEVEL_1) keyboardLevel1(key);
		else if (currentState == LEVEL_2) keyboardLevel2(key);
		else if (currentState == LEVEL_3) keyboardLevel3(key);
		else if (currentState == LEVEL_4) keyboardLevel4(key);
		else if (currentState == LEVEL_5) keyboardLevel5(key);
		else if (currentState == LEVEL_6) keyboardLevel6(key);
	}

void iSpecialKeyboard(unsigned char key) {}
void iMouse(int button, int state, int mx, int my) {
	if (currentState == LEVEL_1) mouseLevel1(button, state, mx, my);
	else if (currentState == LEVEL_4) mouseLevel4(button, state, mx, my);
}
void iMouseMove(int mx, int my) {}
void iPassiveMouseMove(int mx, int my) {
	if (currentState == LEVEL_1) passiveMouseLevel1(mx, my);
}

int main() {
	iInitialize(1280, 720, "AUST CSE: The Grand Archive");
	loadProgress();

	// Ensure we boot to the menu so the player can see the Level Select!
	currentState = MENU;

	iSetTimer(16, fixedUpdate);
	iStart();
	return 0;
}