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

//сигнум
static int sign(int numbr) {
	if (numbr == 0) {
		return 0;
	}
	int sgn = 0;
	numbr < 0 ? sgn = -1 : sgn = 1;

	return sgn;
}


//константный массив неактивных ячеек - для оптимизации скорости пробега по игровому полю
constexpr std::pair<int, int> NonActiveCells[] = {
	{0,1},{0,3},{0,5},{0,7},{1,0},{1,2},{1,4},{1,6},
	{2,1},{2,3},{2,5},{2,7},{3,0},{3,2},{3,4},{3,6},
	{4,1},{4,3},{4,5},{4,7},{5,0},{5,2},{5,4},{5,6},
	{6,1},{6,3},{6,5},{6,7},{7,0},{7,2},{7,4},{7,6}
};

//константный массив активных ячеек - для оптимизации скорости пробега по игровому полю
constexpr std::pair<int, int> ActiveCells[] = {
	{1,1},{1,3},{1,5},{1,7},{0,0},{0,2},{0,4},{0,6},
	{3,1},{3,3},{3,5},{3,7},{2,0},{2,2},{2,4},{2,6},
	{5,1},{5,3},{5,5},{5,7},{4,0},{4,2},{4,4},{4,6},
	{7,1},{7,3},{7,5},{7,7},{6,0},{6,2},{6,4},{6,6}
};


//конструктор бота
SmartBot::SmartBot(int b_or_w, Playboard* instance) : Player(instance) {

	//присваиваем цвет шашек
	color = static_cast<Positions>(b_or_w);

	//инициализируем игровое поле внутри бота - инициализируем неактивные ячейки
	for (auto& nonactcell : NonActiveCells) {
		PosBoard[nonactcell.first][nonactcell.second] = static_cast<int>(Positions::CELL_UNPLBL);
	}

	//инициализируем активные ячейки
	for (auto& actcell : ActiveCells) {
		PosBoard[actcell.first][actcell.second] = static_cast<int>(Positions::CELL_PLBL);
	}

	//определяем цвет шашек соперника бота
	if (color == Positions::WHITE_CHECKER) {
		enemycolor = Positions::BLACK_CHECKER;
	}
	else
		enemycolor = Positions::WHITE_CHECKER;
}


//сканер игрового поля
void SmartBot::BoardScanner() {

	//проходим только по активным ячейкам
	for (auto& actcell : ActiveCells) {

		//определяем цвет на данной ячейке
		CheckerColor check = gameCheckers->getCell(actcell.first, actcell.second)->getCheckerColor();

		//определяем шашка это или король, если что-то стоит
		CheckerHierarchy check_post = gameCheckers->getCell(actcell.first, actcell.second)->getCheckerPost();

		//если ничего не стоит,
		//то это просто активная ячейка
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

//функция хода бота
bool SmartBot::Turn(int x, int y, CheckerColor b_or_w) {

	//на начало хода выполнил сканирование игрового поля
	BoardScanner();

	//получим самый "рейтинговый" ход
	PosTurn turn = ReturnMove();


	//выделим ячейку из которой произведен ход
	gameCheckers->Select(turn.from.first, turn.from.second, b_or_w);
	//сходим на ячейку куда был произведен ход
	gameCheckers->Select(turn.to.first, turn.to.second, b_or_w);

	//если есть множественное съедение, проходимся по вектору множественных съедений
	//прожимая места съедений
	if (!turn.another_eats.empty()) {
		for (auto& eats : turn.another_eats) {
			gameCheckers->Select(eats.first, eats.second, b_or_w);
		}
	}

	//обнуляем вектора ходов
	RefreshVectors();

	//ход выполнен
	return true;
}

//обновление векторов ходов
void SmartBot::RefreshVectors() {
	PosMoves.clear();
	PosMovesEnemy.clear();
	//переносим в начальное состояние флаг съедения врагом
	canEnemyEat = false;
}


//проверка границ хода
inline bool SmartBot::CheckBorders(int x, int y, int dx, int dy) {
	return x + dx >= 0 && x + dx < 8 && y + dy >= 0 && y + dy < 8;
}

//проверка хода на становление дамкой
inline bool SmartBot::CheckBecomeKing(int y, Positions Col) {
	if (Col == Positions::WHITE_CHECKER) {
		//для белых - если достигнута верхняя граница поля
		if (y == 7) {
			return true;
		}
	}
	else if (Col == Positions::BLACK_CHECKER) {
		//для черных - если достигнута нижняя граница поля
		if (y == 0) {
			return true;
		}
	}
	return false;
}


/*функция нахождения возможных съедений обычной шашкой в одну сторону, сторона определяется переменными dx dy
* Board - поле на котором происходит нахажодение хода
* x, y - откуда
* dx dy - приращение координаты
* Col - цвет бота
* enemyCol - цвет соперника
* movesVec - вектор возможных ходов
*/
void SmartBot::CheckEats(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy,
	Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {

	//проверим границы
	if (CheckBorders(x, y, dx, dy)) {

		//проверим соседнюю ячейку в соответствии с вектором приращения на наличие шашки соперника
		if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {

			//проверим ячейку, на которую будем перемещаться в случае съедения, находится ли она в границах поля и пустая ли она
			if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)) {

				//если все условия выполнены
				// 
				//создаем координаты
				//xy - откуда перемещаемся
				//eatedxy - что съели
				//dxdy - куда переместились
				auto xy = std::make_pair(x, y);
				auto eatedxy = std::make_pair(x + dx, y + dy);
				auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);

				//заполняем вектор ходов и заполняем вектор съеденных шашек за этот ход
				movesVec.push_back({ xy, dxdy });
				movesVec[movesVec.size() - 1].whoWasEated.push_back(eatedxy);


				//повышаем рейтинг хода
				//повышаем на рейтинг съедения
				movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO) + static_cast<int>(SituationCost::EAT);

				//если съели дамку, то повышаем на ценность съедения дамки
				if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {
					movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::EAT_KING);
				}
				//если шашка станет дамкой в случае этого хода, то повышаем рейтинг на величину становления дамкой
				if (CheckBecomeKing(dxdy.second, Col)) {
					movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::BECOME_KING);
				}
			}
		}
	}
}


