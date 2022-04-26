#include "checker.h"
#include <cmath>

//�������� ����� (����������)
void Checker::drawFrame() {
	float x, y;
	float dot = 100; //��������� ���������� �������� � ������, ��� ������ ����� ��� ����� ����������
	float r = 0.4f;  //������ ����������
	float alpha = 3.1415926f * 2.0f / dot;
	glPointSize(2);
	glBegin(GL_POINTS); //GL_POINS ������� � ��� ��� �������� �����
	glColor3f(0.0f, 1.0f, 0.0f); //���� � ����� ����� �������


	for (int i = -1; i < dot; i++) {
		x = sin(alpha * i) * r; //������ ������ ����� �� ���������� 
		y = cos(alpha * i) * r;
		glVertex2f(x + 0.5f, y + 0.5f); //��� ����� ����� ���������� ��������� �� ������ ������
	}
	glEnd();
}



void Checker::drawChecker() {
	glBegin(GL_TRIANGLE_FAN); //�������������� ������������� ����������
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

	//���� ��� �����, �� ����� ������ �������
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

	//� ����� ������������ ���������
	//�� ���� ���� �� �������� �����, isSelected ���������� true, � �������� ������� ����� ��� ���������� �����
	if (isSelected()) drawFrame();
}