#define _CRT_SECURE_NO_WARNINGS
#include "iGraphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// =========================================================
// GLOBAL MASTER STATE (Pura game er step track korar jonno)
// =========================================================
enum MasterState { GAME_MENU, GAME_PROLOGUE, GAME_LEVEL1 };
MasterState masterState = GAME_MENU;

// Screen er map, ekdom perfect foundation
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 600

// =========================================================
// SHARED UI ENGINE (Sob jaigai eita use hobe)
// =========================================================
void drawTransparentBox(int x, int y, int w, int h, int alpha) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4ub(0, 0, 0, alpha); // Aesthetic transparent vibe
	glBegin(GL_QUADS);
	glVertex2f(x, y); glVertex2f(x + w, y);
	glVertex2f(x + w, y + h); glVertex2f(x, y + h);
	glEnd();
	glDisable(GL_BLEND);
}

void drawDialogueUI(const char* speakerName, const char* line1, const char* line2) {
	drawTransparentBox(20, 20, 960, 130, 200);

	iSetColor(211, 47, 47); // Red color er nameplate, pura blemish treatment
	iFilledRectangle(20, 150, 180, 35);

	iSetColor(255, 255, 255);
	iText(40, 160, (char*)speakerName, GLUT_BITMAP_HELVETICA_18);
	iText(50, 100, (char*)line1, GLUT_BITMAP_9_BY_15);
	if (line2[0] != '\0') iText(50, 75, (char*)line2, GLUT_BITMAP_9_BY_15);

	iSetColor(150, 150, 150);
	iText(750, 35, "[ SPACE, SHIFT or CLICK ]", GLUT_BITMAP_HELVETICA_12);
}

// =========================================================
// PART 1: TITLE SCREEN VARIABLES (Cleanser Step)
// =========================================================
#define P5_RED 211, 47, 47
#define P5_BLACK 20, 20, 20
#define P5_CYAN 0, 255, 255
#define P5_GRAY 50, 50, 50

double bgStripeOffset = 0;
enum MenuState { MAIN_MENU, SETTINGS_MENU, CREDITS_MENU };
MenuState currentMenu = MAIN_MENU;
MenuState targetMenu = MAIN_MENU;

double jitter = 0;
int menuTimer = 0;
double glitchOffset = 0;

bool isTransitioning = false;
int slideOffset = 0;
int transitionSpeed = 50;

int selectedOption = 0;
const int TOTAL_OPTIONS = 5;
char menuText[5][20] = { "START GAME", "CONTINUE", "SETTINGS", "CREDITS", "QUIT GAME" };

int settingsOption = 0;
int fakeVolume = 80;
int fakeBrightness = 50;

bool upPressed = false; bool downPressed = false;
bool leftPressed = false; bool rightPressed = false;
bool enterPressed = false; bool backspacePressed = false;

// =========================================================
// PART 2: PROLOGUE VARIABLES (Toner Step)
// =========================================================
int scene = 1;
int scene1Step = 0; int scene2Step = 0; int scene5Step = 0;

char *scene1_bg = "entrancefinal.bmp";
char *scene2_bg = "passage2final.bmp";
char *scene3_bg = "frontidfinal.bmp";

int oritri_talk_img, afif_talk_img, samiha_talk_img;
int oritri_scene_img, afif_scene_img, samiha_scene_img;
int oriID, afifID, samID;
int masudSirImg, selectedCharScene5Img;

int selectedCharacterID = -1;
int currentSelection = 0;
bool shiftPressed = false;

const float SLOT_LEFT = 100.0f; const float SLOT_RIGHT = 450.0f;
const float OFF_LEFT = -400.0f; const float OFF_RIGHT = 1000.0f;

float oriX = OFF_LEFT;   float oriTarget = OFF_LEFT;
float afifX = OFF_RIGHT; float afifTarget = OFF_RIGHT;
float samX = OFF_RIGHT;  float samTarget = OFF_RIGHT;
float masudX = OFF_RIGHT; float masudTarget = OFF_RIGHT;
float playerX = OFF_LEFT; float playerTarget = OFF_LEFT;
float slideSpeed = 25.0f;

void drawPNG(float x, float y, int w, int h, int imgID, float brightness = 1.0f) {
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(brightness, brightness, brightness, 1.0f);
	iShowImage((int)x, (int)y, w, h, imgID);
	glDisable(GL_BLEND);
}

// =========================================================
// PART 3: LEVEL 1 VARIABLES (Heavy Moisturizer Step)
// =========================================================
#define ROOM_WIDTH 2000  
#define GROUND_LEVEL 100
#define MAX_PROJECTILES 8

int cameraX = 0;
enum GameState { PRE_BATTLE, PLAYING, POST_BATTLE_WIN, POST_BATTLE_LOSE };
GameState currentState = PRE_BATTLE;
int dialogueStep = 0;

int gameTime = 120;
int frameCounter = 0;
int playerHP = 300;

int imgPaper, imgPen, imgPencil, imgCable;
int imgClassroom;

struct Projectile {
	int x, y, width, height, speed, direction;
	bool active; int imgID;

	// Projectile fire korar logic
	void fire(int startX, int startY, int type, bool facingRight) {
		active = true; direction = facingRight ? 1 : -1;
		x = facingRight ? startX + 100 : startX - 20;

		if (type == 0) { imgID = imgPaper; width = 60; height = 60; speed = 5; y = startY + 70 + (rand() % 40); }
		else if (type == 1) { imgID = imgPen; width = 45; height = 15; speed = 10; y = startY + 40; }
		else if (type == 2) { imgID = imgPencil; width = 50; height = 20; speed = 8; y = startY + 90; }
		else { imgID = imgCable; width = 65; height = 65; speed = 6; y = startY + 20 + (rand() % 80); }
	}

