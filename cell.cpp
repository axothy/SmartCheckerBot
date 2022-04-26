#include "cell.h"

//����������� (������) ������ �������� �����
ActiveCell::ActiveCell(CheckerColor b_or_w) {
	setChecker(b_or_w); //������������� ����� �� ��� ������, ����������� ����������� ������ "Checker"
}

//���� �� �� ���� ������ �����
bool ActiveCell::isFree() {
	if (getChecker() != nullptr) { //����� getChecker �� ���������� ��������� �� �����, � ���� ��������� �� ����� NULL
		return HAS_CHECKER; //�� ���������� HAS_CHECKER
	}
	else //���� null, �� ���������� �����_���
		return HAS_NO_CHECKER;
}

//�������� �� ��� ������ ��� ���
//������������ �����, ����� ��������� ����� ����� ��������� ���� ��� ���� �����
bool ActiveCell::isSelected() {
	if (isFree() == HAS_CHECKER) //���� �� ������ ���� �����
		return getChecker()->isSelected(); //�� ���������� ���������� � ��������� ���� ����� (�������� ��� ��� ���)
	else
		return false;
}
//PS getChecker()->isSelected() �������� ������� isSelected ������ Checker!

//����� ��������
void ActiveCell::selectChecker() {
	if (isFree() == HAS_CHECKER) //���� �� ������ ���� �����
		getChecker()->CheckerSelect(); //�� �� �������� ������� ��������� ��� ���� �����
}

//�������� ����
CheckerColor ActiveCell::getCheckerColor() {
	if (isFree() == HAS_CHECKER) //���� �� ������ ���� �����
		return getChecker()->getColor(); //�������� �� ����
	else
		return EXCEPTION_COLOR; //���� ����� ��� �� ����������
}

//������������� ���������
void ActiveCell::setPost(CheckerHierarchy post) {
	if (isFree() == HAS_CHECKER) { //���� ���� ����� �� ������
		getChecker()->setPost(post); //�������� ������� ��� ���� ����� setPost(post)
	}
}

//�������� ���������
CheckerHierarchy ActiveCell::getCheckerPost() {
	if (isFree() == HAS_CHECKER)
		return getChecker()->getPost();
	else
		return EXCEPTION_POST;
}

//������� ����� �������� ����� �� ����� ������
void ActiveCell::drawFrame() {
	glLineWidth(4.0); //������� �����
	glBegin(GL_LINE_STRIP); //������ �����
	glColor3f(0, 1, 0);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);
	glEnd();
}

//������ ������
void ActiveCell::drawCell() {
	glBegin(GL_TRIANGLE_FAN); //����� ��������������, �� � open gl ��� ������� ��������� ��������
	glColor3f(0.3f, 0.12f, 0.04f); //�����-���������� ����, �� ������ ��������
	glVertex2f(0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);
	glVertex2f(1.0f, 0.0f);
	glEnd();

	//��� �������:
	if (isFree() == HAS_CHECKER) { //���� ���� �����
		getChecker()->drawChecker(); //�� �� ���� ������ �� ������ �� ����������
	}
	if (isPossible() == true) { //���� ��� �������� �� ��� ������
		drawFrame(); //�� ������ ������� �����
	}
}

//������� ������� ���������� ������, �� ������������ � ����
void Cell::drawCell() {
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1.0f, 0.88f, 0.59f);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);
	glVertex2f(1.0f, 0.0f);
	glEnd();
}