/*функция нахождения возможных съедений дамкой в одну сторону, сторона определяется переменными dx dy
* Board - поле на котором происходит нахажодение хода
* x, y - откуда
* dx dy - приращение координаты
* Col - цвет бота
* enemyCol - цвет соперника
* movesVec - вектор возможных ходов
*/
void SmartBot::CheckEatsForKing(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy,
	Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {

	//определяем знак приращения
	int ddx = sign(dx);
	int ddy = sign(dy);

	//проверяем ходы, пока находимся в границах
	while (CheckBorders(x, y, dx, dy)) {

		//проверим соседнюю ячейку в соответствии с вектором приращения на наличие шашки соперника
		if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {

			//проверим ячейку, на которую будем перемещаться в случае съедения, находится ли она в границах поля и пустая ли она
			if (CheckBorders(x, y, dx + ddx, dy + ddy) && Board[x + dx + ddx][y + dy + ddy] == static_cast<int>(Positions::CELL_PLBL)) {

				//если можем съесть шашку, то добавляем её координаты в вектор съедаемых шашек
				auto eatedxy = std::make_pair(x + dx, y + dy);

				//переходим на данную клетку
				dx += ddx;
				dy += ddy;

				//теперь добавим все возможные ходы в одну сторону для дамки после съедения
				while (CheckBorders(x, y, dx, dy) && Board[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) {

					//если все условия выполнены
					// 
					//создаем координаты
					//xy - откуда перемещаемся
					//dxdy - куда переместились
					auto xy = std::make_pair(x, y);
					auto dxdy = std::make_pair(x + dx, y + dy);
					movesVec.push_back({ xy, dxdy });
					movesVec[movesVec.size() - 1].whoWasEated.push_back(eatedxy);

					//если можем съесть повышаем рейтинг
					movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO) + static_cast<int>(SituationCost::EAT);
					//если съедаем дамку то ещё повышаем рейтинг хода
					if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2) {
						movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::EAT_KING);
					}
					//передвигаемся дальше
					dx += ddx;
					dy += ddy;
				}
				//return в случае, если мы начали выходить за границы или ячейка занята
				return;
			}
			else
				return;
		}
		//передвигаемся дальше пока не встретим шашку соперника
		dx += ddx;
		dy += ddy;
	}

}

