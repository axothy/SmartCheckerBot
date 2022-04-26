#include "ternaryTree.h"


void ternaryTree::incrementHeight() {

	/*������� ���������� ������ ����� ������������
	* ������� ���� ��� ����� ������ �����,
	* �� �������� ������ ������ �� 1
	* ��� ���������� ��� ������� ������
	*/
	if (first == nullptr && second == nullptr && third == nullptr) {


		height++;

		//��������� �� ������
		ternaryTree* helpFather = father;

		//���������� �� ���� �������, ������� ������, ���� �� ��������� �� ������ ������
		while (helpFather != nullptr) {
			helpFather->height++;
			helpFather = helpFather->father;

		}
	}
}


void quadrupleTree::incrementHeight() {
	if (way[FIRST] == nullptr && way[SECOND] == nullptr && way[THIRD] == nullptr && way[FOURTH] == nullptr) {
		height++;
		quadrupleTree* helpFather = way[FATHER];
		while (helpFather != nullptr) {
			helpFather->height++;
			helpFather = helpFather->way[FATHER];
		}
	}
}