#pragma once
#include "cell.h"
#include <array>
#include <vector>
#include <algorithm>

#define playboard Playboard::getPlayboard() //макрос для удобства

//Структура xy сделана просто для удобства хранения координат ячеек
struct xy {
	int x, y;

	//получение координат
	int getX() { return x; }
	int getY() { return y; }

	//установка координат
	void setX(int x0) { x = x0; }
	void setY(int y0) { y = y0; }

	//сравнение двух структур xy1 и xy2 например, через оператор, это будет часто использоваться
	bool operator==(const xy xy0) {
		if ((*this).x == xy0.x && (*this).y == xy0.y) {
			return true;
		}
		else
			return false;
	}

	//оператор неравно , тоже что и сверху только отрицание
	bool operator!=(const xy xy0) {
		return !((*this) == xy0);
	}
};

//дефолтная константа, координата за картой
//(используется много где, например если у нас ничего не выделено, то присваивается выделенной координате как то так...)
const xy OFF_MAP = { -1,-1 };

//Класс "Игровая доска", выполнен с помощью шаблона singleton
class Playboard {
private:
	Playboard(); //синглтоновский приватный конструктор

	//Двумерный массив _board, хранит указатели на класс клетка
	std::array<std::array<Cell*, 8>, 8> _board;

	//структура - выбранная координата
	xy _selectedCoor;


	std::vector<xy> _possibleMoves; //Вектор возможных ходов, состоит из структур xy (на эти xy можно будет сходить)
	std::vector<xy> _canEats; //координаты шашек, которые МОГУТ съесть

	int situation = 0; //костыль, нужен для завершения хода (Исп-я только в одном месте)
	bool turn_end = true; //костыль, тоже нужен для завершения хода

	//Ширина и высота игровой доски
	const int _tableW = 8;
	const int _tableH = 8;

	//установить выделенную координату, тут перегрузка одна для двух способов установка
	void setSelectedCoor(xy position) { _selectedCoor = position; }
	void setSelectedCoor(int x, int y) { _selectedCoor.setX(x); _selectedCoor.setY(y); }
	//установить выделенную ячейку (подробнее в определении)
	void setSelectedCell(int x, int y);
	void setSelectedCell(xy position);

	//очистить ячейку от шашки, используется когда шашку съели и нужно ее удалить
	//с удалением памяти клетки удаляетс и шашка, а потом просто создаем заново клетку 
	void refreshCell(int x, int y) { delete _board[x][y]; _board[x][y] = new ActiveCell(); }

	//поменять местами ячейки (используется когда ходим и кушаем)
	//Используется тогда, когда шашка меняет свою позицию
	void swapCells(xy pos1, xy pos2);
	void swapCellsWithoutSelected(xy pos1, xy pos2); //(костыль, подробности потом)

	//найти возможные ходы для съедения
	//она устанавливает возможные ходы для съедения
	void checkEats(int dx, int dy);


	//проверить, может ли отсюда съесть
	//а эта checkEats просто возвращает true, если из этой точки (ячейки) можно съесть, false если низя
	bool checkEats(int x, int y, int dx, int dy);


	//проверить, входит ли в число могущих есть шашек
	//Объяснение: есть правило "обязательно нужно съесть если это возможно".
	//Функция проверяет выбранную xy на принадлежность вектору координат xy, которые могут есть (_canEats).
	//Если принадлежит - возвраащет true (далее шашка будет выделяться, а ходы подсвечиваться), если нет - false
	bool checkEaters(int x, int y);


	//узнать возможность хода в данную ячейку
	//происходит проверка координаты xy на принадлежность вектору возможных ходов _possibleMoves
	bool isMovePossible(int x, int y);


	//узнать, это съедение шашки?
	bool isEating(int x, int y); //съел ли ты шашку или нет
	bool isEatingForKing(xy& pos, int x, int y);

	//узнать, можно ли отсюда ещё есть
	//Используется тогда когда мы съели шашкой, и затем нам нужно узнать можем ли мы еще есть или нет
	bool isThereAnotherEats(int x, int y);


	//установить возможные ходы для выделенной шашки
	void setPossibleMovesForChecker();
	void setPossibleEatesForChecker();

	//установить возможные ходы для дамок
	void setPossibleEatesForKing();
	void setPossibleMovesForKing();
	void findFoodForKing(int dx, int dy); //две вспомогательные функции
	void findMoveForKing(int dx, int dy);

	//получить выделенную координату
	xy getSelectedCoor() { return _selectedCoor; }
	//получить выделенную ячейку 
	Cell* getSelectedCell() { return getCell(_selectedCoor); }

	//убрать подсвечиваемые ходы
	//используется: допустим выбрали шашку и подсветились ходы, и допустим кликнули на пустое поле -
	//тогда будет убираться выделение для шашки и все подсвечиваемые ходы
	void ClearPossible();
	//убрать выделение ячейки
	void ClearSelected();


	//получить вектор шашек, которые могут есть
	std::vector<xy>& getCanEats() { return _canEats; }

	//найти могущих есть
	//то есть здесь мы заполняем вектор _canEats
	void findEaters(CheckerColor b_or_w);

	//очистить вектор могущих есть
	//Где используется: например, мы скушали , соответственно нам нужно уже очистить вектор _canEats
	void clearEaters() { getCanEats().clear(); }

	//очистить вектор могущих есть, кроме одного, который уже съел и может съесть ещё
	void clearEaters(int x, int y);




	//функция выбора шашки
	void SelectChecker(int x, int y, CheckerColor b_or_w);

	//функция перемещения шашки
	int MoveChecker(int x, int y, CheckerColor b_or_w);

public:

	//получить доступ к игровой доске
	static Playboard& getPlayboard(); //статическая функция, возвращает доступ к доске, в ней же вызывается приватный конструктор
	//нарисовать игровую доску
	void drawPlayboard();



	//получить ячейку по координатам
	Cell* getCell(int x, int y) { return _board[x][y]; }
	Cell* getCell(xy position) { return _board[position.getX()][position.getY()]; }
	//получить вектор возможных ходов
	std::vector<xy>& getPossibleMoves() { return _possibleMoves; }


	//получить габариты доски
	int getW() { return _tableW; }
	int getH() { return _tableH; }





	//функция нажатия на ячейку, отвечает за все клики мыши
	bool Select(int x, int y, CheckerColor b_or_w);


};