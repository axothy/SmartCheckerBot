#include "checker.h"
#include <cmath>

//–исуетс€ рамка (окружность)
void Checker::drawFrame() {
	float x, y;
	float dot = 100; //гладкость окружности задаетс€ в точках, чем больше точек тем глаже окружность
	float r = 0.4f;  //радиус окружности
	float alpha = 3.1415926f * 2.0f / dot;
	glPointSize(2);
	glBegin(GL_POINTS); //GL_POINS говорит о том что рисуютс€ точки
	glColor3f(0.0f, 1.0f, 0.0f); //цвет у рамок будет зеленым


	for (int i = -1; i < dot; i++) {
		x = sin(alpha * i) * r; //просто ставим точки по окружности 
		y = cos(alpha * i) * r;
		glVertex2f(x + 0.5f, y + 0.5f); //это нужно чтобы окружность строилась из центра клетки
	}
	glEnd();
}



void Checker::drawChecker() {
	glBegin(GL_TRIANGLE_FAN); //треугольниками закрашиваютс€ окружности
	glColor3f(float(getColor()), float(getColor()), float(getColor()));
	float x, y;
	float cnt = 20;
	float r = 0.4f;
	float alpha = 3.1415926f * 2.0f / cnt;
	for (int i = -1; i < cnt; i++) {
		x = sin(alpha * i) * r;
		y = cos(alpha * i) * r;
		glVertex2f(x + 0.5f, y + 0.5f);
	}
	glEnd();

	//≈сли это дамка, то здесь друга€ рисовка
	if (getPost() == KING) {
		glBegin(GL_TRIANGLE_FAN);
		glColor3f(0.5f, 0.5f, 0.5f);
		r = 0.3f;
		for (int i = -1; i < cnt; i++) {
			x = sin(alpha * i) * r;
			y = cos(alpha * i) * r;
			glVertex2f(x + 0.5f, y + 0.5f);
		}
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
		glColor3f(float(getColor()), float(getColor()), float(getColor()));
		r = 0.2f;
		for (int i = -1; i < cnt; i++) {
			x = sin(alpha * i) * r;
			y = cos(alpha * i) * r;
			glVertex2f(x + 0.5f, y + 0.5f);
		}
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
		glColor3f(0.5f, 0.5f, 0.5f);
		r = 0.1f;
		for (int i = -1; i < cnt; i++) {
			x = sin(alpha * i) * r;
			y = cos(alpha * i) * r;
			glVertex2f(x + 0.5f, y + 0.5f);
		}
		glEnd();
	}

	//ј здесь используетс€ выделение
	//“о есть если мы выделили шашку, isSelected возвращает true, и рисуетс€ зелена€ рамка дл€ выделенной шашки
	if (isSelected()) drawFrame();
}