#include "playboard.h"

//��� �������� 
//�-� ��������� ������� � �����
//�����������: ��� ����� - ��� �����������, �� ���� ��� ����� �� ����� � ������������ ���������� (���� ���������� ����������)
//�� �� ���� ����� ������ ���� ������ ����
Playboard& Playboard::getPlayboard() {
	static Playboard instance;
	return instance; //�� ���� ���������� ������ ���� ��������� ������� �����, ����� ������� ����������
}

//����������� ������ �����
Playboard::Playboard() {
	_selectedCoor = OFF_MAP;

	//� ���� ����� ���������� ���������� ���� �������� � �������
	for (int y = 0; y < getH(); y++) {
		for (int x = 0; x < getW(); x++) {
			if (y < 3) { //���� ���������� y<3 - ���������� ������ �������
				if (y % 2 == 0) { //���������� ������� �� ������� �� ������� y �� 2. ���� �� = 0
					_board[x][y] = new ActiveCell(WHITE); //��������� �������� ������
					x++; //��������� ������
					_board[x][y] = new Cell(); //��������� ���������� ������
				}
				else { //� ���� ������� �� ������� �� ����� 0
					_board[x][y] = new Cell(); //�� ������� ���������� ������
					x++;
					_board[x][y] = new ActiveCell(WHITE); //� ����� ��������
				}
			}
			else if (y > 4) { //���� ����� ��� ������ �����
				if (y % 2 == 0) {
					_board[x][y] = new ActiveCell(BLACK);
					x++;
					_board[x][y] = new Cell();
				}
				else {
					_board[x][y] = new Cell();
					x++;
					_board[x][y] = new ActiveCell(BLACK);
				}
			}

			//��� �� ���� �������� ����
			else {
				if (y % 2 == 0) {
					_board[x][y] = new ActiveCell();
					x++;
					_board[x][y] = new Cell();
				}
				else {
					_board[x][y] = new Cell();
					x++;
					_board[x][y] = new ActiveCell();
				}
			}
		}
	}
}

//������� ���������
void Playboard::drawPlayboard() {
	glLoadIdentity();
	glScalef(2.0f / getW(), 2.0f / getH(), 1);
	glTranslatef(-getW() / 2.0f, -getH() / 2.0f, 0); //�������������� ���������
	//�� ���� �-� ��������� ���� �� 8 ����� 8 ���� � ��

	//� ����� ���������
	for (int x = 0; x < getW(); x++)
	{
		for (int y = 0; y < getH(); y++)
		{
			glPushMatrix();
			glTranslatef(float(x), float(y), 0);
			getCell(x, y)->drawCell();
			glPopMatrix();
		}
	}
}

//������ �-� ��������� ������
void Playboard::setSelectedCell(int x, int y) {
	//���������� ��������� ������ � ��� ������, ���� � ��������� ������ ���� �����
	if (getCell(x, y)->isFree() == HAS_CHECKER) {
		setSelectedCoor(x, y);
		//������� selectCell ������� ������ ������ � ��� ������, ���� � ������ ���� �����
		getSelectedCell()->selectChecker();
	}
}


void Playboard::setSelectedCell(xy position) {
	setSelectedCoor(position);
	//������� selectCell ������� ������ ������ � ��� ������, ���� � ������ ���� �����
	getSelectedCell()->selectChecker();
}

int const DOWN = -1;
int const UP = 1;

