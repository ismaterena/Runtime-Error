#define _CRT_SECURE_NO_WARNINGS
#include "iGraphics.h"
#include "Globals.h"
#include "Level6.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <windows.h>

namespace L6 {
#define SCREEN_W 1280
#define SCREEN_H 720
#define PI 3.14159265

	int img_bg = -1, img_boss = -1, img_boss_stunned = -1;
	int img_p_afif = -1, img_p_ori = -1, img_p_sam = -1;
	int bossImgW = 371, bossImgH = 200;

	enum GameFlowState { DIALOGUE, TRANSITION, BATTLE, GAMEOVER, VICTORY };
	GameFlowState currentFlow = DIALOGUE;
	int transitionTimer = 0;
	int currentDialogue = 0;

	struct Dialogue { char speaker[20]; char line1[100]; char line2[100]; int colorR, colorG, colorB; };
	Dialogue script[11] = {
		{ "SAMIHA", "You made it further than I calculated, Afif.", "And Oritri... you brought yourselves right to the altar.", 255, 50, 50 },
		{ "AFIF", "Samiha, stop! Look around you... the blood, the chains.", "The missing students from 2008... it was you?", 50, 150, 255 },
		{ "SAMIHA", "Not just me. I merely finished the ritual they failed to complete.", "This university is built on a nexus of decay.", 255, 50, 50 },
		{ "ORITRI", "A ritual?! You're insane! You massacred innocent people!", "You manipulated us from the very beginning!", 50, 255, 100 },
		{ "SAMIHA", "Manipulation is such a crude word, Oritri. I *guided* you.", "You two were always the strongest among our peers.", 255, 50, 50 },
		{ "SAMIHA", "Mortal bodies are fragile. They rot. They fade.", "But with the souls of the class of 2008, and your two lives...", 255, 50, 50 },
		{ "SAMIHA", "...I will break the cycle of death forever.", "", 255, 50, 50 },
		{ "AFIF", "We trusted you. We were a team!", "I won't let you hurt her, Samiha. Step away from the ritual circle!", 50, 150, 255 },
		{ "SAMIHA", "Trust is a mortal flaw. And your shield, Afif?", "It cannot block death itself. I have conquered the grave.", 255, 50, 50 },
		{ "ORITRI", "Then we'll just have to put you back in it.", "Afif, get ready!", 50, 255, 100 },
		{ "SAMIHA", "Come then.", "Let your bones join the foundation of my new world.", 255, 50, 50 }
	};

	float pX[2] = { 100, 250 }; float pY = 100; int pW = 40;
	float pHealth[2] = { 150.0f, 80.0f }; float pMaxHealth[2] = { 150.0f, 80.0f };
	bool isShieldActive = false; int shieldTimer = 0; int afifCD[3] = { 0, 0, 0 };
	bool isTimeStopped = false; int timeStopTimer = 0;
	int oritriCD[3] = { 0, 0, 0 };

	struct PlayerAttack { float x, y, vx, vy; int type; bool active; };
	PlayerAttack pAtks[10];

	bool isThunderActive = false; int thunderTimer = 0;

	enum BossState { HOVER, ATK_SKULLS, ATK_CHAINS, ATK_LASERS, ATK_DROPS, STUNNED };
	BossState bossState = HOVER;
	float bossX = 1000, bossY = 400, bossTargetX = 1000, bossTargetY = 400;
	float bossHealth = 100.0f;

	float trailX[10] = { -100, -100, -100, -100, -100, -100, -100, -100, -100, -100 };
	float trailY[10] = { -100, -100, -100, -100, -100, -100, -100, -100, -100, -100 };
	int stateTimer = 0, stunTimer = 0, shakeTimer = 0, spaceCooldown = 0;

	struct Skull { float x, y, vx, vy; bool active; bool orbiting; float angle; };
	Skull skulls[5]; int skullFireIdx = 0;

	struct Hazard { float x, y; bool active; int warnTimer; int attackTimer; };
	Hazard hazards[3];

	struct Drop { float x, y, vy; bool active; };
	Drop drops[15];