	void update() {
		if (!active) return;
		x += speed * direction;
		if (x < cameraX - 200 || x > cameraX + SCREEN_WIDTH + 200) active = false; // Screen er baire gele vanish
	}
	void draw() { if (active) iShowImage(x - cameraX, y, width, height, imgID); }
};

enum BossPhase { NORMAL, AGGRESSIVE, ENRAGED };
struct Player; extern Player hero;

struct Enemy {
	int x, y, width, height, speed;
	bool movingRight, facingRight, isMoving;
	int velocityY, gravity, jumpForce;
	bool isJumping; int landingTimer;

	int throwTimer, throwInterval; BossPhase phase;
	int imgIdleR, imgIdleL, imgRunR, imgRunL, imgJumpR, imgJumpL, imgFallR, imgFallL, imgLandR, imgLandL, imgThrowR, imgThrowL;
	Projectile ammo[MAX_PROJECTILES];

	void init(int sx, int sy) {
		x = sx; y = sy; width = 150; height = 225; speed = 2;
		movingRight = false; facingRight = false; isMoving = false;
		velocityY = 0; gravity = 1; jumpForce = 18;
		isJumping = false; landingTimer = 0;
		throwTimer = 0; throwInterval = 120; phase = NORMAL;

		imgIdleR = iLoadImage("teacher_idle_r.png"); imgIdleL = iLoadImage("teacher_idle_l.png");
		imgRunR = iLoadImage("teacher_run_r.png");   imgRunL = iLoadImage("teacher_run_l.png");
		imgJumpR = iLoadImage("teacher_jump_r.png"); imgJumpL = iLoadImage("teacher_jump_l.png");
		imgFallR = iLoadImage("teacher_fall_r.png"); imgFallL = iLoadImage("teacher_fall_l.png");
		imgLandR = iLoadImage("teacher_land_r.png"); imgLandL = iLoadImage("teacher_land_l.png");
		imgThrowR = iLoadImage("teacher_throw_r.png"); imgThrowL = iLoadImage("teacher_throw_l.png");
		for (int i = 0; i < MAX_PROJECTILES; i++) ammo[i].active = false;
	}
	void update();
	void draw() {
		int drawX = x - cameraX;
		if (throwTimer < 25 && currentState == PLAYING) iShowImage(drawX, y, width, height, facingRight ? imgThrowR : imgThrowL);
		else if (isJumping) {
			if (velocityY > 0) iShowImage(drawX, y, width, height, facingRight ? imgJumpR : imgJumpL);
			else iShowImage(drawX, y, width, height, facingRight ? imgFallR : imgFallL);
		}
		else if (landingTimer > 0) iShowImage(drawX, y, width, height, facingRight ? imgLandR : imgLandL);
		else if (isMoving && currentState == PLAYING) iShowImage(drawX, y, width, height, facingRight ? imgRunR : imgRunL);
		else iShowImage(drawX, y, width, height, facingRight ? imgIdleR : imgIdleL);

		for (int i = 0; i < MAX_PROJECTILES; i++) ammo[i].draw();
	}
};

struct Player {
	int x, y, width, height, velocityX, velocityY, gravity, jumpForce;
	bool isJumping, isMoving, facingRight; int landingTimer;
	int invincibilityFrames; bool isHit, isAttacking; int attackTimer;
	int imgIdleR, imgIdleL, imgRunR, imgRunL, imgJumpR, imgJumpL, imgFallR, imgFallL, imgLandR, imgLandL;

	void init(int sx, int sy) {
		x = sx; y = sy; width = 120; height = 180;
		velocityX = 0; velocityY = 0; gravity = 1; jumpForce = 22;
		isJumping = false; isMoving = false; facingRight = true;
		landingTimer = 0; invincibilityFrames = 0; isHit = false; isAttacking = false; attackTimer = 0;

		imgIdleR = iLoadImage("player_idle_r.png"); imgIdleL = iLoadImage("player_idle_l.png");
		imgRunR = iLoadImage("player_run_r.png");  imgRunL = iLoadImage("player_run_l.png");
		imgJumpR = iLoadImage("player_jump_up_r.png"); imgJumpL = iLoadImage("player_jump_up_l.png");
		imgFallR = iLoadImage("player_fall_r.png"); imgFallL = iLoadImage("player_fall_l.png");
		imgLandR = iLoadImage("player_land_r.png"); imgLandL = iLoadImage("player_land_l.png");
	}

	void takeDamage(int amount) {
		if (invincibilityFrames == 0) { // I-frames for healthy skin barrier
			playerHP -= amount; if (playerHP < 0) playerHP = 0;
			invincibilityFrames = 90;
		}
	}

