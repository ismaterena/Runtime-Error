#define _CRT_SECURE_NO_WARNINGS 
#include "iGraphics.h"
#include "Globals.h"
#include "Level3.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

namespace L3 {
	// ---------------- CONSTANTS (Updated to match Master Screen) ----------------
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define GROUND_LEVEL 100
#define GRAVITY 1
#define JUMP_FORCE 18
#define MAX_HEALTH 100
#define SPAWN_INTERVAL 625 

	// ---------------- GLOBAL STATE ----------------
	bool isIntroActive = true;
	int dialogueIndex = 0;
	bool isConversationActive = true;
	int totalLines = 8;

	// ---------------- ASSETS & UI ----------------
	int oriImg, enemyImg, bgImage;
	int ballSprite, fireSprite, floorTexture;
	int score = 0;
	int playerHealth = MAX_HEALTH;
	int enemyHealth = MAX_HEALTH;
	int musicTimer = 0;
	int currentLetterIdx = 0;
	int cameraX = 0;
	bool isInitialized = false;
	bool isGameOver = false;
	bool isVictory = false;

	// Effects
	int offsetX = 0, offsetY = 0;
	float introShakeDuration = 0;
	bool isIntroShaking = false;
	int gameShakeTimer = 0;
	float zoomScale = 0.0f;

	// ---------------- DATA ----------------
	char* speakers[] = { "SAMIHA CORRUPTION", "AFIF", "SAMIHA CORRUPTION", "AFIF", "SAMIHA CORRUPTION", "AFIF", "SAMIHA CORRUPTION", "AFIF" };
	char* lines[] = {
		"You don't understand... I had no choice. It wasn't my fault!",
		"Save it. I saw what you did at the portal.",
		"The mirror world corrupted the data! I was just a passenger!",
		"Enough lies. Draw your weapon.",
		"If I do... neither of us leaves this server room alive.",
		"I stopped caring about 'living' the moment the system crashed.",
		"Then let the corruption take us both!",
		"Prepare yourself. Delete sequence initiated."
	};
	char* words[] = { "AUST", "CSE", "SPRING", "2025" };

	// ---------------- STRUCTURES ----------------
	struct Particle { float x, y, vx, vy; int life; bool active; };
	std::vector<Particle> sparkles;

	struct Coin { float x, y; bool active; };
	std::vector<Coin> coinCluster;

	struct Ball {
		float x, y, speed; bool active, reflected;
		void update() { x -= speed; if (x < -100 || x > SCREEN_WIDTH + 200) active = false; }
		void draw(int ox, int oy) {
			if (active) {
				if (reflected) iSetColor(255, 0, 0); else iSetColor(255, 255, 255);
				iShowImage((int)x + ox, (int)y + oy, 35, 35, ballSprite);
			}
		}
	};
	std::vector<Ball> enemyBalls;

	struct Fireball {
		float x, y, speed; bool active;
		void update() { x += speed; if (x > SCREEN_WIDTH + 100) active = false; }
		void draw(int ox, int oy) {
			if (active) {
				iSetColor(255, 120, 0);
				for (int r = 40; r > 10; r -= 10) iCircle((int)x + 30 + ox, (int)y + 20 + oy, r);
				iShowImage((int)x + ox, (int)y + oy, 70, 45, fireSprite);
			}
		}
	};
	std::vector<Fireball> playerFires;

	// ---------------- HELPER FUNCTIONS ----------------
	void spawnSparkle(float x, float y) {
		for (int i = 0; i < 10; i++) {
			Particle p = { x, y, (float)(rand() % 11 - 5), (float)(rand() % 11 - 5), 25, true };
			sparkles.push_back(p);
		}
	}

	void drawGradientHealthBar(int x, int y, float health, const char* label, bool isPlayer) {
		float percent = health / 100.0f;
		if (percent < 0) percent = 0;
		iSetColor(60, 0, 0); iFilledRectangle(x, y, 200, 22);
		for (int i = 0; i < 22; i++) {
			if (isPlayer) iSetColor(0, 140 + (i * 5), 60);
			else iSetColor(180 + (i * 3), 40 + (i * i / 4), 0);
			iLine(x, y + i, x + (int)(200 * percent), y + i);
		}
		iSetColor(255, 255, 255); iRectangle(x, y, 200, 22);
		iText(x + 5, y + 28, (char*)label, GLUT_BITMAP_HELVETICA_12);
	}

	void addCoin(int x, int y) { coinCluster.push_back({ (float)x, (float)y, true }); }