//��������� ��������� ���� ��� ����� (�� ���� ������������ ���� ��� ���������� �����)
void Playboard::setPossibleMovesForChecker() {
	int dy = 1;
	int dx = 1;
	if (getSelectedCoor() == OFF_MAP) {
		return; //���� �� �������� ������, �� �������� ���� �� �����������
	}


	int x = getSelectedCoor().getX();
	int y = getSelectedCoor().getY();

	CheckerColor b_or_w = getSelectedCell()->getCheckerColor(); //����������� ����
	//��������� �� �������������� ����� � �������, ������� ����� ����������� ��� ���� ������
	if ((b_or_w == WHITE && y + dy < 8) || (b_or_w == BLACK && y - dy >= 0)) { //������� �������� ������ (��� ����� ������� �������, ��� ������ ������)
		(b_or_w == WHITE) ? dy = UP : dy = DOWN; //���� ���� ����� => ���������� ���� �����, ���� ������ => ����
		if (x + dx < _tableW) { //�������� �� ������ �������
			if (getCell(x + dx, y + dy)->isFree() == HAS_NO_CHECKER) { //���� ����� �� ������ x+dx,y+dy ���, 
				getCell(x + dx, y + dy)->makePossibleMove(); //�� ��� �������� (makePossibleMove ������ ��� ��������� ����������� ������)
				getPossibleMoves().push_back({ x + dx, y + dy }); //� � ������ ��������� ����� ���������� ������ ���
			}
		}
		if (x - dx >= 0) { //�������� �� ����� �������
			if (getCell(x - dx, y + dy)->isFree() == HAS_NO_CHECKER) { //���� �����,������ ��� ����� �������, ���� �� ��������� �����
				getCell(x - dx, y + dy)->makePossibleMove();
				getPossibleMoves().push_back({ x - dx, y + dy });
			}
		}
	}
}

//�������� �� ����������� ��������
void Playboard::checkEats(int dx, int dy) {
	if (getSelectedCoor() == OFF_MAP) {
		return;
	}
	int x = getSelectedCoor().getX();
	int y = getSelectedCoor().getY();
	CheckerColor b_or_w = getSelectedCell()->getCheckerColor();


	if (x + dx < getW() && y + dy < getH() && x + dx >= 0 && y + dy >= 0) { //�������� �� �������������� ��������
		if (getCell(x + dx, y + dy)->isFree() == HAS_CHECKER && //���� x+dx,y+dx ������ �����, �� ��������� ������� ����� � �� ��� ����� ������ (���� ���� �� �������)
			getCell(x + dx, y + dy)->getCheckerColor() != b_or_w)
		{ //��...
			if (x + 2 * dx < getW() && y + 2 * dy < getH() && x + 2 * dx >= 0 && y + 2 * dy >= 0) //�������� �� �������������� ������� ��� ������, ���� �� �������� ����� ��������
			{
				if (getCell(x + 2 * dx, y + 2 * dy)->isFree() == HAS_NO_CHECKER) { //� �������� ��� ������ ������, ���� �������� ����� ��������, �������� ������
					getCell(x + 2 * dx, y + 2 * dy)->makePossibleMove(); //makePossibleMove ������ ��� ���������, ����������� ������
					getPossibleMoves().push_back({ x + 2 * dx, y + 2 * dy }); //��������� ��� ��������� � ������ 
				}
			}
		}
	}
}

//������
int sign(int numbr) {
	int sgn = 0;
	numbr < 0 ? sgn = -1 : sgn = 1;

	return sgn;
}

//����, ��� �� ����� ��������, ������ �� ������� �������� �� �� ����� xy ������� ��� 
bool Playboard::checkEats(int x, int y, int dx, int dy) {
	CheckerColor b_or_w = getCell(x, y)->getCheckerColor();
	if (getCell(x, y)->getCheckerPost() == CHECKER) {
		if (x + dx < getW() && y + dy < getH() && x + dx >= 0 && y + dy >= 0) {
			if (getCell(x + dx, y + dy)->isFree() == HAS_CHECKER &&
				getCell(x + dx, y + dy)->getCheckerColor() != b_or_w)
			{
				if (x + 2 * dx < getW() && y + 2 * dy < getH() && x + 2 * dx >= 0 && y + 2 * dy >= 0)
				{
					if (getCell(x + 2 * dx, y + 2 * dy)->isFree() == HAS_NO_CHECKER) {
						return true;
					}
				}
			}
		}
	}

	//���� �������� �������� ��� � ��� ����� (������� �� �����)
	else if (getCell(x, y)->getCheckerPost() == KING) {
		int ddx = sign(dx);
		int ddy = sign(dy);
		while (x + dx >= 0 && x + dx < getW() && y + dy >= 0 && y + dy < getH()) {
			if (getCell(x + dx, y + dy)->isFree() == HAS_CHECKER &&
				getCell(x + dx, y + dy)->getCheckerColor() != b_or_w) {
				if (x + ddx + dx < getW() && y + ddy + dy < getH() && x + ddx + dx >= 0 && y + ddy + dy >= 0) {
					if (getCell(x + ddx + dx, y + ddy + dy)->isFree() == HAS_NO_CHECKER) {
						return true;
					}
					else
						return false;
				}
			}
			dx += ddx;
			dy += ddy;
		}
	}
	return false;
}

