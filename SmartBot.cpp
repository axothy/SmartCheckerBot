/**
* @brief Smart AI checker bot using cost of situation
* @authors Chebotin Alexandr
* @date 31.01.2022

*/


#include <vector>
#include <array>
#include <iostream>
#include <utility>
#include <algorithm>
#include <time.h>
#include <thread>
#include <chrono>
#include <future>

/**
* Class timer for measuring bot time effectiveness. Uses std::chrono

*/

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

/**
* Math function signum.
* @params numbr Takes int number
*/

int sign(int numbr) {
	int sgn = 0;
	numbr < 0 ? sgn = -1 : sgn = 1;

	return sgn;
}

enum SituationCost {
	EAT = 300,
	EAT_MORE = 600,
	EAT_KING = 1000,
	SAVE = 500,
	BLOCK = 40,
	BECOME_KING = 1000,
	DIE = -400,
	DIE_MORE = -1000,
	CAN_GO = 20
};

enum Positions {
	CELL_UNPLBL = 0,
	CELL_PLBL = 1,
	WHITE_CHECKER = 2,
	BLACK_CHECKER = 3,
	WHITE_KING = 4,
	BLACK_KING = 5
};

struct ternaryTree {
	int height = 0;

	std::pair<int, int> _from;
	std::pair<int, int> _to;
	ternaryTree* first = nullptr;
	ternaryTree* second = nullptr;
	ternaryTree* third = nullptr;
	ternaryTree* father = nullptr;

	ternaryTree(std::pair<int, int> from, std::pair<int, int> to) { _from = from; _to = to; }

	ternaryTree(std::pair<int, int> from, std::pair<int, int> to, ternaryTree* fatherTree) {
		father = fatherTree;
		_from = from; _to = to;
	}

	void incrementHeight() {
		if (first == nullptr && second == nullptr && third == nullptr) {
			height++;
			ternaryTree* helpFather = father;
			while (helpFather != nullptr) {
				helpFather->height++;
				helpFather = helpFather->father;
			}
		}
	}

	int getXto() { return _to.first; }
	int getYto() { return _to.second; }

	int getDX() { return (_to.first - _from.first) / 2; }
	int getDY() { return (_to.second - _from.second) / 2; }

};

struct PosTurn {
	std::pair<int, int> from;
	std::pair<int, int> to;
	std::vector<std::pair<int, int>> whoWasEated;
	std::vector<std::pair<int, int>> another_eats;
	int relevance = 0;
};

class BotV3 {
public:
	BotV3(Positions b_or_w) {
		color = b_or_w;
		initBoard();
		if (color == WHITE_CHECKER) {
			enemycolor = BLACK_CHECKER;
		}
		else
			enemycolor = WHITE_CHECKER;
	}

	Positions color;
	Positions enemycolor;
	std::array<std::array<int, 8>, 8> PosBoard;

	std::vector<PosTurn> PosMoves;
	std::vector<PosTurn> PosMovesEnemy;

	bool canEnemyEat = false;
	
	/**
	* 
	* Bot analyses any playboard.
	
	*/

	void initBoard() {
		PosBoard[0] = { 2,0,2,0,1,0,1,0 };
		PosBoard[1] = { 0,1,0,1,0,1,0,1 };
		PosBoard[2] = { 2,0,2,0,1,0,1,0 };
		PosBoard[3] = { 0,1,0,1,0,3,0,1 };
		PosBoard[4] = { 2,0,2,0,1,0,1,0 };
		PosBoard[5] = { 0,1,0,1,0,3,0,1 };
		PosBoard[6] = { 2,0,2,0,1,0,1,0 };
		PosBoard[7] = { 0,1,0,1,0,1,0,1 };
	}

