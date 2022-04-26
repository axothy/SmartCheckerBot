#pragma once
#include <utility>
#include <vector>

/*
* ������������ ������
* ������������ ��� ���������� ������������ ����
* �������������� �������� �����
*/
struct ternaryTree {
	//������ ������
	int height = 0;

	//���������� ���� ������-����
	std::pair<int, int> _from;
	std::pair<int, int> _to;

	//����� ������
	ternaryTree* first = nullptr;
	ternaryTree* second = nullptr;
	ternaryTree* third = nullptr;
	//��������� �� ������
	ternaryTree* father = nullptr;

	//��� ������������
	ternaryTree(std::pair<int, int> from, std::pair<int, int> to) { _from = from; _to = to; }

	ternaryTree(std::pair<int, int> from, std::pair<int, int> to, ternaryTree* fatherTree) {
		father = fatherTree;
		_from = from; _to = to;
	}

	//������� ���������� ������� ������ ������
	void incrementHeight();

	//��������� ��������� ��������� � �� ����������c
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