//�� ���� �� ������ �������� �� �������� � ������ �������
void Playboard::setPossibleEatesForChecker() {
	checkEats(1, 1);
	checkEats(-1, 1);
	checkEats(1, -1);
	checkEats(-1, -1);
}

//�-� �������� ���� �� ��� ����������� ������ ����� ���� ��� ������� ��������
bool Playboard::isThereAnotherEats(int x, int y) {
	if (checkEats(x, y, 1, 1) ||
		checkEats(x, y, -1, 1) ||
		checkEats(x, y, 1, -1) ||
		checkEats(x, y, -1, -1))
		return true;
	else
		return false;
}

//�-� �������� �� ���� �����, ��������� ������ ����� ������������� ����� �� ��, ����� �� ��� ����
void Playboard::findEaters(CheckerColor b_or_w) {
	for (int y = 0; y < getH(); y++) {
		for (int x = 0; x < getW(); x++) {
			if (getCell(x, y)->getCheckerColor() == b_or_w) {
				if (checkEats(x, y, 1, 1) ||
					checkEats(x, y, -1, 1) ||
					checkEats(x, y, 1, -1) ||
					checkEats(x, y, -1, -1)) {
					getCanEats().push_back(xy{ x,y }); //���� ���� �� ���� �� ���� 4-�� ���������� true, �� ���������� � �-� _canEats ���������� �����, ������� ����� ������
				}
			}
		}
	}
}

//�-� ��������� ����� �� �� ����� �� ��� ���� ��� ���. (������������ � ������� "����� ����������� ����")
//������: �������� �� ����� ������ ���� ����� ������ M. ��� ����� �� ����� ������ ����� ����� ������������ false
bool Playboard::checkEaters(int x, int y) {
	if (getCanEats().size() != 0) {
		for (auto position = getCanEats().begin(); position != getCanEats().end(); position++) { //���� ��������� ��������� � �������� canEats
			if ((*position) == xy{ x, y }) {
				return true; //�� ����� ������
			}
		}
		return false; //�� ����� ������
	}
	return true;
}

//�-�, ������� ������� ��� ���������� ����� ������� ����� ������, ����� ���������
//������ �������������: ����� ����� ����� ������ 2+ ���, ��� ������������ ��� �������� ��� ������ ���� (��� ��� �� ���� �����)
void Playboard::clearEaters(int x, int y) {
	if (getCanEats().size() > 1) { //�� ���� ���� ��������� �������� ������ 1
		for (auto position = getCanEats().begin(); position != getCanEats().end(); position++) { //������ �� �������
			if ((*position) != xy{ x, y }) {
				getCanEats().erase(position); //������� ����������, �� ������ �������
				if (getCanEats().size() == 1) {
					return;
				}
			}
		}
	}
}

void Playboard::ClearPossible() {
	for (auto position = getPossibleMoves().begin(); position < getPossibleMoves().end(); position++) {
		//������� ���������� ��������� ������ ��� ���������� ����
		getCell(*position)->makeImPossibleMove();
	}
	getPossibleMoves().clear();
}

