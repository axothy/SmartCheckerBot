#include "SmartBot.h"
#include <array>
#include <iostream>
#include <time.h>
#include <thread>
#include <chrono>
#include <future>


class Timer {
public:
	Timer() {
		start = std::chrono::high_resolution_clock::now();
	}
	~Timer() {
		end = std::chrono::high_resolution_clock::now();
		auto duration = end - start;
		std::cout << duration.count() << std::endl;
	}
private:
	std::chrono::time_point<std::chrono::steady_clock> start, end;
};

//������
static int sign(int numbr) {
	if (numbr == 0) {
		return 0;
	}
	int sgn = 0;
	numbr < 0 ? sgn = -1 : sgn = 1;

	return sgn;
}


//����������� ������ ���������� ����� - ��� ����������� �������� ������� �� �������� ����
constexpr std::pair<int, int> NonActiveCells[] = {
	{0,1},{0,3},{0,5},{0,7},{1,0},{1,2},{1,4},{1,6},
	{2,1},{2,3},{2,5},{2,7},{3,0},{3,2},{3,4},{3,6},
	{4,1},{4,3},{4,5},{4,7},{5,0},{5,2},{5,4},{5,6},
	{6,1},{6,3},{6,5},{6,7},{7,0},{7,2},{7,4},{7,6}
};

//����������� ������ �������� ����� - ��� ����������� �������� ������� �� �������� ����
constexpr std::pair<int, int> ActiveCells[] = {
	{1,1},{1,3},{1,5},{1,7},{0,0},{0,2},{0,4},{0,6},
	{3,1},{3,3},{3,5},{3,7},{2,0},{2,2},{2,4},{2,6},
	{5,1},{5,3},{5,5},{5,7},{4,0},{4,2},{4,4},{4,6},
	{7,1},{7,3},{7,5},{7,7},{6,0},{6,2},{6,4},{6,6}
};


//����������� ����
SmartBot::SmartBot(int b_or_w, Playboard* instance) : Player(instance) {

	//����������� ���� �����
	color = static_cast<Positions>(b_or_w);

	//�������������� ������� ���� ������ ���� - �������������� ���������� ������
	for (auto& nonactcell : NonActiveCells) {
		PosBoard[nonactcell.first][nonactcell.second] = static_cast<int>(Positions::CELL_UNPLBL);
	}

	//�������������� �������� ������
	for (auto& actcell : ActiveCells) {
		PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::CELL_PLBL);
	}

	//���������� ���� ����� ��������� ����
	if (color == Positions::WHITE_CHECKER) {
		enemycolor = Positions::BLACK_CHECKER;
	}
	else
		enemycolor = Positions::WHITE_CHECKER;
}


//������ �������� ����
void SmartBot::BoardScanner() {

	//�������� ������ �� �������� �������
	for (auto& actcell : ActiveCells) {

		//���������� ���� �� ������ ������
		CheckerColor check = gameCheckers->getCell(actcell.first, actcell.second)->getCheckerColor();

		//���������� ����� ��� ��� ������, ���� ���-�� �����
		CheckerHierarchy check_post = gameCheckers->getCell(actcell.first, actcell.second)->getCheckerPost();

		//���� ������ �� �����,
		//�� ��� ������ �������� ������
		if (check == EXCEPTION_COLOR) {
			PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::CELL_PLBL);
		}
		else if (check == WHITE) {
			if (check_post == CHECKER) {
				PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::WHITE_CHECKER);
			}
			else
				PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::WHITE_KING);
		}
		else if (check == BLACK) {
			if (check_post == CHECKER) {
				PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::BLACK_CHECKER);
			}
			else
				PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::BLACK_KING);
		}
	}
}

//������� ���� ����
bool SmartBot::Turn(int x, int y, CheckerColor b_or_w) {

	//�� ������ ���� �������� ������������ �������� ����
	BoardScanner();

	//������� ����� "�����������" ���
	PosTurn turn = ReturnMove();


	//������� ������ �� ������� ���������� ���
	gameCheckers->Select(turn.from.first, turn.from.second, b_or_w);
	//������ �� ������ ���� ��� ���������� ���
	gameCheckers->Select(turn.to.first, turn.to.second, b_or_w);

	//���� ���� ������������� ��������, ���������� �� ������� ������������� ��������
	//�������� ����� ��������
	if (!turn.another_eats.empty()) {
		for (auto& eats : turn.another_eats) {
			gameCheckers->Select(eats.first, eats.second, b_or_w);
		}
	}

	//�������� ������� �����
	RefreshVectors();

	//��� ��������
	return true;
}

//���������� �������� �����
void SmartBot::RefreshVectors() {
	PosMoves.clear();
	PosMovesEnemy.clear();
	//��������� � ��������� ��������� ���� �������� ������
	canEnemyEat = false;
}


