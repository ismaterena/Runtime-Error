#define _CRT_SECURE_NO_WARNINGS
#include "iGraphics.h"
#include "Globals.h"
#include "Level4.h"
#include <stdio.h>
#include <stdlib.h>

namespace L4 {
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

	// --- Game States ---
	enum L4State {
		INTRO_CONVERSATION,
		TRANSITION_FLICKER,
		EXPLORING,
		QUIZ,
		GAME_OVER,
		WIN_MESSAGE,
		COOLDOWN,
		FINAL_WIN,
		FINAL_CONVERSATION
	};
	L4State l4State = INTRO_CONVERSATION;

	// --- Image IDs ---
	int player_front, player_back, player_left, player_right;
	int enemySprite, it_center_bg, door_view_bg, glowingFile;
	int currentBackground;

	// --- Intro Dialogue Variables ---
	int dialogueIndex = 0;
	const int totalLines = 19;
	bool showEnemyIntro = false;

	// --- Flicker Effect Variables ---
	bool flickerToggle = false;
	float flickerDuration = 0;

	// --- Frame-Based Timer Variables ---
	int frameCounter = 0;
	int modeTimer = 0; // Replaces time_t startTime
	int quizTimer = 5, winTimer = 5, finalWinTimer = 5, convoTimer = 2;
	int currentQuizIndex = 0, currentPhase = 0, convoStep = 0;
	int offsetX = 0, offsetY = 0;

	// --- Dialogue Data ---
	char* speakers[] = {
		"Player", "Player", "Player", "Shadow", "Player",
		"Shadow", "Shadow", "Player", "Shadow", "Shadow",
		"Shadow", "Shadow", "Shadow", "Player", "Shadow",
		"Shadow", "Player", "Shadow", "Shadow"
	};

	char* lines[] = {
		"Why am I still here...?", "Everyone left hours ago.",
		"And what is this 'missing file' warning?", "You're late.",
		"Who's there?!", "Relax.", "If I wanted you gone, you wouldn't be standing.",
		"What are you?", "A fragment.", "A leftover.", "A guide... if you're smart enough.",
		"You're looking for a file.", "So is the system.", "Where is it?",
		"Not so easy.", "You'll have to earn it.", "How?",
		"Answer correctly... and I'll guide you.", "Fail... and the system rejects you."
	};

	// --- Quiz Banks ---
	struct Question {
		char text[100];
		char options[3][50];
		int correctOption;
	};

	Question cQuizBank[5] = {
		{ "Size of int in C?", { "A. 1 byte", "B. 2 or 4 bytes", "C. 8 bytes" }, 1 },
		{ "Which is a loop in C?", { "A. if", "B. while", "C. break" }, 1 },
		{ "Format specifier for float?", { "A. %d", "B. %f", "C. %c" }, 1 },
		{ "Ending character of a string?", { "A. \\0", "B. \\n", "C. \\t" }, 0 },
		{ "Which is a header file?", { "A. stdio.h", "B. main.cpp", "C. return" }, 0 }
	};

	Question javaQuizBank[5] = {
		{ "Java: Keyword for inheritance?", { "A. extends", "B. implements", "C. inherits" }, 0 },
		{ "Java: Default boolean value?", { "A. true", "B. false", "C. null" }, 1 },
		{ "Java: Which is primitive?", { "A. String", "B. int", "C. Array" }, 1 },
		{ "Java: Compiled file extension?", { "A. java", "B. class", "C. exe" }, 1 },
		{ "Java: Keyword for constant?", { "A. final", "B. static", "C. const" }, 0 }
	};

	Question igBank[5] = {
		{ "iGraphics: Draw a circle?", { "A. iCircle", "B. iDrawCircle", "C. iFilledCircle" }, 0 },
		{ "iGraphics: Load images?", { "A. iLoadImage", "B. iShowImage", "C. iReadImage" }, 0 },
		{ "iGraphics: Starts engine?", { "A. iStart", "B. iRun", "C. iInitialize" }, 0 },
		{ "iGraphics: Timer function?", { "A. iSetTimer", "B. iTimer", "C. iUpdate" }, 0 },
		{ "iGraphics: Clear screen?", { "A. iClear", "B. iDelete", "C. iReset" }, 0 }
	};