void Playboard::ClearSelected() {
	if (getSelectedCoor() != OFF_MAP) {
		//������� selectCell ������ ������ ����������
		getSelectedCell()->selectChecker();
		setSelectedCoor(OFF_MAP);
	}
}


//����� ��� ��� �����
void Playboard::findFoodForKing(int dx, int dy)
{
	//����� ���� � ����� ������� �� ��������
	int ddx = sign(dx);
	int ddy = sign(dy);

	//�������� ���������� ��������� ����� � ����
	int x0 = getSelectedCoor().getX();
	int y0 = getSelectedCoor().getY();
	CheckerColor b_or_w = getSelectedCell()->getCheckerColor();

	//�������� �� ����� �� ��� ���, ���� �� � �������� �������� ���� (������ ��� �����)
	while (x0 + dx >= 0 && x0 + dx < getW() && y0 + dy >= 0 && y0 + dy < getH()) {
		if (getCell(x0 + dx, y0 + dy)->isFree() == HAS_CHECKER && //����� ��������, ���� �� � ������ x0+dx,y0+dy ����� ������� �����
			getCell(x0 + dx, y0 + dy)->getCheckerColor() != b_or_w) {
			if (x0 + ddx + dx >= 0 && x0 + ddx + dx < getW() && y0 + ddy + dy >= 0 && y0 + ddy + dy < getH() && //���� ������ ������
				getCell(x0 + ddx + dx, y0 + ddy + dy)->isFree() == HAS_NO_CHECKER) { // � ��� ����� � ������
				dx += ddx;
				dy += ddy;//�� ��������� �� ��� ������
				while (x0 + dx >= 0 && x0 + dx < getW() && y0 + dy >= 0 && y0 + dy < getH() && // � ���� �� ��������� �� ����� ����� ��� �����
					getCell(x0 + dx, y0 + dy)->isFree() == HAS_NO_CHECKER) {
					getCell(x0 + dx, y0 + dy)->makePossibleMove();//������ ������ ��������� � ����
					getPossibleMoves().push_back({ x0 + dx, y0 + dy });
					dx += ddx;
					dy += ddy;
				}
				return;
			}
			else
				return;
		}
		dx += ddx;
		dy += ddy;
	}
}

void Playboard::setPossibleEatesForKing() {//�������� �������� ���������� ������� �� ��� ������ �������
	findFoodForKing(1, 1);
	findFoodForKing(-1, 1);
	findFoodForKing(1, -1);
	findFoodForKing(-1, -1);

}

//�-� ���� ���� ��� �����, ��� ����������� ����� ����, ����� ��� ��������� �������� ��� �����
void Playboard::findMoveForKing(int dx, int dy) {
	int x0 = getSelectedCoor().getX();
	int y0 = getSelectedCoor().getY();
	int ddx = sign(dx);
	int ddy = sign(dy);
	while (x0 + dx >= 0 && x0 + dx < getW() && y0 + dy >= 0 && y0 + dy < getH() && //���� � �������� �������� ����
		getCell(x0 + dx, y0 + dy)->isFree() == HAS_NO_CHECKER) { //� ���� ��� ����� �� ������
		getCell(x0 + dx, y0 + dy)->makePossibleMove(); //������ ���� ��� ���������
		getPossibleMoves().push_back({ x0 + dx, y0 + dy }); //���������� � ������ ��������� �����
		dx += ddx;
		dy += ddy; //������������� ������
	}
}

//�������� �������� ���������� ������� �� ��� ������ �������
void Playboard::setPossibleMovesForKing() {
	if (getSelectedCoor() == OFF_MAP) {
		return;
	}
	findMoveForKing(1, 1);
	findMoveForKing(1, -1);
	findMoveForKing(-1, 1);
	findMoveForKing(-1, -1);
}