//найдем все возможные съедения для всех шашек
void SmartBot::FindEats(std::array<std::array<int, 8>, 8>& Board, Positions Col, Positions enemyCol, std::vector<PosTurn>& movesVec) {
	//проходимся по всем ячейкам(не оптимизировано)
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {

			//если цвет ячейки совпадает с заданным, то проверяем на съедение во все 4 стороны
			if (Board[x][y] == static_cast<int>(Col)) {
				CheckEats(Board, x, y, 1, 1, Col, enemyCol, movesVec);
				CheckEats(Board, x, y, -1, 1, Col, enemyCol, movesVec);
				CheckEats(Board, x, y, 1, -1, Col, enemyCol, movesVec);
				CheckEats(Board, x, y, -1, -1, Col, enemyCol, movesVec);
			}
			//если цвет ячейки совпадает с нашим, но является дамкой, то проверяем на съедение дамкой во все 4 стороны
			if (Board[x][y] == static_cast<int>(Col) + 2) {
				CheckEatsForKing(Board, x, y, 1, 1, Col, enemyCol, movesVec);
				CheckEatsForKing(Board, x, y, -1, 1, Col, enemyCol, movesVec);
				CheckEatsForKing(Board, x, y, 1, -1, Col, enemyCol, movesVec);
				CheckEatsForKing(Board, x, y, -1, -1, Col, enemyCol, movesVec);
			}
		}
	}
}


/*функция нахождения возможных ходов обычной шашкой во все стороны, стороны определяются цветом шашки
* Board - поле на котором происходит нахажодение хода
* x, y - откуда
* Col - цвет бота
* movesVec - вектор возможных ходов
*/
void SmartBot::CheckMoves(std::array<std::array<int, 8>, 8>& Board, int x, int y, Positions Col, std::vector<PosTurn>& movesVec) {
	//переменные перемещения
	int dx = 1, dy;

	//если белые - идем вверх, с черными иначе
	if (Col == Positions::WHITE_CHECKER) {
		dy = 1;
	}
	else
		dy = -1;

	//сначала вправо
	//проверяем границы
	if (CheckBorders(x, y, dx, dy)) {

		//если ячейка пустая, можем сходить на неё
		if (Board[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) {

			//создаем координаты хода, первое - откуда, второе - куда
			auto xy = std::make_pair(x, y);
			auto dxdy = std::make_pair(x + dx, y + dy);
			movesVec.push_back({ xy, dxdy });

			//если можем сходить то добавляем соответствующее значение
			movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO);

			//если становимся дамкой, то добавляем соответствующий рейтинг
			if (CheckBecomeKing(dxdy.second, Col)) {
				movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::BECOME_KING);
			}
		}
	}

	//теперь то же самое только влево
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


/*функция нахождения возможных ходов дамкой в одну сторону, сторона определяется переменными dx dy
* Board - поле на котором происходит нахажодение хода
* x, y - откуда
* dx dy - приращение координаты
* movesVec - вектор возможных ходов
*/
void SmartBot::CheckMovesForKing(std::array<std::array<int, 8>, 8>& Board, int x, int y, int dx, int dy, std::vector<PosTurn>& movesVec) {
	//определяем знак приращения
	int ddx = sign(dx);
	int ddy = sign(dy);

	//проверяем границы и отсутствие шашек на клетке
	while (CheckBorders(x, y, dx, dy) && PosBoard[x + dx][y + dy] == static_cast<int>(Positions::CELL_PLBL)) { //и пока нет шашек на клетке

		//добавим координаты откуда-куда в вектор хода
		auto xy = std::make_pair(x, y);
		auto dxdy = std::make_pair(x + dx, y + dy);
		movesVec.push_back({ xy, dxdy });
		movesVec[movesVec.size() - 1].relevance += static_cast<int>(SituationCost::CAN_GO);
		dx += ddx;
		dy += ddy; //передвигаемся вперед
	}
}


/*функция нахождения всех возможных ходов
* Board - поле на котором происходит нахажодение хода
* Col - цвет бота
* movesVec - вектор возможных ходов
*/
void SmartBot::FindMoves(std::array<std::array<int, 8>, 8>& Board, Positions Col, std::vector<PosTurn>& movesVec) {

	//проходимся по всему полю
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			//если обычная шашка
			if (Board[x][y] == static_cast<int>(Col)) {
				CheckMoves(Board, x, y, Col, movesVec);
			}
			//если дамка
			else if (Board[x][y] == static_cast<int>(Col) + 2) {
				//проходим во все стороны
				CheckMovesForKing(Board, x, y, 1, 1, movesVec);
				CheckMovesForKing(Board, x, y, -1, 1, movesVec);
				CheckMovesForKing(Board, x, y, 1, -1, movesVec);
				CheckMovesForKing(Board, x, y, -1, -1, movesVec);
			}
		}
	}
}


