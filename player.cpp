#include "player.h"
#include <iostream>
#include <time.h>

bool Player::Turn(int x, int y, CheckerColor b_or_w) {
	return	gameCheckers->Select(x, y, b_or_w);
}

bool EasyBot::Turn(int x, int y, CheckerColor b_or_w) {
	for (x = 0; x < gameCheckers->getW(); x++) {
		for (y = 0; y < gameCheckers->getH(); y++) {
			gameCheckers->Select(x, y, b_or_w);
			if (gameCheckers->getPossibleMoves().size() != 0) {
				break;
			}
		}
		if (gameCheckers->getPossibleMoves().size() != 0) {
			break;
		}
	}

	if (x == 8 && y == 8 && gameCheckers->getPossibleMoves().size() == 0) {
		exit(0);
	}

	while (gameCheckers->Select(gameCheckers->getPossibleMoves()[0].getX(), gameCheckers->getPossibleMoves()[0].getY(), b_or_w) != true);
	return true;
}

bool MediumBot::Turn(int x, int y, CheckerColor b_or_w) {
	int rand_lim = 10000;
	int rand_ctr = 0;
	srand(time(NULL));
	do {
		x = rand() % (gameCheckers->getW() - 1);
		y = rand() % (gameCheckers->getH() - 1);
		gameCheckers->Select(x, y, b_or_w);
		rand_ctr++;
		if (rand_ctr == rand_lim) {
			for (x = 0; x < gameCheckers->getW(); x++) {
				for (y = 0; y < gameCheckers->getH(); y++) {
					gameCheckers->Select(x, y, b_or_w);
					if (gameCheckers->getPossibleMoves().size() != 0) {
						break;
					}
				}
				if (gameCheckers->getPossibleMoves().size() != 0) {
					break;
				}
				if (x == 7 && y == 7 && gameCheckers->getPossibleMoves().size() == 0) {
					exit(0);
				}
			}
		}
	} while (gameCheckers->getPossibleMoves().size() == 0);

	int position = rand() % (gameCheckers->getPossibleMoves().size());

	if (x == 8 && y == 8 && gameCheckers->getPossibleMoves().size() == 0) {
		exit(0);
	}

	while (gameCheckers->Select(gameCheckers->getPossibleMoves()[0].getX(), gameCheckers->getPossibleMoves()[0].getY(), b_or_w) != true) {
		position = rand() % (gameCheckers->getPossibleMoves().size());
	}
	return true;
}
