#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include "Globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace L2 {
	// =========================================================
	// GLOBAL CONFIGURATION
	// =========================================================
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define ROOM_WIDTH 4000
#define GROUND_Y 100
#define CEILING_Y 620

	// --- LOCAL STATE MACHINE (Renamed to avoid Globals.h conflict) ---
	enum L2State {
		VN_PROLOGUE,
		STEALTH_PLAYING, STEALTH_GAMEOVER, STEALTH_VICTORY,
		VN_INTERLUDE,
		BOSS_INTRO, BOSS_PHASE1, BOSS_TRANSITION, BOSS_PHASE2, BOSS_PLAYER_DEAD, BOSS_DEAD
	};
	L2State l2State = VN_PROLOGUE;

	// --- GLOBAL INPUT FLAGS ---
	bool spacePressed = false;
	bool enterPressed = false;
	bool clickPressed = false;
	bool jPressed = false, kPressed = false, wPressed = false;

	// --- SHARED TIMERS ---
	int globalStateTimer = 0;

	// =========================================================
	// PHASE 1 & 3: VISUAL NOVEL VARIABLES
	// =========================================================
	int prologueStep = 0;
	int interludeStep = 0;
	int shakeX = 0, shakeY = 0;

	int vn_bg_lab, img_bg_mirror;
	int ori_img, afif_img, sam_img, vn_guard_img;

	int charIndex = 0;
	int textTimer = 0;
	const int TEXT_SPEED = 2;

	// =========================================================
	// PHASE 2: STEALTH VARIABLES
	// =========================================================
	int img_bg_stealth, img_player_stealth, img_guard_walk, img_guard_look;
	int img_prop, img_alert, img_mirror;

	int cameraX = 0;