	void update() {
		isMoving = false; velocityX = 0;

		if (GetAsyncKeyState('D') || GetAsyncKeyState(VK_RIGHT)) { velocityX = 5; facingRight = true; isMoving = true; }
		else if (GetAsyncKeyState('A') || GetAsyncKeyState(VK_LEFT)) { velocityX = -5; facingRight = false; isMoving = true; }

		if ((GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState(VK_UP)) && !isJumping) {
			velocityY = jumpForce; isJumping = true; landingTimer = 0;
		}
		if (GetAsyncKeyState('F') && attackTimer == 0) { isAttacking = true; attackTimer = 20; }

		x += velocityX; y += velocityY;

		if (y > GROUND_LEVEL) { velocityY -= gravity; isJumping = true; }
		else { if (isJumping) landingTimer = 8; y = GROUND_LEVEL; velocityY = 0; isJumping = false; }

		if (landingTimer > 0) landingTimer--;
		if (invincibilityFrames > 0) invincibilityFrames--;
		if (attackTimer > 0) { attackTimer--; if (attackTimer == 0) isAttacking = false; }

		if (x < 0) x = 0;
		if (x > ROOM_WIDTH - width) x = ROOM_WIDTH - width;

		cameraX = x - (SCREEN_WIDTH / 2) + (width / 2); // Camera track kortese
		if (cameraX < 0) cameraX = 0;
		if (cameraX > ROOM_WIDTH - SCREEN_WIDTH) cameraX = ROOM_WIDTH - SCREEN_WIDTH;
	}

	void draw() {
		if (invincibilityFrames > 0 && (invincibilityFrames / 5) % 2 == 0) return;
		int drawX = x - cameraX;

		if (isJumping) {
			if (velocityY > 0) iShowImage(drawX, y, width, height, facingRight ? imgJumpR : imgJumpL);
			else iShowImage(drawX, y, width, height, facingRight ? imgFallR : imgFallL);
		}
		else if (isMoving && currentState == PLAYING) iShowImage(drawX, y, width, height, facingRight ? imgRunR : imgRunL);
		else if (landingTimer > 0) iShowImage(drawX, y, width, height, facingRight ? imgLandR : imgLandL);
		else iShowImage(drawX, y, width, height, facingRight ? imgIdleR : imgIdleL);

		if (isAttacking && currentState == PLAYING) {
			iSetColor(0, 255, 255);
			if (facingRight) {
				double slashX[] = { (double)drawX + width, (double)drawX + width + 60, (double)drawX + width + 75, (double)drawX + width + 15 };
				double slashY[] = { (double)y + 30, (double)y + 120, (double)y + 180, (double)y + 90 };
				iFilledPolygon(slashX, slashY, 4);
			}
			else {
				double slashX[] = { (double)drawX, (double)drawX - 60, (double)drawX - 75, (double)drawX - 15 };
				double slashY[] = { (double)y + 30, (double)y + 120, (double)y + 180, (double)y + 90 };
				iFilledPolygon(slashX, slashY, 4);
			}
		}
	}
};

Player hero; Enemy masudSir;

void Enemy::update() {
	if (gameTime <= 40) { phase = ENRAGED; speed = 5; throwInterval = 40; }
	else if (gameTime <= 80) { phase = AGGRESSIVE; speed = 3; throwInterval = 70; }

	facingRight = (hero.x > x);
	int distance = abs(hero.x - x);
	int optimalDistance = 450; int buffer = 50;
	isMoving = false;

	// Sir er distance kiting logic (personal space maintain kore)
	if (distance > optimalDistance + buffer) { x += facingRight ? speed : -speed; isMoving = true; }
	else if (distance < optimalDistance - buffer) { x += facingRight ? -speed : speed; isMoving = true; }

	if (x < 0) x = 0; if (x > ROOM_WIDTH - width) x = ROOM_WIDTH - width;

	throwTimer++;
	if (throwTimer >= throwInterval) {
		for (int i = 0; i < MAX_PROJECTILES; i++) {
			if (!ammo[i].active) {
				ammo[i].fire(x, y, rand() % 4, facingRight);
				if (phase == ENRAGED && !isJumping) { velocityY = jumpForce; isJumping = true; landingTimer = 0; }
				break;
			}
		}
		throwTimer = 0;
	}

	y += velocityY;
	if (y > GROUND_LEVEL) { velocityY -= gravity; isJumping = true; }
	else { if (isJumping) landingTimer = 8; y = GROUND_LEVEL; velocityY = 0; isJumping = false; }
	if (landingTimer > 0) landingTimer--;
	for (int i = 0; i < MAX_PROJECTILES; i++) ammo[i].update();
}

void resetLevel1() {
	playerHP = 300; gameTime = 120;
	hero.init(100, GROUND_LEVEL); masudSir.init(1500, GROUND_LEVEL);
	cameraX = 0; currentState = PRE_BATTLE; dialogueStep = 0;
}

// =========================================================
// RENDER MODULES (Menu, Prologue, Level 1)
// =========================================================
void drawGlitchTitle(int x, int y, const char* text) {
	iSetColor(P5_CYAN); iText(x - glitchOffset, y, (char*)text, GLUT_BITMAP_TIMES_ROMAN_24);
	iSetColor(P5_RED); iText(x + glitchOffset, y, (char*)text, GLUT_BITMAP_TIMES_ROMAN_24);
	iSetColor(255, 255, 255); iText(x, y, (char*)text, GLUT_BITMAP_TIMES_ROMAN_24);
}