//�-� �������� �����
void Playboard::SelectChecker(int x, int y, CheckerColor b_or_w) {

	if (checkEaters(x, y) == true) { //true ����� ���� ������ ���� ��� ����������� �������� (�-� ����) ��� ������� �� �����, ������� ����� ������
		//���� ����� ��������� � ������ � � ������, �� ������
		if (getCell(x, y)->getCheckerColor() == b_or_w) {
			setSelectedCell(x, y); //�������� �����
			//��������� ������ �� ��� ����� � ����� ������� ����, ���� �� �� ��������
			if (getCell(x, y)->isFree() == HAS_CHECKER) {
				if (getCell(x, y)->getCheckerPost() == CHECKER) {
					//��������� ��������� �������� �����
					setPossibleEatesForChecker();
					//���� ��������� �������� ���, �� ������������� ���� ��� ��������
					if (getPossibleMoves().size() == 0) {
						setPossibleMovesForChecker();
					}
				}
				else if (getCell(x, y)->getCheckerPost() == KING) {
					//��������� ��������� �������� �����
					setPossibleEatesForKing();
					//���� ��������� �������� ���, �� ������������� ���� ��� ��������
					if (getPossibleMoves().size() == 0) {
						setPossibleMovesForKing();
					}
				}
			}
		}
	}
}

//������ ��������� ��������� ����� �� ������������� ������� ��������� �����
bool Playboard::isMovePossible(int x, int y) {
	for (auto position = getPossibleMoves().begin(); position != getPossibleMoves().end(); position++) {
		if ((*position) == xy{ x, y }) {
			return true;
		}
	}
	return false;
}

//�-� ����������, �������� ��� ���� ��� ���
bool Playboard::isEating(int x, int y) {
	int dx = x - getSelectedCoor().getX();
	int dy = y - getSelectedCoor().getY();
	return (abs(dx) == 2 && abs(dy) == 2); //���� ������� ����� �������� ==2, �������������� ��� ���� ��������, � �� ������ ���
}

//�-� ������ ������ ������� (��� ����� ��� ���������)
void Playboard::swapCells(xy pos1, xy pos2) {
	ClearSelected();
	std::swap(_board[pos1.x][pos1.y], _board[pos2.x][pos2.y]);
}

//��� ������������ ����� ����� �������� ���� ��� ���� ��������
void Playboard::swapCellsWithoutSelected(xy pos1, xy pos2) {
	std::swap(_board[pos1.x][pos1.y], _board[pos2.x][pos2.y]);
	setSelectedCoor(pos2); //����� �������� ����������� ������ 
	if (getSelectedCell()->getCheckerPost() == CHECKER) { //����� �������, �������� �� ����� ������� �� ����� ������ ��� ������
		setPossibleEatesForChecker();
	}
	else if (getSelectedCell()->getCheckerPost() == KING) {
		setPossibleEatesForKing();
	}
}

const int THIS_IS_MOVE = 0; // � ������ ���� �� ������� � ����� ��������� ���
const int THERE_IS_ANOTHER_MOVE = 1; // � ������ ���� �� ����� � ����� ������ ���
const int THIS_IS_NOT_A_MOVE = 2; // � ������ ���� �� �� �������, ������ �������� �� �����

//��� �-� - ������ �-�� ���� isEating, ������ ��� �����
bool Playboard::isEatingForKing(xy& pos, int x, int y) {
	int ddx = sign(x - getSelectedCoor().getX());
	int ddy = sign(y - getSelectedCoor().getY());
	int dx = ddx;
	int dy = ddy;
	while (getSelectedCoor().getX() + dx != x && getSelectedCoor().getY() + dy != y) {
		if (getCell(getSelectedCoor().getX() + dx, getSelectedCoor().getY() + dy)->isFree() == HAS_CHECKER) { //���� �� ���� ���� ���� �����...
			pos.x = getSelectedCoor().getX() + dx;
			pos.y = getSelectedCoor().getY() + dy;
			return true; //�� ��� ��������
		}
		dx += ddx;
		dy += ddy;
	}
	return false;
}

