#include "iGraphics.h"
#include "bitmap_loader.h"
#include <stdlib.h>
#include <windows.h>


int screenWidth = 800;
int screenHeight = 600;

int timerCount = 0;

int scene = 1;
int introStep = 0;


char *scene1_bg = "entrancefinal.bmp";
char *scene2_bg = "passage2final.bmp";

char *scene3_bg = "frontidfinal.bmp";


int oritri_talk_img, afif_talk_img, samiha_talk_img;
int oritri_scene_img, afif_scene_img, samiha_scene_img;


char *oritri_text = "Oritri: Loves sarcasm, coffee, and avoiding lectures.";
char *afif_text = "Afif: always late, yet top of class.";
char *samiha_text = "Samiha: Smart, sarcastic, and always rolling eyes.";


 
int oriID, afifID, samID;     



int selectedCharacterID = -1; 
int currentSelection = 0;    


// Scene 5 er jonno global variables
char *scene5_bg = "Classroom_final.bmp";

int masudSirImg;
int selectedCharScene5Img;

// Scene 5 control er jonno
int scene5Step = 0;
int scene6 = 6;






void drawPNG(int x, int y, int w, int h, int imgID) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	

	glColor4f(1.0, 1.0, 1.0, 1.0);
	iShowImage(x, y, w, h, imgID);
	glDisable(GL_BLEND);
}

// Scene 1 eikhan theke start, this part is done FINALLY. DO NOT TOUCH THIS PART GUYS.

void drawScene1()
{
	iShowBMP(0, 0, scene1_bg);

	int charHeight = screenHeight / 2;
	int charWidth = charHeight * 0.66;

	// Drawing Characters here
	drawPNG(100, 0, charWidth, charHeight, oritri_talk_img);
	drawPNG(300, 0, charWidth, charHeight, afif_talk_img);
	drawPNG(500, 0, charWidth, charHeight, samiha_talk_img);

	
	int boxY = 320;
	int boxW = 180;
	int boxH = 100; 

	iSetColor(0, 0, 0);
	iText(700, 20, "Press SHIFT to continue ...", GLUT_BITMAP_HELVETICA_12);

	// Oritri 1
	if (timerCount >= 0 && timerCount < 3) {
		iSetColor(0, 0, 0); iFilledRectangle(100, boxY, boxW, boxH);
		iSetColor(255, 255, 255);
		iText(110, boxY + 75, "Oritri: Hello! Congrats!", GLUT_BITMAP_9_BY_15);
		iText(110, boxY + 55, "You're now part of the species", GLUT_BITMAP_9_BY_15);
		iText(110, boxY + 35, "that survives on caffeine,", GLUT_BITMAP_9_BY_15);
		iText(110, boxY + 15, "code, and last minute panic.", GLUT_BITMAP_9_BY_15);
	}

	// Afif 1
	else if (timerCount >= 3 && timerCount < 6) {
		iSetColor(0, 0, 0); iFilledRectangle(300, boxY, boxW, boxH);
		iSetColor(255, 255, 255);
		iText(310, boxY + 75, "Afif: Enjoy the fresh air", GLUT_BITMAP_9_BY_15);
		iText(310, boxY + 55, "while it lasts. Soon, the", GLUT_BITMAP_9_BY_15);
		iText(310, boxY + 35, "only breeze is your laptop", GLUT_BITMAP_9_BY_15);
		iText(310, boxY + 15, "fan overheating.", GLUT_BITMAP_9_BY_15);
	}

	// Samiha 1
	else if (timerCount >= 6 && timerCount < 9) {
		iSetColor(0, 0, 0); iFilledRectangle(540, boxY, boxW, boxH);
		iSetColor(255, 255, 255);
		iText(550, boxY + 75, "Samiha: Don't worry. Soon", GLUT_BITMAP_9_BY_15);
		iText(550, boxY + 55, "you'll be a coding wizard", GLUT_BITMAP_9_BY_15);
		iText(550, boxY + 35, "or an expert at Googling", GLUT_BITMAP_9_BY_15);
		iText(550, boxY + 15, "Stack Overflow.", GLUT_BITMAP_9_BY_15);
	}

	// Oritri 2
	else if (timerCount >= 9 && timerCount < 12) {
		iSetColor(0, 0, 0); iFilledRectangle(100, boxY, boxW, boxH);
		iSetColor(255, 255, 255);
		iText(110, boxY + 75, "Oritri: Welcome to AUST!", GLUT_BITMAP_9_BY_15);
		iText(110, boxY + 55, "Where you enter as a human", GLUT_BITMAP_9_BY_15);
		iText(110, boxY + 35, "and leave as someone who", GLUT_BITMAP_9_BY_15);
		iText(110, boxY + 15, "loves semicolons.", GLUT_BITMAP_9_BY_15);
	}

	// Afif 2
	else if (timerCount >= 12 && timerCount < 15) {
		iSetColor(0, 0, 0); iFilledRectangle(300, boxY, boxW, boxH);
		iSetColor(255, 255, 255);
		iText(310, boxY + 75, "Afif: Remember, when your", GLUT_BITMAP_9_BY_15);
		iText(310, boxY + 55, "code errors at 2 AM,", GLUT_BITMAP_9_BY_15);
		iText(310, boxY + 35, "it's not personal.", GLUT_BITMAP_9_BY_15);
		iText(310, boxY + 15, "It's just CSE.", GLUT_BITMAP_9_BY_15);
	}

	// Samiha 2
	else if (timerCount >= 15) {
		iSetColor(0, 0, 0); iFilledRectangle(540, boxY, boxW, boxH);
		iSetColor(255, 255, 255);
		iText(550, boxY + 75, "Samiha: Welcome to CSE!", GLUT_BITMAP_9_BY_15);
		iText(550, boxY + 55, "Where 'Hello World' is", GLUT_BITMAP_9_BY_15);
		iText(550, boxY + 35, "the last easy thing", GLUT_BITMAP_9_BY_15);
		iText(550, boxY + 15, "you'll ever write.", GLUT_BITMAP_9_BY_15);
	}

	
}