void drawMenuSystem() {
	iSetColor(P5_BLACK); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	int offset = slideOffset;

	if (currentMenu == MAIN_MENU) {
		iSetColor(10, 10, 10);
		for (int i = -100; i < SCREEN_WIDTH; i += 100) {
			double stripeX[] = { i + bgStripeOffset + offset, i + 30 + bgStripeOffset + offset, i - 100 + 30 + bgStripeOffset + offset, i - 100 + bgStripeOffset + offset };
			double stripeY[] = { 0, 0, 600, 600 }; iFilledPolygon(stripeX, stripeY, 4);
		}
		iSetColor(P5_RED);
		double bgX[] = { 0 + offset, 450 + offset, 300 + offset, 0 + offset };
		double bgY[] = { 0, 0, 600, 600 }; iFilledPolygon(bgX, bgY, 4);
		iSetColor(P5_GRAY);
		double blockX[] = { 480 + offset, 950 + offset, 980 + offset, 510 + offset };
		double blockY[] = { 320, 320, 400, 400 }; iFilledPolygon(blockX, blockY, 4);

		drawGlitchTitle(530 + offset, 350, "RUNTIME ERROR");
		iSetColor(200, 200, 200); iText(630 + offset, 290, "SYSTEM FAILURE IMMINENT", GLUT_BITMAP_HELVETICA_12);

		double breathPulse = (sin(menuTimer * 0.1) + 1) * 5;
		for (int i = 0; i < TOTAL_OPTIONS; i++) {
			int baseY = 400 - (i * 60);
			double expandX = (i == selectedOption) ? (40 + breathPulse) : 0;
			double pushX = (i == selectedOption) ? (30 + (breathPulse / 2)) : 0;

			if (i == selectedOption) iSetColor(P5_CYAN); else iSetColor(P5_BLACK);
			double btnX[] = { 50 + jitter + pushX + offset, 300 + jitter + pushX + expandX + offset, 270 + jitter + pushX + expandX + offset, 20 + jitter + pushX + offset };
			double btnY[] = { baseY, baseY, baseY + 45, baseY + 45 }; iFilledPolygon(btnX, btnY, 4);

			if (i == selectedOption) iSetColor(P5_BLACK); else iSetColor(255, 255, 255);
			iText(70 + jitter + pushX + offset, baseY + 15, menuText[i], GLUT_BITMAP_HELVETICA_18);
		}
	}
	else if (currentMenu == SETTINGS_MENU) {
		drawGlitchTitle(100 + offset, 500, "SYSTEM SETTINGS");
		char options[3][20] = { "MASTER VOLUME", "BRIGHTNESS", "RETURN" };
		for (int i = 0; i < 3; i++) {
			int baseY = 350 - (i * 100);
			if (i == settingsOption) { iSetColor(P5_CYAN); iText(80 + jitter + offset, baseY, ">", GLUT_BITMAP_TIMES_ROMAN_24); iSetColor(255, 255, 255); }
			else iSetColor(150, 150, 150);
			iText(120 + offset, baseY, options[i], GLUT_BITMAP_HELVETICA_18);

			if (i < 2) {
				iSetColor(P5_GRAY); iFilledRectangle(400 + offset, baseY - 5, 300, 20);
				iSetColor(P5_CYAN); iFilledRectangle(400 + offset, baseY - 5, (i == 0) ? (fakeVolume * 3) : (fakeBrightness * 3), 20);
				iSetColor(255, 255, 255);
				char valText[10]; sprintf(valText, "%d%%", (i == 0) ? fakeVolume : fakeBrightness);
				iText(720 + offset, baseY, valText, GLUT_BITMAP_HELVETICA_18);
			}
		}
	}
	else if (currentMenu == CREDITS_MENU) {
		drawGlitchTitle(100 + offset, 500, "MASTER FORMULATORS");
		iSetColor(P5_CYAN); iFilledRectangle(100 + offset, 380, 5, 40); iSetColor(255, 255, 255); iText(120 + offset, 400, "Afif Bin Zaman", GLUT_BITMAP_TIMES_ROMAN_24); iSetColor(P5_RED); iText(120 + offset, 380, "ID: 00724205101039", GLUT_BITMAP_HELVETICA_12);
		iSetColor(P5_CYAN); iFilledRectangle(100 + offset, 280, 5, 40); iSetColor(255, 255, 255); iText(120 + offset, 300, "Ismat Erena Siddique", GLUT_BITMAP_TIMES_ROMAN_24); iSetColor(P5_RED); iText(120 + offset, 280, "ID: 00724205101059", GLUT_BITMAP_HELVETICA_12);
		iSetColor(P5_CYAN); iFilledRectangle(100 + offset, 180, 5, 40); iSetColor(255, 255, 255); iText(120 + offset, 200, "Samiha Ali", GLUT_BITMAP_TIMES_ROMAN_24); iSetColor(P5_RED); iText(120 + offset, 180, "ID: 00724205101061", GLUT_BITMAP_HELVETICA_12);
	}
}

