#pragma once
#include <windows.h>
#include <gl/gl.h>

#pragma comment(lib, "opengl32.lib") //����������� opengl

//������������ - ���� � �����
enum CheckerColor {
	BLACK, WHITE, EXCEPTION_COLOR //exception_color - ���� �� ������ ��� ����� �������, �� ��� ���� ����������
};

//��� ������������ - ��������� �����
enum CheckerHierarchy {
	CHECKER, KING, EXCEPTION_POST //�.e ���� ����� ���� �����, ���� �� ������ ��� ������� �����
};


class Checker {
public:
	Checker(CheckerColor b_or_w) { _color = b_or_w; _post = CHECKER; } //�����������, ��������� �� ���� ����, � ���������� ����� �� ���������


	void drawChecker(); //������� �����
	void drawFrame();   //������� ����� ��� ����� (���������� ��������)

	void CheckerSelect() { _selected == true ? _selected = false : _selected = true; } //�������� �����, ���� ��� ��������, �� ��������� ���������, � ��������
	void CheckerSelectON() { _selected = true; } //�������� �����
	void CheckerSelectOFF() { _selected = false; } //������� ��������� ����� (����� ��� 2 ������� �� �����������)

	bool isSelected() { return _selected; } //bool, ���� ��� ������ �������� ������ ����� ��� �� ��������

	CheckerColor getColor() { return _color; } //���������� ���� �����
	CheckerHierarchy getPost() { return _post; } //���������� ��������� �����

	void setPost(CheckerHierarchy post) { _post = post; } //������������� ��������� ����� (������������ ������ ����� ����� ������� �����)

//��������� ������
private:
	bool _selected = false; //�������� ��
	CheckerColor _color;    //����
	CheckerHierarchy _post; //���������
};