	void draw() {
		int dx = (shakeTimer > 0) ? (rand() % 16) - 8 : 0;
		int dy = (shakeTimer > 0) ? (rand() % 16) - 8 : 0;
		if (shakeTimer > 0) shakeTimer--;

		if (isTimeStopped) {
			iSetColor(0, 0, 0); glEnable(GL_BLEND); glColor4ub(0, 0, 150, 80);
			iFilledRectangle(0, 0, SCREEN_W, SCREEN_H); glDisable(GL_BLEND);
		}

		if (img_bg != -1) { iSetColor(255, 255, 255); iShowImage(dx, dy, SCREEN_W, SCREEN_H, img_bg); }
		else { iSetColor(15, 5, 5); iFilledRectangle(0, 0, SCREEN_W, SCREEN_H); }

		if (currentFlow != DIALOGUE) {
			iSetColor(150, 0, 0); iFilledRectangle(340, 680, bossHealth * 6, 20);
			iSetColor(255, 255, 255); iRectangle(340, 680, 600, 20);
			iText(340, 705, "SAMIHA - THE NECROMANCER", GLUT_BITMAP_HELVETICA_12);

			iSetColor(0, 100, 255); iFilledRectangle(20, 20, pHealth[0], 15);
			iSetColor(255, 255, 255); iText(20, 40, "AFIF (WASD | Q,E,R)", GLUT_BITMAP_HELVETICA_12);

			iSetColor(0, 255, 100); iFilledRectangle(1000, 20, pHealth[1] * 1.5f, 15);
			iSetColor(255, 255, 255); iText(1000, 40, "ORITRI (ARROWS | Z,X,C)", GLUT_BITMAP_HELVETICA_12);
		}

		iSetColor(0, 100, 255); iFilledRectangle(pX[0] + dx, pY + dy, pW, 80);
		iSetColor(0, 255, 100); iFilledRectangle(pX[1] + dx, pY + dy, pW, 80);

		if (currentFlow != DIALOGUE) {
			if (isShieldActive) { iSetColor(0, 200, 255); iCircle(pX[0] + 20 + dx, pY + 40 + dy, 60 + (sin(shieldTimer * 0.5f) * 5)); }

			for (int i = 0; i<10; i++) {
				if (pAtks[i].active) {
					if (pAtks[i].type == 1) {
						iSetColor(255, 100, 0); iFilledCircle(pAtks[i].x + dx, pAtks[i].y + dy, 15);
						iSetColor(255, 255, 0); iFilledCircle(pAtks[i].x - 10 + dx, pAtks[i].y + dy, 8);
					}
					else { iSetColor(50, 200, 50); iFilledEllipse(pAtks[i].x + dx, pAtks[i].y + dy, 15, 8); }
				}
			}

			iSetColor(200, 255, 200);
			for (int i = 0; i < 5; i++) if (skulls[i].active) iFilledCircle(skulls[i].x + dx, skulls[i].y + dy, 12);

			for (int i = 0; i < 3; i++) {
				if (hazards[i].active) {
					if (bossState == ATK_CHAINS) {
						if (hazards[i].warnTimer > 0) { iSetColor(150, 0, 0); iFilledEllipse(hazards[i].x + dx, 100 + dy, 40, 15); }
						else if (hazards[i].attackTimer > 0) { iSetColor(100, 100, 100); iFilledRectangle(hazards[i].x - 20 + dx, 0 + dy, 40, SCREEN_H); }
					}
					else if (bossState == ATK_LASERS) {
						if (hazards[i].warnTimer > 0) { iSetColor(255, 0, 0); iLine(0, hazards[i].y + dy, SCREEN_W, hazards[i].y + dy); }
						else if (hazards[i].attackTimer > 0) {
							iSetColor(255, 0, 0); iFilledRectangle(0, hazards[i].y - 30 + dx, SCREEN_W, 60);
							iSetColor(255, 200, 200); iFilledRectangle(0, hazards[i].y - 10 + dx, SCREEN_W, 20);
						}
					}
				}
			}

			iSetColor(130, 80, 50);
			for (int i = 0; i < 15; i++) if (drops[i].active) iFilledRectangle(drops[i].x + dx, drops[i].y + dy, 20, 30);

			if (isThunderActive) {
				iSetColor(255, 255, 0); iFilledRectangle(bossX - 10 + dx, bossY + dy, 20, SCREEN_H);
				iSetColor(255, 255, 255); iFilledRectangle(bossX - 4 + dx, bossY + dy, 8, SCREEN_H);
			}
		}

		bool isEnraged = (bossHealth <= 50.0f);

		if (bossState == STUNNED) {
			if (img_boss_stunned != -1) { iSetColor(255, 255, 255); iShowImage(bossX - (bossImgW / 2) + dx, bossY - (bossImgH / 2) + dy, bossImgW, bossImgH, img_boss_stunned); }
			else if (img_boss != -1) {
				iSetColor(255, 255, 255); iShowImage(bossX - (bossImgW / 2) + dx, bossY - (bossImgH / 2) + dy, bossImgW, bossImgH, img_boss);
				glEnable(GL_BLEND); glColor4ub(0, 0, 0, 150); iFilledRectangle(bossX - (bossImgW / 2) + dx, bossY - (bossImgH / 2) + dy, bossImgW, bossImgH); glDisable(GL_BLEND);
			}
			else { iSetColor(80, 80, 80); iFilledCircle(bossX + dx, bossY + dy, 50); }

			if (currentFlow == BATTLE) { iSetColor(255, 255, 255); iText(bossX - 70, bossY + (bossImgH / 2) + 20, "[USE ORITRI ATTACKS]", GLUT_BITMAP_HELVETICA_12); }
		}
		else {
			if (currentFlow != DIALOGUE) {
				for (int i = 0; i < 10; i++) if (trailX[i] > 0) { iSetColor(120 - (i * 10), 0, 0); iFilledCircle(trailX[i] + dx, trailY[i] + dy, 60 - (i * 4)); }
				if (isEnraged) { iSetColor(255, 50, 0); if (bossState == ATK_SKULLS) iCircle(bossX + dx, bossY + dy, 100 + (sin(stateTimer * 0.2f) * 15)); }
				else { iSetColor(0, 255, 0); if (bossState == ATK_SKULLS) iCircle(bossX + dx, bossY + dy, 90); }
			}
			if (img_boss != -1) { iSetColor(255, 255, 255); iShowImage(bossX - (bossImgW / 2) + dx, bossY - (bossImgH / 2) + dy, bossImgW, bossImgH, img_boss); }
			else { iSetColor(120, 0, 0); iFilledCircle(bossX + dx, bossY + dy, 60); }
		}

		if (currentFlow == DIALOGUE) {
			iSetColor(255, 255, 255);
			if (strcmp(script[currentDialogue].speaker, "AFIF") == 0 && img_p_afif != -1) iShowImage(120, 180, 250, 300, img_p_afif);
			else if (strcmp(script[currentDialogue].speaker, "ORITRI") == 0 && img_p_ori != -1) iShowImage(120, 180, 250, 300, img_p_ori);
			else if (strcmp(script[currentDialogue].speaker, "SAMIHA") == 0 && img_p_sam != -1) iShowImage(SCREEN_W - 370, 180, 250, 300, img_p_sam);

			glEnable(GL_BLEND); glColor4ub(0, 0, 0, 220); iFilledRectangle(100, 20, SCREEN_W - 200, 160); glDisable(GL_BLEND);
			iSetColor(200, 150, 50); iRectangle(100, 20, SCREEN_W - 200, 160);

			iSetColor(script[currentDialogue].colorR, script[currentDialogue].colorG, script[currentDialogue].colorB);
			iText(130, 140, script[currentDialogue].speaker, GLUT_BITMAP_TIMES_ROMAN_24);

			iSetColor(255, 255, 255); iText(130, 100, script[currentDialogue].line1, GLUT_BITMAP_9_BY_15); iText(130, 75, script[currentDialogue].line2, GLUT_BITMAP_9_BY_15);
			if (stateTimer % 60 < 30) { iSetColor(150, 150, 150); iText(SCREEN_W - 300, 40, "[PRESS SPACE]", GLUT_BITMAP_HELVETICA_12); }
		}

		if (currentFlow == TRANSITION) {
			glEnable(GL_BLEND); glColor4ub(0, 0, 0, 150); iFilledRectangle(0, 0, SCREEN_W, SCREEN_H); glDisable(GL_BLEND);
			iSetColor(255, 0, 0); iText(SCREEN_W / 2 - 120, SCREEN_H / 2, "BOSS FIGHT START", GLUT_BITMAP_TIMES_ROMAN_24);
		}
		else if (currentFlow == GAMEOVER) {
			glEnable(GL_BLEND); glColor4ub(150, 0, 0, 180); iFilledRectangle(0, 0, SCREEN_W, SCREEN_H); glDisable(GL_BLEND);
			iSetColor(255, 255, 255); iText(SCREEN_W / 2 - 80, SCREEN_H / 2, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
			iText(SCREEN_W / 2 - 120, SCREEN_H / 2 - 30, "The Foundation Claims Another...", GLUT_BITMAP_HELVETICA_18);
			iText(SCREEN_W / 2 - 110, SCREEN_H / 2 - 70, "[PRESS SPACE TO RETURN TO MENU]", GLUT_BITMAP_HELVETICA_12);
		}
		else if (currentFlow == VICTORY) {
			glEnable(GL_BLEND); glColor4ub(255, 200, 50, 100); iFilledRectangle(0, 0, SCREEN_W, SCREEN_H); glDisable(GL_BLEND);
			iSetColor(255, 255, 255); iText(SCREEN_W / 2 - 70, SCREEN_H / 2, "YOU WIN", GLUT_BITMAP_TIMES_ROMAN_24);
			iText(SCREEN_W / 2 - 100, SCREEN_H / 2 - 30, "The Archive is Finally Sealed.", GLUT_BITMAP_HELVETICA_18);
			iText(SCREEN_W / 2 - 110, SCREEN_H / 2 - 70, "[PRESS SPACE TO FINISH THE GAME]", GLUT_BITMAP_HELVETICA_12);
		}
	}

	void update() {
		stateTimer++;
		if (spaceCooldown > 0) spaceCooldown--;

		if (currentFlow == DIALOGUE) {
			if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && spaceCooldown == 0) {
				currentDialogue++; spaceCooldown = 20;
				if (currentDialogue >= 11) { currentFlow = TRANSITION; transitionTimer = 120; stateTimer = 0; }
			}
			return;
		}

		if (currentFlow == TRANSITION) { transitionTimer--; if (transitionTimer <= 0) currentFlow = BATTLE; return; }

		// Exit handlers for end game!
		if (currentFlow == GAMEOVER || currentFlow == VICTORY) {
			if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && spaceCooldown == 0) {
				spaceCooldown = 20;
				::currentState = MENU; // Sends player back to the main menu!
			}
			return;
		}

