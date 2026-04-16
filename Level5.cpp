#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include "iGraphics.h"
#include "Globals.h"
#include "Level5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

namespace L5 {
	// =========================================================
	// LEVEL 5 - FAST COMBAT GAUNTLET
	// =========================================================
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define GROUND_Y 60 

	enum L5State { VN_BOSS_INTRO, BOSS_FIGHT, BOSS_FIGHT_DEAD, VN_POST_BOSS, VN_EPILOGUE };
	L5State l5State = VN_BOSS_INTRO;

	// VN Variables
	bool spacePressed = false;
	int charIndex = 0, textTimer = 0;
	const int TEXT_SPEED = 2;
	int vnBossIntroStep = 0, vnPostBossStep = 0, vnEpilogueStep = 0;

	// Visual Assets
	int img_boss_angry, img_mirror_world, img_boss_arena, img_player, img_electric;
	int ori_img, afif_img, sam_img;
	int enemy_img[8];

	// =========================================================
	// ENEMY SYSTEM (28 ENEMIES)
	// =========================================================
	struct Enemy {
		float x, y, velY;
		bool onGround;
		int hp, maxHP, imgIdx, w, h;
		bool active, dead;
		int flash;
		float jumpTimer;
		bool isJumping;
	};

	const int TOTAL_ENEMIES = 28;
	Enemy army[TOTAL_ENEMIES];
	int totalEnemiesSpawned = 0;
	int totalEnemiesDefeated = 0;

	// Base stats for the 8 types of enemies
	int enemyW[] = { 70, 75, 72, 78, 70, 74, 76, 73 };
	int enemyH[] = { 100, 110, 105, 115, 95, 108, 112, 102 };
	int enemyBaseHP[] = { 3, 3, 3, 3, 3, 3, 3, 3 };

	// Spawn positioning
	const float SPAWN_X = 1400.0f;
	const float SPAWN_Y = (float)GROUND_Y;
	const float MIN_ENEMY_DISTANCE = 500.0f;

	// Player Variables
	float bpX = 150.0f, bpY = (float)GROUND_Y, bpVelY = 0.0f;
	bool bpOnGround = true, shootPressed = false;
	int bpW = 100, bpH = 100;
	int playerHP = 150, playerMaxHP = 150;
	float playerSpeed = 5.0f;
	int playerIFrames = 0;

	// Projectiles
#define MAX_ELECTRIC 8
	struct Projectile {
		float x, y;
		bool active;
		int lifespan;
	};

	Projectile electric[MAX_ELECTRIC];

	// Game Variables
	float bgScrollX = 0;
	int waveNumber = 1;
	int enemiesInCurrentWave = 0;
	int enemiesDefeatedInWave = 0;
	int globalFrame = 0;

