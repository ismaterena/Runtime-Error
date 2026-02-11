#include "iGraphics.h"
#include "bitmap_loader.h"
#include <stdlib.h>


int screenWidth = 800;
int screenHeight = 600;


int scene = 1;
int introStep = 0;


char *scene1_bg = "entrancefinal.bmp";
char *scene2_bg = "passage2final.bmp";


int oritri_talk_img, afif_talk_img, samiha_talk_img;
int oritri_scene_img, afif_scene_img, samiha_scene_img;


char *oritri_text = "Oritri: Loves sarcasm, coffee, and avoiding lectures.";
char *afif_text = "Afif: Meme king, always late, yet top of class.";
char *samiha_text = "Samiha: Smart, sarcastic, and always rolling eyes.";


void drawPNG(int x, int y, int w, int h, int imgID) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	

	glColor4f(1.0, 1.0, 1.0, 1.0);
	iShowImage(x, y, w, h, imgID);
	glDisable(GL_BLEND);
}

// Scene 1 eikhan theke start
void drawScene1()
{
	iShowBMP(0, 0, scene1_bg);

	// Dialogue er jonno background box 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0, 1.0, 1.0, 0.75); // Semi-transparent White eikhane 
	iFilledRectangle(40, screenHeight - 200, 720, 150);
	glDisable(GL_BLEND);

	int charHeight = screenHeight / 2;
	int charWidth = charHeight * 0.66;

	// character er positioning
	drawPNG(100, 0, charWidth, charHeight, oritri_talk_img);
	drawPNG(300, 0, charWidth, charHeight, afif_talk_img);
	drawPNG(500, 0, charWidth, charHeight, samiha_talk_img);

	iSetColor(0, 0, 0);
	iText(60, screenHeight - 100, "Oritri: This university has WiFi, but no coffee!", GLUT_BITMAP_HELVETICA_18);
	iText(60, screenHeight - 130, "Afif: Attendance is optional, stress is mandatory.", GLUT_BITMAP_HELVETICA_18);
	iText(60, screenHeight - 160, "Samiha: I survived 3 assignments in one night.", GLUT_BITMAP_HELVETICA_18);

	iSetColor(255, 255, 255);
	iText(50, 20, "Press ENTER to continue...", GLUT_BITMAP_HELVETICA_12);
}

// scene 2 eikhan theke start
void drawScene2()
{
	iShowBMP(0, 0, scene2_bg);

	int smallHeight = screenHeight / 4;
	int smallWidth = smallHeight * 0.66;

	if (introStep == 0)
	{
		drawPNG(150, 50, smallWidth, smallHeight, oritri_scene_img);
		drawPNG(350, 50, smallWidth, smallHeight, afif_scene_img);
		drawPNG(550, 50, smallWidth, smallHeight, samiha_scene_img);

		iSetColor(255, 255, 255);
		iText(50, screenHeight - 50, "Characters ready. Press ENTER.", GLUT_BITMAP_HELVETICA_12);
	}
	else if (introStep >= 1 && introStep <= 3)
	{
		// this one here is the Bio Box for intro
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0, 0.0, 0.0, 0.6);
		iFilledRectangle(380, 270, 400, 80);
		glDisable(GL_BLEND);

		double zoomFactor = (screenHeight / 3.0) / 300.0;
		int drawWidth = 300 * zoomFactor;
		int drawHeight = 450 * zoomFactor;

		if (introStep == 1) drawPNG(50, 100, drawWidth, drawHeight, oritri_scene_img);
		if (introStep == 2) drawPNG(50, 100, drawWidth, drawHeight, afif_scene_img);
		if (introStep == 3) drawPNG(50, 100, drawWidth, drawHeight, samiha_scene_img);

		iSetColor(255, 255, 255);
		if (introStep == 1) iText(400, 310, oritri_text, GLUT_BITMAP_HELVETICA_18);
		if (introStep == 2) iText(400, 310, afif_text, GLUT_BITMAP_HELVETICA_18);
		if (introStep == 3) iText(400, 310, samiha_text, GLUT_BITMAP_HELVETICA_18);

		iText(50, 50, "Press ENTER to continue...", GLUT_BITMAP_HELVETICA_12);
	}
}

void iDraw() {
	iClear();
	if (scene == 1) drawScene1();
	else if (scene == 2) drawScene2();
}

void iKeyboard(unsigned char key) {
	if (key == '\r') {
		if (scene == 1) { scene = 2; introStep = 0; }
		else if (scene == 2 && introStep < 3) introStep++;
	}
	if (key == 'q') exit(0);
}

void iMouseMove(int mx, int my) {}
void iMouse(int button, int state, int mx, int my) {}
void iSpecialKeyboard(unsigned char key) {}
void iPassiveMouseMove(int mx, int my) {}
void fixedUpdate() {}

int main() {
	
	iInitialize(screenWidth, screenHeight, "Runtime Error");

	
	oritri_talk_img = iLoadImage("talkingori.png");
	afif_talk_img = iLoadImage("talkingafif.png");
	samiha_talk_img = iLoadImage("talkingsam.png");

	oritri_scene_img = iLoadImage("scene2ori.png");
	afif_scene_img = iLoadImage("scene2afif.png");
	samiha_scene_img = iLoadImage("scene2sam.png");

	iStart();
	return 0;
}