void drawPrologueSystem() {
	int charHeight = SCREEN_HEIGHT / 2 + 100; int charWidth = charHeight * 0.66; int yPos = 150;

	if (scene == 1) {
		iShowBMP(0, 0, scene1_bg);
		int activeSpeaker = -1; char speaker[50] = ""; char line1[100] = ""; char line2[100] = "";

		if (scene1Step == 0) { oriTarget = SLOT_LEFT; activeSpeaker = 0; strcpy(speaker, "Oritri"); strcpy(line1, "Welcome to AUST! You're now part of the species"); strcpy(line2, "that survives on caffeine, code, and last-minute panic."); }
		else if (scene1Step == 1) { afifTarget = SLOT_RIGHT; activeSpeaker = 1; strcpy(speaker, "Afif"); strcpy(line1, "Enjoy the fresh air while it lasts."); strcpy(line2, "Soon, the only breeze is your laptop fan overheating."); }
		else if (scene1Step == 2) { activeSpeaker = 0; strcpy(speaker, "Oritri"); strcpy(line1, "Exactly. You enter as a human and leave as someone"); strcpy(line2, "who gets emotionally offended by missing semicolons."); }
		else if (scene1Step == 3) { activeSpeaker = 1; strcpy(speaker, "Afif"); strcpy(line1, "Wait until CSE 1205. You'll be dreaming in Java."); strcpy(line2, "And don't even get me started on the lab assignments."); }
		else if (scene1Step == 4) { activeSpeaker = 0; strcpy(speaker, "Oritri"); strcpy(line1, "Oh, don't scare the newcomer! Though..."); strcpy(line2, "Discrete Math might actually break your spirit a little."); }
		else if (scene1Step == 5) { afifTarget = OFF_RIGHT; samTarget = SLOT_RIGHT; activeSpeaker = 2; strcpy(speaker, "Samiha"); strcpy(line1, "Did someone say Discrete Math? I already have"); strcpy(line2, "three backup spreadsheets for the syllabus."); }
		else if (scene1Step == 6) { activeSpeaker = 0; strcpy(speaker, "Oritri"); strcpy(line1, "See? We survive by over-preparing..."); strcpy(line2, "Or completely ignoring reality until the night before."); }
		else if (scene1Step == 7) { activeSpeaker = 2; strcpy(speaker, "Samiha"); strcpy(line1, "A four-year psychological thriller."); strcpy(line2, "Now, before the semester chooses violence... lock in."); }

		drawPNG(oriX, yPos, charWidth, charHeight, oritri_talk_img, (activeSpeaker == 0) ? 1.0f : 0.4f);
		drawPNG(afifX, yPos, charWidth, charHeight, afif_talk_img, (activeSpeaker == 1) ? 1.0f : 0.4f);
		drawPNG(samX, yPos, charWidth, charHeight, samiha_talk_img, (activeSpeaker == 2) ? 1.0f : 0.4f);
		drawDialogueUI(speaker, line1, line2);
	}
	else if (scene == 2) {
		iShowBMP(0, 0, scene2_bg);
		int mH = SCREEN_HEIGHT / 1.75; int mW = mH * 0.66; int bH = SCREEN_HEIGHT;
		if (scene2Step == 0) { drawPNG(100, 0, mW, mH, oritri_scene_img); drawPNG(300, 0, mW, mH, afif_scene_img); drawPNG(500, 0, mW, mH, samiha_scene_img); drawDialogueUI("System", "Welcome to AUST! Let's get to know the team.", ""); }
		else if (scene2Step == 1) { drawPNG(150, 0, mW, bH, oritri_scene_img); drawDialogueUI("Oritri", "Runs on panic, caffeine, and last minute confidence.", "Believes every bug can be fixed if stared at long enough."); }
		else if (scene2Step == 2) { drawPNG(150, -50, mW, bH, afif_scene_img); drawDialogueUI("Afif", "Professional procrastinator turned emergency performer.", "Debugs best under extreme emotional pressure."); }
		else if (scene2Step == 3) { drawPNG(150, -50, mW, bH, samiha_scene_img); drawDialogueUI("Samiha", "Organized to the point where her backup files have backups.", "Finds mistakes faster than teachers find surprise quizzes."); }
		else if (scene2Step == 4) { drawPNG(100, 0, mW, bH, oritri_scene_img, 1.0f); drawPNG(300, 0, mW, mH, afif_scene_img, 0.4f); drawPNG(500, 0, mW, mH, samiha_scene_img, 0.4f); drawDialogueUI("Oritri", "Before the semester chooses violence...", "You must choose a character."); }
	}
	else if (scene == 3) {
		iShowBMP(0, 0, scene3_bg);
		int x[3] = { 200, 425, 650 }; // Shifted to center for 1000px screen!
		drawPNG(x[0], 140, 160, 270, oriID); drawPNG(x[1], 140, 160, 270, afifID); drawPNG(x[2], 140, 160, 270, samID);
		if (selectedCharacterID == -1) { iSetColor(0, 255, 0); iRectangle(x[currentSelection], 140, 160, 270); }
		drawDialogueUI("System", "Use LEFT/RIGHT to select your avatar.", "Press SHIFT to confirm your choice.");
	}
	else if (scene == 4) {
		iShowBMP(0, 0, scene3_bg);
		int displayImg; char nameText[50];
		if (selectedCharacterID == 0) { displayImg = oriID; strcpy(nameText, "ORITRI SELECTED"); }
		else if (selectedCharacterID == 1) { displayImg = afifID; strcpy(nameText, "AFIF SELECTED"); }
		else { displayImg = samID; strcpy(nameText, "SAMIHA SELECTED"); }
		drawPNG(400, 150, 200, 350, displayImg); // Centered
		drawDialogueUI("System", nameText, "Press SHIFT or SPACE to enter the classroom.");
	}
	else if (scene == 5) {
		iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, imgClassroom);
		int activeSpeaker = -1; playerTarget = SLOT_LEFT; masudTarget = SLOT_RIGHT + 200;

		if (scene5Step == 0) { activeSpeaker = 1; drawDialogueUI("Masud Sir", "So, another batch of optimists. Do you even know", "what a Null Pointer Exception is?"); }
		else if (scene5Step == 1) { activeSpeaker = 0; drawDialogueUI("You", "I've watched three CodeBeauty tutorials.", "I am invincible."); }
		else if (scene5Step == 2) { activeSpeaker = 1; drawDialogueUI("Masud Sir", "Tutorials won't save you from my midterm.", "Let's see if your reflexes match your confidence."); }
		else if (scene5Step == 3) { activeSpeaker = 1; drawDialogueUI("Masud Sir", "Prepare yourself.", ""); }

		drawPNG(playerX, 100, 200, 350, selectedCharScene5Img, (activeSpeaker == 0) ? 1.0f : 0.4f);
		drawPNG(masudX, 100, 200, 350, masudSirImg, (activeSpeaker == 1) ? 1.0f : 0.4f);
	}
}

