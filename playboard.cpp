#include "playboard.h"

//Это синглтон 
//Ф-я получения доступа к доске
//Обоснование: эта доска - она статическая, то есть она живет всё время в единственном экземпляре (типа глобальных переменных)
//тк по сути доска должна быть только одна
Playboard& Playboard::getPlayboard() {
	static Playboard instance;
	return instance; //то есть возвраащет только этот экземпляр игровой доски, новый создать невозможно
}

//конструктор класса доска
Playboard::Playboard() {
	_selectedCoor = OFF_MAP;

	//В этом цикле происходит заполнение поля клетками и шашками
	for (int y = 0; y < getH(); y++) {
		for (int x = 0; x < getW(); x++) {
			if (y < 3) { //если координата y<3 - заполнение белыми шашками
				if (y % 2 == 0) { //заполнение зависит от остатка от деления y на 2. Если он = 0
					_board[x][y] = new ActiveCell(WHITE); //создается активная клетка
					x++; //двигаемся вправо
					_board[x][y] = new Cell(); //заполняем неактивную клетку
				}
				else { //а если остаток от деления не равен 0
					_board[x][y] = new Cell(); //то сначала неактивная клетка
					x++;
					_board[x][y] = new ActiveCell(WHITE); //а затем активная
				}
			}
			else if (y > 4) { //тоже самое для черных шашек
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

			//это та сама середина поля
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

//Функция рисования
void Playboard::drawPlayboard() {
	glLoadIdentity();
	glScalef(2.0f / getW(), 2.0f / getH(), 1);
	glTranslatef(-getW() / 2.0f, -getH() / 2.0f, 0); //преобразование координат
	//то есть ф-я разбивает поле на 8 вверх 8 вниз и тд

	//В цикле рисование
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

//собсна ф-я выделения ячейки
void Playboard::setSelectedCell(int x, int y) {
	//произойдет выделение только в том случае, если в выбранной ячейке есть шашка
	if (getCell(x, y)->isFree() == HAS_CHECKER) {
		setSelectedCoor(x, y);
		//функция selectCell выделит ячейку только в том случае, если в ячейке есть шашка
		getSelectedCell()->selectChecker();
	}
}


void Playboard::setSelectedCell(xy position) {
	setSelectedCoor(position);
	//функция selectCell выделит ячейку только в том случае, если в ячейке есть шашка
	getSelectedCell()->selectChecker();
}

int const DOWN = -1;
int const UP = 1;

//Вычисляем возможные ходы для шашки (то есть подсвечивать ходы для выделенной шашки)
void Playboard::setPossibleMovesForChecker() {
	int dy = 1;
	int dx = 1;
	if (getSelectedCoor() == OFF_MAP) {
		return; //если не выделено ничего, то алгоритм ниже не выполняется
	}


	int x = getSelectedCoor().getX();
	int y = getSelectedCoor().getY();

	CheckerColor b_or_w = getSelectedCell()->getCheckerColor(); //присваеваем цвет
	//проверяем на принадлежность цвету и границы, которые будут проверяться для этих цветов
	if ((b_or_w == WHITE && y + dy < 8) || (b_or_w == BLACK && y - dy >= 0)) { //условие проверки границ (для белых верхняя граница, для черных нижняя)
		(b_or_w == WHITE) ? dy = UP : dy = DOWN; //если цвет белый => приращение идет вверх, если чорный => вниз
		if (x + dx < _tableW) { //проверка на правую границу
			if (getCell(x + dx, y + dy)->isFree() == HAS_NO_CHECKER) { //Если шашки на клетке x+dx,y+dy НЕТ, 
				getCell(x + dx, y + dy)->makePossibleMove(); //то ход возможен (makePossibleMove делает ход возможным подсвечивая ячейку)
				getPossibleMoves().push_back({ x + dx, y + dy }); //а в вектор возможных ходов запихиваем данный ход
			}
		}
		if (x - dx >= 0) { //проверка на левую границу
			if (getCell(x - dx, y + dy)->isFree() == HAS_NO_CHECKER) { //Тоже самое,только для левой границы, ходы по диагонали влево
				getCell(x - dx, y + dy)->makePossibleMove();
				getPossibleMoves().push_back({ x - dx, y + dy });
			}
		}
	}
}

//Проверка на возможность съедения
void Playboard::checkEats(int dx, int dy) {
	if (getSelectedCoor() == OFF_MAP) {
		return;
	}
	int x = getSelectedCoor().getX();
	int y = getSelectedCoor().getY();
	CheckerColor b_or_w = getSelectedCell()->getCheckerColor();


	if (x + dx < getW() && y + dy < getH() && x + dx >= 0 && y + dy >= 0) { //Проверка на принадлежность границам
		if (getCell(x + dx, y + dy)->isFree() == HAS_CHECKER && //если x+dx,y+dx внутри карты, то проверяем наличие шашки и то что цвета разные (чтоб свою не сожрать)
			getCell(x + dx, y + dy)->getCheckerColor() != b_or_w)
		{ //то...
			if (x + 2 * dx < getW() && y + 2 * dy < getH() && x + 2 * dx >= 0 && y + 2 * dy >= 0) //проверка на принадлежность границы той клетки, куда мы перейдем после съедения
			{
				if (getCell(x + 2 * dx, y + 2 * dy)->isFree() == HAS_NO_CHECKER) { //и проверим что данная клетка, куда перейдем после съедения, является пустой
					getCell(x + 2 * dx, y + 2 * dy)->makePossibleMove(); //makePossibleMove делает ход возможным, подсвечивая ячейку
					getPossibleMoves().push_back({ x + 2 * dx, y + 2 * dy }); //возможный ход вставляем в вектор 
				}
			}
		}
	}
}

//сигнум
int sign(int numbr) {
	int sgn = 0;
	numbr < 0 ? sgn = -1 : sgn = 1;

	return sgn;
}

//буль, Тот же самый алгоритм, только он говорит возможно ли из точки xy сделать ход 
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

	//Этот алгоритм работает еще и для дамки (оставим на потом)
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

//То есть мы делаем проверку на съедение в четыре стороны
void Playboard::setPossibleEatesForChecker() {
	checkEats(1, 1);
	checkEats(-1, 1);
	checkEats(1, -1);
	checkEats(-1, -1);
}

//Ф-я проверки есть ли еще возможность съесть после того как сделали съедение
bool Playboard::isThereAnotherEats(int x, int y) {
	if (checkEats(x, y, 1, 1) ||
		checkEats(x, y, -1, 1) ||
		checkEats(x, y, 1, -1) ||
		checkEats(x, y, -1, -1))
		return true;
	else
		return false;
}

//Ф-я работает по всей карте, проверяет каждую шашку определенного цвета на то, может ли она есть
void Playboard::findEaters(CheckerColor b_or_w) {
	for (int y = 0; y < getH(); y++) {
		for (int x = 0; x < getW(); x++) {
			if (getCell(x, y)->getCheckerColor() == b_or_w) {
				if (checkEats(x, y, 1, 1) ||
					checkEats(x, y, -1, 1) ||
					checkEats(x, y, 1, -1) ||
					checkEats(x, y, -1, -1)) {
					getCanEats().push_back(xy{ x,y }); //если хотя бы одна из этих 4-ех возвращает true, то запихиваем в в-р _canEats координату шашки, которая может съесть
				}
			}
		}
	}
}

//Ф-я проверяет шашку на то может ли она есть или нет. (Используется в правиле "нужно обязательно есть")
//Пример: допустим мы можем съесть одну шашку шашкой M. При клике на любую другую шашку будет возвращаться false
bool Playboard::checkEaters(int x, int y) {
	if (getCanEats().size() != 0) {
		for (auto position = getCanEats().begin(); position != getCanEats().end(); position++) { //идет сравнение координат с вектором canEats
			if ((*position) == xy{ x, y }) {
				return true; //мы можем съесть
			}
		}
		return false; //не можем ходить
	}
	return true;
}

//ф-я, которая удаляет все координаты шашки которые могут ходить, кроме выбранной
//Пример использования: когда шашка может съесть 2+ раз, она используется для съедения еще одного раза (был баг до этой штуки)
void Playboard::clearEaters(int x, int y) {
	if (getCanEats().size() > 1) { //то есть если возможных съедений больше 1
		for (auto position = getCanEats().begin(); position != getCanEats().end(); position++) { //пробег по вектору
			if ((*position) != xy{ x, y }) {
				getCanEats().erase(position); //удаляем координаты, не равные ходящей
				if (getCanEats().size() == 1) {
					return;
				}
			}
		}
	}
}

void Playboard::ClearPossible() {
	for (auto position = getPossibleMoves().begin(); position < getPossibleMoves().end(); position++) {
		//сделает неактивным выделение ячейки для возможного хода
		getCell(*position)->makeImPossibleMove();
	}
	getPossibleMoves().clear();
}

void Playboard::ClearSelected() {
	if (getSelectedCoor() != OFF_MAP) {
		//функция selectCell делает ячейку неактивной
		getSelectedCell()->selectChecker();
		setSelectedCoor(OFF_MAP);
	}
}


//поиск еды для дамки
void Playboard::findFoodForKing(int dx, int dy)
{
	//нужен знак в какую стороны мы движемся
	int ddx = sign(dx);
	int ddy = sign(dy);

	//получаем координаты выбранной шашки и цвет
	int x0 = getSelectedCoor().getX();
	int y0 = getSelectedCoor().getY();
	CheckerColor b_or_w = getSelectedCell()->getCheckerColor();

	//движемся по циклу до тех пор, пока мы в границах игрового поля (потому што дамка)
	while (x0 + dx >= 0 && x0 + dx < getW() && y0 + dy >= 0 && y0 + dy < getH()) {
		if (getCell(x0 + dx, y0 + dy)->isFree() == HAS_CHECKER && //затем проверка, есть ли в клетке x0+dx,y0+dy шашка другого цвета
			getCell(x0 + dx, y0 + dy)->getCheckerColor() != b_or_w) {
			if (x0 + ddx + dx >= 0 && x0 + ddx + dx < getW() && y0 + ddy + dy >= 0 && y0 + ddy + dy < getH() && //если внутри границ
				getCell(x0 + ddx + dx, y0 + ddy + dy)->isFree() == HAS_NO_CHECKER) { // и нет шашки в клетке
				dx += ddx;
				dy += ddy;//то переходим на эту клетку
				while (x0 + dx >= 0 && x0 + dx < getW() && y0 + dy >= 0 && y0 + dy < getH() && // и пока не наткнемся на конец карты или шашку
					getCell(x0 + dx, y0 + dy)->isFree() == HAS_NO_CHECKER) {
					getCell(x0 + dx, y0 + dy)->makePossibleMove();//делаем клетку возможной к ходу
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

void Playboard::setPossibleEatesForKing() {//алгоритм вызывает предыдущую функцию во все четыре стороны
	findFoodForKing(1, 1);
	findFoodForKing(-1, 1);
	findFoodForKing(1, -1);
	findFoodForKing(-1, -1);

}

//ф-я ищет ходы для дамки, она вызываетсчя после того, когда нет возможных съедений для дамки
void Playboard::findMoveForKing(int dx, int dy) {
	int x0 = getSelectedCoor().getX();
	int y0 = getSelectedCoor().getY();
	int ddx = sign(dx);
	int ddy = sign(dy);
	while (x0 + dx >= 0 && x0 + dx < getW() && y0 + dy >= 0 && y0 + dy < getH() && //пока в границах игрового поля
		getCell(x0 + dx, y0 + dy)->isFree() == HAS_NO_CHECKER) { //и пока нет шашек на клетке
		getCell(x0 + dx, y0 + dy)->makePossibleMove(); //делаем этот ход возможным
		getPossibleMoves().push_back({ x0 + dx, y0 + dy }); //запихиваем в вектор возможных ходов
		dx += ddx;
		dy += ddy; //передвигаемся вперед
	}
}

//алгоритм вызывает предыдущую функцию во все четыре стороны
void Playboard::setPossibleMovesForKing() {
	if (getSelectedCoor() == OFF_MAP) {
		return;
	}
	findMoveForKing(1, 1);
	findMoveForKing(1, -1);
	findMoveForKing(-1, 1);
	findMoveForKing(-1, -1);
}

//Ф-я выделяет шашку
void Playboard::SelectChecker(int x, int y, CheckerColor b_or_w) {

	if (checkEaters(x, y) == true) { //true может быть только если нет возможности съедения (в-р пуст) или выбрана та шашка, которая может съесть
		//если цвета совпадают у ячейки и у игрока, то входим
		if (getCell(x, y)->getCheckerColor() == b_or_w) {
			setSelectedCell(x, y); //выделяем шашку
			//проверяем входит ли эта шашка в число могущих есть, если да то выделяем
			if (getCell(x, y)->isFree() == HAS_CHECKER) {
				if (getCell(x, y)->getCheckerPost() == CHECKER) {
					//проверяем возможные съедения шашек
					setPossibleEatesForChecker();
					//если возможных съедений нет, то рассматриваем ходы без съедения
					if (getPossibleMoves().size() == 0) {
						setPossibleMovesForChecker();
					}
				}
				else if (getCell(x, y)->getCheckerPost() == KING) {
					//проверяем возможные съедения шашек
					setPossibleEatesForKing();
					//если возможных съедений нет, то рассматриваем ходы без съедения
					if (getPossibleMoves().size() == 0) {
						setPossibleMovesForKing();
					}
				}
			}
		}
	}
}

//Просто проверяет выбранную точку на принадлежость вектору возможных ходов
bool Playboard::isMovePossible(int x, int y) {
	for (auto position = getPossibleMoves().begin(); position != getPossibleMoves().end(); position++) {
		if ((*position) == xy{ x, y }) {
			return true;
		}
	}
	return false;
}

//Ф-я определяет, съедение это было или нет
bool Playboard::isEating(int x, int y) {
	int dx = x - getSelectedCoor().getX();
	int dy = y - getSelectedCoor().getY();
	return (abs(dx) == 2 && abs(dy) == 2); //если разница между клетками ==2, соответственно это было съедение, а не просто ход
}

//Ф-я меняет ячейки местами (При ходах или съедениях)
void Playboard::swapCells(xy pos1, xy pos2) {
	ClearSelected();
	std::swap(_board[pos1.x][pos1.y], _board[pos2.x][pos2.y]);
}

//Это используется когда после съедения есть еще одно съедение
void Playboard::swapCellsWithoutSelected(xy pos1, xy pos2) {
	std::swap(_board[pos1.x][pos1.y], _board[pos2.x][pos2.y]);
	setSelectedCoor(pos2); //снова выбираем обновленную клетку 
	if (getSelectedCell()->getCheckerPost() == CHECKER) { //затем условие, является ли шашка которой мы съели пешкой или дамкой
		setPossibleEatesForChecker();
	}
	else if (getSelectedCell()->getCheckerPost() == KING) {
		setPossibleEatesForKing();
	}
}

const int THIS_IS_MOVE = 0; // в случае если мы сходили и нужно завершить ход
const int THERE_IS_ANOTHER_MOVE = 1; // в случае если мы съели и можем съесть ещё
const int THIS_IS_NOT_A_MOVE = 2; // в случае если мы не сходили, просто кликнули на карту

//Эта ф-я - аналог ф-ии выше isEating, только для дамки
bool Playboard::isEatingForKing(xy& pos, int x, int y) {
	int ddx = sign(x - getSelectedCoor().getX());
	int ddy = sign(y - getSelectedCoor().getY());
	int dx = ddx;
	int dy = ddy;
	while (getSelectedCoor().getX() + dx != x && getSelectedCoor().getY() + dy != y) {
		if (getCell(getSelectedCoor().getX() + dx, getSelectedCoor().getY() + dy)->isFree() == HAS_CHECKER) { //если на всем пути была шашка...
			pos.x = getSelectedCoor().getX() + dx;
			pos.y = getSelectedCoor().getY() + dy;
			return true; //то это съедение
		}
		dx += ddx;
		dy += ddy;
	}
	return false;
}

//Ф-я перемещения шашки, принимает координаты и цвет
int Playboard::MoveChecker(int x, int y, CheckerColor b_or_w) {


	if (isMovePossible(x, y) == true) { //Проверка на то, возможен ли ход
		if (getSelectedCell()->getCheckerPost() == CHECKER) { //если выделенная шашка - не дамка
			if ((b_or_w == WHITE && y == getH() - 1) || (b_or_w == BLACK && y == 0)) { //и если белые достигли верха и черные достигли низа, то они становятся дамками
				getCell(getSelectedCoor().getX(), getSelectedCoor().getY())->setPost(KING); //для выбранной ячейки ставим должность дамки
			}

			if (isEating(x, y)) { //проверяем, было ли это съедением
				//отсюда
				int dx = (x - getSelectedCoor().getX()) / 2; //делим на 2 тк нужно получить середину (в середине стояла съеденная шашка)
				int dy = (y - getSelectedCoor().getY()) / 2;
				refreshCell(getSelectedCoor().getX() + dx, getSelectedCoor().getY() + dy);
				//посюда хаваем

				ClearPossible(); //очищаем поле от возможных ходов (тк мы уже сходили)

				//перемещаем ячейку (вместе с шашкой) в ту ячейку куда мы кликнули
				swapCellsWithoutSelected(getSelectedCoor(), xy{ x,y });
				if (isThereAnotherEats(x, y)) { //еще здесь проверяем можем ли мы ЕЩЕ съесть
					return THERE_IS_ANOTHER_MOVE; //возвращаем что здесь есть еще возможность съедения
				}
				//если нет еще возможности съедения, то убираем выделение клетки и заканчиваем ход
				ClearSelected();
				return THIS_IS_MOVE;
			}
			//а если не съедение...
			else {// просто передвижение
				ClearPossible();
				swapCells(getSelectedCoor(), xy{ x,y });
				return THIS_IS_MOVE;
			}
		}
		//а если это дамка...
		if (getSelectedCell()->getCheckerPost() == KING) {
			xy pos_with_checker = OFF_MAP;
			if (isEatingForKing(pos_with_checker, x, y)) {
				refreshCell(pos_with_checker.getX(), pos_with_checker.getY()); // схавали удалили шашку

				ClearPossible();

				//перемещаем
				swapCellsWithoutSelected(getSelectedCoor(), xy{ x,y });
				if (isThereAnotherEats(x, y)) {
					return THERE_IS_ANOTHER_MOVE;
				}
				ClearSelected();
				return THIS_IS_MOVE;
			}
			//а если это не съедение...
			else {
				ClearPossible();
				swapCells(getSelectedCoor(), xy{ x,y }); //то просто меняем местами (обычный ход был)
				return THIS_IS_MOVE;
			}
		}
	}
	return THIS_IS_NOT_A_MOVE;
}

//Ф-я клика мыши на игровую карту
bool Playboard::Select(int x, int y, CheckerColor b_or_w) {

	//Сначала выполняется ф-я заполнения вектора шашек, которые могут есть
	findEaters(b_or_w);

	if (situation == THERE_IS_ANOTHER_MOVE || turn_end == false) {
		clearEaters(getSelectedCoor().getX(), getSelectedCoor().getY());
	}//на случай когда мы съели один раз, можем съесть ещё, но ещё есть друга шашка которая тоже может есть
	situation = MoveChecker(x, y, b_or_w);
	if (situation == THIS_IS_MOVE) {
		turn_end = true; //конец хода
		clearEaters();   //чистим в-р 
		return true;
	}
	else if (situation == THERE_IS_ANOTHER_MOVE) { //ситуация когда есть еще одна возможность скушать
		turn_end = false; //флаг того что ход не закончен
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

