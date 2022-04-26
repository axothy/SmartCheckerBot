#pragma once
#include <utility>
#include <vector>

/*
* триветвистое дерево
* используется для нахождения оптимального пути
* множественного съедения шашек
*/
struct ternaryTree {
	//высота дерева
	int height = 0;

	//координаты хода откуда-куда
	std::pair<int, int> _from;
	std::pair<int, int> _to;

	//ветви дерева
	ternaryTree* first = nullptr;
	ternaryTree* second = nullptr;
	ternaryTree* third = nullptr;
	//указатель на предка
	ternaryTree* father = nullptr;

	//два конструктора
	ternaryTree(std::pair<int, int> from, std::pair<int, int> to) { _from = from; _to = to; }

	ternaryTree(std::pair<int, int> from, std::pair<int, int> to, ternaryTree* fatherTree) {
		father = fatherTree;
		_from = from; _to = to;
	}

	//функция повышающая счетчик высоты дерева
	void incrementHeight();

	//получение требуемых координат и их приращенияc
	int getXto() { return _to.first; }
	int getYto() { return _to.second; }

	int getDX() { return (_to.first - _from.first) / 2; }
	int getDY() { return (_to.second - _from.second) / 2; }

};

enum Ways {
	FIRST, SECOND, THIRD, FOURTH, FATHER
};

struct quadrupleTree {


	int height = 0;

	std::pair<int, int> _from;
	std::pair<int, int> _to;
	std::pair<int, int> eated;


	std::vector<quadrupleTree*> way;


	quadrupleTree(std::pair<int, int> from, std::pair<int, int> to) : way(5) { _from = from; _to = to; }

	quadrupleTree(std::pair<int, int> from, std::pair<int, int> to, quadrupleTree* fatherTree) : way(5) {
		way[FATHER] = fatherTree;
		_from = from; _to = to;
	}

	void incrementHeight();

	int getXto() { return _to.first; }
	int getYto() { return _to.second; }

	int getDX() { return (_to.first - _from.first) / 2; }
	int getDY() { return (_to.second - _from.second) / 2; }

};