	// --- Player Structure ---
	struct Player {
		int x, y, width, height, speed, direction;
		void init(int startX, int startY) {
			x = startX; y = startY; width = 60; height = 90; speed = 7; direction = 0;
		}
		void update() {
			if (l4State != EXPLORING && l4State != COOLDOWN && l4State != FINAL_WIN) return;
			if (GetAsyncKeyState('W') || GetAsyncKeyState(VK_UP)) { y += speed; direction = 1; }
			else if (GetAsyncKeyState('S') || GetAsyncKeyState(VK_DOWN)) { y -= speed; direction = 0; }
			else if (GetAsyncKeyState('D') || GetAsyncKeyState(VK_RIGHT)) { x += speed; direction = 3; }
			else if (GetAsyncKeyState('A') || GetAsyncKeyState(VK_LEFT)) { x -= speed; direction = 2; }
			if (x < 0) x = 0; if (x > SCREEN_WIDTH - width) x = SCREEN_WIDTH - width;
			if (y < 0) y = 0; if (y > SCREEN_HEIGHT - height) y = SCREEN_HEIGHT - height;
		}
		void draw() {
			int dX = x + offsetX, dY = y + offsetY;
			if (direction == 0) iShowImage(dX, dY, width, height, player_front);
			else if (direction == 1) iShowImage(dX, dY, width, height, player_back);
			else if (direction == 2) iShowImage(dX, dY, width, height, player_left);
			else if (direction == 3) iShowImage(dX, dY, width, height, player_right);
		}
	} hero;

	void iDrawNeonText(int x, int y, char* str, void* font, int r, int g, int b) {
		iSetColor(r, g, b); iText(x - 1, y, str, font); iText(x + 1, y, str, font);
		iText(x, y - 1, str, font); iText(x, y + 1, str, font);
		iSetColor(255, 255, 255); iText(x, y, str, font);
	}

	void advanceIntro() {
		if (l4State == INTRO_CONVERSATION) {
			dialogueIndex++;
			if (dialogueIndex >= 3) showEnemyIntro = true;
			if (dialogueIndex >= totalLines) {
				dialogueIndex = totalLines - 1;
				l4State = TRANSITION_FLICKER;
			}
		}
	}

	void checkAnswer(int choice) {
		int correct = (currentPhase == 0) ? cQuizBank[currentQuizIndex].correctOption :
			(currentPhase == 1) ? javaQuizBank[currentQuizIndex].correctOption :
			igBank[currentQuizIndex].correctOption;
		if (choice == correct) { l4State = WIN_MESSAGE; winTimer = 5; }
		else { l4State = GAME_OVER; }
	}