		for (int i = 0; i < 3; i++) { if (afifCD[i] > 0) afifCD[i]--; if (oritriCD[i] > 0) oritriCD[i]--; }
		if (isShieldActive) { shieldTimer--; if (shieldTimer <= 0) isShieldActive = false; }
		if (isTimeStopped) { timeStopTimer--; if (timeStopTimer <= 0) isTimeStopped = false; }
		if (isThunderActive) { thunderTimer--; if (thunderTimer <= 0) isThunderActive = false; }

		for (int i = 0; i < 10; i++) {
			if (pAtks[i].active) {
				pAtks[i].x += pAtks[i].vx; pAtks[i].y += pAtks[i].vy;
				if (pAtks[i].y > bossY - 60 && pAtks[i].y < bossY + 60 && pAtks[i].x > bossX - 60 && pAtks[i].x < bossX + 60) {
					pAtks[i].active = false;
					if (bossState == STUNNED) { bossHealth -= (pAtks[i].type == 1) ? 5.0f : 3.0f; shakeTimer = 5; }
					else { bossHealth -= 1.0f; }
				}
				if (pAtks[i].x > SCREEN_W) pAtks[i].active = false;
			}
		}

		if (GetAsyncKeyState('D')) pX[0] += 5.0f; if (GetAsyncKeyState('A')) pX[0] -= 5.0f;
		if (GetAsyncKeyState(VK_RIGHT)) pX[1] += 6.0f; if (GetAsyncKeyState(VK_LEFT)) pX[1] -= 6.0f;