	void spawnLetterPattern(char* word) {
		int numLetters = strlen(word);
		int spawnBaseX = cameraX + SCREEN_WIDTH + 100;
		int startY = 250, dotStep = 20, charGap = 120;
		for (int i = 0; i < numLetters; i++) {
			char c = word[i]; int x = spawnBaseX + (i * charGap);
			if (c == 'A') { for (int h = 0; h < 7; h++) { addCoin(x, startY + h * dotStep); addCoin(x + 60, startY + h * dotStep); } addCoin(x + 20, startY + 120); addCoin(x + 40, startY + 120); addCoin(x + 20, startY + 60); addCoin(x + 40, startY + 60); }
			else if (c == 'U') { for (int h = 1; h < 7; h++) { addCoin(x, startY + h * dotStep); addCoin(x + 60, startY + h * dotStep); } addCoin(x + 20, startY); addCoin(x + 40, startY); }
			else if (c == 'S' || c == '5' || c == '2') { for (int w = 0; w <= 60; w += 20) { addCoin(x + w, startY); addCoin(x + w, startY + 60); addCoin(x + w, startY + 120); } if (c == 'S' || c == '5') { addCoin(x, startY + 90); addCoin(x + 60, startY + 30); } if (c == '2') { addCoin(x + 60, startY + 90); addCoin(x, startY + 30); } }
			else if (c == 'T') { for (int h = 0; h < 120; h += 20) addCoin(x + 30, startY + h); for (int w = 0; w <= 60; w += 20) addCoin(x + w, startY + 120); }
			else if (c == 'C') { for (int h = 20; h < 110; h += 20) addCoin(x, startY + h); for (int w = 0; w <= 60; w += 20) { addCoin(x + w, startY); addCoin(x + w, startY + 120); } }
			else if (c == 'E') { for (int h = 0; h < 130; h += 20) addCoin(x, startY + h); for (int w = 20; w <= 60; w += 20) { addCoin(x + w, startY); addCoin(x + w, startY + 60); addCoin(x + w, startY + 120); } }
			else if (c == 'R') { for (int h = 0; h < 130; h += 20) addCoin(x, startY + h); addCoin(x + 30, startY + 120); addCoin(x + 60, startY + 120); addCoin(x + 60, startY + 85); addCoin(x + 30, startY + 70); addCoin(x + 55, startY + 70); addCoin(x + 40, startY + 40); addCoin(x + 60, startY + 10); }
			else if (c == 'P') { for (int h = 0; h < 130; h += 20) addCoin(x, startY + h); for (int w = 20; w <= 60; w += 20) { addCoin(x + w, startY + 65); addCoin(x + w, startY + 120); } addCoin(x + 60, startY + 95); }
			else if (c == 'I') { for (int h = 0; h < 130; h += 20) addCoin(x + 30, startY + h); for (int w = 0; w <= 60; w += 20) { addCoin(x + w, startY); addCoin(x + w, startY + 120); } }
			else if (c == 'N') { for (int h = 0; h < 130; h += 20) { addCoin(x, startY + h); addCoin(x + 65, startY + h); } addCoin(x + 20, startY + 90); addCoin(x + 45, startY + 45); }
			else if (c == 'G' || c == '0') { for (int h = 0; h < 130; h += 20) { addCoin(x, startY + h); addCoin(x + 60, startY + h); } for (int w = 20; w <= 40; w += 20) { addCoin(x + w, startY); addCoin(x + w, startY + 120); } if (c == 'G') { addCoin(x + 60, startY + 40); addCoin(x + 40, startY + 40); } }
		}
	}

