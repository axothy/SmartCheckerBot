#pragma once
#include "cell.h"
#include <array>
#include <vector>
#include <algorithm>

#define playboard Playboard::getPlayboard() //������ ��� ��������

//��������� xy ������� ������ ��� �������� �������� ��������� �����
struct xy {
	int x, y;

	//��������� ���������
	int getX() { return x; }
	int getY() { return y; }

	//��������� ���������
	void setX(int x0) { x = x0; }
	void setY(int y0) { y = y0; }

	//��������� ���� �������� xy1 � xy2 ��������, ����� ��������, ��� ����� ����� ��������������
	bool operator==(const xy xy0) {
		if ((*this).x == xy0.x && (*this).y == xy0.y) {
			return true;
		}
		else
			return false;
	}

	//�������� ������� , ���� ��� � ������ ������ ���������
	bool operator!=(const xy xy0) {
		return !((*this) == xy0);
	}
};

//��������� ���������, ���������� �� ������
//(������������ ����� ���, �������� ���� � ��� ������ �� ��������, �� ������������� ���������� ���������� ��� �� ���...)
const xy OFF_MAP = { -1,-1 };

//����� "������� �����", �������� � ������� ������� singleton
class Playboard {
private:
	Playboard(); //�������������� ��������� �����������

	//��������� ������ _board, ������ ��������� �� ����� ������
	std::array<std::array<Cell*, 8>, 8> _board;

	//��������� - ��������� ����������
	xy _selectedCoor;


	std::vector<xy> _possibleMoves; //������ ��������� �����, ������� �� �������� xy (�� ��� xy ����� ����� �������)
	std::vector<xy> _canEats; //���������� �����, ������� ����� ������

	int situation = 0; //�������, ����� ��� ���������� ���� (���-� ������ � ����� �����)
	bool turn_end = true; //�������, ���� ����� ��� ���������� ����

	//������ � ������ ������� �����
	const int _tableW = 8;
	const int _tableH = 8;

	//���������� ���������� ����������, ��� ���������� ���� ��� ���� �������� ���������
	void setSelectedCoor(xy position) { _selectedCoor = position; }
	void setSelectedCoor(int x, int y) { _selectedCoor.setX(x); _selectedCoor.setY(y); }
	//���������� ���������� ������ (��������� � �����������)
	void setSelectedCell(int x, int y);
	void setSelectedCell(xy position);

	//�������� ������ �� �����, ������������ ����� ����� ����� � ����� �� �������
	//� ��������� ������ ������ �������� � �����, � ����� ������ ������� ������ ������ 
	void refreshCell(int x, int y) { delete _board[x][y]; _board[x][y] = new ActiveCell(); }

	//�������� ������� ������ (������������ ����� ����� � ������)
	//������������ �����, ����� ����� ������ ���� �������
	void swapCells(xy pos1, xy pos2);
	void swapCellsWithoutSelected(xy pos1, xy pos2); //(�������, ����������� �����)

	//����� ��������� ���� ��� ��������
	//��� ������������� ��������� ���� ��� ��������
	void checkEats(int dx, int dy);


	//���������, ����� �� ������ ������
	//� ��� checkEats ������ ���������� true, ���� �� ���� ����� (������) ����� ������, false ���� ����
	bool checkEats(int x, int y, int dx, int dy);


	//���������, ������ �� � ����� ������� ���� �����
	//����������: ���� ������� "����������� ����� ������ ���� ��� ��������".
	//������� ��������� ��������� xy �� �������������� ������� ��������� xy, ������� ����� ���� (_canEats).
	//���� ����������� - ���������� true (����� ����� ����� ����������, � ���� ��������������), ���� ��� - false
	bool checkEaters(int x, int y);


	//������ ����������� ���� � ������ ������
	//���������� �������� ���������� xy �� �������������� ������� ��������� ����� _possibleMoves
	bool isMovePossible(int x, int y);


	//������, ��� �������� �����?
	bool isEating(int x, int y); //���� �� �� ����� ��� ���
	bool isEatingForKing(xy& pos, int x, int y);

	//������, ����� �� ������ ��� ����
	//������������ ����� ����� �� ����� ������, � ����� ��� ����� ������ ����� �� �� ��� ���� ��� ���
	bool isThereAnotherEats(int x, int y);


	//���������� ��������� ���� ��� ���������� �����
	void setPossibleMovesForChecker();
	void setPossibleEatesForChecker();

	//���������� ��������� ���� ��� �����
	void setPossibleEatesForKing();
	void setPossibleMovesForKing();
	void findFoodForKing(int dx, int dy); //��� ��������������� �������
	void findMoveForKing(int dx, int dy);

	//�������� ���������� ����������
	xy getSelectedCoor() { return _selectedCoor; }
	//�������� ���������� ������ 
	Cell* getSelectedCell() { return getCell(_selectedCoor); }

	//������ �������������� ����
	//������������: �������� ������� ����� � ������������ ����, � �������� �������� �� ������ ���� -
	//����� ����� ��������� ��������� ��� ����� � ��� �������������� ����
	void ClearPossible();
	//������ ��������� ������
	void ClearSelected();


	//�������� ������ �����, ������� ����� ����
	std::vector<xy>& getCanEats() { return _canEats; }

	//����� ������� ����
	//�� ���� ����� �� ��������� ������ _canEats
	void findEaters(CheckerColor b_or_w);

	//�������� ������ ������� ����
	//��� ������������: ��������, �� ������� , �������������� ��� ����� ��� �������� ������ _canEats
	void clearEaters() { getCanEats().clear(); }

	//�������� ������ ������� ����, ����� ������, ������� ��� ���� � ����� ������ ���
	void clearEaters(int x, int y);




	//������� ������ �����
	void SelectChecker(int x, int y, CheckerColor b_or_w);

	//������� ����������� �����
	int MoveChecker(int x, int y, CheckerColor b_or_w);

public:

	//�������� ������ � ������� �����
	static Playboard& getPlayboard(); //����������� �������, ���������� ������ � �����, � ��� �� ���������� ��������� �����������
	//���������� ������� �����
	void drawPlayboard();



	//�������� ������ �� �����������
	Cell* getCell(int x, int y) { return _board[x][y]; }
	Cell* getCell(xy position) { return _board[position.getX()][position.getY()]; }
	//�������� ������ ��������� �����
	std::vector<xy>& getPossibleMoves() { return _possibleMoves; }


	//�������� �������� �����
	int getW() { return _tableW; }
	int getH() { return _tableH; }





	//������� ������� �� ������, �������� �� ��� ����� ����
	bool Select(int x, int y, CheckerColor b_or_w);


};