void drawHUD() {
	iSetColor(20, 20, 20); iFilledRectangle(0, 530, SCREEN_WIDTH, 70);
	iSetColor(255, 255, 255); iText(20, 560, "HP:", GLUT_BITMAP_HELVETICA_18);
	iSetColor(50, 50, 50); iFilledRectangle(60, 555, 200, 20);
	if (playerHP > 30) iSetColor(0, 255, 255); else iSetColor(255, 50, 50);
	iFilledRectangle(60, 555, playerHP * 2, 20);

	char timeStr[20]; sprintf(timeStr, "%02d:%02d", gameTime / 60, gameTime % 60);
	iSetColor(255, 255, 255); iText(460, 555, timeStr, GLUT_BITMAP_TIMES_ROMAN_24);

	char phaseStr[30];
	if (masudSir.phase == NORMAL) { iSetColor(150, 150, 150); sprintf(phaseStr, "PHASE: LECTURE"); }
	else if (masudSir.phase == AGGRESSIVE) { iSetColor(255, 165, 0); sprintf(phaseStr, "PHASE: MIDTERM"); }
	else { iSetColor(255, 0, 0); sprintf(phaseStr, "PHASE: FINAL EXAM"); }
	iText(750, 555, phaseStr, GLUT_BITMAP_HELVETICA_18);
}

void drawLevel1System() {
	iShowImage(-cameraX, 0, ROOM_WIDTH, SCREEN_HEIGHT, imgClassroom);
	masudSir.draw();
	hero.draw();

	if (currentState == PLAYING) drawHUD();
	else if (currentState == PRE_BATTLE) {
		if (dialogueStep == 0) drawDialogueUI("Masud Sir", "Ah, the fresh batch of AUST CSE students.", "You look... unprepared.");
		else if (dialogueStep == 1) drawDialogueUI("Player", "I've watched three CodeBeauty tutorials.", "I am invincible.");
		else if (dialogueStep == 2) drawDialogueUI("Masud Sir", "Tutorials won't save you from a null pointer exception.", "Let's test your reflexes.");
	}
	else if (currentState == POST_BATTLE_WIN) {
		if (dialogueStep == 0) drawDialogueUI("Masud Sir", "Hmph. No syntax errors... this time.", "You pass.");
		else if (dialogueStep == 1) drawDialogueUI("Player", "Is it over? Can I sleep now?", "");
		else if (dialogueStep == 2) drawDialogueUI("Masud Sir", "For now. But remember...", "Discrete Math is next semester.");
	}
	else if (currentState == POST_BATTLE_LOSE) {
		if (dialogueStep == 0) drawDialogueUI("Masud Sir", "Academic Dismissal. Your stamina is zero.", "And your code is a mess.");
		else if (dialogueStep == 1) drawDialogueUI("Player", "I... I just forgot a semicolon...", "");
		else if (dialogueStep == 2) drawDialogueUI("Masud Sir", "See you next semester in the retake batch.", "");
	}
}

void iDraw() {
	iClear();
	if (masterState == GAME_MENU) drawMenuSystem();
	else if (masterState == GAME_PROLOGUE) drawPrologueSystem();
	else if (masterState == GAME_LEVEL1) drawLevel1System();
}

// =========================================================
// LOGIC CONTROLLERS (Sob logic ekhane update hoy)
// =========================================================
bool checkCollision(int px, int py, int pw, int ph, int ex, int ey, int ew, int eh) {
	int hitX = px + 20; int hitW = pw - 40; int hitY = py; int hitH = ph - 10;
	return (hitX < ex + ew && hitX + hitW > ex && hitY < ey + eh && hitY + hitH > ey);
}

