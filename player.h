#pragma once
#include "playboard.h"
#include <utility>

class Playboard;
enum CheckerColor;
enum CheckerHierarchy;


class Player {
protected:
	Playboard* gameCheckers;
public:
	Player(Playboard* instance) { gameCheckers = instance; }

	virtual bool Turn(int x, int y, CheckerColor b_or_w);

};

class EasyBot : public Player {

public:
	EasyBot(Playboard* instance) : Player(instance) {}
	bool Turn(int x, int y, CheckerColor b_or_w);
};

class MediumBot : public Player {

public:
	MediumBot(Playboard* instance) : Player(instance) {}
	bool Turn(int x, int y, CheckerColor b_or_w);
};