	// ---------------- ENTITIES ----------------
	struct Enemy {
		float x, y, vY; int width, height; bool isJumping;
		int spriteIdle, spriteJump, throwTimer;
		void init() {
			x = SCREEN_WIDTH - 180; y = GROUND_LEVEL; width = 85; height = 120; vY = 0;
			isJumping = false; throwTimer = 0;
			spriteIdle = iLoadImage("enemy.png"); spriteJump = iLoadImage("enemy.png");
		}
		void update() {
			if (isGameOver || isVictory || isIntroActive) return;
			if (!isJumping && rand() % 200 == 2) { vY = 20; isJumping = true; }
			throwTimer++;
			if (throwTimer >= 80) { Ball b = { x, y + 60, 10.0f, true, false }; enemyBalls.push_back(b); throwTimer = 0; }
			y += vY; if (isJumping) vY -= GRAVITY;
			if (y <= GROUND_LEVEL) { y = GROUND_LEVEL; vY = 0; isJumping = false; }
			for (size_t i = 0; i < enemyBalls.size(); i++) {
				if (enemyBalls[i].active && enemyBalls[i].reflected) {
					if (enemyBalls[i].x + 30 > x && enemyBalls[i].x < x + width && enemyBalls[i].y + 30 > y && enemyBalls[i].y < y + height) {
						enemyBalls[i].active = false; enemyHealth -= 10; if (enemyHealth <= 0) isVictory = true;
					}
				}
			}
			for (size_t i = 0; i < playerFires.size(); i++) {
				if (playerFires[i].active && playerFires[i].x + 60 > x && playerFires[i].x < x + width && playerFires[i].y + 40 > y && playerFires[i].y < y + height) {
					playerFires[i].active = false; enemyHealth -= 15; if (enemyHealth <= 0) isVictory = true;
				}
			}
		}
		void draw(int ox, int oy) { if (!isVictory) iShowImage((int)x + ox, (int)y + oy, width, height, isJumping ? spriteJump : spriteIdle); }
	} bot;

	struct Player {
		int screenX, y, width, height, speed, vY, jumpCount, punchTimer, fireCooldown;
		bool isJumping, isMoving, facingRight, isPunching;
		int sIdleR, sIdleL, sRunR, sRunL, sJumpR, sJumpL;
		void init() {
			screenX = 150; y = GROUND_LEVEL; width = 85; height = 120; speed = 10; vY = 0;
			isJumping = false; facingRight = true; jumpCount = 0; isPunching = false; punchTimer = 0; fireCooldown = 0;
			sIdleR = iLoadImage("afif_idle_r.png"); sIdleL = iLoadImage("afif_idle_r.png");
			sRunR = iLoadImage("afif_idle_r.png"); sRunL = iLoadImage("afif_idle_r.png");
			sJumpR = iLoadImage("afif_idle_r.png"); sJumpL = iLoadImage("afif_idle_r.png");
		}
		void update() {
			if (isGameOver || isVictory || isIntroActive) return;
			isMoving = false;
			if (GetAsyncKeyState('D') || GetAsyncKeyState(VK_RIGHT)) { cameraX += speed; facingRight = true; isMoving = true; }
			if (GetAsyncKeyState('A') || GetAsyncKeyState(VK_LEFT)) { if (cameraX > 0) cameraX -= speed; facingRight = false; isMoving = true; }
			if (GetAsyncKeyState('F') && score >= 25 && !isPunching) { isPunching = true; punchTimer = 20; }
			if (isPunching) { punchTimer--; if (punchTimer <= 0) isPunching = false; }
			if (fireCooldown > 0) fireCooldown--;
			if (GetAsyncKeyState('G') && score >= 50 && fireCooldown <= 0) {
				Fireball f = { (float)screenX + width - 20, (float)y + 50, 14.0f, true }; playerFires.push_back(f); fireCooldown = 30;
			}
			static bool spacePressedLastFrame = false;
			bool spaceCurrent = (GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState(VK_UP));
			if (spaceCurrent && !spacePressedLastFrame && jumpCount < 2) { vY = JUMP_FORCE; isJumping = true; jumpCount++; }
			spacePressedLastFrame = spaceCurrent;
			y += vY; if (isJumping) vY -= GRAVITY;
			if (y <= GROUND_LEVEL) { y = GROUND_LEVEL; vY = 0; isJumping = false; jumpCount = 0; }

			for (size_t i = 0; i < enemyBalls.size(); i++) {
				if (enemyBalls[i].active && !enemyBalls[i].reflected) {
					if (screenX < enemyBalls[i].x + 30 && screenX + width > enemyBalls[i].x && y < enemyBalls[i].y + 30 && y + height > enemyBalls[i].y) {
						if (isPunching) { enemyBalls[i].reflected = true; enemyBalls[i].speed *= -2.0f; }
						else { enemyBalls[i].active = false; playerHealth -= 20; gameShakeTimer = 15; if (playerHealth <= 0) isGameOver = true; }
					}
				}
			}
			for (size_t i = 0; i < coinCluster.size(); i++) {
				if (coinCluster[i].active) {
					float relX = coinCluster[i].x - cameraX;
					if (screenX < relX + 20 && screenX + width > relX && y < coinCluster[i].y + 20 && y + height > coinCluster[i].y) {
						coinCluster[i].active = false; score++; spawnSparkle(relX, coinCluster[i].y);
					}
				}
			}
		}
	} hero;