//�-� ����������� �����, ��������� ���������� � ����
int Playboard::MoveChecker(int x, int y, CheckerColor b_or_w) {


	if (isMovePossible(x, y) == true) { //�������� �� ��, �������� �� ���
		if (getSelectedCell()->getCheckerPost() == CHECKER) { //���� ���������� ����� - �� �����
			if ((b_or_w == WHITE && y == getH() - 1) || (b_or_w == BLACK && y == 0)) { //� ���� ����� �������� ����� � ������ �������� ����, �� ��� ���������� �������
				getCell(getSelectedCoor().getX(), getSelectedCoor().getY())->setPost(KING); //��� ��������� ������ ������ ��������� �����
			}

			if (isEating(x, y)) { //���������, ���� �� ��� ���������
				//������
				int dx = (x - getSelectedCoor().getX()) / 2; //����� �� 2 �� ����� �������� �������� (� �������� ������ ��������� �����)
				int dy = (y - getSelectedCoor().getY()) / 2;
				refreshCell(getSelectedCoor().getX() + dx, getSelectedCoor().getY() + dy);
				//������ ������

				ClearPossible(); //������� ���� �� ��������� ����� (�� �� ��� �������)

				//���������� ������ (������ � ������) � �� ������ ���� �� ��������
				swapCellsWithoutSelected(getSelectedCoor(), xy{ x,y });
				if (isThereAnotherEats(x, y)) { //��� ����� ��������� ����� �� �� ��� ������
					return THERE_IS_ANOTHER_MOVE; //���������� ��� ����� ���� ��� ����������� ��������
				}
				//���� ��� ��� ����������� ��������, �� ������� ��������� ������ � ����������� ���
				ClearSelected();
				return THIS_IS_MOVE;
			}
			//� ���� �� ��������...
			else {// ������ ������������
				ClearPossible();
				swapCells(getSelectedCoor(), xy{ x,y });
				return THIS_IS_MOVE;
			}
		}
		//� ���� ��� �����...
		if (getSelectedCell()->getCheckerPost() == KING) {
			xy pos_with_checker = OFF_MAP;
			if (isEatingForKing(pos_with_checker, x, y)) {
				refreshCell(pos_with_checker.getX(), pos_with_checker.getY()); // ������� ������� �����

				ClearPossible();

				//����������
				swapCellsWithoutSelected(getSelectedCoor(), xy{ x,y });
				if (isThereAnotherEats(x, y)) {
					return THERE_IS_ANOTHER_MOVE;
				}
				ClearSelected();
				return THIS_IS_MOVE;
			}
			//� ���� ��� �� ��������...
			else {
				ClearPossible();
				swapCells(getSelectedCoor(), xy{ x,y }); //�� ������ ������ ������� (������� ��� ���)
				return THIS_IS_MOVE;
			}
		}
	}
	return THIS_IS_NOT_A_MOVE;
}

//�-� ����� ���� �� ������� �����
bool Playboard::Select(int x, int y, CheckerColor b_or_w) {

	//������� ����������� �-� ���������� ������� �����, ������� ����� ����
	findEaters(b_or_w);

	if (situation == THERE_IS_ANOTHER_MOVE || turn_end == false) {
		clearEaters(getSelectedCoor().getX(), getSelectedCoor().getY());
	}//�� ������ ����� �� ����� ���� ���, ����� ������ ���, �� ��� ���� ����� ����� ������� ���� ����� ����
	situation = MoveChecker(x, y, b_or_w);
	if (situation == THIS_IS_MOVE) {
		turn_end = true; //����� ����
		clearEaters();   //������ �-� 
		return true;
	}
	else if (situation == THERE_IS_ANOTHER_MOVE) { //�������� ����� ���� ��� ���� ����������� �������
		turn_end = false; //���� ���� ��� ��� �� ��������
		clearEaters();
		return false;
	}
	else if (situation == THIS_IS_NOT_A_MOVE && turn_end == true) {
		turn_end = true;
		ClearSelected();
		ClearPossible();
		SelectChecker(x, y, b_or_w);
		clearEaters();
	}
	clearEaters();
	return false;
}