//�������� ������ ����
inline bool SmartBot::CheckBorders(int x, int y, int dx, int dy) {
	return x + dx >= 0 && x + dx < 8 && y + dy >= 0 && y + dy < 8;
}

//�������� ���� �� ����������� ������
inline bool SmartBot::CheckBecomeKing(int y, Positions Col) {
	if (Col == Positions::WHITE_CHECKER) {
		//��� ����� - ���� ���������� ������� ������� ����
		if (y == 7) {
			return true;
		}
	}
	else if (Col == Positions::BLACK_CHECKER) {
		//��� ������ - ���� ���������� ������ ������� ����
		if (y == 0) {
			return true;
		}
	}
	return false;
}


/*������� ���������� ��������� �������� ������� ������ � ���� �������, ������� ������������ ����������� dx dy
* Board - ���� �� ������� ���������� ����������� ����
* x, y - ������
* dx dy - ���������� ����������
* Col - ���� ����
* enemyCol - ���� ���������
* movesVec - ������ ��������� �����
*/
void SmartBot::CheckEats(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy,
	Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {

	//�������� �������
	if (CheckBorders(x, y, dx, dy)) {

		//�������� �������� ������ � ������������ � �������� ���������� �� ������� ����� ���������
		if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {

			//�������� ������, �� ������� ����� ������������ � ������ ��������, ��������� �� ��� � �������� ���� � ������ �� ���
			if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)) {

				//���� ��� ������� ���������
				// 
				//������� ����������
				//xy - ������ ������������
				//eatedxy - ��� �����
				//dxdy - ���� �������������
				auto xy = std::make_pair(x, y);
				auto eatedxy = std::make_pair(x + dx, y + dy);
				auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);

				//��������� ������ ����� � ��������� ������ ��������� ����� �� ���� ���
				movesVec.push_back({ xy, dxdy });
				movesVec[movesVec.size() - 1].whoWasEated.push_back(eatedxy);


				//�������� ������� ����
				//�������� �� ������� ��������
				movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO) + static_cast<int>(SituationCost::EAT);

				//���� ����� �����, �� �������� �� �������� �������� �����
				if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {
					movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::EAT_KING);
				}
				//���� ����� ������ ������ � ������ ����� ����, �� �������� ������� �� �������� ����������� ������
				if (CheckBecomeKing(dxdy.second, Col)) {
					movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::BECOME_KING);
				}
			}
		}
	}
}


/*������� ���������� ��������� �������� ������ � ���� �������, ������� ������������ ����������� dx dy
* Board - ���� �� ������� ���������� ����������� ����
* x, y - ������
* dx dy - ���������� ����������
* Col - ���� ����
* enemyCol - ���� ���������
* movesVec - ������ ��������� �����
*/
void SmartBot::CheckEatsForKing(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy,
	Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {

	//���������� ���� ����������
	int ddx = sign(dx);
	int ddy = sign(dy);

	//��������� ����, ���� ��������� � ��������
	while (CheckBorders(x, y, dx, dy)) {

		//�������� �������� ������ � ������������ � �������� ���������� �� ������� ����� ���������
		if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {

			//�������� ������, �� ������� ����� ������������ � ������ ��������, ��������� �� ��� � �������� ���� � ������ �� ���
			if (CheckBorders(x, y, dx + ddx, dy + ddy) && Board[x + dx + ddx][y + dy + ddy] == static_cast<int>(Positions::CELL_PLBL)) {

				//���� ����� ������ �����, �� ��������� � ���������� � ������ ��������� �����
				auto eatedxy = std::make_pair(x + dx, y + dy);

				//��������� �� ������ ������
				dx += ddx;
				dy += ddy;

				//������ ������� ��� ��������� ���� � ���� ������� ��� ����� ����� ��������
				while (CheckBorders(x, y, dx, dy) && Board[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) {

					//���� ��� ������� ���������
					// 
					//������� ����������
					//xy - ������ ������������
					//dxdy - ���� �������������
					auto xy = std::make_pair(x, y);
					auto dxdy = std::make_pair(x + dx, y + dy);
					movesVec.push_back({ xy, dxdy });
					movesVec[movesVec.size() - 1].whoWasEated.push_back(eatedxy);

					//���� ����� ������ �������� �������
					movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO) + static_cast<int>(SituationCost::EAT);
					//���� ������� ����� �� ��� �������� ������� ����
					if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {
						movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::EAT_KING);
					}
					//������������� ������
					dx += ddx;
					dy += ddy;
				}
				//return � ������, ���� �� ������ �������� �� ������� ��� ������ ������
				return;
			}
			else
				return;
		}
		//������������� ������ ���� �� �������� ����� ���������
		dx += ddx;
		dy += ddy;
	}

}