	// ---------------- SYSTEM LOGIC ----------------
	void advanceDialogue() {
		if (isConversationActive) {
			dialogueIndex++;
			if (dialogueIndex >= totalLines) {
				isConversationActive = false; isIntroShaking = true; introShakeDuration = 0;
			}
		}
	}

	void update() {
		if (isIntroActive) {
			if (isIntroShaking) {
				introShakeDuration += 0.030;
				offsetX = (rand() % 21) - 10; offsetY = (rand() % 21) - 10;
				if (introShakeDuration >= 1.5) { isIntroShaking = false; isIntroActive = false; offsetX = 0; offsetY = 0; }
			}
			return;
		}
		if (isGameOver || isVictory) { if (zoomScale < 1.0f) zoomScale += 0.04f; return; }

		hero.update(); bot.update();
		for (size_t i = 0; i < enemyBalls.size(); i++) if (enemyBalls[i].active) enemyBalls[i].update();
		for (size_t i = 0; i < playerFires.size(); i++) if (playerFires[i].active) playerFires[i].update();
		for (size_t i = 0; i < sparkles.size(); i++) {
			if (sparkles[i].active) {
				sparkles[i].x += sparkles[i].vx; sparkles[i].y += sparkles[i].vy; sparkles[i].life--;
				if (sparkles[i].life <= 0) sparkles[i].active = false;
			}
		}
		musicTimer++;
		if (musicTimer >= SPAWN_INTERVAL) { spawnLetterPattern(words[currentLetterIdx]); currentLetterIdx = (currentLetterIdx + 1) % 4; musicTimer = 0; }
	}