// Eita holo amader forward declaration (Heads-up for the compiler!)
void advanceNarrative();
void fixedUpdate() {
	if (masterState == GAME_MENU) {
		menuTimer++;
		if (menuTimer % 8 == 0) jitter = (jitter == 0) ? 3 : 0;
		if (menuTimer % 3 == 0) glitchOffset = (rand() % 5);
		bgStripeOffset += 1.5; if (bgStripeOffset > 100) bgStripeOffset = 0;

		if (isTransitioning) {
			slideOffset -= transitionSpeed;
			if (slideOffset <= -1000) {
				// Menu theke prologue e shift korar logic!
				if (targetMenu == MAIN_MENU && selectedOption == 0 && currentMenu == MAIN_MENU) {
					masterState = GAME_PROLOGUE;
					scene = 1; scene1Step = 0; // Fresh start for prologue
				}
				else {
					currentMenu = targetMenu;
				}
				slideOffset = 1000; isTransitioning = false;
			}
		}
		else if (slideOffset > 0) {
			slideOffset -= transitionSpeed; if (slideOffset < 0) slideOffset = 0;
		}

		if (slideOffset == 0) {
			if (GetAsyncKeyState(VK_DOWN)) { if (!downPressed) { if (currentMenu == MAIN_MENU) { selectedOption++; if (selectedOption >= TOTAL_OPTIONS) selectedOption = 0; } else if (currentMenu == SETTINGS_MENU) { settingsOption++; if (settingsOption > 2) settingsOption = 0; } downPressed = true; } }
			else downPressed = false;
			if (GetAsyncKeyState(VK_UP)) { if (!upPressed) { if (currentMenu == MAIN_MENU) { selectedOption--; if (selectedOption < 0) selectedOption = TOTAL_OPTIONS - 1; } else if (currentMenu == SETTINGS_MENU) { settingsOption--; if (settingsOption < 0) settingsOption = 2; } upPressed = true; } }
			else upPressed = false;
			if (GetAsyncKeyState(VK_LEFT)) { if (!leftPressed && currentMenu == SETTINGS_MENU) { if (settingsOption == 0 && fakeVolume > 0) fakeVolume -= 10; if (settingsOption == 1 && fakeBrightness > 0) fakeBrightness -= 10; leftPressed = true; } }
			else leftPressed = false;
			if (GetAsyncKeyState(VK_RIGHT)) { if (!rightPressed && currentMenu == SETTINGS_MENU) { if (settingsOption == 0 && fakeVolume < 100) fakeVolume += 10; if (settingsOption == 1 && fakeBrightness < 100) fakeBrightness += 10; rightPressed = true; } }
			else rightPressed = false;

			if (GetAsyncKeyState(VK_RETURN)) {
				if (!enterPressed) {
					if (currentMenu == MAIN_MENU) {
						// "START GAME" chaple prologue shuru hobe
						if (selectedOption == 0) { isTransitioning = true; }
						else if (selectedOption == 2) { targetMenu = SETTINGS_MENU; isTransitioning = true; }
						else if (selectedOption == 3) { targetMenu = CREDITS_MENU; isTransitioning = true; }
						else if (selectedOption == 4) exit(0);
					}
					else if (currentMenu == SETTINGS_MENU && settingsOption == 2) { targetMenu = MAIN_MENU; isTransitioning = true; }
					enterPressed = true;
				}
			}
			else enterPressed = false;

			if (GetAsyncKeyState(VK_BACK)) { if (!backspacePressed && (currentMenu == SETTINGS_MENU || currentMenu == CREDITS_MENU)) { targetMenu = MAIN_MENU; isTransitioning = true; backspacePressed = true; } }
			else backspacePressed = false;
		}
	}
	else if (masterState == GAME_PROLOGUE) {
		if (oriX < oriTarget) { oriX += slideSpeed; if (oriX > oriTarget) oriX = oriTarget; }
		else if (oriX > oriTarget) { oriX -= slideSpeed; if (oriX < oriTarget) oriX = oriTarget; }
		if (afifX < afifTarget) { afifX += slideSpeed; if (afifX > afifTarget) afifX = afifTarget; }
		else if (afifX > afifTarget) { afifX -= slideSpeed; if (afifX < afifTarget) afifX = afifTarget; }
		if (samX < samTarget) { samX += slideSpeed; if (samX > samTarget) samX = samTarget; }
		else if (samX > samTarget) { samX -= slideSpeed; if (samX < samTarget) samX = samTarget; }
		if (masudX < masudTarget) { masudX += slideSpeed; if (masudX > masudTarget) masudX = masudTarget; }
		else if (masudX > masudTarget) { masudX -= slideSpeed; if (masudX < masudTarget) masudX = masudTarget; }
		if (playerX < playerTarget) { playerX += slideSpeed; if (playerX > playerTarget) playerX = playerTarget; }
		else if (playerX > playerTarget) { playerX -= slideSpeed; if (playerX < playerTarget) playerX = playerTarget; }

		if (scene == 3) {
			if (GetAsyncKeyState(VK_LEFT) & 0x8000) { if (!leftPressed) { leftPressed = true; currentSelection--; if (currentSelection < 0) currentSelection = 2; } }
			else leftPressed = false;
			if (GetAsyncKeyState(VK_RIGHT) & 0x8000) { if (!rightPressed) { rightPressed = true; currentSelection++; if (currentSelection > 2) currentSelection = 0; } }
			else rightPressed = false;
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000 || GetAsyncKeyState(VK_LSHIFT) & 0x8000) { if (!shiftPressed) { shiftPressed = true; selectedCharacterID = currentSelection; scene = 4; } }
			else shiftPressed = false;
		}
		// Ekhane notun treatment add korlam for Scene 4 and 5 Shift support!
		else if (scene == 4 || scene == 5) {
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000 || GetAsyncKeyState(VK_LSHIFT) & 0x8000) {
				if (!shiftPressed) { shiftPressed = true; advanceNarrative(); }
			}
			else shiftPressed = false;
		}
	}
	else if (masterState == GAME_LEVEL1) {
		if (currentState == PLAYING) {
			hero.update(); masudSir.update();

			// Blemish remove korar jonno collision check
			for (int i = 0; i < MAX_PROJECTILES; i++) {
				if (masudSir.ammo[i].active) {
					if (hero.isAttacking) {
						int slashX = hero.facingRight ? hero.x + hero.width : hero.x - 75; int slashW = 75;
						if (checkCollision(slashX, hero.y, slashW, hero.height, masudSir.ammo[i].x, masudSir.ammo[i].y, masudSir.ammo[i].width, masudSir.ammo[i].height)) {
							masudSir.ammo[i].active = false; continue;
						}
					}
					if (checkCollision(hero.x, hero.y, hero.width, hero.height, masudSir.ammo[i].x, masudSir.ammo[i].y, masudSir.ammo[i].width, masudSir.ammo[i].height)) {
						hero.takeDamage(20); masudSir.ammo[i].active = false;
					}
				}
			}

			if (playerHP <= 0) { dialogueStep = 0; currentState = POST_BATTLE_LOSE; }

			frameCounter++;
			if (frameCounter >= 60) {
				gameTime--; frameCounter = 0;
				if (gameTime <= 0) { dialogueStep = 0; currentState = POST_BATTLE_WIN; }
			}
		}
	}
}

