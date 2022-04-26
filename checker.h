#pragma once
#include <windows.h>
#include <gl/gl.h>

#pragma comment(lib, "opengl32.lib") //подключение opengl

//Перечисление - цвет у шашки
enum CheckerColor {
	BLACK, WHITE, EXCEPTION_COLOR //exception_color - если на клетке нет шашки никакой, то это дает исключение
};

//Это перечисление - должность шашки
enum CheckerHierarchy {
	CHECKER, KING, EXCEPTION_POST //Т.e либо пешка либо дамка, либо на клетке нет никакой шашки
};


class Checker {
public:
	Checker(CheckerColor b_or_w) { _color = b_or_w; _post = CHECKER; } //Конструктор, принимает на вход цвет, и выставляет шашке ее должность


	void drawChecker(); //рисовка шашки
	void drawFrame();   //рисовка рамки для шашки (окружность рисуется)

	void CheckerSelect() { _selected == true ? _selected = false : _selected = true; } //выделяет шашку, если она выделена, то выделение убирается, и наоборот
	void CheckerSelectON() { _selected = true; } //выделяет шашку
	void CheckerSelectOFF() { _selected = false; } //убирает выделение шашки (особо эти 2 функции не пригодились)

	bool isSelected() { return _selected; } //bool, дает нам понять выделена данная шашка или не выделена

	CheckerColor getColor() { return _color; } //возвращает цвет шашки
	CheckerHierarchy getPost() { return _post; } //возвращает должность шашки

	void setPost(CheckerHierarchy post) { _post = post; } //устанавливает должность шашки (Используется только когда нужно сделать дамку)

//Приватные данные
private:
	bool _selected = false; //выделено ли
	CheckerColor _color;    //цвет
	CheckerHierarchy _post; //должность
};