		for (int i = 0; i < 2; i++) { if (pX[i] < 0) pX[i] = 0; if (pX[i] > (SCREEN_W / 2) - 80) pX[i] = (SCREEN_W / 2) - 80; }

		if ((GetAsyncKeyState('Q') & 0x8000) && afifCD[0] == 0) { isShieldActive = true; shieldTimer = 120; afifCD[0] = 300; }
		if ((GetAsyncKeyState('E') & 0x8000) && afifCD[1] == 0) { pHealth[1] += 30.0f; if (pHealth[1] > pMaxHealth[1]) pHealth[1] = pMaxHealth[1]; afifCD[1] = 600; }
		if ((GetAsyncKeyState('R') & 0x8000) && afifCD[2] == 0) { isTimeStopped = true; timeStopTimer = 180; afifCD[2] = 1200; }

		if ((GetAsyncKeyState('Z') & 0x8000) && oritriCD[0] == 0) {
			for (int i = 0; i < 10; i++) { if (!pAtks[i].active) { pAtks[i] = { pX[1] + pW, pY + 40, 15.0f, 0.0f, 1, true }; break; } }
			oritriCD[0] = 30;
		}
		if ((GetAsyncKeyState('X') & 0x8000) && oritriCD[1] == 0) {
			int spawned = 0;
			for (int i = 0; i < 10; i++) {
				if (!pAtks[i].active) {
					float vy = (spawned == 0) ? -3.0f : (spawned == 1) ? 0.0f : 3.0f;
					pAtks[i] = { pX[1] + pW, pY + 40, 12.0f, vy, 2, true }; spawned++;
					if (spawned >= 3) break;
				}
			}
			oritriCD[1] = 90;
		}
		if ((GetAsyncKeyState('C') & 0x8000) && oritriCD[2] == 0) {
			isThunderActive = true; thunderTimer = 20;
			if (bossState == STUNNED) bossHealth -= 25.0f; else bossHealth -= 2.0f;
			shakeTimer = 30; oritriCD[2] = 300;
		}