	// =========================================================
	// UTILITIES
	// =========================================================
	void drawPNG(float x, float y, int w, int h, int imgID, float brightness = 1.0f) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(brightness, brightness, brightness, 1.0f);
		iShowImage((int)x, (int)y, w, h, imgID);
		glDisable(GL_BLEND);
	}

	void drawVNUI(const char* name, const char* l1, const char* l2) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4ub(0, 0, 0, 220);
		iFilledRectangle(50, 30, 1180, 150);
		glDisable(GL_BLEND);

		iSetColor(211, 47, 47);
		iFilledRectangle(50, 185, 200, 40);
		iSetColor(255, 255, 255);
		iText(70, 195, (char*)name, GLUT_BITMAP_HELVETICA_18);

		char dL1[100] = "", dL2[100] = "";
		int len1 = (int)strlen(l1);
		if (charIndex <= len1) {
			strncpy(dL1, l1, charIndex);
			dL1[charIndex] = '\0';
		}
		else {
			strcpy(dL1, l1);
			int overflow = charIndex - len1;
			if (overflow > (int)strlen(l2)) overflow = (int)strlen(l2);
			strncpy(dL2, l2, overflow);
			dL2[overflow] = '\0';
		}
		iText(80, 110, dL1, GLUT_BITMAP_9_BY_15);
		if (dL2[0] != '\0') iText(80, 80, dL2, GLUT_BITMAP_9_BY_15);
	}

	void drawGameUI() {
		iSetColor(40, 40, 40);
		iFilledRectangle(50, 660, 300, 20);

		float healthPercent = (float)playerHP / playerMaxHP;
		if (healthPercent > 0.5f) iSetColor(0, 255, 100);
		else if (healthPercent > 0.25f) iSetColor(255, 165, 0);
		else iSetColor(255, 0, 0);

		iFilledRectangle(50, 660, (int)(playerHP * 2), 20);

		iSetColor(200, 200, 200);
		iRectangle(50, 660, 300, 20);

		iSetColor(255, 255, 255);
		iText(50, 685, "INTEGRITY", GLUT_BITMAP_HELVETICA_12);

		char totalText[50];
		sprintf(totalText, "TOTAL DEFEATED: %d/%d", totalEnemiesDefeated, TOTAL_ENEMIES);
		iText(900, 685, totalText, GLUT_BITMAP_HELVETICA_12);
	}

	// =========================================================
	// GAME LOGIC
	// =========================================================
	void spawnEnemy(int enemyIndex) {
		if (totalEnemiesSpawned >= TOTAL_ENEMIES) return;

		int type = enemyIndex % 8;
		army[enemyIndex].x = SPAWN_X;
		army[enemyIndex].y = SPAWN_Y;
		army[enemyIndex].velY = 0;
		army[enemyIndex].onGround = true;
		army[enemyIndex].hp = enemyBaseHP[type];
		army[enemyIndex].maxHP = enemyBaseHP[type];
		army[enemyIndex].active = true;
		army[enemyIndex].dead = false;
		army[enemyIndex].flash = 0;
		army[enemyIndex].jumpTimer = (rand() % 100) + 50;
		army[enemyIndex].isJumping = false;
		army[enemyIndex].imgIdx = type;
		army[enemyIndex].w = enemyW[type];
		army[enemyIndex].h = enemyH[type];

		totalEnemiesSpawned++;
	}

	void initEnemies() {
		for (int i = 0; i < TOTAL_ENEMIES; i++) {
			army[i] = { 0, 0, 0, false, 0, 0, 0, 0, 0, false, false, 0, 0, false };
		}
		totalEnemiesSpawned = 0;
		totalEnemiesDefeated = 0;
		waveNumber = 1;
		enemiesInCurrentWave = 0;
		enemiesDefeatedInWave = 0;
		playerIFrames = 0;
		globalFrame = 0;

		for (int i = 0; i < 2; i++) {
			spawnEnemy(i);
			army[i].x += i * 600.0f;
		}
	}

	void updateEnemyAI(Enemy& e) {
		e.jumpTimer--;
		if (e.jumpTimer <= 0 && e.onGround && rand() % 20 == 0) {
			e.velY = 13.0f + (rand() % 4) - 2;
			e.onGround = false;
			e.isJumping = true;
			e.jumpTimer = (rand() % 80) + 60;
		}

		if (!e.onGround) {
			e.velY -= 0.6f;
			e.y += e.velY;
			if (e.y <= GROUND_Y) {
				e.y = GROUND_Y;
				e.velY = 0;
				e.onGround = true;
				e.isJumping = false;
			}
		}
	}

	void updateBossFight() {
		if (l5State == BOSS_FIGHT_DEAD) return;

		bgScrollX += 2.0f;
		if (playerIFrames > 0) playerIFrames--;

		// === PLAYER PHYSICS ===
		if (bpY > GROUND_Y || !bpOnGround) {
			bpVelY -= 0.6f;
			bpY += bpVelY;
		}
		if (bpY <= GROUND_Y) {
			bpY = (float)GROUND_Y;
			bpVelY = 0;
			bpOnGround = true;
		}

		if (((GetAsyncKeyState(VK_UP) & 0x8000) || (GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000)) && bpOnGround) {
			bpVelY = 19.0f;
			bpOnGround = false;
		}

		if ((GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState('A') & 0x8000)) bpX -= playerSpeed;
		if ((GetAsyncKeyState(VK_RIGHT) & 0x8000) || (GetAsyncKeyState('D') & 0x8000)) bpX += playerSpeed;

		if (bpX < 50) bpX = 50;
		if (bpX > SCREEN_WIDTH - bpW - 50) bpX = SCREEN_WIDTH - bpW - 50;

		// === SHOOTING ===
		if ((GetAsyncKeyState(VK_RETURN) & 0x8000) && !shootPressed) {
			shootPressed = true;
			for (int i = 0; i < MAX_ELECTRIC; i++) {
				if (!electric[i].active) {
					electric[i].x = bpX + bpW / 2;
					electric[i].y = bpY + bpH / 2;
					electric[i].active = true;
					electric[i].lifespan = 60;
					break;
				}
			}
		}
		else if (!(GetAsyncKeyState(VK_RETURN) & 0x8000)) {
			shootPressed = false;
		}

		// === PROJECTILE UPDATES ===
		for (int i = 0; i < MAX_ELECTRIC; i++) {
			if (electric[i].active) {
				electric[i].x += 25.0f;
				electric[i].lifespan--;

				for (int j = 0; j < TOTAL_ENEMIES; j++) {
					if (army[j].active &&
						electric[i].x + 5 > army[j].x && electric[i].x < army[j].x + army[j].w &&
						electric[i].y + 5 > army[j].y && electric[i].y < army[j].y + army[j].h) {

						electric[i].active = false;
						electric[i].lifespan = 0;
						army[j].hp--;
						army[j].flash = 10;

						if (army[j].hp <= 0) {
							army[j].active = false;
							army[j].dead = true;
							totalEnemiesDefeated++;
						}
						break;
					}
				}
				if (electric[i].x > SCREEN_WIDTH + 50 || electric[i].lifespan <= 0) electric[i].active = false;
			}
		}

		// === ENEMY UPDATES ===
		int activeEnemyCount = 0;
		for (int i = 0; i < TOTAL_ENEMIES; i++) {
			if (army[i].active) {
				activeEnemyCount++;
				army[i].x -= 2.5f;

				if (army[i].x + army[i].w < bpX - 20) {
					army[i].active = false;
					army[i].dead = true;
					totalEnemiesDefeated++;
					continue;
				}

				for (int j = 0; j < TOTAL_ENEMIES; j++) {
					if (i != j && army[j].active) {
						float dist = army[i].x - army[j].x;
						if (abs(dist) < 70.0f) {
							if (dist >= 0) army[i].x += 2.0f;
							else army[i].x -= 2.0f;
						}
					}
				}

				if (playerIFrames <= 0 &&
					army[i].x < bpX + bpW - 20 && army[i].x + army[i].w - 20 > bpX &&
					army[i].y < bpY + bpH && army[i].y + army[i].h > bpY) {

					playerHP -= 15;
					playerIFrames = 30;
					army[i].x = SPAWN_X + (rand() % 200);
				}

				updateEnemyAI(army[i]);
				if (army[i].flash > 0) army[i].flash--;
			}
		}

		// === CONTINUOUS SPAWNING SYSTEM ===
		if (totalEnemiesSpawned < TOTAL_ENEMIES) {
			bool canSpawn = true;
			for (int i = 0; i < TOTAL_ENEMIES; i++) {
				if (army[i].active && army[i].x > SPAWN_X - MIN_ENEMY_DISTANCE) {
					canSpawn = false; break;
				}
			}
			if (canSpawn) {
				int nextEnemyIndex = -1;
				for (int i = 0; i < TOTAL_ENEMIES; i++) {
					if (!army[i].active && !army[i].dead) { nextEnemyIndex = i; break; }
				}
				if (nextEnemyIndex != -1) spawnEnemy(nextEnemyIndex);
			}
		}

		// === GAME END CONDITION ===
		if (totalEnemiesDefeated >= TOTAL_ENEMIES && activeEnemyCount == 0) {
			l5State = VN_POST_BOSS;
			charIndex = 0;
		}

		if (playerHP <= 0) l5State = BOSS_FIGHT_DEAD;
	}

	// =========================================================
	// RENDERING
	// =========================================================
	void draw() {
		iSetColor(255, 255, 255); // OpenGL fix

		if (l5State == VN_BOSS_INTRO) {
			iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_boss_arena);
			char name[20] = "", l1[100] = "", l2[100] = "";
			int activeChar = -1;

			if (vnBossIntroStep == 0) { activeChar = 3; strcpy(name, "???"); strcpy(l1, "Ha. Three little students, crawling out of my mirror."); strcpy(l2, "How... predictable."); }
			else if (vnBossIntroStep == 1) { activeChar = 0; strcpy(name, "Oritri"); strcpy(l1, "Who are you? Why are students disappearing?"); strcpy(l2, "What is this place?"); }
			else if (vnBossIntroStep == 2) { activeChar = 3; strcpy(name, "The Warden"); strcpy(l1, "I am The Warden. I test those who dare call"); strcpy(l2, "themselves engineers. Most fail. They stay."); }
			else if (vnBossIntroStep == 3) { activeChar = 2; strcpy(name, "Samiha"); strcpy(l1, "You trapped them? They're real people!"); strcpy(l2, "You can't just decide who deserves to exist!"); }
			else if (vnBossIntroStep == 4) { activeChar = 3; strcpy(name, "The Warden"); strcpy(l1, "I am the examination. I am the grade. I am the god"); strcpy(l2, "of this institution. Your protest is a zero mark."); }
			else if (vnBossIntroStep == 5) { activeChar = 1; strcpy(name, "Afif"); strcpy(l1, "Oh that's it. I have been STRESSED about this project"); strcpy(l2, "for three weeks and now THIS? Oritri, I'm fighting him."); }
			else if (vnBossIntroStep == 6) { activeChar = 0; strcpy(name, "Oritri"); strcpy(l1, "...Yeah. Let's end this."); strcpy(l2, ""); }

			if (activeChar == 3) drawPNG(SCREEN_WIDTH / 2 - 175, 80, 350, 550, img_boss_angry);
			else {
				drawPNG(80, 100, 380, 520, ori_img, (activeChar == 0) ? 1.0f : 0.3f);
				drawPNG(440, 100, 380, 520, afif_img, (activeChar == 1) ? 1.0f : 0.3f);
				drawPNG(800, 100, 380, 520, sam_img, (activeChar == 2) ? 1.0f : 0.3f);
			}
			drawVNUI(name, l1, l2);
		}
		else if (l5State == BOSS_FIGHT || l5State == BOSS_FIGHT_DEAD) {
			iShowImage(-(int)bgScrollX % 1280, 0, 1280, 720, img_boss_arena);
			iShowImage(1280 - (int)bgScrollX % 1280, 0, 1280, 720, img_boss_arena);

			float playerBob = (bpOnGround && l5State != BOSS_FIGHT_DEAD) ? fabs(sin(globalFrame * 0.3f)) * 10.0f : 0;

			if (playerIFrames == 0 || (playerIFrames % 4 < 2)) {
				drawPNG(bpX, bpY + playerBob, bpW, bpH, img_player);
			}

			for (int i = 0; i < MAX_ELECTRIC; i++) {
				if (electric[i].active) {
					iSetColor(0, 255, 200);
					iFilledCircle((int)(electric[i].x + 2), (int)(electric[i].y + 2), 8);
					drawPNG(electric[i].x, electric[i].y, 5, 5, img_electric);
				}
			}

			for (int i = 0; i < TOTAL_ENEMIES; i++) {
				if (army[i].active) {
					float brightness = (army[i].flash > 0) ? 0.3f : 1.0f;
					float enemyBob = army[i].onGround ? fabs(sin(globalFrame * 0.2f + i)) * 8.0f : 0;
					drawPNG(army[i].x, army[i].y + enemyBob, army[i].w, army[i].h, enemy_img[army[i].imgIdx], brightness);

					iSetColor(50, 50, 50); iFilledRectangle((int)army[i].x - 5, (int)army[i].y + army[i].h + 10, army[i].w + 10, 12);
					iSetColor(255, 0, 0);
					float hpWidth = ((float)army[i].hp / army[i].maxHP) * army[i].w;
					iFilledRectangle((int)army[i].x, (int)army[i].y + army[i].h + 12, (int)hpWidth, 8);
					iSetColor(200, 200, 200); iRectangle((int)army[i].x - 5, (int)army[i].y + army[i].h + 10, army[i].w + 10, 12);
				}
			}

			drawGameUI();

			if (l5State == BOSS_FIGHT_DEAD) {
				glEnable(GL_BLEND); glColor4ub(0, 0, 0, 180);
				iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); glDisable(GL_BLEND);
				iSetColor(255, 0, 0); iText(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 + 50, "SYSTEM OVERRIDE: FAILED", GLUT_BITMAP_HELVETICA_18);
				iSetColor(255, 255, 255); iText(SCREEN_WIDTH / 2 - 180, SCREEN_HEIGHT / 2 - 50, "Press SPACE to Restart", GLUT_BITMAP_HELVETICA_12);
			}
		}
		else if (l5State == VN_POST_BOSS) {
			iShowImage(0, 0, 1280, 720, img_mirror_world);
			if (vnPostBossStep == 0) drawVNUI("Oritri", "They're gone. But the Warden was just a distraction.", "The real master is still in the archives.");
			else drawVNUI("Samiha", "We did it... but at what cost? All those people...", "We have to find the source of this mirror world.");
		}
	}

	void update() {
		globalFrame++;

		if (l5State == VN_BOSS_INTRO || l5State == VN_POST_BOSS) {
			textTimer++;
			if (textTimer >= TEXT_SPEED) { charIndex++; textTimer = 0; }

			if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && !spacePressed) {
				spacePressed = true; charIndex = 0;
				if (l5State == VN_BOSS_INTRO) {
					vnBossIntroStep++;
					if (vnBossIntroStep > 6) { l5State = BOSS_FIGHT; initEnemies(); }
				}
				else if (l5State == VN_POST_BOSS) {
					vnPostBossStep++;
					if (vnPostBossStep > 1) {
						// MAGIC BULLET: Unlock Final Boss / Level 6!
						::unlockedLevel = 6;
						saveProgress(6);
						::currentState = LEVEL_6; // Transition to the Master Global State
					}
				}
			}
			else if (!(GetAsyncKeyState(VK_SPACE) & 0x8000)) spacePressed = false;
		}
		else if (l5State == BOSS_FIGHT) updateBossFight();
		else if (l5State == BOSS_FIGHT_DEAD && (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
			playerHP = 150; initEnemies(); l5State = BOSS_FIGHT;
		}
	}

	void keyboard(unsigned char key) {}

	void init() {
		img_boss_angry = iLoadImage("boss_warden_angry.png");
		img_boss_arena = iLoadImage("boss_arena_bg.png");
		img_player = iLoadImage("player_stealth.png");
		img_electric = iLoadImage("electric.png");
		ori_img = iLoadImage("talkingori.png");
		afif_img = iLoadImage("talkingafif.png");
		sam_img = iLoadImage("talkingsam.png");
		img_mirror_world = iLoadImage("mirror_world_bg.png");

		enemy_img[0] = iLoadImage("enemy_1.png"); enemy_img[1] = iLoadImage("enemy_2.png");
		enemy_img[2] = iLoadImage("enemy_3.png"); enemy_img[3] = iLoadImage("enemy_4.png");
		enemy_img[4] = iLoadImage("enemy_5.png"); enemy_img[5] = iLoadImage("enemy_6.png");
		enemy_img[6] = iLoadImage("enemy_7.png"); enemy_img[7] = iLoadImage("enemy_medium.png");

		for (int i = 0; i < MAX_ELECTRIC; i++) electric[i] = { 0, 0, false, 0 };

		// Reset state
		l5State = VN_BOSS_INTRO;
		vnBossIntroStep = 0; vnPostBossStep = 0; vnEpilogueStep = 0;
		charIndex = 0; textTimer = 0; playerHP = 150; bgScrollX = 0;
		bpX = 150.0f; bpY = (float)GROUND_Y; bpOnGround = true;
	}
}

// ---------------- INTERFACE ----------------
void initLevel5() { L5::init(); }
void drawLevel5() { L5::draw(); }
void updateLevel5() { L5::update(); }
void keyboardLevel5(unsigned char key) { L5::keyboard(key); }