//проверка вектора съеденных шашек, была ли съедена шашка по данной координате
bool SmartBot::IsWasChecked(int x, int y, std::vector<std::pair<int, int>>& enemyPos) {
	for (auto& enemy : enemyPos) {
		//если координаты шашки и заданных координат совпадают то была съедена
		if (enemy.first == x && enemy.second == y) {
			return true;
		}
	}
	//иначе нет
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


/*функция проверки возможных съедений после того как съели шашку
* Board - игровое поле
* wayTree - троичное дерево оптимального пути
* enemyCol - цвет соперника
* stop - координата остановки, чтобы не ходить по кругу
* enemyPos - вектор съеденных шашек
*/
void SmartBot::CheckContinue
(std::array<std::array<int, 8>, 8>& Board, ternaryTree* wayTree, Positions enemyCol,
	std::pair<int, int> stop, std::vector<std::pair<int, int>>& enemyPos) {

	//создадим переменные для удобства
	int dx = wayTree->getDX();
	int dy = wayTree->getDY();
	int x = wayTree->getXto();
	int y = wayTree->getYto();

	//проверка границ
	if (CheckBorders(x, y, dx, dy)) {

		//проверим соседнюю ячейку в соответствии с вектором приращения на наличие шашки соперника
		if ((Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2)

			//проверим была ли съедена данная шашка и не является ли данная координата координатой остановки
			&& !IsWasChecked(x + dx, y + dy, enemyPos) && (x + dx != stop.first && y + dy != stop.second)) {

			//проверяем границы ячейки, куда будем перемещаться после съедения
			if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)) {


				//добавим координаты откуда-куда в дерево пути
				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);

				//повысим высоту дерева
				wayTree->incrementHeight();
				wayTree->first = new ternaryTree(xy, dxdy, wayTree);

				//проверим возможность дальнейшего съедения для этой ветки пути
				CheckContinue(Board, wayTree->first, enemyCol, stop, enemyPos);

			}
		}
	}
	//то же самое что и сверху только для другого направления
	if (CheckBorders(x, y, dx, -dy)) {

		if ((Board[x + dx][y - dy] == static_cast<int>(enemyCol) || Board[x + dx][y - dy] == static_cast<int>(enemyCol) + 2)
			&& !IsWasChecked(x + dx, y - dy, enemyPos) && (x + dx != stop.first && y - dy != stop.second)) {

			if (CheckBorders(x, y, 2 * dx, -2 * dy) && Board[x + 2 * dx][y - 2 * dy] == static_cast<int>(Positions::CELL_PLBL)
				&& !(x + 2 * dx == stop.first && y - 2 * dy == stop.second) &&
				//добавляется ещё одна проверка, был ли проверен данный ход предками
				//создано для того, чтобы не ходить по кругу
				//в первом случае этого не происходит, так как в случае многократного съедения,
				//передвижение в одном направлении не даст зацикливания
				IsWasCheckedByFather(x + 2 * dx, y - 2 * dy, wayTree)) {

				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + 2 * dx, y - 2 * dy);


				wayTree->incrementHeight();
				wayTree->second = new ternaryTree(xy, dxdy, wayTree);


				CheckContinue(Board, wayTree->second, enemyCol, stop, enemyPos);
			}
		}
	}
	//то же самое
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



//находим оптимальный ход, в случае наличия множественных ходов и развилок
//подбирается сначала исключением
//потом у кого высота больше
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


//функция возвращающая индекс направления, по которому будет проложен путь
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