	void draw() {
		iSetColor(0, 0, 0); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		// Scaled background logic
		if (currentBackground != -1) {
			iShowImage(offsetX, offsetY, SCREEN_WIDTH, SCREEN_HEIGHT, currentBackground);
		}

		if (l4State == INTRO_CONVERSATION) {
			iShowImage(250, 150, 200, 320, player_front); // Scaled for 1280x720
			if (showEnemyIntro) iShowImage(750, 50, 450, 600, enemySprite);

			glEnable(GL_BLEND); glColor4ub(15, 15, 20, 200);
			iFilledRectangle(340, 50, 600, 150); glDisable(GL_BLEND);

			iSetColor(255, 255, 255); iRectangle(340, 50, 600, 150);
			iSetColor(255, 255, 0); iText(360, 170, speakers[dialogueIndex], GLUT_BITMAP_HELVETICA_18);
			iSetColor(255, 255, 255); iText(360, 110, lines[dialogueIndex], GLUT_BITMAP_9_BY_15);
			iSetColor(150, 150, 150); iText(750, 60, "[SPACE TO ADVANCE]", GLUT_BITMAP_HELVETICA_12);
		}
		else if (l4State == TRANSITION_FLICKER) {
			if (flickerToggle) { iSetColor(0, 0, 0); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); }
			else { iSetColor(255, 0, 0); iText(540, 360, "SYSTEM ACTIVATED", GLUT_BITMAP_TIMES_ROMAN_24); }
		}
		else {
			// Main Gameplay Screen
			if (l4State == FINAL_CONVERSATION && rand() % 3 == 0) {
				offsetX = (rand() % 7) - 3; offsetY = (rand() % 7) - 3;
			}
			else { offsetX = 0; offsetY = 0; }

			hero.draw();

			if (l4State == QUIZ) {
				glEnable(GL_BLEND); glColor4ub(0, 0, 0, 180); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); glDisable(GL_BLEND);
				iShowImage(SCREEN_WIDTH / 2 + 150 + offsetX, 100 + offsetY, 400, 500, enemySprite);

				char* qText = (currentPhase == 0) ? cQuizBank[currentQuizIndex].text : (currentPhase == 1) ? javaQuizBank[currentQuizIndex].text : igBank[currentQuizIndex].text;
				iDrawNeonText(150, 550, qText, GLUT_BITMAP_HELVETICA_18, 0, 200, 255);

				char tStr[20]; sprintf(tStr, "Time: %d", quizTimer);
				iSetColor(255, 50, 50); iText(550, 550, tStr, GLUT_BITMAP_HELVETICA_18);

				for (int i = 0; i < 3; i++) {
					int optY = 450 - (i * 70);
					iSetColor(20, 20, 20); iFilledRectangle(140, optY - 10, 400, 50);
					iSetColor(200, 200, 200); iRectangle(140, optY - 10, 400, 50);
					char* rawOpt = (currentPhase == 0) ? cQuizBank[currentQuizIndex].options[i] : (currentPhase == 1) ? javaQuizBank[currentQuizIndex].options[i] : igBank[currentQuizIndex].options[i];
					iSetColor(255, 255, 255); iText(150, optY + 5, rawOpt, GLUT_BITMAP_HELVETICA_18);
				}
			}
			else if (l4State == WIN_MESSAGE) {
				glEnable(GL_BLEND); glColor4ub(0, 0, 0, 150); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); glDisable(GL_BLEND);
				iShowImage(700, 100, 400, 500, enemySprite);
				iSetColor(255, 255, 255); iFilledRectangle(150, 500, 450, 100);
				iSetColor(0, 0, 0); iText(170, 560, "Correct. Closer...", GLUT_BITMAP_HELVETICA_18);
				if (currentPhase == 0) iText(170, 520, "Clue: Machines never sleep here.", GLUT_BITMAP_HELVETICA_18);
				else if (currentPhase == 1) iText(170, 520, "Clue: Logic repeats endlessly.", GLUT_BITMAP_HELVETICA_12);
				else iDrawNeonText(170, 520, "GO TO FRONT DESK!", GLUT_BITMAP_HELVETICA_18, 255, 0, 0);
			}
			else if (l4State == FINAL_WIN) {
				iShowImage(600, 400, 80, 80, glowingFile);
				iDrawNeonText(500, 600, "You found the source file!", GLUT_BITMAP_HELVETICA_18, 0, 255, 0);
			}
			else if (l4State == FINAL_CONVERSATION) {
				glEnable(GL_BLEND); glColor4ub(0, 0, 0, 180); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); glDisable(GL_BLEND);
				iShowImage(750, 100, 400, 500, enemySprite);
				if (convoStep != 1) {
					iSetColor(0, 0, 0); iFilledRectangle(300, 150, 450, 120);
					iSetColor(255, 0, 0); iRectangle(300, 150, 450, 120);
					iDrawNeonText(320, 230, "Shadow:", GLUT_BITMAP_HELVETICA_18, 255, 0, 0);
					if (convoStep == 0) iText(320, 190, "You finally see it. But do you understand?", GLUT_BITMAP_HELVETICA_18);
					else {
						iText(320, 190, "You didn't find it. You became part of it.", GLUT_BITMAP_HELVETICA_18);
						iSetColor(150, 150, 150); iText(320, 160, "[PRESS SPACE TO ADVANCE]", GLUT_BITMAP_HELVETICA_12);
					}
				}
				else {
					iSetColor(255, 255, 255); iFilledRectangle(hero.x - 20, hero.y + 110, 200, 40);
					iSetColor(0, 0, 0); iText(hero.x, hero.y + 125, "What do you mean?", GLUT_BITMAP_HELVETICA_18);
				}
			}
			else if (l4State == GAME_OVER) {
				iSetColor(0, 0, 0); iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
				iDrawNeonText(450, 400, "NEED TO WORK ON SYNTAX", GLUT_BITMAP_TIMES_ROMAN_24, 255, 0, 0);
				iSetColor(255, 255, 255); iText(490, 350, "[PRESS SPACE TO RETRY]", GLUT_BITMAP_HELVETICA_18);
			}
		}
	}

	void gameTick() {
		if (l4State == INTRO_CONVERSATION) advanceIntro();
		else if (l4State == QUIZ) {
			quizTimer--;
			if (quizTimer <= 0) l4State = GAME_OVER;
		}
		else if (l4State == WIN_MESSAGE) {
			winTimer--;
			if (winTimer <= 0) {
				if (currentPhase == 2) { l4State = FINAL_WIN; finalWinTimer = 5; }
				else { currentPhase++; l4State = COOLDOWN; modeTimer = 0; }
			}
		}
		else if (l4State == FINAL_WIN) {
			finalWinTimer--;
			if (finalWinTimer <= 0) { l4State = FINAL_CONVERSATION; convoStep = 0; convoTimer = 3; }
		}
		else if (l4State == FINAL_CONVERSATION) {
			convoTimer--;
			if (convoTimer <= 0 && convoStep < 2) { convoStep++; convoTimer = 3; }
		}
	}

	void updateEffects() {
		if (l4State == TRANSITION_FLICKER) {
			flickerDuration += 0.030;
			flickerToggle = !flickerToggle;
			if (flickerDuration >= 1.5) {
				l4State = EXPLORING;
				currentBackground = it_center_bg;
				modeTimer = 0;
			}
		}
	}

	void update() {
		frameCounter++;
		if (frameCounter % 2 == 0) updateEffects(); // Runs ~30 times a sec
		if (frameCounter % 60 == 0) gameTick();     // Runs ~1 time a sec

		hero.update();

		if (l4State == EXPLORING || l4State == COOLDOWN) {
			int waitTimeFrames = (currentPhase == 2) ? 10 * 60 : 8 * 60; // 10s or 8s
			modeTimer++;
			if (modeTimer >= waitTimeFrames) {
				currentQuizIndex = rand() % 5;
				quizTimer = 7;
				l4State = QUIZ;
				modeTimer = 0;
			}
		}
	}

	void keyboard(unsigned char key) {
		if (l4State == INTRO_CONVERSATION && (key == 13 || key == 10 || key == ' ')) advanceIntro();

		if (l4State == QUIZ) {
			if (key == 'a' || key == 'A') checkAnswer(0);
			else if (key == 'b' || key == 'B') checkAnswer(1);
			else if (key == 'c' || key == 'C') checkAnswer(2);
		}

		if (l4State == GAME_OVER && key == ' ') {
			// Retry
			currentPhase = 0;
			l4State = EXPLORING;
			modeTimer = 0;
			hero.init(640, 360);
		}

		if (l4State == FINAL_CONVERSATION && convoStep == 2 && key == ' ') {
			// MAGIC BULLET: Unlock Level 5 and transition!
			::unlockedLevel = 5;
			saveProgress(5);
			::currentState = LEVEL_5;
		}
	}

	void mouse(int button, int state, int mx, int my) {
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && l4State == QUIZ) {
			for (int i = 0; i < 3; i++) {
				int optY = 450 - (i * 70);
				if (mx >= 140 && mx <= 540 && my >= optY - 10 && my <= optY + 40) {
					checkAnswer(i); break;
				}
			}
		}
	}

	void init() {
		// Load Images
		player_front = iLoadImage("player_front.png");
		player_back = iLoadImage("player_back.png");
		player_left = iLoadImage("player_left.png");
		player_right = iLoadImage("player_right.png");
		enemySprite = iLoadImage("enemy.png");
		it_center_bg = iLoadImage("it_center.bmp");
		door_view_bg = iLoadImage("door_view.bmp");
		glowingFile = iLoadImage("file_glowing.png");

		// Reset State for fresh start
		l4State = INTRO_CONVERSATION;
		dialogueIndex = 0;
		showEnemyIntro = false;
		flickerToggle = false;
		flickerDuration = 0;
		currentPhase = 0;
		convoStep = 0;
		modeTimer = 0;

		currentBackground = door_view_bg;
		hero.init(640, 360); // Centered start
	}
}

// ---------------- INTERFACE ----------------
void initLevel4() { L4::init(); }
void drawLevel4() { L4::draw(); }
void updateLevel4() { L4::update(); }
void keyboardLevel4(unsigned char key) { L4::keyboard(key); }
void mouseLevel4(int button, int state, int mx, int my) { L4::mouse(button, state, mx, my); }