//������ ��� ��������� �������� ��� ���� �����
void SmartBot::FindEats(std::array<std::array<int, 8>, 8>& Board, Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {
	//���������� �� ���� �������(�� ��������������)
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {

			//���� ���� ������ ��������� � ��������, �� ��������� �� �������� �� ��� 4 �������
			if (Board[x][y] == static_cast<int>(Col)) {
				CheckEats(Board, x, y, 1, 1, Col, enemyCol, movesVec);
				CheckEats(Board, x, y, -1, 1, Col, enemyCol, movesVec);
				CheckEats(Board, x, y, 1, -1, Col, enemyCol, movesVec);
				CheckEats(Board, x, y, -1, -1, Col, enemyCol, movesVec);
			}
			//���� ���� ������ ��������� � �����, �� �������� ������, �� ��������� �� �������� ������ �� ��� 4 �������
			if (Board[x][y] == static_cast<int>(Col) + 2) {
				CheckEatsForKing(Board, x, y, 1, 1, Col, enemyCol, movesVec);
				CheckEatsForKing(Board, x, y, -1, 1, Col, enemyCol, movesVec);
				CheckEatsForKing(Board, x, y, 1, -1, Col, enemyCol, movesVec);
				CheckEatsForKing(Board, x, y, -1, -1, Col, enemyCol, movesVec);
			}
		}
	}
}


/*������� ���������� ��������� ����� ������� ������ �� ��� �������, ������� ������������ ������ �����
* Board - ���� �� ������� ���������� ����������� ����
* x, y - ������
* Col - ���� ����
* movesVec - ������ ��������� �����
*/
void SmartBot::CheckMoves(std::array<std::array<int, 8>, 8>& Board, int x, int y, Positions Col, std::vector<PosTurn>& movesVec) {
	//���������� �����������
	int dx = 1, dy;

	//���� ����� - ���� �����, � ������� �����
	if (Col == Positions::WHITE_CHECKER) {
		dy = 1;
	}
	else
		dy = -1;

	//������� ������
	//��������� �������
	if (CheckBorders(x, y, dx, dy)) {

		//���� ������ ������, ����� ������� �� ��
		if (Board[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) {

			//������� ���������� ����, ������ - ������, ������ - ����
			auto xy = std::make_pair(x, y);
			auto dxdy = std::make_pair(x + dx, y + dy);
			movesVec.push_back({ xy, dxdy });

			//���� ����� ������� �� ��������� ��������������� ��������
			movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO);

			//���� ���������� ������, �� ��������� ��������������� �������
			if (CheckBecomeKing(dxdy.second, Col)) {
				movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::BECOME_KING);
			}
		}
	}

	//������ �� �� ����� ������ �����
	dx = -1;

	if (CheckBorders(x, y, dx, dy)) {
		if (PosBoard[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) {
			auto xy = std::make_pair(x, y);
			auto dxdy = std::make_pair(x + dx, y + dy);
			movesVec.push_back({ xy, dxdy });
			movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO);
			if (CheckBecomeKing(dxdy.second, Col)) {
				movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::BECOME_KING);
			}
		}
	}
}


/*������� ���������� ��������� ����� ������ � ���� �������, ������� ������������ ����������� dx dy
* Board - ���� �� ������� ���������� ����������� ����
* x, y - ������
* dx dy - ���������� ����������
* movesVec - ������ ��������� �����
*/
void SmartBot::CheckMovesForKing(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy, std::vector<PosTurn>& movesVec) {
	//���������� ���� ����������
	int ddx = sign(dx);
	int ddy = sign(dy);

	//��������� ������� � ���������� ����� �� ������
	while (CheckBorders(x, y, dx, dy) && PosBoard[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) { //� ���� ��� ����� �� ������

		//������� ���������� ������-���� � ������ ����
		auto xy = std::make_pair(x, y);
		auto dxdy = std::make_pair(x + dx, y + dy);
		movesVec.push_back({ xy, dxdy });
		movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO);
		dx += ddx;
		dy += ddy; //������������� ������
	}
}


/*������� ���������� ���� ��������� �����
* Board - ���� �� ������� ���������� ����������� ����
* Col - ���� ����
* movesVec - ������ ��������� �����
*/
void SmartBot::FindMoves(std::array<std::array<int, 8>, 8>& Board, Positions Col, std::vector<PosTurn>& movesVec) {

	//���������� �� ����� ����
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			//���� ������� �����
			if (Board[x][y] == static_cast<int>(Col)) {
				CheckMoves(Board, x, y, Col, movesVec);
			}
			//���� �����
			else if (Board[x][y] == static_cast<int>(Col) + 2) {
				//�������� �� ��� �������
				CheckMovesForKing(Board, x, y, 1, 1, movesVec);
				CheckMovesForKing(Board, x, y, -1, 1, movesVec);
				CheckMovesForKing(Board, x, y, 1, -1, movesVec);
				CheckMovesForKing(Board, x, y, -1, -1, movesVec);
			}
		}
	}
}