// scene 2 eikhan theke start, space key is working. scene 2 is done. do not touch this part either.

void drawScene2()
{
	iShowBMP(0, 0, scene2_bg);

	int midHeight = screenHeight /1.75;
	int midWidth = midHeight * 0.66;

	int bigHeight = screenHeight ;
	int bigWidth = midHeight * 0.66;


	int centerY = 0;
	int centerX1 = 100;
	int centerX2 = 300;
	int centerX3 = 500;

	int boxW = 400;
	int boxH = 60;


	
	if (introStep == 0)
	{
		drawPNG(centerX1, centerY, midWidth, midHeight, oritri_scene_img);
		drawPNG(centerX2, centerY, midWidth, midHeight, afif_scene_img);
		drawPNG(centerX3, centerY, midWidth, midHeight, samiha_scene_img);

		iSetColor(0, 0, 0);
		iFilledRectangle(200, 480, boxW, boxH);
		iSetColor(255, 255, 255);
		iText(220, 505, "welcome to Aust! Let's know us as individuals", GLUT_BITMAP_HELVETICA_18);
		iSetColor(0, 0, 0);
		iText(700, 20, "Press SHIFT to continue ...", GLUT_BITMAP_HELVETICA_12);
	}

	
	else if (introStep == 1)
	{
		drawPNG(150, centerY, midWidth, bigHeight, oritri_scene_img);

		iSetColor(0, 0, 0);
		iFilledRectangle(350, 250, boxW, boxH);
		iSetColor(255, 255, 255);
		iText(360, 285, "Oritri: Runs on panic, caffeine, and last minute confidence. Believes every bug can be fixed ", GLUT_BITMAP_HELVETICA_18);
		iText(360, 270, "if you stare at it long enough. Special ability: staying calm while internally screaming. ", GLUT_BITMAP_HELVETICA_18);
		iSetColor(0, 0, 0);
		iText(700, 20, "Press SHIFT to continue...", GLUT_BITMAP_HELVETICA_12);

	}

	
	else if (introStep == 2)
	{
		drawPNG(150, -50, midWidth, bigHeight, afif_scene_img);

		iSetColor(0, 0, 0);
		iFilledRectangle(350, 250, boxW, boxH);
		iSetColor(255, 255, 255);
		iText(360, 285, "Afif: Professional procrastinator turned emergency performer. Debugs best under extreme", GLUT_BITMAP_HELVETICA_18);
		iText(360, 270, "emotional pressure. Secret talent : convincing himself “there’s still time. ", GLUT_BITMAP_HELVETICA_18);
		iSetColor(0, 0, 0);
		iText(700, 20, "Press SHIFT to continue...", GLUT_BITMAP_HELVETICA_12);
	}

	
	else if (introStep == 3)
	{
		drawPNG(150, -50, midWidth, bigHeight, samiha_scene_img);

		iSetColor(0, 0, 0);
		iFilledRectangle(350, 250, boxW, boxH);
		iSetColor(255, 255, 255);
		iText(360, 290, "Samiha: Organized to the point where even her backup files have backup files. Finds", GLUT_BITMAP_HELVETICA_18);
		iText(360, 275, "mistakes faster than teachers find surprise quizzes. Weakness: gets emotionally ", GLUT_BITMAP_HELVETICA_18);
		iText(360, 260, "offended by messy variable names.", GLUT_BITMAP_HELVETICA_18);
		iSetColor(0, 0, 0);
		iText(700, 20, "Press SHIFT to continue...", GLUT_BITMAP_HELVETICA_12);
	}

	
	else if (introStep == 4)
	{
		drawPNG(centerX1, centerY, midWidth, bigHeight, oritri_scene_img);
		drawPNG(centerX2, centerY, midWidth, bigHeight, afif_scene_img);
		drawPNG(centerX3, centerY, midWidth, bigHeight, samiha_scene_img);

		iSetColor(0, 0, 0);
		iFilledRectangle(260, 0, 350, 20);
		iSetColor(255, 255, 255);
		iText(270, 5, "Oritri: Before the semester chooses violence,  you must choose a character ", GLUT_BITMAP_HELVETICA_18);
		iSetColor(0, 0, 0);
		iText(700, 20, "Press SHIFT to continue ", GLUT_BITMAP_HELVETICA_12);
	}
}