	/*void initBoard() {
		PosBoard[0] = { 2,0,2,0,1,0,3,0 };
		PosBoard[1] = { 0,2,0,1,0,3,0,3 };
		PosBoard[2] = { 2,0,2,0,1,0,3,0 };
		PosBoard[3] = { 0,2,0,1,0,3,0,3 };
		PosBoard[4] = { 2,0,2,0,1,0,3,0 };
		PosBoard[5] = { 0,2,0,1,0,3,0,3 };
		PosBoard[6] = { 2,0,2,0,1,0,3,0 };
		PosBoard[7] = { 0,2,0,1,0,3,0,3 };
	}*/


	/**
	* Prints board
	* @params Board Takes 8x8 array of playboard
	
	*/
	void printBoard(std::array<std::array<int, 8>, 8>& Board) {
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				std::cout << Board[x][y];
			}
			std::cout << std::endl;
		}
	}

	/**
	* Check borders
	* @params x y dx dy Takes coordinates
	
	*/
	inline bool CheckBorders(int x, int y, int dx, int dy) {
		return x + dx >= 0 && x + dx < 8 && y + dy >= 0 && y + dy < 8;
	}

	inline bool CheckBecomeKing(int y, Positions Col) {
		if (Col == WHITE_CHECKER) {
			if (y == 7) {
				return true;
			}
		}
		else if (Col == BLACK_CHECKER) {
			if (y == 0) {
				return true;
			}
		}
		return false;
	}

	void CheckEats(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy, 
		Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {

		if (CheckBorders(x, y, dx, dy)) {
			if (Board[x + dx][y + dy] == enemyCol || Board[x + dx][y + dy] == enemyCol + 2) {
				if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == CELL_PLBL) {
					auto xy = std::make_pair(x, y);
					auto eatedxy = std::make_pair(x + dx, y + dy);
					auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);
					movesVec.push_back({ xy, dxdy });
					movesVec[movesVec.size() - 1].whoWasEated.push_back(eatedxy);

					movesVec[movesVec.size() - 1].relevance += CAN_GO + EAT;
					if (Board[x + dx][y + dy] == enemyCol + 2) {
						movesVec[movesVec.size() - 1].relevance += EAT_KING;
					}
					if (CheckBecomeKing(dxdy.second, Col)) {
						movesVec[movesVec.size() - 1].relevance += BECOME_KING;
					}
				}
			}
		}
	}

	void CheckEatsForKing(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy, 
		Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {

		int ddx = sign(dx);
		int ddy = sign(dy);
		while (CheckBorders(x, y, dx, dy)) {
			if (Board[x + dx][y + dy] == enemyCol || Board[x + dx][y + dy] == enemyCol + 2) {
				if (CheckBorders(x, y, dx + ddx, dy + ddy) && Board[x + dx + ddx][y + dy + ddy] == CELL_PLBL) {

					auto eatedxy = std::make_pair(x + dx, y + dy);

					dx += ddx;
					dy += ddy;//то переходим на эту клетку
					while (CheckBorders(x, y, dx, dy) && Board[x + dx][y + dy] == CELL_PLBL) {
						auto xy = std::make_pair(x, y);
						auto dxdy = std::make_pair(x + dx, y + dy);
						movesVec.push_back({ xy, dxdy });
						movesVec[movesVec.size() - 1].whoWasEated.push_back(eatedxy);
						movesVec[movesVec.size() - 1].relevance += CAN_GO + EAT;
						if (Board[x + dx][y + dy] == enemyCol + 2) {
							movesVec[movesVec.size() - 1].relevance += EAT_KING;
						}
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


	void FindEats(std::array<std::array<int, 8>, 8>& Board, Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				if (Board[x][y] == Col) {
					CheckEats(Board, x, y, 1, 1, Col, enemyCol, movesVec);
					CheckEats(Board, x, y, -1, 1, Col, enemyCol, movesVec);
					CheckEats(Board, x, y, 1, -1, Col, enemyCol, movesVec);
					CheckEats(Board, x, y, -1, -1, Col, enemyCol, movesVec);
				}
				if (Board[x][y] == Col + 2) {
					CheckEatsForKing(Board, x, y, 1, 1, Col, enemyCol, movesVec);
					CheckEatsForKing(Board, x, y, -1, 1, Col, enemyCol, movesVec);
					CheckEatsForKing(Board, x, y, 1, -1, Col, enemyCol, movesVec);
					CheckEatsForKing(Board, x, y, -1, -1, Col, enemyCol, movesVec);
				}
			}
		}
	}

	void CheckMoves(std::array<std::array<int, 8>, 8>& Board, int x, int y, Positions Col, std::vector<PosTurn>& movesVec) {
		int dx = 1, dy;

		if (Col == WHITE_CHECKER) {
			dy = 1;
		}
		else
			dy = -1;

		if (CheckBorders(x, y, dx, dy)) {
			if (Board[x + dx][y + dy] == CELL_PLBL) {
				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + dx, y + dy);
				movesVec.push_back({ xy, dxdy });
				movesVec[movesVec.size() - 1].relevance += CAN_GO;
				if (CheckBecomeKing(dxdy.second, Col)) {
					movesVec[movesVec.size() - 1].relevance += BECOME_KING;
				}
			}
		}

		dx = -1;

		if (CheckBorders(x, y, dx, dy)) {
			if (PosBoard[x + dx][y + dy] == CELL_PLBL) {
				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + dx, y + dy);
				movesVec.push_back({ xy, dxdy });
				movesVec[movesVec.size() - 1].relevance += CAN_GO;
				if (CheckBecomeKing(dxdy.second, Col)) {
					movesVec[movesVec.size() - 1].relevance += BECOME_KING;
				}
			}
		}
	}

	void CheckMovesForKing(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy, std::vector<PosTurn>& movesVec) {
		int ddx = sign(dx);
		int ddy = sign(dy);
		while (CheckBorders(x, y, dx, dy) && PosBoard[x + dx][y + dy] == CELL_PLBL) { //и пока нет шашек на клетке
			auto xy = std::make_pair(x, y);
			auto dxdy = std::make_pair(x + dx, y + dy);
			movesVec.push_back({ xy, dxdy });
			movesVec[movesVec.size() - 1].relevance += CAN_GO;
			dx += ddx;
			dy += ddy; //передвигаемся вперед
		}
	}

	void FindMoves(std::array<std::array<int, 8>, 8>& Board, Positions Col, std::vector<PosTurn>& movesVec) {
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				if (Board[x][y] == Col) {
					CheckMoves(Board, x, y, Col, movesVec);
				}
				else if (Board[x][y] == Col + 2) {
					CheckMovesForKing(Board, x, y, 1, 1, movesVec);
					CheckMovesForKing(Board, x, y, -1, 1, movesVec);
					CheckMovesForKing(Board, x, y, 1, -1, movesVec);
					CheckMovesForKing(Board, x, y, -1, -1, movesVec);
				}
			}
		}
	}

	bool IsWasChecked(int x, int y, std::vector<std::pair<int, int>>& enemyPos) {
		for (auto& enemy : enemyPos) {
			if (enemy.first == x && enemy.second == y) {
				return true;
			}
		}
		return false;
	}

	bool IsWasCheckedByFather(int x, int y, ternaryTree* wayTree) {
		ternaryTree* helpTree = wayTree;
		while (helpTree->father != nullptr && helpTree->father->father != nullptr) {
			if (helpTree->father->getXto() == x && helpTree->father->getYto() == y) {
				return false;
			}
			helpTree = helpTree->father;
		}


	}

	void CheckContinue
	(std::array<std::array<int, 8>, 8>& Board, ternaryTree* wayTree, Positions enemyCol, 
		std::pair<int, int> stop, std::vector<std::pair<int, int>>& enemyPos) {

		int dx = wayTree->getDX();
		int dy = wayTree->getDY();
		int x = wayTree->getXto();
		int y = wayTree->getYto();

		if (CheckBorders(x, y, dx, dy)) {

			if ((Board[x + dx][y + dy] == enemyCol || Board[x + dx][y + dy] == enemyCol + 2)
				&& !IsWasChecked(x + dx, y + dy, enemyPos) && (x + dx != stop.first && y + dy != stop.second)) {

				if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == CELL_PLBL) {

					auto xy = std::make_pair(x, y);
					auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);


					wayTree->incrementHeight();
					wayTree->first = new ternaryTree(xy, dxdy, wayTree);


					CheckContinue(Board, wayTree->first, enemyCol, stop, enemyPos);

				}
			}
		}
		if (CheckBorders(x, y, dx, -dy)) {

			if ((Board[x + dx][y - dy] == enemyCol || Board[x + dx][y - dy] == enemyCol + 2)
				&& !IsWasChecked(x + dx, y - dy, enemyPos) && (x + dx != stop.first && y - dy != stop.second)) {

				if (CheckBorders(x, y, 2 * dx, -2 * dy) && Board[x + 2 * dx][y - 2 * dy] == CELL_PLBL
					&& !(x + 2 * dx == stop.first && y - 2 * dy == stop.second) &&
					IsWasCheckedByFather(x + 2 * dx, y - 2 * dy, wayTree)) {

					auto xy = std::make_pair(x, y);
					auto dxdy = std::make_pair(x + 2 * dx, y - 2 * dy);


					wayTree->incrementHeight();
					wayTree->second = new ternaryTree(xy, dxdy, wayTree);


					CheckContinue(Board, wayTree->second, enemyCol, stop, enemyPos);
				}
			}
		}
		if (CheckBorders(x, y, -dx, dy)) {

			if ((Board[x - dx][y + dy] == enemyCol || Board[x - dx][y + dy] == enemyCol + 2)
				&& !IsWasChecked(x - dx, y + dy, enemyPos) && (x - dx != stop.first && y + dy != stop.second)) {

				if (CheckBorders(x, y, -2 * dx, 2 * dy) && Board[x - 2 * dx][y + 2 * dy] == CELL_PLBL
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

	enum Ways {
		FIRST, SECOND, THIRD, FOURTH, FATHER
	};

	struct quadrupleTree {


		int height = 0;

		std::pair<int, int> _from;
		std::pair<int, int> _to;

		std::vector<quadrupleTree*> way;


		quadrupleTree(std::pair<int, int> from, std::pair<int, int> to) : way(5) { _from = from; _to = to; }

		quadrupleTree(std::pair<int, int> from, std::pair<int, int> to, quadrupleTree* fatherTree) : way(5) {
			way[FATHER] = fatherTree;
			_from = from; _to = to;
		}

		void incrementHeight() {
			if (way[FIRST] == nullptr && way[SECOND] == nullptr && way[THIRD] == nullptr && way[FOURTH] == nullptr) {
				height++;
				quadrupleTree* helpFather = way[FATHER];
				while (helpFather != nullptr) {
					helpFather->height++;
					helpFather = helpFather->way[FATHER];
				}
			}
		}

		int getXto() { return _to.first; }
		int getYto() { return _to.second; }

		int getDX() { return (_to.first - _from.first) / 2; }
		int getDY() { return (_to.second - _from.second) / 2; }

	};

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

	void FindWay(int x, int y, int dx, int dy, std::array<std::array<int, 8>, 8> Board, quadrupleTree* wayTree, Positions enemyCol) {

		auto destiny = getWay(dx, dy);

		while (CheckBorders(x, y, dx, dy)) {
			if (Board[x + dx][y + dy] == enemyCol || Board[x + dx][y + dy] == enemyCol + 2)
			{

				if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == CELL_PLBL) {

					std::swap(Board[x][y], Board[x + 2 * dx][y + 2 * dy]);

					auto xy = std::make_pair(x, y);
					auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);

					Board[x + dx][y + dy] = CELL_PLBL;

					wayTree->incrementHeight();
					wayTree->way[destiny] = new quadrupleTree(xy, dxdy, wayTree);


					CheckContinueForKing(Board, wayTree->way[destiny], enemyCol);

				}
			}
			x += dx;
			y += dy;
		}
	}

	void CheckContinueForKing
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




	void FillAnotherEats(ternaryTree* wayTree, PosTurn& pos) {
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

	void FillAnotherEatsForKing(quadrupleTree* wayTree, PosTurn& pos) {
		int height = -1;
		for (int i = 0; i < 4; i++) {
			if (wayTree->way[i] != nullptr && wayTree->way[i]->height > height) {

				height = wayTree->way[i]->height;

			}
		}
		for (int i = 0; i < 4; i++) {
			if (wayTree->way[i] != nullptr && wayTree->way[i]->height == height) {

				pos.another_eats.push_back(wayTree->way[i]->_to);
				FillAnotherEatsForKing(wayTree->way[i], pos);

			}
		}
	}

	void FindContinue(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {


		for (auto& pos : movesVec) {


			if (Board[pos.from.first][pos.from.second] == Col) {


				ternaryTree* wayTree = new ternaryTree(pos.from, pos.to);


				std::vector<std::pair<int, int>> enemyPos;

				auto enemy = std::make_pair(pos.from.first + wayTree->getDX(), pos.from.second + wayTree->getDY());
				enemyPos.push_back(enemy);

				CheckContinue(Board, wayTree, enemyCol, pos.from, enemyPos);

				FillAnotherEats(wayTree, pos);

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

					pos.relevance += pos.another_eats.size() * EAT_MORE;
				}
			}
			else if (Board[pos.from.first][pos.from.second] == Col + 2) {

				quadrupleTree* wayTree = new quadrupleTree(pos.from, pos.to);

				std::vector<std::pair<int, int>> enemyPos;

				auto enemy = std::make_pair(pos.from.first + wayTree->getDX(), pos.from.second + wayTree->getDY());
				enemyPos.push_back(enemy);

				CheckContinueForKing(Board, wayTree, enemyCol);

				FillAnotherEatsForKing(wayTree, pos);

				pos.relevance += pos.another_eats.size() * EAT_MORE;

			}
		}
	}



	void PrintMoves(std::vector<PosTurn>& movesVec) {
		if (movesVec.size() == 0) {
			std::cout << "no moves";
			return;
		}
		for (auto& mov : movesVec) {
			std::cout << mov.relevance << " from " << mov.from.first << " " << mov.from.second << " to " << mov.to.first << " " << mov.to.second << std::endl;
		}
	}

	void FillPosTurns(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {
		FindEats(Board, Col, enemyCol, movesVec);
		if (movesVec.empty()) {
			FindMoves(Board, Col, movesVec);
		}
		else {
			if (enemyCol == color) {
				canEnemyEat = true;
			}
			FindContinue(Board, movesVec, Col, enemyCol);
		}
	}

	void TryMoveEnemy
	(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, PosTurn& pos, PosTurn& enemyTurn,
		Positions Col, Positions enemyCol) {


		if (!enemyTurn.another_eats.empty()) {
			std::swap(Board[enemyTurn.from.first][enemyTurn.from.second],
				Board[enemyTurn.another_eats[enemyTurn.another_eats.size() - 1].first][pos.another_eats[pos.another_eats.size() - 1].second]);
		}
		else {
			std::swap(Board[enemyTurn.from.first][enemyTurn.from.second], Board[enemyTurn.to.first][enemyTurn.to.second]);
		}
		if (!enemyTurn.whoWasEated.empty()) {
			for (auto& eated : enemyTurn.whoWasEated) {
				Board[eated.first][eated.second] = CELL_PLBL;
			}
		}

		std::vector<PosTurn> movesVecMy;

		FindEats(Board, Col, enemyCol, movesVecMy);

		if (movesVecMy.empty()) {

			FindMoves(Board, Col, movesVecMy);

		}
		else {

			pos.relevance += EAT * movesVecMy.size() / 5;


			FindContinue(Board, movesVecMy, enemyCol, Col);

			for (auto& moves : movesVecMy) {


				if (!moves.another_eats.empty()) {

					pos.relevance += EAT_MORE * moves.another_eats.size() / 5;

				}


			}
		}
	}



	void TryMove(PosTurn& pos, Positions Col, Positions enemyCol) {


		auto testBoard = PosBoard;
		if (!pos.another_eats.empty()) {
			std::swap(testBoard[pos.from.first][pos.from.second],
				testBoard[pos.another_eats[pos.another_eats.size() - 1].first][pos.another_eats[pos.another_eats.size() - 1].second]);
		}
		else {
			std::swap(testBoard[pos.from.first][pos.from.second], testBoard[pos.to.first][pos.to.second]);
		}
		if (!pos.whoWasEated.empty()) {
			for (auto& eated : pos.whoWasEated) {
				testBoard[eated.first][eated.second] = CELL_PLBL;
			}
		}


		std::vector<PosTurn> movesVecEnemy;


		FindEats(testBoard, enemyCol, Col, movesVecEnemy);

		if (movesVecEnemy.empty()) {
			if (canEnemyEat == true) {
				pos.relevance += SAVE * PosMovesEnemy.size();
			}

			FindMoves(testBoard, Col, movesVecEnemy);

			pos.relevance += BLOCK * (PosMovesEnemy.size() - movesVecEnemy.size());
		}
		else {

			pos.relevance += DIE * movesVecEnemy.size();


			FindContinue(testBoard, movesVecEnemy, enemyCol, Col);

			for (auto& moves : movesVecEnemy) {


				if (!moves.another_eats.empty()) {

					pos.relevance += DIE_MORE * moves.another_eats.size();

				}


			}


		}

		for (auto& enemyTurn : movesVecEnemy) {
			TryMoveEnemy(testBoard, movesVecEnemy, pos, enemyTurn, Col, enemyCol);
		}

	}

	void TryMoveAllPoss(std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {
		//пытаемся сходить каждым ходом, чтобы произвести оценку
		Timer t;
		for (auto& pos : movesVec) {
			TryMove(pos, Col, enemyCol);
		}
	}

	void TryMoveAllPoss1(std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {
		//пытаемся сходить каждым ходом, чтобы произвести оценку

		std::vector<std::future<void>> vecAsy(movesVec.size());

		Timer t;


		for (int i = 0; i < movesVec.size(); i++) {
			vecAsy[i] = std::async(std::launch::async, [&]() { TryMove(movesVec.at(i), Col, enemyCol); });
		}

		vecAsy.clear();
	}


	PosTurn ReturnMove() {
		std::vector<PosTurn> moves;
		PosTurn max;
		max.relevance = INT_MIN;
		for (auto& move : PosMoves) {
			if (move.relevance > max.relevance) {
				max = move;
			}
		}

		for (auto& move : PosMoves) {
			if (move.relevance == max.relevance) {
				moves.push_back(move);
			}
		}
		moves.push_back(max);

		if (moves.size() > 1) {
			int randomINDEX = rand() % (moves.size() - 1);
			return moves[randomINDEX];
		}
		else if (moves.size() == 1) {
			return moves[0];
		}
	}

};


int main() {
	BotV3 player(WHITE_CHECKER);

	std::cout << "mnogopotochnost' test:" << std::endl;
	for (int i = 0; i < 10; i++) {
		player.FillPosTurns(player.PosBoard, player.PosMoves, player.color, player.enemycolor);
		player.FillPosTurns(player.PosBoard, player.PosMovesEnemy, player.enemycolor, player.color);
		player.TryMoveAllPoss1(player.PosMoves, player.color, player.enemycolor);
		player.PosMoves.clear();
		player.PosMovesEnemy.clear();
	}

	std::cout << std::endl << "odnopotochnost' test:" << std::endl;
	for (int i = 0; i < 10; i++) {
		player.FillPosTurns(player.PosBoard, player.PosMoves, player.color, player.enemycolor);
		player.FillPosTurns(player.PosBoard, player.PosMovesEnemy, player.enemycolor, player.color);
		player.TryMoveAllPoss(player.PosMoves, player.color, player.enemycolor);
		player.PosMoves.clear();
		player.PosMovesEnemy.clear();
	}

	return 0;
}