//�������� ������� ��������� �����, ���� �� ������� ����� �� ������ ����������
bool SmartBot::IsWasChecked(int x, int y, std::vector<std::pair<int, int>>& enemyPos) {
	for (auto& enemy : enemyPos) {
		//���� ���������� ����� � �������� ��������� ��������� �� ���� �������
		if (enemy.first == x && enemy.second == y) {
			return true;
		}
	}
	//����� ���
	return false;
}


bool SmartBot::IsWasCheckedByFather(int x, int y, ternaryTree* wayTree) {
	ternaryTree* helpTree = wayTree;
	while (helpTree->father != nullptr && helpTree->father->father != nullptr) {
		if (helpTree->father->getXto() == x && helpTree->father->getYto() == y) {
			return false;
		}
		helpTree = helpTree->father;
	}


}


/*������� �������� ��������� �������� ����� ���� ��� ����� �����
* Board - ������� ����
* wayTree - �������� ������ ������������ ����
* enemyCol - ���� ���������
* stop - ���������� ���������, ����� �� ������ �� �����
* enemyPos - ������ ��������� �����
*/
void SmartBot::CheckContinue
(std::array<std::array<int, 8>, 8>& Board, ternaryTree* wayTree, Positions enemyCol,
	std::pair<int, int> stop, std::vector<std::pair<int, int>>& enemyPos) {

	//�������� ���������� ��� ��������
	int dx = wayTree->getDX();
	int dy = wayTree->getDY();
	int x = wayTree->getXto();
	int y = wayTree->getYto();

	//�������� ������
	if (CheckBorders(x, y, dx, dy)) {

		//�������� �������� ������ � ������������ � �������� ���������� �� ������� ����� ���������
		if ((Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2)

			//�������� ���� �� ������� ������ ����� � �� �������� �� ������ ���������� ����������� ���������
			&& !IsWasChecked(x + dx, y + dy, enemyPos) && (x + dx != stop.first && y + dy != stop.second)) {

			//��������� ������� ������, ���� ����� ������������ ����� ��������
			if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)) {


				//������� ���������� ������-���� � ������ ����
				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);

				//������� ������ ������
				wayTree->incrementHeight();
				wayTree->first = new ternaryTree(xy, dxdy, wayTree);

				//�������� ����������� ����������� �������� ��� ���� ����� ����
				CheckContinue(Board, wayTree->first, enemyCol, stop, enemyPos);

			}
		}
	}
	//�� �� ����� ��� � ������ ������ ��� ������� �����������
	if (CheckBorders(x, y, dx, -dy)) {

		if ((Board[x + dx][y - dy] == static_cast<int>(enemyCol) || Board[x + dx][y - dy] == static_cast<int>(enemyCol) + 2)
			&& !IsWasChecked(x + dx, y - dy, enemyPos) && (x + dx != stop.first && y - dy != stop.second)) {

			if (CheckBorders(x, y, 2 * dx, -2 * dy) && Board[x + 2 * dx][y - 2 * dy] == static_cast<int>(Positions::CELL_PLBL)
				&& !(x + 2 * dx == stop.first && y - 2 * dy == stop.second) &&
				//����������� ��� ���� ��������, ��� �� �������� ������ ��� ��������
				//������� ��� ����, ����� �� ������ �� �����
				//� ������ ������ ����� �� ����������, ��� ��� � ������ ������������� ��������,
				//������������ � ����� ����������� �� ���� ������������
				IsWasCheckedByFather(x + 2 * dx, y - 2 * dy, wayTree)) {

				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + 2 * dx, y - 2 * dy);


				wayTree->incrementHeight();
				wayTree->second = new ternaryTree(xy, dxdy, wayTree);


				CheckContinue(Board, wayTree->second, enemyCol, stop, enemyPos);
			}
		}
	}
	//�� �� �����
	if (CheckBorders(x, y, -dx, dy)) {

		if ((Board[x - dx][y + dy] == static_cast<int>(enemyCol) || Board[x - dx][y + dy] == static_cast<int>(enemyCol) + 2)
			&& !IsWasChecked(x - dx, y + dy, enemyPos)) {

			if (CheckBorders(x, y, -2 * dx, 2 * dy) && Board[x - 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)
				&& !(x - 2 * dx == stop.first && y + 2 * dy == stop.second) &&
				IsWasCheckedByFather(x - 2 * dx, y + 2 * dy, wayTree)) {

				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x - 2 * dx, y + 2 * dy);


				wayTree->incrementHeight();
				wayTree->third = new ternaryTree(xy, dxdy, wayTree);


				CheckContinue(Board, wayTree->third, enemyCol, stop, enemyPos);

			}
		}
	}
}



