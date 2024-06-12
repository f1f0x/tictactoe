#pragma once
#include <ostream>
#include "player.h"

class PriorityPlayer : public Player {
	std::string _name;
	Point _last_move;
	Mark _my_mark;

	char* line_count(const GameView& game, const Point& point, char* line, int& len, const Point& dir);
	int give_priority(const char* line, int* loc);

public:
	PriorityPlayer(const std::string& name) : _name(name) {}
	std::string get_name() const override { return _name; }
	Point play(const GameView& game) override;
	void assign_mark(Mark player_mark) override;
	void notify(const GameView&, const Event&) override;
};