#pragma once
#include "checker.h"

//КЛАСС КЛЕТКА
//Это просто пустая клетка, неактивная, не может иметь шашку. (это будут светлые клетки)
class Cell {
public:
	virtual void drawCell(); //рисовка клетки (Просто закрашенный квадрат)
	//(Тк этот класс является клеткой неактивной, поэтому большинство функций - ничего не делают)
	virtual void selectChecker() {}
	void selectCheckerOFF() {}

	virtual void makePossibleMove() {}
	virtual void makeImPossibleMove() {}


	virtual bool isPossible() { return false; } //можно ли на эту клетку сходить - нет, нельзя, так как клетка неактивна
	virtual bool isSelected() { return false; } //может ли эта клетка быть выделенной - нет, так как она не может иметь шашку
	virtual bool isFree() { return true; }      //свободна ли эта клетка - да, всегда свободна, тк на ней ничего не стоит

	virtual void setPost(CheckerHierarchy post) {}

	//Функции получения цвета шашки и должности шашки возввращают исключения, 
	//поскольку на клетке ничего не стоит по определению класса
	virtual CheckerColor getCheckerColor() { return EXCEPTION_COLOR; }
	virtual CheckerHierarchy getCheckerPost() { return EXCEPTION_POST; }
};

//Перечисление - для удобства, чтобы понимать если ли на этой клетке шашка или нет
enum CHECKERS_HAVE {
	HAS_CHECKER = true, HAS_NO_CHECKER = false
};

//Класс активная клетка - подкласс 
//На такую клетку можно сходить и она может иметь шашку
class ActiveCell : public Cell {
	Checker* _checker;           //это указатель на шашку
	bool _possibleMove = false;  //bool можно ли сходить на эту клетку или нельзя


	Checker* getChecker() { return _checker; } //возвращает указатель на шашку

	void setChecker(CheckerColor b_or_w) { _checker = new Checker(b_or_w); } //установить шашку на эту клетку по цвету

public:

	//Тут 2 конструктора
	ActiveCell() {} //этот используется тогда, когда на этой клетке нет шашки, просто выделяем память
	ActiveCell(CheckerColor b_or_w); //а этот используется тогда, когда на этой клетке есть шашка

	void drawCell(); //просто рисовка клетки
	void drawFrame(); //рисовка рамки (используется для рисовки рамок тех клеток, на которые можно сходить)

	bool isSelected(); //возвращает выделена ли шашка на этой клетке (true) или нет(false)
	bool isPossible() { return _possibleMove; } //возможен ли ход на эту клетку
	bool isFree();      //есть ли на этой клетке есть шашка (возвращает HAS_CHECKER из enum), если нет (HAS_NO_CHECKER)

	void selectChecker(); //выбрать клетку
	void selectCheckerOFF() { getChecker()->CheckerSelectOFF(); } //убрать выделение

	void setPost(CheckerHierarchy post); //установить должность для шашки

	void makePossibleMove() { _possibleMove = true; } //установить возможность хода на клетку 
	void makeImPossibleMove() { _possibleMove = false; } //установить невозможность хода на эту клетку

	//Получение цвета или должности 
	CheckerColor getCheckerColor();
	CheckerHierarchy getCheckerPost();

};