		if (!isTimeStopped) {
			for (int i = 9; i > 0; i--) { trailX[i] = trailX[i - 1]; trailY[i] = trailY[i - 1]; }
			trailX[0] = bossX; trailY[0] = bossY;
			bool isEnraged = (bossHealth <= 50.0f);

			if (bossState != STUNNED) {
				float dirX = bossTargetX - bossX; float dirY = bossTargetY - bossY;
				float dist = sqrt(dirX * dirX + dirY * dirY);
				if (dist > 5.0f) { bossX += (dirX / dist) * 4.0f; bossY += (dirY / dist) * 4.0f; }
				else if (bossState == HOVER && rand() % 60 == 0) { bossTargetX = (SCREEN_W / 2) + 100 + (rand() % 400); bossTargetY = 200 + (rand() % 400); }
			}

			if (bossState == HOVER) {
				if (stateTimer > 180) {
					bossState = (BossState)((rand() % 4) + 1); stateTimer = 0; bossTargetX = 900; bossTargetY = 400;
					if (bossState == ATK_SKULLS) { skullFireIdx = 0; for (int i = 0; i < 5; i++) skulls[i] = { bossX, bossY, 0, 0, true, true, (float)i * (2 * PI / 5) }; }
					else if (bossState == ATK_CHAINS) { for (int i = 0; i < 3; i++) hazards[i] = { pX[rand() % 2] + (rand() % 150 - 75), 0, true, 60, 30 }; }
					else if (bossState == ATK_LASERS) { for (int i = 0; i < 2; i++) hazards[i] = { 0, pY + 40 + (i * 150), true, 60, 20 }; }
					else if (bossState == ATK_DROPS) { for (int i = 0; i < 15; i++) drops[i] = { (float)(rand() % (SCREEN_W / 2)), SCREEN_H + (rand() % 600), 6.0f + (rand() % 5), true }; }
				}
			}
			else if (bossState == ATK_SKULLS) {
				for (int i = 0; i < 5; i++) {
					if (skulls[i].orbiting) { skulls[i].angle += 0.05f; skulls[i].x = bossX + cos(skulls[i].angle) * 120; skulls[i].y = bossY + sin(skulls[i].angle) * 120; }
					else if (skulls[i].active) {
						skulls[i].x += skulls[i].vx; skulls[i].y += skulls[i].vy;
						float dAfif = sqrt(pow(skulls[i].x - (pX[0] + 20), 2) + pow(skulls[i].y - (pY + 40), 2));
						float dOritri = sqrt(pow(skulls[i].x - (pX[1] + 20), 2) + pow(skulls[i].y - (pY + 40), 2));
						if (dAfif < 30) { if (!isShieldActive) pHealth[0] -= 10; skulls[i].active = false; }
						else if (dOritri < 30) { pHealth[1] -= 10; skulls[i].active = false; }
					}
				}
				int fireRate = isEnraged ? 25 : 45; float skullSpeed = isEnraged ? 12.0f : 8.0f;
				if (stateTimer % fireRate == 0 && skullFireIdx < 5) {
					int tgt = rand() % 2; float diffX = pX[tgt] - skulls[skullFireIdx].x; float diffY = pY - skulls[skullFireIdx].y;
					float dist = sqrt(diffX * diffX + diffY * diffY);
					skulls[skullFireIdx].vx = (diffX / dist) * skullSpeed; skulls[skullFireIdx].vy = (diffY / dist) * skullSpeed;
					skulls[skullFireIdx].orbiting = false; skullFireIdx++;
				}
				if (stateTimer > 300) { bossState = STUNNED; stateTimer = 0; }
			}
			else if (bossState == ATK_CHAINS || bossState == ATK_LASERS) {
				bool attackOngoing = false;
				for (int i = 0; i < 3; i++) {
					if (hazards[i].active) {
						attackOngoing = true;
						if (hazards[i].warnTimer > 0) hazards[i].warnTimer--;
						else if (hazards[i].attackTimer > 0) {
							if (hazards[i].attackTimer == 30 && bossState == ATK_CHAINS) shakeTimer = 25;
							if (hazards[i].attackTimer == 20 && bossState == ATK_LASERS) shakeTimer = 25;
							if (bossState == ATK_CHAINS && !isShieldActive && abs(hazards[i].x - pX[0]) < 30) pHealth[0] -= 0.5f;
							if (bossState == ATK_CHAINS && abs(hazards[i].x - pX[1]) < 30) pHealth[1] -= 0.5f;
							hazards[i].attackTimer--;
						}
						else hazards[i].active = false;
					}
				}
				if (!attackOngoing) { bossState = STUNNED; stateTimer = 0; }
			}
			else if (bossState == ATK_DROPS) {
				bool dropsOngoing = false;
				for (int i = 0; i < 15; i++) {
					if (drops[i].active) {
						dropsOngoing = true; drops[i].y -= drops[i].vy;
						if (drops[i].y < -50) drops[i].active = false;
						if (drops[i].y < pY + 80 && drops[i].y > pY && abs(drops[i].x - pX[0]) < 20 && !isShieldActive) pHealth[0] -= 2.0f;
						if (drops[i].y < pY + 80 && drops[i].y > pY && abs(drops[i].x - pX[1]) < 20) pHealth[1] -= 2.0f;
					}
				}
				if (!dropsOngoing) { bossState = STUNNED; stateTimer = 0; }
			}
			else if (bossState == STUNNED) {
				if (bossY > 150) bossY -= 5.0f;
				stunTimer++;
				if (stunTimer > 200) { bossState = HOVER; stateTimer = 0; stunTimer = 0; bossTargetY = 400; }
			}
		}