void drawScene3()
{
	iShowBMP(0, 0, scene3_bg); // background

	int charWidth = 160;
	int charHeight = 270;
	int yPos = 140;

	int x[3] = { 100, 325, 550 };

	// Draw characters
	drawPNG(x[0], yPos, charWidth, charHeight, oriID);
	drawPNG(x[1], yPos, charWidth, charHeight, afifID);
	drawPNG(x[2], yPos, charWidth, charHeight, samID);

	//  selection rectangle draw eikhane
	if (selectedCharacterID == -1)
	{
		iSetColor(0, 255, 0);
		iRectangle(x[currentSelection], yPos, charWidth, charHeight);
	}


	iSetColor(0, 0, 0);
	iFilledRectangle(200, 520, 400, 40);
	iSetColor(255, 255, 255);
	iText(260, 535, "Use LEFT/RIGHT to select, Shift to confirm", GLUT_BITMAP_HELVETICA_18);
}



void drawScene4() {
	
	iShowBMP(0, 0, scene3_bg);

	int displayImg;
	char* nameText;

	// the correct image based user er click e
	if (selectedCharacterID == 0) {
		displayImg = oriID;
		nameText = "ORITRI SELECTED";
	}
	else if (selectedCharacterID == 1) {
		displayImg = afifID;
		nameText = "AFIF SELECTED";
	}
	else {
		displayImg = samID;
		nameText = "SAMIHA SELECTED";
	}

	
	drawPNG(300, 150, 200, 350, displayImg);

	iSetColor(0, 0, 0);
	iFilledRectangle(250, 520, 300, 40);
	iSetColor(255, 255, 255);
	iText(310, 535, nameText, GLUT_BITMAP_HELVETICA_18);

	iSetColor(0, 0, 0);
	iText(340, 100, "Press Shift to start the game", GLUT_BITMAP_HELVETICA_18);
}