//������� ����������� ���, � ������ ������� ������������� ����� � ��������
//����������� ������� �����������
//����� � ���� ������ ������
void SmartBot::FillAnotherEats(ternaryTree* wayTree, PosTurn& pos) {
	if (wayTree->first == nullptr && wayTree->second == nullptr && wayTree->third == nullptr) {
		return;
	}
	else if (wayTree->first != nullptr && wayTree->second == nullptr && wayTree->third == nullptr) {
		pos.another_eats.push_back(wayTree->first->_to);
		FillAnotherEats(wayTree->first, pos);
	}
	else if (wayTree->second != nullptr && wayTree->first == nullptr && wayTree->third == nullptr) {
		pos.another_eats.push_back(wayTree->second->_to);
		FillAnotherEats(wayTree->second, pos);
	}
	else if (wayTree->third != nullptr && wayTree->second == nullptr && wayTree->first == nullptr) {
		pos.another_eats.push_back(wayTree->third->_to);
		FillAnotherEats(wayTree->third, pos);
	}
	else if (wayTree->third == nullptr) {
		if (wayTree->first->height > wayTree->second->height) {
			pos.another_eats.push_back(wayTree->first->_to);
			FillAnotherEats(wayTree->first, pos);
		}
		else {
			pos.another_eats.push_back(wayTree->second->_to);
			FillAnotherEats(wayTree->second, pos);

		}
	}
	else if (wayTree->second == nullptr) {
		if (wayTree->first->height > wayTree->third->height) {
			pos.another_eats.push_back(wayTree->first->_to);
			FillAnotherEats(wayTree->first, pos);
		}
		else {
			pos.another_eats.push_back(wayTree->third->_to);
			FillAnotherEats(wayTree->third, pos);

		}
	}
	else if (wayTree->first == nullptr) {
		if (wayTree->second->height > wayTree->third->height) {
			pos.another_eats.push_back(wayTree->second->_to);
			FillAnotherEats(wayTree->second, pos);
		}
		else {
			pos.another_eats.push_back(wayTree->third->_to);
			FillAnotherEats(wayTree->third, pos);
		}
	}
	else if (wayTree->first->height > wayTree->second->height && wayTree->first->height > wayTree->third->height) {
		pos.another_eats.push_back(wayTree->first->_to);
		FillAnotherEats(wayTree->first, pos);

	}
	else if (wayTree->second->height > wayTree->first->height && wayTree->second->height > wayTree->third->height) {
		pos.another_eats.push_back(wayTree->second->_to);
		FillAnotherEats(wayTree->second, pos);
	}
	else if (wayTree->third->height > wayTree->first->height && wayTree->third->height > wayTree->second->height) {
		pos.another_eats.push_back(wayTree->second->_to);
		FillAnotherEats(wayTree->second, pos);
	}
	else if (wayTree->first->height == wayTree->second->height) {
		pos.another_eats.push_back(wayTree->first->_to);
		FillAnotherEats(wayTree->first, pos);
	}
	else if (wayTree->second->height == wayTree->third->height) {
		pos.another_eats.push_back(wayTree->second->_to);
		FillAnotherEats(wayTree->second, pos);
	}
}


//������� ������������ ������ �����������, �� �������� ����� �������� ����
inline Ways getWay(int dx, int dy) {
	if (dx == 1 && dy == 1) {
		return FIRST;
	}
	else if (dx == -1 && dy == 1) {
		return SECOND;
	}
	else if (dx == 1 && dy == -1) {
		return THIRD;
	}
	else if (dx == -1 && dy == -1) {
		return FOURTH;
	}
	return FATHER;
}


//������� ���������� �������������� �������� ������
void SmartBot::FindWay(int x, int y, int dx, int dy, std::array<std::array<int, 8>, 8> Board, quadrupleTree* wayTree, Positions enemyCol) {

	//�� ������� ����������� �������� ����
	auto destiny = getWay(dx, dy);

	//��������� ������� ����
	while (CheckBorders(x, y, dx, dy)) {

		//��������� ������� �� ��, ���� �� ����� �� ����
		if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2)
		{
			//���� ���� �� ��������� �������� �� �� ���
			if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)) {

				//������ ������� ����� 
				std::swap(Board[x][y], Board[x + 2 * dx][y + 2 * dy]);

				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);

				Board[x + dx][y + dy] = static_cast<int>(Positions::CELL_PLBL);


				//��������� ������ ���� ����� �����
				wayTree->incrementHeight();
				wayTree->way[destiny] = new quadrupleTree(xy, dxdy, wayTree);
				wayTree->way[destiny]->eated = std::make_pair(x + dx, y + dy);

				//��������
				CheckContinueForKing(Board, wayTree->way[destiny], enemyCol);

			}
		}
		//���� ���, �� ���� ������ � ������ �����
		x += dx;
		y += dy;
	}
}