//функция нахождения дополнительных съедений дамкой
void SmartBot::FindWay(int x, int y, int dx, int dy, std::array<std::array<int, 8>, 8> Board, quadrupleTree* wayTree, Positions enemyCol) {

	//по вектору направления выбираем путь
	auto destiny = getWay(dx, dy);

	//проверяем границы поля
	while (CheckBorders(x, y, dx, dy)) {

		//проверяем сначала на то, есть ли шашка на пути
		if (Board[x + dx][y + dy] == static_cast<int>(enemyCol) || Board[x + dx][y + dy] == static_cast<int>(enemyCol) + 2)
		{
			//если есть то проверяем свободно ли за ней
			if (CheckBorders(x, y, 2 * dx, 2 * dy) && Board[x + 2 * dx][y + 2 * dy] == static_cast<int>(Positions::CELL_PLBL)) {

				//меняем местами шашку 
				std::swap(Board[x][y], Board[x + 2 * dx][y + 2 * dy]);

				auto xy = std::make_pair(x, y);
				auto dxdy = std::make_pair(x + 2 * dx, y + 2 * dy);

				Board[x + dx][y + dy] = static_cast<int>(Positions::CELL_PLBL);


				//заполняем дерево пути новым ходом
				wayTree->incrementHeight();
				wayTree->way[destiny] = new quadrupleTree(xy, dxdy, wayTree);
				wayTree->way[destiny]->eated = std::make_pair(x + dx, y + dy);

				//рекурсия
				CheckContinueForKing(Board, wayTree->way[destiny], enemyCol);

			}
		}
		//если нет, то идем дальше в поиске шашки
		x += dx;
		y += dy;
	}
}


//функция, находящая возможные дополнительные съедения для дамки во все 4 стороны
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


//функция, выбирающая оптимальный путь съедения шашек, для того чтобы съесть по максимуму
void SmartBot::FillAnotherEatsForKing(quadrupleTree* wayTree, PosTurn& pos) {
	int height = -1;

	//сначала находим максимальную высоту дерева
	for (int i = 0; i < 4; i++) {
		if (wayTree->way[i] != nullptr && wayTree->way[i]->height > height) {

			height = wayTree->way[i]->height;

		}
	}

	//затем для максимальной высоты запихиваем в вектор доп ходов наш новый дополнительный ход съедения
	for (int i = 0; i < 4; i++) {
		if (wayTree->way[i] != nullptr && wayTree->way[i]->height == height) {

			pos.another_eats.push_back(wayTree->way[i]->_to);
			pos.whoWasEated.push_back(wayTree->way[i]->eated);
			FillAnotherEatsForKing(wayTree->way[i], pos);
		}
	}
}