void drawScene5()
{
	iShowBMP(0, 0, scene5_bg);

	int charWidth = 200;
	int charHeight = 350;

	// Left e Selected character thakbe
	drawPNG(100, 100, charWidth, charHeight, selectedCharScene5Img);

	// Right e Masud Sir
	drawPNG(500, 100, charWidth, charHeight, masudSirImg);

	// Dialogue box code here
	iSetColor(0, 0, 0);
	iFilledRectangle(150, 20, 500, 80);
	iSetColor(255, 255, 255);

	if (scene5Step == 0)
		iText(170, 60, "Masud Sir: So you think you are ready for this?", GLUT_BITMAP_HELVETICA_18);

	else if (scene5Step == 1)
		iText(170, 60, "You: I was born ready, sir.", GLUT_BITMAP_HELVETICA_18);   //change the convo please.

	else if (scene5Step == 2)
		iText(170, 60, "Masud Sir: We'll see about that.", GLUT_BITMAP_HELVETICA_18);

	else if (scene5Step == 3)
		iText(170, 60, "Masud Sir: Prepare yourself.", GLUT_BITMAP_HELVETICA_18);

	iText(600, 10, "Press SHIFT to continue", GLUT_BITMAP_HELVETICA_12);
}

void drawScene6()
{
	iClear();
	iSetColor(255, 255, 255);
	iText(300, 300, "Scene 6 Starts - Fight Begins!", GLUT_BITMAP_HELVETICA_18);
}




void iDraw() {
	iClear();

	if (scene == 1) drawScene1();
	else if (scene == 2) drawScene2();
	else if (scene == 3) drawScene3();
	else if (scene == 4) drawScene4();
	else if (scene == 5) drawScene5();
	else if (scene == 6) drawScene6();
}



void iMouse(int button, int state, int mx, int my)
{
}

void iMouseMove(int mx, int my) {}
void iPassiveMouseMove(int mx, int my) {}

void iSpecialKeyboard(unsigned char key) {
}








void fixedUpdate()
{
	static int spacePressed = 0;
	static int leftPressed = 0;
	static int rightPressed = 0;
	static int shiftPressed = 0;

	// SPACE KEY use kora Scene 1 & 2 te only

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		if (!spacePressed)
		{
			spacePressed = 1;

			if (scene == 1)
			{
				scene = 2;
				introStep = 0;
				timerCount = 16;
			}
			else if (scene == 2)
			{
				introStep++;
				if (introStep > 4)
				{
					scene = 3;
				}
			}
		}
	}
	else
	{
		spacePressed = 0;
	}

	// SCENE 3 er control logic eikhane
	if (scene == 3)
	{
		// LEFT arrow er jonno
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			if (!leftPressed)
			{
				leftPressed = 1;
				currentSelection--;

				if (currentSelection < 0)
					currentSelection = 2;   
			}
		}
		else
		{
			leftPressed = 0;
		}

		// RIGHT arrow er jonno
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			if (!rightPressed)
			{
				rightPressed = 1;
				currentSelection++;

				if (currentSelection > 2)
					currentSelection = 0;  
			}
		}
		else
		{
			rightPressed = 0;
		}

		// SHIFT press kora lagbe to confirm selection 
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		{
			if (!shiftPressed)
			{
				shiftPressed = 1;

				selectedCharacterID = currentSelection;
				scene = 4;
			}
		}
		else
		{
			shiftPressed = 0;
		}
	}



	// GLOBAL SHIFT CONTROL (Scene 4 & 5)
	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	{
		if (!shiftPressed)
		{
			shiftPressed = 1;

			// Scene 4 to Scene 5
			if (scene == 4)
			{
				if (selectedCharacterID == 0)
					selectedCharScene5Img = oriID;
				else if (selectedCharacterID == 1)
					selectedCharScene5Img = afifID;
				else
					selectedCharScene5Img = samID;

				scene5Step = 0;
				scene = 5;
			}

			// Scene 5 theke Next dialogue or Scene 6, basically game start hobe after scene 6
			else if (scene == 5)
			{
				scene5Step++;

				if (scene5Step > 3)
				{
					scene = 6;
				}
			}
		}
	}
	else
	{
		shiftPressed = 0;
	}



}





void changeTimer()
{
	timerCount++;
}


int main() {
	
	iInitialize(screenWidth, screenHeight, "Runtime Error");
	iSetTimer(1000, changeTimer);
	iSetTimer(10, fixedUpdate);

	
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



	iStart();
	return 0;
}