//�������, ��������� ��������� �������������� �������� ��� ����� �� ��� 4 �������
void SmartBot::CheckContinueForKing
(std::array<std::array<int, 8>, 8> Board, quadrupleTree* wayTree, Positions enemyCol) {
	int dx = sign(wayTree->getDX());
	int dy = sign(wayTree->getDY());
	int x = wayTree->getXto();
	int y = wayTree->getYto();

	FindWay(x, y, dx, dy, Board, wayTree, enemyCol);
	FindWay(x, y, -dx, dy, Board, wayTree, enemyCol);
	FindWay(x, y, dx, -dy, Board, wayTree, enemyCol);
	FindWay(x, y, -dx, -dy, Board, wayTree, enemyCol);
}


//�������, ���������� ����������� ���� �������� �����, ��� ���� ����� ������ �� ���������
void SmartBot::FillAnotherEatsForKing(quadrupleTree* wayTree, PosTurn& pos) {
	int height = -1;

	//������� ������� ������������ ������ ������
	for (int i = 0; i < 4; i++) {
		if (wayTree->way[i] != nullptr && wayTree->way[i]->height > height) {

			height = wayTree->way[i]->height;

		}
	}

	//����� ��� ������������ ������ ���������� � ������ ��� ����� ��� ����� �������������� ��� ��������
	for (int i = 0; i < 4; i++) {
		if (wayTree->way[i] != nullptr && wayTree->way[i]->height == height) {

			pos.another_eats.push_back(wayTree->way[i]->_to);
			pos.whoWasEated.push_back(wayTree->way[i]->eated);
			FillAnotherEatsForKing(wayTree->way[i], pos);
		}
	}
}

