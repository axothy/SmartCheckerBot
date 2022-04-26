#include "ternaryTree.h"


void ternaryTree::incrementHeight() {

	/*функция вызывается только после конструктора
	* поэтому если три ветки дерева пусты,
	* то повышаем высоту дерева на 1
	* это происходит для каждого предка
	*/
	if (first == nullptr && second == nullptr && third == nullptr) {


		height++;

		//указатель на предка
		ternaryTree* helpFather = father;

		//проходимся по всем предкам, повышая высоту, пока не наткнемся на корень дерева
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