// =========================================================
// INPUT HANDLERS (Space, Click, Shift diye text agabe)
// =========================================================
void advanceNarrative() {
	if (masterState == GAME_PROLOGUE) {
		if (scene == 1) { scene1Step++; if (scene1Step > 7) { scene = 2; scene2Step = 0; } }
		else if (scene == 2) { scene2Step++; if (scene2Step > 4) scene = 3; }
		else if (scene == 4) {
			if (selectedCharacterID == 0) selectedCharScene5Img = oriID;
			else if (selectedCharacterID == 1) selectedCharScene5Img = afifID;
			else selectedCharScene5Img = samID;
			scene = 5; scene5Step = 0;
		}
		else if (scene == 5) {
			scene5Step++;
			if (scene5Step > 3) {
				// Prologue theke Level 1 e transition!
				masterState = GAME_LEVEL1;
				resetLevel1(); // Clean skin for the boss fight
			}
		}
	}
	else if (masterState == GAME_LEVEL1) {
		if (currentState == PRE_BATTLE) {
			dialogueStep++; if (dialogueStep > 2) currentState = PLAYING;
		}
		else if (currentState == POST_BATTLE_WIN || currentState == POST_BATTLE_LOSE) {
			dialogueStep++;
			if (dialogueStep > 2) {
				// Game shesh hole abar Main Menu te back korbe!
				masterState = GAME_MENU;
				currentMenu = MAIN_MENU;
				slideOffset = 1000;
				isTransitioning = false;
			}
		}
	}
}

void iMouse(int button, int state, int mx, int my) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (masterState == GAME_MENU) {
			// Menu er mouse logic
			if (!isTransitioning && slideOffset == 0 && currentMenu == MAIN_MENU) {
				for (int i = 0; i < TOTAL_OPTIONS; i++) {
					int baseY = 400 - (i * 60);
					if (mx > 50 && mx < 340 && my > baseY && my < baseY + 45) {
						selectedOption = i;
						if (i == 0) { isTransitioning = true; } // Start Game
						else if (i == 2) { targetMenu = SETTINGS_MENU; isTransitioning = true; }
						else if (i == 3) { targetMenu = CREDITS_MENU; isTransitioning = true; }
						else if (i == 4) exit(0);
					}
				}
			}
			else if (!isTransitioning && slideOffset == 0 && currentMenu == SETTINGS_MENU) {
				int baseY = 350 - (2 * 100);
				if (mx > 80 && mx < 350 && my > baseY && my < baseY + 30) { targetMenu = MAIN_MENU; isTransitioning = true; }
			}
			else if (!isTransitioning && slideOffset == 0 && currentMenu == CREDITS_MENU) {
				targetMenu = MAIN_MENU; isTransitioning = true;
			}
		}
		else if (masterState == GAME_PROLOGUE && scene != 3) advanceNarrative();
		else if (masterState == GAME_LEVEL1 && currentState != PLAYING) advanceNarrative();
	}
}

void iKeyboard(unsigned char key) {
	if (key == ' ') {
		if (masterState == GAME_PROLOGUE && scene != 3) advanceNarrative();
		else if (masterState == GAME_LEVEL1 && currentState != PLAYING) advanceNarrative();
	}
}

void iPassiveMouseMove(int mx, int my) {
	if (masterState == GAME_MENU && !isTransitioning && slideOffset == 0) {
		if (currentMenu == MAIN_MENU) {
			for (int i = 0; i < TOTAL_OPTIONS; i++) {
				int baseY = 400 - (i * 60);
				if (mx > 50 && mx < 340 && my > baseY && my < baseY + 45) selectedOption = i;
			}
		}
		else if (currentMenu == SETTINGS_MENU) {
			for (int i = 0; i < 3; i++) {
				int baseY = 350 - (i * 100);
				if (mx > 80 && mx < 350 && my > baseY && my < baseY + 30) settingsOption = i;
			}
		}
	}
}

void iMouseMove(int mx, int my) {}

void iSpecialKeyboard(unsigned char key) {
	// Pura faka rakhlam, no extra chemicals
}

// =========================================================
// MAIN FUNCTION (Shob assets ekbare load korar jonno)
// =========================================================
int main() {
	iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "RUNTIME ERROR - The Complete Experience");

	// Prologue assets load kortesi bhai
	oritri_talk_img = iLoadImage("talkingori.png");
	afif_talk_img = iLoadImage("talkingafif.png");
	samiha_talk_img = iLoadImage("talkingsam.png");
	oritri_scene_img = iLoadImage("scene2ori.png");
	afif_scene_img = iLoadImage("scene2afif.png");
	samiha_scene_img = iLoadImage("scene2sam.png");
	oriID = iLoadImage("oriID.png");
	afifID = iLoadImage("afifID.png");
	samID = iLoadImage("samID.png");
	masudSirImg = iLoadImage("masudSirnew.png");

	// Level 1 er blemish/projectiles load kortesi
	imgClassroom = iLoadImage("classroom.png");
	imgPaper = iLoadImage("deadline_paper.png");
	imgPen = iLoadImage("pen.png");
	imgPencil = iLoadImage("pencil.png");
	imgCable = iLoadImage("cable.png");

	iSetTimer(16, fixedUpdate); // 60 FPS update loop
	iStart();
	return 0;
}