//������� ���������� �������������� �������� ��� ������� ����, ���������� ����� ����, ��� ���� ������� ���� ��� ��������
void SmartBot::FindContinue(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {

	std::vector<PosTurn> newMoves;

	//��� ������� ���� ��������
	for (auto& pos : movesVec) {

		//��������� ������� ������ ��� ������ ��� ��� ������
		if (Board[pos.from.first][pos.from.second] == static_cast<int>(Col)) {

			//������� ������ ����
			ternaryTree* wayTree = new ternaryTree(pos.from, pos.to);

			//��������� �����
			std::vector<std::pair<int, int>> enemyPos;

			//��������� ��������� ����� � ������
			auto enemy = std::make_pair(pos.from.first + wayTree->getDX(), pos.from.second + wayTree->getDY());
			enemyPos.push_back(enemy);

			//��������� ������� ���������� ����� �������� �����
			CheckContinue(Board, wayTree, enemyCol, pos.from, enemyPos);

			//��������� ����� ������� �� ������� ����� ������ ����
			FillAnotherEats(wayTree, pos);


			//����� ��������� ������ ���� ��������� �����
			if (!pos.another_eats.empty()) {
				int dx = (pos.to.first - pos.another_eats[0].first) / 2;
				int dy = (pos.to.second - pos.another_eats[0].second) / 2;
				int x = pos.another_eats[0].first + dx;
				int y = pos.another_eats[0].second + dy;

				pos.whoWasEated.push_back(std::make_pair(x, y));


				for (int i = 0; i < pos.another_eats.size() - 1; i++) {
					int dx = (pos.another_eats[i + 1].first - pos.another_eats[i].first) / 2;
					int dy = (pos.another_eats[i + 1].second - pos.another_eats[i].second) / 2;
					int x = pos.another_eats[i].first + dx;
					int y = pos.another_eats[i].second + dy;

					pos.whoWasEated.push_back(std::make_pair(x, y));
				}

				//��������� �������� ����, � ���������� �� ���������� ��������� �����
				pos.relevance += pos.another_eats.size() * static_cast<int>(SituationCost::EAT_MORE);
			}
		}
		else if (Board[pos.from.first][pos.from.second] == static_cast<int>(Col) + 2) {


			//������� ��������� ������
			quadrupleTree* wayTree = new quadrupleTree(pos.from, pos.to);

			std::vector<std::pair<int, int>> enemyPos;

			auto enemy = std::make_pair(pos.from.first + wayTree->getDX(), pos.from.second + wayTree->getDY());
			enemyPos.push_back(enemy);


			//���������� ����� ����������� ���� ��� ����� ������� �����
			CheckContinueForKing(Board, wayTree, enemyCol);

			FillAnotherEatsForKing(wayTree, pos);

			//���� ����� �� ����������� �������� ���� � ������������ �� ���-�� ��������� �����
			pos.relevance += pos.another_eats.size() * static_cast<int>(SituationCost::EAT_MORE);


			//����� �� ������� �������������� ���� ����� ��������
			//��� ��� ����� ���� ��� �� ����� ��������� �����
			//�������� �������������� ���� ��� �����������
			if (!pos.another_eats.empty()) {

				//���������� ������� ����� � ������� ������ ����� ���������� ��������
				int x = pos.another_eats[pos.another_eats.size() - 1].first;
				int y = pos.another_eats[pos.another_eats.size() - 1].second;

				//���������� ������, �� �������� ����� ��������� �� ����� ���������� ��������
				int dx = x - pos.whoWasEated[pos.whoWasEated.size() - 1].first;
				int dy = y - pos.whoWasEated[pos.whoWasEated.size() - 1].second;


				//��������� ������ �������������� �����, ������� ������� � ������� � ���� ��������� ���
				while (CheckBorders(x, y, dx, dy) && Board[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) {
					PosTurn nextpos = pos;
					nextpos.another_eats[nextpos.another_eats.size() - 1].first = x + dx;
					nextpos.another_eats[nextpos.another_eats.size() - 1].second = y + dy;
					newMoves.push_back(nextpos);
					dx += dx;
					dy += dy;
				}
			}

		}

	}

	//��������� ������ ����� ���������� ������� �������������� �����
	if (!newMoves.empty()) {
		for (int i = 0; i < newMoves.size() - 1; i++) {
			movesVec.push_back(newMoves[i]);
		}
	}
}


//������� ���������� ������� ����� ������������� ������
void SmartBot::FillPosTurns(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {


	//������ ��� ��������� ���� ��� ��������
	FindEats(Board, Col, enemyCol, movesVec);

	//���� ����� ��� �������� ���, ���� ������� ����
	if (movesVec.empty()) {
		FindMoves(Board, Col, movesVec);
	}
	else {
		//���� ���� ��� �������� ����, �������� ���� ����, ��� ���� ������� �����
		if (enemyCol == color) {
			canEnemyEat = true;
		}
		//������� �������������� ������������� ��������
		FindContinue(Board, movesVec, Col, enemyCol);
	}
}

//������� ����������� ����� ���������
void SmartBot::TryMoveEnemy
(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, PosTurn& pos, PosTurn& enemyTurn, Positions Col, Positions enemyCol) {


	//���� � ��������� ���� ������������� ��������
	if (!enemyTurn.another_eats.empty()) {
		//�� ������ ������� ������ ����������� � ����� ��������� ������
		std::swap(Board[enemyTurn.from.first][enemyTurn.from.second],
			Board[enemyTurn.another_eats[enemyTurn.another_eats.size() - 1].first][enemyTurn.another_eats[enemyTurn.another_eats.size() - 1].second]);
	}
	else {
		//���� �������������� �������� ���, �� ������ ������ ������ ������ ������� � ������� ����
		std::swap(Board[enemyTurn.from.first][enemyTurn.from.second], Board[enemyTurn.to.first][enemyTurn.to.second]);
	}

	//���� ���� ��������
	if (!enemyTurn.whoWasEated.empty()) {
		for (auto& eated : enemyTurn.whoWasEated) {
			//�� ��� ������ ������, ��� ���� ������� ����� ���������� ������ �������� ������
			Board[eated.first][eated.second] = static_cast<int>(Positions::CELL_PLBL);
		}
	}

	std::vector<PosTurn> movesVecMy;

	//������ ������� ��������� �������� ��� ���� �����
	FindEats(Board, Col, enemyCol, movesVecMy);

	//���� �� ������
	if (movesVecMy.empty()) {
		//�� ������ ������� ��������� ����
		FindMoves(Board, Col, movesVecMy);

	}
	else {
		//���� ����� ������, �� ��������� ������� � ������� ���� � ���������� �������������
		//��� ��� ����������� ������� ������� ������������ ����
		pos.relevance += static_cast<int>(SituationCost::EAT) * movesVecMy.size() / 5;

		//������� ������������� ��������
		FindContinue(Board, movesVecMy, enemyCol, Col);

		for (auto& moves : movesVecMy) {


			if (!moves.another_eats.empty()) {
				//���� ����� ������, �� ��������� ������� � ������� ���� � ���������� �������������
				pos.relevance += static_cast<int>(SituationCost::EAT_MORE) * moves.another_eats.size() / 5;

			}


		}
	}
}


//������� ����������� � ������ ���� �� �������� ����
void SmartBot::TryMove(PosTurn& pos, Positions Col, Positions enemyCol) {

	//������� �������� ����
	auto testBoard = PosBoard;

	//���� ���� ������������� ��������
	if (!pos.another_eats.empty()) {

		//�� ������ ������� ������ ����������� � ����� ��������� ������
		std::swap(testBoard[pos.from.first][pos.from.second],
			testBoard[pos.another_eats[pos.another_eats.size() - 1].first][pos.another_eats[pos.another_eats.size() - 1].second]);
	}
	else {

		//���� �������������� �������� ���, �� ������ ������ ������ ������ ������� � ������� ����
		std::swap(testBoard[pos.from.first][pos.from.second], testBoard[pos.to.first][pos.to.second]);
	}

	//���� ���� ��������
	if (!pos.whoWasEated.empty()) {
		for (auto& eated : pos.whoWasEated) {
			//�� ��� ������ ������, ��� ���� ������� ����� ���������� ������ �������� ������
			testBoard[eated.first][eated.second] = static_cast<int>(Positions::CELL_PLBL);
		}
	}

	//������ ����� ���������
	std::vector<PosTurn> movesVecEnemy;

	//������ ���� �������� ��� ���������
	FindEats(testBoard, enemyCol, Col, movesVecEnemy);


	//���� ���� ������
	if (movesVecEnemy.empty()) {
		if (canEnemyEat == true) {
			//��������� ������� � ��������� ��������� ���������� �� ����������� ���������� �����
			pos.relevance += static_cast<int>(SituationCost::SAVE) * PosMovesEnemy.size();
		}

		//������� ��������� ����
		FindMoves(testBoard, Col, movesVecEnemy);

		//��������� ������� � ��������� ����� �� � ����� ������ ���� ������������ �� ����������� ���������� ����
		pos.relevance += static_cast<int>(SituationCost::BLOCK) * (PosMovesEnemy.size() - movesVecEnemy.size());
	}
	else {

		//���� �������� ����, �� ���� ��� �� ��� �����, � �������� ������� � ����������� �� ����� �����, ������� ����� ����
		pos.relevance += static_cast<int>(SituationCost::DIE) * movesVecEnemy.size();

		//������� ������������� ��������
		FindContinue(testBoard, movesVecEnemy, enemyCol, Col);

		for (auto& moves : movesVecEnemy) {


			if (!moves.another_eats.empty()) {
				//���� ������������� �������� ����, �� ��� �� �������� ������� � ������ �������
				pos.relevance += static_cast<int>(SituationCost::DIE_MORE) * moves.another_eats.size();

			}


		}


	}

	//������ ���������� ������������ ������ ����� ���������
	for (auto& enemyTurn : movesVecEnemy) {
		TryMoveEnemy(testBoard, movesVecEnemy, pos, enemyTurn, Col, enemyCol);
	}
}

/*void BotV3::TryMoveAllPoss(std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {
	//�������� ������� ������ �����, ����� ���������� ������
	Timer t;
	for (auto& pos : movesVec) {
		TryMove(pos, Col, enemyCol);
	}
}*/

void SmartBot::TryMoveAllPoss(std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {
	//�������� ������� ������ �����, ����� ���������� ������

	std::vector<std::future<void>> vecAsy(movesVec.size());


	for (int i = 0; i < movesVec.size(); i++) {
		vecAsy[i] = std::async(std::launch::async, [&]() { TryMove(movesVec.at(i), Col, enemyCol); });
	}

	vecAsy.clear();
}



//������� ����������� ������ ������������ ����
SmartBot::PosTurn SmartBot::ReturnMove() {

	//��������� ������� ���� ����� � ��������� ����� ���������
	FillPosTurns(PosBoard, PosMoves, color, enemycolor);
	FillPosTurns(PosBoard, PosMovesEnemy, enemycolor, color);

	//���������� ������ ���� ����� ��������� �����
	TryMoveAllPoss(PosMoves, color, enemycolor);

	std::vector<PosTurn> moves;
	PosTurn max;
	max.relevance = INT_MIN;

	//��������� ������������ ��� �� ��������
	for (auto& move : PosMoves) {
		if (move.relevance > max.relevance) {
			max = move;
		}
	}


	//���� ���� ���������� ���� �� ��������, ��������� ��� ������ �����
	for (auto& move : PosMoves) {
		if (move.relevance == max.relevance) {
			moves.push_back(move);
		}
	}
	moves.push_back(max);

	srand(time(NULL));
	//���� ���� ������ �� �������� ����, ���������� ��������� �� ���
	if (moves.size() > 1) {
		int randomINDEX = rand() % (moves.size() - 1);
		return moves[randomINDEX];
	}//���� ���, �� ����� ������
	else if (moves.size() == 1) {
		return moves[0];
	}
}