	void draw() {
		if (isIntroActive) {
			iSetColor(0, 0, 0); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			iShowImage(offsetX, offsetY, SCREEN_WIDTH, SCREEN_HEIGHT, bgImage);
			iShowImage(150 + offsetX, 150 + offsetY, 150, 250, oriImg);
			iShowImage(SCREEN_WIDTH - 300 + offsetX, 150 + offsetY, 150, 250, enemyImg);

			if (isConversationActive) {
				iSetColor(20, 20, 20); iFilledRectangle(SCREEN_WIDTH / 2 - 300, 50, 600, 150);
				iSetColor(255, 255, 255); iRectangle(SCREEN_WIDTH / 2 - 300, 50, 600, 150);
				iSetColor(255, 255, 0); iText(SCREEN_WIDTH / 2 - 280, 170, speakers[dialogueIndex], GLUT_BITMAP_HELVETICA_18);
				iSetColor(255, 255, 255); iText(SCREEN_WIDTH / 2 - 280, 110, lines[dialogueIndex], GLUT_BITMAP_9_BY_15);
				iSetColor(150, 150, 150); iText(SCREEN_WIDTH / 2 + 100, 60, "[PRESS ENTER TO ADVANCE]", GLUT_BITMAP_HELVETICA_12);
			}
			else {
				iSetColor(255, 0, 0); iText(SCREEN_WIDTH / 2 - 100 + offsetX, 310 + offsetY, "BATTLE START", GLUT_BITMAP_TIMES_ROMAN_24);
			}
		}
		else {
			int sx = 0, sy = 0;
			if (gameShakeTimer > 0) { sx = rand() % 16 - 8; sy = rand() % 16 - 8; gameShakeTimer--; }

			int bgScroll = -(cameraX % SCREEN_WIDTH);
			iSetColor(255, 255, 255);
			iShowImage(bgScroll + sx, sy, SCREEN_WIDTH, SCREEN_HEIGHT, bgImage);
			iShowImage(bgScroll + SCREEN_WIDTH + sx, sy, SCREEN_WIDTH, SCREEN_HEIGHT, bgImage);

			// Aesthetic Floor
			iSetColor(20, 20, 25); iFilledRectangle(0, 0, SCREEN_WIDTH, GROUND_LEVEL);
			int tileWidth = 200; int floorScroll = -(cameraX % tileWidth);
			for (int x = floorScroll; x < SCREEN_WIDTH; x += tileWidth) iShowImage(x + sx, sy, tileWidth, GROUND_LEVEL, floorTexture);
			iSetColor(0, 255, 255); iLine(0, GROUND_LEVEL + sy, SCREEN_WIDTH, GROUND_LEVEL + sy);

			for (size_t i = 0; i < coinCluster.size(); i++) if (coinCluster[i].active) {
				float dx = coinCluster[i].x - cameraX;
				iSetColor(255, 215, 0); iFilledCircle((int)dx + sx, (int)coinCluster[i].y + sy, 9);
				iSetColor(255, 255, 255); iCircle((int)dx + sx, (int)coinCluster[i].y + sy, 10);
			}
			for (size_t i = 0; i < sparkles.size(); i++) if (sparkles[i].active) { iSetColor(255, 255, 180); iFilledCircle((int)sparkles[i].x + sx, (int)sparkles[i].y + sy, 2); }

			for (size_t i = 0; i < enemyBalls.size(); i++) enemyBalls[i].draw(sx, sy);
			for (size_t i = 0; i < playerFires.size(); i++) playerFires[i].draw(sx, sy);
			bot.draw(sx, sy);

			int hImg = hero.isJumping ? (hero.facingRight ? hero.sJumpR : hero.sJumpL) : (hero.isMoving ? (hero.facingRight ? hero.sRunR : hero.sRunL) : (hero.facingRight ? hero.sIdleR : hero.sIdleL));
			if (hero.isPunching) { iSetColor(255, 255, 0); iCircle(hero.screenX + (hero.facingRight ? 70 : 15) + sx, hero.y + 60 + sy, 30); }
			iSetColor(255, 255, 255);
			iShowImage(hero.screenX + sx, hero.y + sy, hero.width, hero.height, hImg);

			// UI (Adjusted for 1280x720)
			int uiY = SCREEN_HEIGHT - 60;
			char scoreTxt[200]; sprintf(scoreTxt, "SCORE: %d | DIST: %dm | %s %s", score, cameraX / 10, (score >= 25 ? "F:REFLECT" : ""), (score >= 50 ? "G:FIRE" : ""));
			iSetColor(255, 255, 255); iText(20, uiY + 40, scoreTxt, GLUT_BITMAP_HELVETICA_18);
			drawGradientHealthBar(20, uiY, (float)playerHealth, "PLAYER UNIT", true);
			if (!isVictory) drawGradientHealthBar(SCREEN_WIDTH - 220, uiY, (float)enemyHealth, "SYSTEM CORE", false);

			if (isGameOver || isVictory) {
				int cw = (int)(550 * zoomScale), ch = (int)(250 * zoomScale);
				int dx = (SCREEN_WIDTH - cw) / 2, dy = (SCREEN_HEIGHT - ch) / 2;
				iSetColor(0, 0, 0); iFilledRectangle(dx, dy, cw, ch);
				iSetColor(255, 255, 255); iRectangle(dx, dy, cw, ch);
				if (zoomScale > 0.85f) {
					iSetColor(isVictory ? 0 : 255, isVictory ? 255 : 50, 0);
					iText(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 + 30, isVictory ? "VICTORY!" : "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
					iSetColor(255, 255, 255);
					iText(SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 20, isVictory ? "System Defeated. Press SPACE to advance." : "System Overload. Press SPACE to retry.", GLUT_BITMAP_HELVETICA_18);
				}
			}
		}
	}

	void keyboard(unsigned char key) {
		if (isIntroActive && (key == 13 || key == 10)) advanceDialogue();

		// Handle Game Over / Victory Screen Exits
		if ((isGameOver || isVictory) && zoomScale > 0.85f && key == ' ') {
			if (isVictory) {
				::unlockedLevel = 4; // Unlock the grand finale!
				saveProgress(4);
				::currentState = LEVEL_4; // Transition to final boss
			}
			else {
				// Retry
				::currentState = MENU;
			}
		}
	}

	void init() {
		oriImg = iLoadImage("talkingafif.png");
		enemyImg = iLoadImage("enemy.png");
		bgImage = iLoadImage("final_level_bg.png");
		ballSprite = iLoadImage("ball.png");
		fireSprite = iLoadImage("fire.png");
		floorTexture = iLoadImage("floor_tile.png");

		// Reset all data
		score = 0; playerHealth = MAX_HEALTH; enemyHealth = MAX_HEALTH;
		musicTimer = 0; currentLetterIdx = 0; cameraX = 0;
		isGameOver = false; isVictory = false; zoomScale = 0.0f;
		isIntroActive = true; dialogueIndex = 0; isConversationActive = true;

		sparkles.clear(); coinCluster.clear(); enemyBalls.clear(); playerFires.clear();

		hero.init(); bot.init();
	}
}

// ---------------- INTERFACE ----------------
void initLevel3() { L3::init(); }
void drawLevel3() { L3::draw(); }
void updateLevel3() { L3::update(); }
void keyboardLevel3(unsigned char key) { L3::keyboard(key); }