//функция нахождения множественного съедения для каждого хода, вызывается после того, как были найдены ходы для съедения
void SmartBot::FindContinue(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {

	std::vector<PosTurn> newMoves;

	//для каждого хода съедения
	for (auto& pos : movesVec) {

		//проверяем обычной шашкой был сделан ход или дамкой
		if (Board[pos.from.first][pos.from.second] == static_cast<int>(Col)) {

			//создаем дерево пути
			ternaryTree* wayTree = new ternaryTree(pos.from, pos.to);

			//съеденные шашки
			std::vector<std::pair<int, int>> enemyPos;

			//добавляем съеденную шашку в вектор
			auto enemy = std::make_pair(pos.from.first + wayTree->getDX(), pos.from.second + wayTree->getDY());
			enemyPos.push_back(enemy);

			//выполняем функцию нахождения путей съедения шашек
			CheckContinue(Board, wayTree, enemyCol, pos.from, enemyPos);

			//заполняем самым большим по глубине путем вектор пути
			FillAnotherEats(wayTree, pos);


			//здесь заполняем вектор всех съеденных шашек
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

				//оцениваем ценность хода, в заивимости от количества съеденных шашек
				pos.relevance += pos.another_eats.size() * static_cast<int>(SituationCost::EAT_MORE);
			}
		}
		else if (Board[pos.from.first][pos.from.second] == static_cast<int>(Col) + 2) {


			//создаем четверное дерево
			quadrupleTree* wayTree = new quadrupleTree(pos.from, pos.to);

			std::vector<std::pair<int, int>> enemyPos;

			auto enemy = std::make_pair(pos.from.first + wayTree->getDX(), pos.from.second + wayTree->getDY());
			enemyPos.push_back(enemy);


			//попытаемся найти продолжение хода для дамки которая съела
			CheckContinueForKing(Board, wayTree, enemyCol);

			FillAnotherEatsForKing(wayTree, pos);

			//если съела то увеличиваем ценность хода в заивисимости от кол-ва съеденных шашек
			pos.relevance += pos.another_eats.size() * static_cast<int>(SituationCost::EAT_MORE);


			//здесь мы находим дополнительные ходы после съедения
			//так как после того как мы съели последнюю шашку
			//остаются дополнительные ходы для перемещения
			if (!pos.another_eats.empty()) {

				//определяем позицию дамки в текущий момент после последнего съедения
				int x = pos.another_eats[pos.another_eats.size() - 1].first;
				int y = pos.another_eats[pos.another_eats.size() - 1].second;

				//определяем вектор, по которому дамка двигалась во время последнего съедения
				int dx = x - pos.whoWasEated[pos.whoWasEated.size() - 1].first;
				int dy = y - pos.whoWasEated[pos.whoWasEated.size() - 1].second;


				//заполняем вектор дополнительных ходов, копируя текущий и изменяя у него последний ход
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

	//заполняем вектор ходов содержимым вектора дополнительных ходов
	if (!newMoves.empty()) {
		for (int i = 0; i < newMoves.size() - 1; i++) {
			movesVec.push_back(newMoves[i]);
		}
	}
}


//функция заполнения вектора ходов всевозможными ходами
void SmartBot::FillPosTurns(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {


	//найдем все возможные ходы для съедения
	FindEats(Board, Col, enemyCol, movesVec);

	//если ходов для съедения нет, ищем обычные ходы
	if (movesVec.empty()) {
		FindMoves(Board, Col, movesVec);
	}
	else {
		//если ходы для съедения есть, изменяем флаг того, что были съедены шашки
		if (enemyCol == color) {
			canEnemyEat = true;
		}
		//находим дополнительные множественные съедения
		FindContinue(Board, movesVec, Col, enemyCol);
	}
}

//функция перемещения шашки соперника
void SmartBot::TryMoveEnemy
(std::array<std::array<int, 8>, 8>& Board, std::vector<PosTurn>& movesVec, PosTurn& pos, PosTurn& enemyTurn, Positions Col, Positions enemyCol) {


	//если у соперника есть множественное съедение
	if (!enemyTurn.another_eats.empty()) {
		//то меняем местами ячейку отправления и самую последнюю ячейку
		std::swap(Board[enemyTurn.from.first][enemyTurn.from.second],
			Board[enemyTurn.another_eats[enemyTurn.another_eats.size() - 1].first][enemyTurn.another_eats[enemyTurn.another_eats.size() - 1].second]);
	}
	else {
		//если множественного съедения нет, то просто меняем ячейку откуда местами с ячейкой куда
		std::swap(Board[enemyTurn.from.first][enemyTurn.from.second], Board[enemyTurn.to.first][enemyTurn.to.second]);
	}

	//если были съедения
	if (!enemyTurn.whoWasEated.empty()) {
		for (auto& eated : enemyTurn.whoWasEated) {
			//то для каждой ячейки, где была съедена шашка выставляем пустую активную ячейку
			Board[eated.first][eated.second] = static_cast<int>(Positions::CELL_PLBL);
		}
	}

	std::vector<PosTurn> movesVecMy;

	//теперь находим возможные съедения для моих шашек
	FindEats(Board, Col, enemyCol, movesVecMy);

	//если он пустой
	if (movesVecMy.empty()) {
		//то просто находим возможные ходы
		FindMoves(Board, Col, movesVecMy);

	}
	else {
		//если можем съесть, то добавляем рейтинг к данному ходу с пониженным коэффициентом
		//так как вероятность данного события относительно мала
		pos.relevance += static_cast<int>(SituationCost::EAT) * movesVecMy.size() / 5;

		//находим множественное съедение
		FindContinue(Board, movesVecMy, enemyCol, Col);

		for (auto& moves : movesVecMy) {


			if (!moves.another_eats.empty()) {
				//если можем съесть, то добавляем рейтинг к данному ходу с пониженным коэффициентом
				pos.relevance += static_cast<int>(SituationCost::EAT_MORE) * moves.another_eats.size() / 5;

			}


		}
	}
}


//функция перемещения и оценки хода на тестовом поле
void SmartBot::TryMove(PosTurn& pos, Positions Col, Positions enemyCol) {

	//создаем тестовое поле
	auto testBoard = PosBoard;

	//если есть множественное съедение
	if (!pos.another_eats.empty()) {

		//то меняем местами ячейку отправления и самую последнюю ячейку
		std::swap(testBoard[pos.from.first][pos.from.second],
			testBoard[pos.another_eats[pos.another_eats.size() - 1].first][pos.another_eats[pos.another_eats.size() - 1].second]);
	}
	else {

		//если множественного съедения нет, то просто меняем ячейку откуда местами с ячейкой куда
		std::swap(testBoard[pos.from.first][pos.from.second], testBoard[pos.to.first][pos.to.second]);
	}

	//если были съедения
	if (!pos.whoWasEated.empty()) {
		for (auto& eated : pos.whoWasEated) {
			//то для каждой ячейки, где была съедена шашка выставляем пустую активную ячейку
			testBoard[eated.first][eated.second] = static_cast<int>(Positions::CELL_PLBL);
		}
	}

	//вектор ходов соперника
	std::vector<PosTurn> movesVecEnemy;

	//найдем ходы съедения для соперника
	FindEats(testBoard, enemyCol, Col, movesVecEnemy);


	//если ходы пустые
	if (movesVecEnemy.empty()) {
		if (canEnemyEat == true) {
			//добавляем разницу в возможных съедениях умноженных на коэффициент сохранения шашки
			pos.relevance += static_cast<int>(SituationCost::SAVE) * PosMovesEnemy.size();
		}

		//находим возможные ходы
		FindMoves(testBoard, Col, movesVecEnemy);

		//добавляем разницу в возможных ходах до и после нашего хода помноженного на коэффициент блокировки хода
		pos.relevance += static_cast<int>(SituationCost::BLOCK) * (PosMovesEnemy.size() - movesVecEnemy.size());
	}
	else {

		//если съедение было, то этот ход не так хорош, и убавляем рейтинг в зависимости от числа ходов, которые могут есть
		pos.relevance += static_cast<int>(SituationCost::DIE) * movesVecEnemy.size();

		//находим множественные съедения
		FindContinue(testBoard, movesVecEnemy, enemyCol, Col);

		for (auto& moves : movesVecEnemy) {


			if (!moves.another_eats.empty()) {
				//если множественные съедения есть, то так же изменяем рейтинг в худшую сторону
				pos.relevance += static_cast<int>(SituationCost::DIE_MORE) * moves.another_eats.size();

			}


		}


	}

	//теперь производим передвижение каждым ходом соперника
	for (auto& enemyTurn : movesVecEnemy) {
		TryMoveEnemy(testBoard, movesVecEnemy, pos, enemyTurn, Col, enemyCol);
	}
}

/*void BotV3::TryMoveAllPoss(std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {
	//пытаемся сходить каждым ходом, чтобы произвести оценку
	Timer t;
	for (auto& pos : movesVec) {
		TryMove(pos, Col, enemyCol);
	}
}*/

void SmartBot::TryMoveAllPoss(std::vector<PosTurn>& movesVec, Positions Col, Positions enemyCol) {
	//пытаемся сходить каждым ходом, чтобы произвести оценку

	std::vector<std::future<void>> vecAsy(movesVec.size());


	for (int i = 0; i < movesVec.size(); i++) {
		vecAsy[i] = std::async(std::launch::async, [&]() { TryMove(movesVec.at(i), Col, enemyCol); });
	}

	vecAsy.clear();
}



//функция возвращения самого рейтингового хода
SmartBot::PosTurn SmartBot::ReturnMove() {

	//заполняем вектора моих ходов и возможных ходов соперника
	FillPosTurns(PosBoard, PosMoves, color, enemycolor);
	FillPosTurns(PosBoard, PosMovesEnemy, enemycolor, color);

	//прозиводим оценку всем нашим возможным ходам
	TryMoveAllPoss(PosMoves, color, enemycolor);

	std::vector<PosTurn> moves;
	PosTurn max;
	max.relevance = INT_MIN;

	//вычисляем максимальный ход по рейтингу
	for (auto& move : PosMoves) {
		if (move.relevance > max.relevance) {
			max = move;
		}
	}


	//если есть одинаковые ходы по рейтингу, заполняем ими вектор ходов
	for (auto& move : PosMoves) {
		if (move.relevance == max.relevance) {
			moves.push_back(move);
		}
	}
	moves.push_back(max);

	srand(time(NULL));
	//если есть равные по ценности ходы, возвращаем случайный из них
	if (moves.size() > 1) {
		int randomINDEX = rand() % (moves.size() - 1);
		return moves[randomINDEX];
	}//если нет, то самый первый
	else if (moves.size() == 1) {
		return moves[0];
	}
}



