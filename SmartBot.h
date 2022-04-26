#pragma once
#include "player.h"
#include "ternaryTree.h"

class Playboard;

//расценка различных ситуаций, в зависимости от этих чисел каждый ход приобретает рейтинг
enum class SituationCost {
	EAT = 300,
	EAT_MORE = 600,
	EAT_KING = 1000,
	SAVE = 500,
	BLOCK = 40,
	BECOME_KING = 3000,
	DIE = -400,
	DIE_MORE = -1000,
	CAN_GO = 20
};

//класс имеющихся состояний ячеек на поле-массиве класа BotV3
enum class Positions {
	CELL_UNPLBL = 0,
	CELL_PLBL = 1,
	WHITE_CHECKER = 2,
	BLACK_CHECKER = 3,
	WHITE_KING = 4,
	BLACK_KING = 5
};

/* Сам класс "умного" бота
* Находит наилучший ход по следующему приблизительному алгоритму:
*
* 1.Находит все возможные ходы в соответствии с правилами
* 2.На копии игрового поля выполняет эти ходы, оценивая их результативность
* 3.Для каждого шага на копии игрового поля выполняются все возможные ходы соперника по очереди,
* оценивается результативность этих ходов, делаются поправки в рейтинг нашего шага
* 4.Оцениваются все возможные появившиеся ходы после "ходов" соперника, рейтинг ходов бота изменяется с пониженным коэффициентом
* 5.Среди всех ходов выбирается ход с максимальным рейтингом
* 6.Выполняется ход с максимальным рейтингом
*
*/
class SmartBot : public Player {

	//Структура хода 
	struct PosTurn {
		//пары координат откуда-куда
		std::pair<int, int> from;
		std::pair<int, int> to;

		//вектор съеденных шашек
		std::vector<std::pair<int, int>> whoWasEated;

		//вектор дополнительных ходов после съедения шашки
		std::vector<std::pair<int, int>> another_eats;

		//рейтинг хода
		int relevance = 0;
	};


	//цвет, за который играет бот
	Positions color;

	//цвет, за который играет соперник
	Positions enemycolor;

	//двумерный массив игрового поля
	std::array<std::array<int, 8>, 8> PosBoard;


	//вектор возможных ходов бота
	std::vector<PosTurn> PosMoves;

	//вектор возможных ходов соперника
	std::vector<PosTurn> PosMovesEnemy;

	//ел ли шашки соперник на этом ходу?
	bool canEnemyEat = false;



public:


	SmartBot(int b_or_w, Playboard* instance);

	void BoardScanner();

	bool Turn(int x, int y, CheckerColor b_or_w);

	void RefreshVectors();

	inline bool CheckBorders(int x, int y, int dx, int dy);

	inline bool CheckBecomeKing(int y, Positions Col);

	void CheckEats(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy,
		Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec);

	void CheckEatsForKing(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy,
		Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec);

	void FindEats(std::array<std::array<int, 8>, 8>& Board, Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec);

	void CheckMoves(std::array<std::array<int, 8>, 8>& Board, int x, int y, Positions Col, std::vector<PosTurn>& movesVec);

	void CheckMovesForKing(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy, std::vector<PosTurn>& movesVec);

	void FindMoves(std::array<std::array<int, 8>, 8>& Board, Positions Col, std::vector<PosTurn>& movesVec);

	bool IsWasChecked(int x, int y, std::vector<std::pair<int, int>>& enemyPos);

	bool IsWasCheckedByFather(int x, int y, ternaryTree* wayTree);

	void CheckContinue
	(std::array<std::array<int, 8>, 8>& Board, ternaryTree* wayTree, Positions enemyCol,
		std::pair<int, int> stop, std::vector<std::pair<int, int>>& enemyPos);

	void FillAnotherEats(ternaryTree* wayTree, PosTurn& pos);

	void FindContinue(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol);

	void FindWay(int x, int y, int dx, int dy, std::array<std::array<int, 8>, 8> Board, quadrupleTree* wayTree, Positions enemyCol);

	void FillPosTurns(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol);

	void TryMoveEnemy
	(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec,
		PosTurn& pos, PosTurn& enemyTurn, Positions Col, Positions enemyCol);

	void CheckContinueForKing
	(std::array<std::array<int, 8>, 8> Board, quadrupleTree* wayTree, Positions enemyCol);

	void FillAnotherEatsForKing(quadrupleTree* wayTree, PosTurn& pos);


	void TryMove(PosTurn& pos, Positions Col, Positions enemyCol);

	void TryMoveAllPoss(std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol);



	PosTurn ReturnMove();

};