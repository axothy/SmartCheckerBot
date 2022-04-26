#pragma once
#include "checker.h"

//����� ������
//��� ������ ������ ������, ����������, �� ����� ����� �����. (��� ����� ������� ������)
class Cell {
public:
	virtual void drawCell(); //������� ������ (������ ����������� �������)
	//(�� ���� ����� �������� ������� ����������, ������� ����������� ������� - ������ �� ������)
	virtual void selectChecker() {}
	void selectCheckerOFF() {}

	virtual void makePossibleMove() {}
	virtual void makeImPossibleMove() {}


	virtual bool isPossible() { return false; } //����� �� �� ��� ������ ������� - ���, ������, ��� ��� ������ ���������
	virtual bool isSelected() { return false; } //����� �� ��� ������ ���� ���������� - ���, ��� ��� ��� �� ����� ����� �����
	virtual bool isFree() { return true; }      //�������� �� ��� ������ - ��, ������ ��������, �� �� ��� ������ �� �����

	virtual void setPost(CheckerHierarchy post) {}

	//������� ��������� ����� ����� � ��������� ����� ����������� ����������, 
	//��������� �� ������ ������ �� ����� �� ����������� ������
	virtual CheckerColor getCheckerColor() { return EXCEPTION_COLOR; }
	virtual CheckerHierarchy getCheckerPost() { return EXCEPTION_POST; }
};

//������������ - ��� ��������, ����� �������� ���� �� �� ���� ������ ����� ��� ���
enum CHECKERS_HAVE {
	HAS_CHECKER = true, HAS_NO_CHECKER = false
};

//����� �������� ������ - �������� 
//�� ����� ������ ����� ������� � ��� ����� ����� �����
class ActiveCell : public Cell {
	Checker* _checker;           //��� ��������� �� �����
	bool _possibleMove = false;  //bool ����� �� ������� �� ��� ������ ��� ������


	Checker* getChecker() { return _checker; } //���������� ��������� �� �����

	void setChecker(CheckerColor b_or_w) { _checker = new Checker(b_or_w); } //���������� ����� �� ��� ������ �� �����

public:

	//��� 2 ������������
	ActiveCell() {} //���� ������������ �����, ����� �� ���� ������ ��� �����, ������ �������� ������
	ActiveCell(CheckerColor b_or_w); //� ���� ������������ �����, ����� �� ���� ������ ���� �����

	void drawCell(); //������ ������� ������
	void drawFrame(); //������� ����� (������������ ��� ������� ����� ��� ������, �� ������� ����� �������)

	bool isSelected(); //���������� �������� �� ����� �� ���� ������ (true) ��� ���(false)
	bool isPossible() { return _possibleMove; } //�������� �� ��� �� ��� ������
	bool isFree();      //���� �� �� ���� ������ ���� ����� (���������� HAS_CHECKER �� enum), ���� ��� (HAS_NO_CHECKER)

	void selectChecker(); //������� ������
	void selectCheckerOFF() { getChecker()->CheckerSelectOFF(); } //������ ���������

	void setPost(CheckerHierarchy post); //���������� ��������� ��� �����

	void makePossibleMove() { _possibleMove = true; } //���������� ����������� ���� �� ������ 
	void makeImPossibleMove() { _possibleMove = false; } //���������� ������������� ���� �� ��� ������

	//��������� ����� ��� ��������� 
	CheckerColor getCheckerColor();
	CheckerHierarchy getCheckerPost();

};