		if (bossHealth <= 0) { bossHealth = 0; currentFlow = VICTORY; }
		if (pHealth[0] <= 0 || pHealth[1] <= 0) {
			if (pHealth[0] < 0) pHealth[0] = 0; if (pHealth[1] < 0) pHealth[1] = 0; currentFlow = GAMEOVER;
		}
	}

	void keyboard(unsigned char key) {}

	void init() {
		// Load images
		img_bg = iLoadImage("final_level_bg.png");
		img_boss = iLoadImage("samiha_final_form.png");
		img_boss_stunned = iLoadImage("samiha_final_form_defeated.png");
		img_p_afif = iLoadImage("talkingafif.png");
		img_p_ori = iLoadImage("talkingori.png");
		img_p_sam = iLoadImage("talkingsam.png");

		// Reset all variables so the boss fight can be replayed safely
		currentFlow = DIALOGUE;
		currentDialogue = 0;
		transitionTimer = 0;
		bossHealth = 100.0f;
		pHealth[0] = 150.0f; pHealth[1] = 80.0f;
		pX[0] = 100; pX[1] = 250; pY = 100;
		bossX = 1000; bossY = 400; bossTargetX = 1000; bossTargetY = 400;
		bossState = HOVER;
		isShieldActive = false; shieldTimer = 0;
		isTimeStopped = false; timeStopTimer = 0;
		isThunderActive = false; thunderTimer = 0;
		stateTimer = 0; stunTimer = 0; shakeTimer = 0; spaceCooldown = 0;
		skullFireIdx = 0;

		for (int i = 0; i < 3; i++) { afifCD[i] = 0; oritriCD[i] = 0; }
		for (int i = 0; i < 10; i++) { pAtks[i].active = false; trailX[i] = -100; trailY[i] = -100; }
		for (int i = 0; i < 5; i++) skulls[i].active = false;
		for (int i = 0; i < 3; i++) hazards[i].active = false;
		for (int i = 0; i < 15; i++) drops[i].active = false;
	}
}

// ---------------- INTERFACE ----------------
void initLevel6() { L6::init(); }
void drawLevel6() { L6::draw(); }
void updateLevel6() { L6::update(); }
void keyboardLevel6(unsigned char key) { L6::keyboard(key); }