#define NUM_PROPS 3
	int propX[NUM_PROPS] = { 800, 2000, 3100 };
	int propW = 400, propH = 400;

	float s_pX = 100, s_pY = GROUND_Y;
	int s_pW = 300, s_pH = 300;
	bool isHiding = false;
	int suspicionMeter = 0;

	float gX = 700, gY = GROUND_Y;
	int gW = 350, gH = 350;
	int guardState = 0;
	int guardTimer = 0;

	// =========================================================
	// PHASE 4: BOSS FIGHT VARIABLES
	// =========================================================
	int img_bg_boss, img_player_fight, img_doppelganger, img_shield, img_slash, img_projectile;

	int bossPhase = 1;

	int b_pW = 100, b_pH = 140;
	float b_pX = 200, b_pY = GROUND_Y;
	float b_pVy = 0;
	bool isJumping = false;
	int playerHP = 100, playerStamina = 100;

	bool isShielding = false; int shieldTimer = 0;
	bool isAttacking = false; int attackTimer = 0; bool hasDealtDamage = false;
	bool pProjActive = false; float pProjX = 0, pProjY = 0;

	int bossW = 100, bossH = 140;
	float bossX = SCREEN_WIDTH - 250, bossY = GROUND_Y;
	int bossHP = 100;
	int bossShootTimer = 0;
	bool bProjActive = false; float bProjX = 0, bProjY = 0;

	int parryFlashTimer = 0;

	// =========================================================
	// RENDERING HELPERS
	// =========================================================
	void drawPNG(float x, float y, int w, int h, int imgID, float brightness = 1.0f) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(brightness, brightness, brightness, 1.0f);
		iShowImage((int)x, (int)y, w, h, imgID);
		glDisable(GL_BLEND);
	}

	void drawTransparentBox(int x, int y, int w, int h, int alpha) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4ub(0, 0, 0, alpha);
		glBegin(GL_QUADS);
		glVertex2f(x, y); glVertex2f(x + w, y);
		glVertex2f(x + w, y + h); glVertex2f(x, y + h);
		glEnd();
		glDisable(GL_BLEND);
	}

	void drawVNUI(const char* name, const char* l1, const char* l2, bool isCorrupted) {
		drawTransparentBox(50, 30, 1180, 150, 220);
		if (isCorrupted) iSetColor(180, 0, 255);
		else iSetColor(211, 47, 47);

		iFilledRectangle(50, 185, 200, 40);
		iSetColor(255, 255, 255);
		iText(70, 195, (char*)name, GLUT_BITMAP_HELVETICA_18);

		char displayL1[100] = ""; char displayL2[100] = "";
		int len1 = strlen(l1); int len2 = strlen(l2);

		if (charIndex <= len1) {
			strncpy(displayL1, l1, charIndex); displayL1[charIndex] = '\0';
		}
		else {
			strcpy(displayL1, l1);
			int overflow = charIndex - len1;
			if (overflow > len2) overflow = len2;
			strncpy(displayL2, l2, overflow); displayL2[overflow] = '\0';
		}

		iText(80, 110, displayL1, GLUT_BITMAP_9_BY_15);
		if (displayL2[0] != '\0') iText(80, 80, displayL2, GLUT_BITMAP_9_BY_15);

		if (charIndex >= len1 + len2) {
			iSetColor(150, 150, 150);
			iText(1050, 50, "[ CLICK or SPACE ]", GLUT_BITMAP_HELVETICA_12);
		}
	}

	// =========================================================
	// SCENE RENDERERS
	// =========================================================
	void drawPrologue() {
		iShowImage(shakeX, shakeY, SCREEN_WIDTH, SCREEN_HEIGHT, vn_bg_lab);

		int charW = 400; int charH = 550;
		int activeChar = -1;
		char name[20] = ""; char line1[100] = ""; char line2[100] = "";

		if (prologueStep == 0) { activeChar = 1; strcpy(name, "Afif"); strcpy(line1, "Guys, I think I'm going to have a heart attack."); strcpy(line2, "I left the ESP32 and the final circuit in the lab."); }
		else if (prologueStep == 1) { activeChar = 2; strcpy(name, "Samiha"); strcpy(line1, "Afif, we spent 14 hours on that wiring..."); strcpy(line2, "If that lab assistant finds it, we're failing the semester."); }
		else if (prologueStep == 2) { activeChar = 0; strcpy(name, "Oritri"); strcpy(line1, "Relax. The campus is empty at midnight."); strcpy(line2, "We'll just sneak in, grab the board, and get out."); }
		else if (prologueStep == 3) { activeChar = 1; strcpy(name, "Afif"); strcpy(line1, "Midnight? Oritri, AUST at night feels like"); strcpy(line2, "a Resident Evil map. I don't like this."); }
		else if (prologueStep == 4) { activeChar = 2; strcpy(name, "Samiha"); strcpy(line1, "Stop being a baby. Look, the main corridor is clear."); strcpy(line2, "Wait... who is that by the East Wing door?"); }
		else if (prologueStep == 5) { shakeX = (rand() % 10) - 5; shakeY = (rand() % 10) - 5; activeChar = 3; strcpy(name, "???"); strcpy(line1, "...The Night Guard? Why is he moving like that?"); strcpy(line2, "He's looking around like he's hiding something."); }
		else if (prologueStep == 6) { shakeX = 0; shakeY = 0; activeChar = 0; strcpy(name, "Oritri"); strcpy(line1, "That's not a normal patrol route."); strcpy(line2, "He's headed toward the server room. Let's stay quiet."); }
		else if (prologueStep == 7) { activeChar = 1; strcpy(name, "Afif"); strcpy(line1, "My panic levels are at 100%. If we get caught,"); strcpy(line2, "expulsion is the best-case scenario."); }
		else if (prologueStep == 8) { activeChar = 2; strcpy(name, "Samiha"); strcpy(line1, "Shh! He's turning around. Hide behind that pillar!"); strcpy(line2, "Level 2: Stealth Phase Commencing..."); }

		if (activeChar != 3) {
			if (activeChar == 0 || prologueStep < 8) drawPNG(100, 120, charW, charH, ori_img, (activeChar == 0) ? 1.0f : 0.4f);
			if (activeChar == 1 || prologueStep < 8) drawPNG(440, 120, charW, charH, afif_img, (activeChar == 1) ? 1.0f : 0.4f);
			if (activeChar == 2 || prologueStep < 8) drawPNG(780, 120, charW, charH, sam_img, (activeChar == 2) ? 1.0f : 0.4f);
		}
		else {
			drawPNG(515, 150, 250, 450, vn_guard_img, 1.0f);
		}

		drawVNUI(name, line1, line2, false);
	}

	void drawStealth() {
		iShowImage(-cameraX, 0, ROOM_WIDTH, SCREEN_HEIGHT, img_bg_stealth);
		drawPNG(ROOM_WIDTH - 400 - cameraX, GROUND_Y, 400, 400, img_mirror);

		if (l2State == STEALTH_PLAYING || l2State == STEALTH_VICTORY) {
			if (isHiding) drawPNG(s_pX - cameraX, s_pY, s_pW, s_pH, img_player_stealth, 0.5f);
			else drawPNG(s_pX - cameraX, s_pY, s_pW, s_pH, img_player_stealth, 1.0f);

			for (int i = 0; i < NUM_PROPS; i++) {
				drawPNG(propX[i] - cameraX, GROUND_Y, propW, propH, img_prop);
			}

			if (guardState == 2) drawPNG(gX - cameraX, gY, gW, gH, img_guard_look);
			else drawPNG(gX - cameraX, gY, gW, gH, img_guard_walk);

			if (guardState == 1) drawPNG(gX - cameraX + (gW / 2) - 25, gY + gH + 20, 50, 80, img_alert);

			iSetColor(255, 255, 255); iText(50, 650, "SUSPICION METER:", GLUT_BITMAP_HELVETICA_18);
			iSetColor(50, 50, 50); iFilledRectangle(230, 645, 200, 20);
			iSetColor(255, 0, 0); iFilledRectangle(230, 645, suspicionMeter * 2, 20);
			iSetColor(200, 200, 200); iText(50, 620, "Hold 'S' or DOWN ARROW behind objects to hide!", GLUT_BITMAP_HELVETICA_12);
		}

		if (l2State == STEALTH_GAMEOVER) {
			iSetColor(255, 0, 0); iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, "CAUGHT!", GLUT_BITMAP_TIMES_ROMAN_24);
			iSetColor(255, 255, 255); iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 30, "Press SPACE to Restart Stealth.", GLUT_BITMAP_HELVETICA_18);
		}
		else if (l2State == STEALTH_VICTORY) {
			iSetColor(0, 255, 255); iText(SCREEN_WIDTH / 2 - 350, SCREEN_HEIGHT / 2, "GUARD ENTERED THE MIRROR. PRESS ENTER TO FOLLOW.", GLUT_BITMAP_TIMES_ROMAN_24);
		}
	}

	void drawInterlude() {
		iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_bg_mirror);

		int charW = 400; int charH = 550;
		int activeChar = -1;
		char name[20] = ""; char line1[100] = ""; char line2[100] = "";

		if (interludeStep == 0) { activeChar = 1; strcpy(name, "Afif"); strcpy(line1, "Did we just walk into a router? My head is spinning."); strcpy(line2, "Why does the ceiling look like the floor?"); }
		else if (interludeStep == 1) { activeChar = 2; strcpy(name, "Samiha"); strcpy(line1, "It's a localized logic inversion. The guard didn't just steal"); strcpy(line2, "the ESP32... he plugged it into the mainframe backward."); }
		else if (interludeStep == 2) { activeChar = 0; strcpy(name, "Oritri"); strcpy(line1, "Look at our hands. Try to step forward."); strcpy(line2, "Everything is flipped. Left is right."); }
		else if (interludeStep == 3) { activeChar = 1; strcpy(name, "Afif"); strcpy(line1, "Okay, great. The physics engine is broken."); strcpy(line2, "Wait, where did the guard go? And who is THAT?"); }
		else if (interludeStep == 4) { activeChar = 2; strcpy(name, "Samiha"); strcpy(line1, "It's a Doppelg�nger. A physical manifestation of a"); strcpy(line2, "runtime error. It's mirroring our exact coordinates."); }
		else if (interludeStep == 5) { activeChar = 0; strcpy(name, "Oritri"); strcpy(line1, "It has the circuit board. If we want it back,"); strcpy(line2, "we have to delete this thing from the system."); }
		else if (interludeStep == 6) { activeChar = 1; strcpy(name, "Afif"); strcpy(line1, "Remember: left is right, right is left."); strcpy(line2, "If it shoots, use the Mirror Shield to reflect it!"); }

		if (activeChar == 0 || interludeStep < 7) drawPNG(100, 120, charW, charH, ori_img, (activeChar == 0) ? 1.0f : 0.4f);
		if (activeChar == 1 || interludeStep < 7) drawPNG(440, 120, charW, charH, afif_img, (activeChar == 1) ? 1.0f : 0.4f);
		if (activeChar == 2 || interludeStep < 7) drawPNG(780, 120, charW, charH, sam_img, (activeChar == 2) ? 1.0f : 0.4f);

		drawVNUI(name, line1, line2, true);
	}

	void drawBossFight() {
		if (parryFlashTimer > 0) {
			iSetColor(200, 255, 200); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			parryFlashTimer--;
		}

		iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_bg_boss);

		if (l2State != BOSS_DEAD || globalStateTimer < 60) drawPNG(bossX, bossY, bossW, bossH, img_doppelganger);
		if (l2State != BOSS_PLAYER_DEAD || globalStateTimer < 60) drawPNG(b_pX, b_pY, b_pW, b_pH, img_player_fight);

		if (isShielding) {
			drawPNG(b_pX + b_pW / 2 - 100, b_pY + b_pH / 2 - 100, 200, 200, img_shield);
			if (shieldTimer < 15) { iSetColor(0, 255, 0); iCircle(b_pX + b_pW / 2, b_pY + b_pH / 2, 95); }
		}

		if (isAttacking) drawPNG(b_pX + b_pW - 20, b_pY + b_pH / 2 - 40, 120, 80, img_slash);

		if (pProjActive) drawPNG(pProjX - 30, pProjY - 30, 60, 60, img_projectile);
		if (bProjActive) drawPNG(bProjX - 30, bProjY - 30, 60, 60, img_projectile);

		// Gothic UI
		iSetColor(180, 0, 255); iText(50, 680, ">>_P1_STATUS.hp", GLUT_BITMAP_9_BY_15);
		iSetColor(50, 50, 50); iFilledRectangle(190, 680, 200, 15);
		iSetColor(0, 255, 255); iFilledRectangle(190, 680, playerHP * 2, 15);

		iSetColor(180, 0, 255); iText(50, 650, ">>_PWR.stamina", GLUT_BITMAP_9_BY_15);
		iSetColor(100, 100, 100); iFilledRectangle(190, 650, 200, 10);
		iSetColor(255, 100, 255);   iFilledRectangle(190, 650, playerStamina * 2, 10);

		iSetColor(255, 0, 100); iText(1000, 680, "REF_ERR.hp", GLUT_BITMAP_9_BY_15);
		iSetColor(50, 50, 50); iFilledRectangle(1180 - 200, 680, 200, 15);
		if (bossHP > 0) {
			iSetColor(255, 0, 0);
			float hpScale = (bossPhase == 1) ? 2.0f : (200.0f / 150.0f);
			iFilledRectangle(1180 - (bossHP * hpScale), 680, bossHP * hpScale, 15);
		}

		iSetColor(180, 0, 255);
		iText(SCREEN_WIDTH / 2 - 250, 680, "[A/D]: Flipped Move | [SPACE]: Jump | [J]: Slash | [K]: Shoot | [L]: Shield", GLUT_BITMAP_9_BY_15);

		// Overlays
		if (l2State == BOSS_INTRO) {
			glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4ub(0, 0, 0, 150); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); glDisable(GL_BLEND);
			iSetColor(255, 0, 100);
			if (globalStateTimer < 120) iText(SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT / 2, "RUN( ) ERROR: LOGIC INVERTED", GLUT_BITMAP_9_BY_15);
			else { iSetColor(255, 255, 255); iText(SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2, "EXECUTE.", GLUT_BITMAP_TIMES_ROMAN_24); }
		}
		else if (l2State == BOSS_TRANSITION) {
			glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4ub(100, 0, 150, 100); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); glDisable(GL_BLEND);
			iSetColor(0, 255, 255); iText(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2, "GRAVITY::INVERT( ); //SHIFTING PHYSICAL LAYER", GLUT_BITMAP_9_BY_15);
		}
		else if (l2State == BOSS_DEAD) {
			iSetColor(0, 255, 255); iText(SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT / 2 + 50, "REF_ERR(DELETED) == TRUE.", GLUT_BITMAP_9_BY_15);
			iSetColor(255, 255, 255); iText(SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT / 2, "ESP32 RETRIEVED. PRESS SPACE TO ADVANCE.", GLUT_BITMAP_TIMES_ROMAN_24);
		}
		else if (l2State == BOSS_PLAYER_DEAD) {
			iSetColor(255, 0, 0); iText(SCREEN_WIDTH / 2 - 220, SCREEN_HEIGHT / 2 + 50, "SYSTEM CORRUPTED. YOU ARE THE REFLECTION.", GLUT_BITMAP_TIMES_ROMAN_24);
			iSetColor(255, 255, 255); iText(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, "Press SPACE to Reload( );", GLUT_BITMAP_9_BY_15);
		}
	}

	void draw() {
		if (l2State == VN_PROLOGUE) drawPrologue();
		else if (l2State == STEALTH_PLAYING || l2State == STEALTH_GAMEOVER || l2State == STEALTH_VICTORY) drawStealth();
		else if (l2State == VN_INTERLUDE) drawInterlude();
		else drawBossFight();
	}

	// =========================================================
	// LOGIC CONTROLLERS
	// =========================================================
	void advancePrologue() {
		if (charIndex < 200) charIndex = 200;
		else {
			prologueStep++; charIndex = 0;
			if (prologueStep > 8) {
				s_pX = 100; gX = 700; guardState = 0; guardTimer = 0; suspicionMeter = 0;
				l2State = STEALTH_PLAYING;
			}
		}
	}

	void advanceInterlude() {
		if (charIndex < 200) charIndex = 200;
		else {
			interludeStep++; charIndex = 0;
			if (interludeStep > 6) {
				globalStateTimer = 0;
				l2State = BOSS_INTRO;
			}
		}
	}

	void update() {
		// --- GLOBAL INPUTS ---
		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			if (!spacePressed) {
				spacePressed = true;
				if (l2State == VN_PROLOGUE) advancePrologue();
				else if (l2State == STEALTH_GAMEOVER) {
					s_pX = 100; gX = 700; guardState = 0; guardTimer = 0; suspicionMeter = 0;
					l2State = STEALTH_PLAYING;
				}
				else if (l2State == VN_INTERLUDE) advanceInterlude();
				else if (l2State == BOSS_DEAD) {
					// MAGIC BULLET: Win Level 2 -> Save Game -> Go to Boss Level!
					::unlockedLevel = 3;
					saveProgress(3);
					::initLevel3();
					::currentState = LEVEL_3;
				}
				else if (l2State == BOSS_PLAYER_DEAD) {
					playerHP = 100; bossHP = 100; playerStamina = 100; bossPhase = 1;
					b_pY = GROUND_Y; bossY = GROUND_Y; b_pX = 200; pProjActive = false; bProjActive = false;
					l2State = BOSS_INTRO; globalStateTimer = 0;
				}
				else if (!isAttacking && !isShielding && !isJumping && (l2State == BOSS_PHASE1 || l2State == BOSS_PHASE2)) {
					isJumping = true;
					if (bossPhase == 1) b_pVy = 16.0f; else b_pVy = -16.0f;
				}
			}
		}
		else spacePressed = false;

		if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
			if (!enterPressed) {
				enterPressed = true;
				if (l2State == STEALTH_VICTORY) {
					charIndex = 0; textTimer = 0;
					l2State = VN_INTERLUDE;
				}
			}
		}
		else enterPressed = false;

		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
			if (!clickPressed) {
				clickPressed = true;
				if (l2State == VN_PROLOGUE) advancePrologue();
				else if (l2State == VN_INTERLUDE) advanceInterlude();
			}
		}
		else clickPressed = false;

		// --- STATE SPECIFIC UPDATES ---
		if (l2State == VN_PROLOGUE || l2State == VN_INTERLUDE) {
			textTimer++;
			if (textTimer >= TEXT_SPEED) { charIndex++; textTimer = 0; }
		}
		else if (l2State == STEALTH_PLAYING) {
			guardTimer++;
			if (guardState == 0) { gX += 2.0f; if (guardTimer > 180) { guardState = 1; guardTimer = 0; } }
			else if (guardState == 1) { if (guardTimer > 60) { guardState = 2; guardTimer = 0; } }
			else if (guardState == 2) { if (guardTimer > 120) { guardState = 0; guardTimer = 0; } }

			isHiding = false;
			if (GetAsyncKeyState('D') || GetAsyncKeyState(VK_RIGHT)) s_pX += 5.0f;
			if (GetAsyncKeyState('A') || GetAsyncKeyState(VK_LEFT))  s_pX -= 5.0f;
			if (s_pX < 0) s_pX = 0; if (s_pX > gX - 100) s_pX = gX - 100;

			for (int i = 0; i < NUM_PROPS; i++) {
				if (s_pX + (s_pW / 2) > propX[i] && s_pX + (s_pW / 2) < propX[i] + propW) {
					if (GetAsyncKeyState('S') || GetAsyncKeyState(VK_DOWN)) isHiding = true;
				}
			}

			if (guardState == 2 && !isHiding) { suspicionMeter += 2; if (suspicionMeter > 100) l2State = STEALTH_GAMEOVER; }
			else { if (suspicionMeter > 0) suspicionMeter--; }

			if (gX >= ROOM_WIDTH - 400) l2State = STEALTH_VICTORY;

			cameraX = s_pX - (SCREEN_WIDTH / 3);
			if (cameraX < 0) cameraX = 0; if (cameraX > ROOM_WIDTH - SCREEN_WIDTH) cameraX = ROOM_WIDTH - SCREEN_WIDTH;
		}
		else if (l2State == BOSS_INTRO) {
			globalStateTimer++; if (globalStateTimer > 180) { l2State = BOSS_PHASE1; globalStateTimer = 0; }
		}
		else if (l2State == BOSS_TRANSITION) {
			globalStateTimer++;
			if (b_pY < CEILING_Y - b_pH) b_pY += 5.0f; else b_pY = CEILING_Y - b_pH;
			if (bossY < CEILING_Y - bossH) bossY += 5.0f; else bossY = CEILING_Y - bossH;
			if (globalStateTimer > 180) { l2State = BOSS_PHASE2; globalStateTimer = 0; }
		}
		else if (l2State == BOSS_PHASE1 || l2State == BOSS_PHASE2) {
			if (!isAttacking && !isShielding) {
				if (GetAsyncKeyState('A') || GetAsyncKeyState(VK_LEFT))  b_pX += 6.0f;
				if (GetAsyncKeyState('D') || GetAsyncKeyState(VK_RIGHT)) b_pX -= 6.0f;
			}

			if (isJumping) {
				b_pY += b_pVy;
				if (bossPhase == 1) {
					b_pVy -= 1.0f;
					if (b_pY <= GROUND_Y) { b_pY = GROUND_Y; isJumping = false; b_pVy = 0; }
				}
				else {
					b_pVy += 1.0f;
					if (b_pY >= CEILING_Y - b_pH) { b_pY = CEILING_Y - b_pH; isJumping = false; b_pVy = 0; }
				}
			}

			if (b_pX < 0) b_pX = 0; if (b_pX > SCREEN_WIDTH / 2 - b_pW) b_pX = SCREEN_WIDTH / 2 - b_pW;

			bossX = SCREEN_WIDTH - b_pX - bossW;
			bossY = b_pY;

			bossShootTimer++;
			int shootCooldown = (bossPhase == 1) ? 100 : 70;
			if (bossShootTimer > shootCooldown && !bProjActive) {
				bProjActive = true; bProjX = bossX; bProjY = bossY + bossH / 2; bossShootTimer = 0;
			}

			if (GetAsyncKeyState('L') & 0x8000) {
				if (playerStamina > 0 && !isAttacking && !isJumping) {
					isShielding = true; shieldTimer++;
					if (shieldTimer % 3 == 0) playerStamina--;
				}
				else isShielding = false;
			}
			else { isShielding = false; shieldTimer = 0; if (playerStamina < 100) playerStamina++; }

			if (GetAsyncKeyState('J') & 0x8000) {
				if (!jPressed && !isShielding) { isAttacking = true; attackTimer = 15; hasDealtDamage = false; jPressed = true; }
			}
			else jPressed = false;

			if (isAttacking) {
				attackTimer--;
				float sLeft = b_pX + b_pW; float sRight = b_pX + b_pW + 80;
				float sTop = b_pY + b_pH / 2 + 30; float sBot = b_pY + b_pH / 2 - 30;

				if (bProjActive && bProjX > sLeft && bProjX < sRight && bProjY > sBot && bProjY < sTop) bProjActive = false;
				if (!hasDealtDamage && bossX < sRight && bossX + bossW > sLeft && abs(b_pY - bossY) < 50) { bossHP -= 10; hasDealtDamage = true; }
				if (attackTimer <= 0) isAttacking = false;
			}

			if (GetAsyncKeyState('K') & 0x8000) {
				if (!kPressed && !pProjActive && !isShielding && !isAttacking) { pProjActive = true; pProjX = b_pX + b_pW; pProjY = b_pY + b_pH / 2; kPressed = true; }
			}
			else kPressed = false;

			if (pProjActive) {
				pProjX += 14.0f;
				if (pProjX > SCREEN_WIDTH) pProjActive = false;
				if (pProjX + 30 > bossX && pProjX - 30 < bossX + bossW && pProjY + 30 > bossY && pProjY - 30 < bossY + bossH) { bossHP -= 8; pProjActive = false; }
			}

			if (bProjActive) {
				bProjX -= 10.0f;
				if (bProjX < 0) bProjActive = false;
				if (bProjX + 30 > b_pX && bProjX - 30 < b_pX + b_pW && bProjY + 30 > b_pY && bProjY - 30 < b_pY + b_pH) {
					if (isShielding) {
						if (shieldTimer <= 15) { parryFlashTimer = 5; bProjActive = false; pProjActive = true; pProjX = b_pX + b_pW; pProjY = bProjY; }
						else { bProjActive = false; playerStamina -= 30; if (playerStamina < 0) playerStamina = 0; }
					}
					else { bProjActive = false; playerHP -= 15; }
				}
			}

			if (playerHP <= 0) { playerHP = 0; l2State = BOSS_PLAYER_DEAD; globalStateTimer = 0; }
			if (bossHP <= 0) {
				bossHP = 0;
				if (bossPhase == 1) { bossPhase = 2; bossHP = 150; bProjActive = false; pProjActive = false; l2State = BOSS_TRANSITION; globalStateTimer = 0; }
				else { l2State = BOSS_DEAD; globalStateTimer = 0; }
			}
		}
	}

	void keyboard(unsigned char key) {} // Most input handled in update via GetAsyncKeyState

	void init() {
		vn_bg_lab = iLoadImage("lab_night_bg.png");
		ori_img = iLoadImage("talkingori.png");
		afif_img = iLoadImage("talkingafif.png");
		sam_img = iLoadImage("talkingsam.png");
		vn_guard_img = iLoadImage("guard_shadow.png");

		img_bg_stealth = iLoadImage("corridor_long.png");
		img_player_stealth = iLoadImage("player_stealth.png");
		img_guard_walk = iLoadImage("guard_walk.png");
		img_guard_look = iLoadImage("guard_look.png");
		img_prop = iLoadImage("hiding_prop.png");
		img_alert = iLoadImage("alert_icon.png");
		img_mirror = iLoadImage("mirror_portal.png");

		img_bg_mirror = iLoadImage("mirror_world_bossfight_bg.png");
		img_bg_boss = iLoadImage("mirror_world_bossfight_bg.png");
		img_player_fight = iLoadImage("ori_fight.png");
		img_doppelganger = iLoadImage("doppleganger_fight.png");
		img_shield = iLoadImage("shield_fx.png");
		img_slash = iLoadImage("slash_fx.png");
		img_projectile = iLoadImage("projectile_fx.png");
	}
}

// =========================================================
// INTERFACE: Connects to your Main Director File
// =========================================================
void initLevel2() { L2::init(); }
void drawLevel2() { L2::draw(); }
void updateLevel2() { L2::update(); }
void keyboardLevel2(unsigned char key) { L2::keyboard(key); }