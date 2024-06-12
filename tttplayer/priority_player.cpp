#include "priority_player.h"

template <typename T> T* expand(T* mass, int& number_in, const T& new_element) {
	if (mass == nullptr) {
		T* tmp = new T[++number_in]{ new_element };
		return tmp;
	}
	else {
		T* tmp = new T[number_in + 1];
		for (int i = 0; i < number_in; i++) { tmp[i] = mass[i]; }
		tmp[number_in++] = new_element;
		delete[] mass;
		return tmp;
	}
}

template <typename T> bool in(const T* mass, const int& number_in, const T& check) {
	for (int i = 0; i < number_in; i++) { if (mass[i] == check) { return true; } }
	return false;
}

static field_index_t rand_int(field_index_t min, field_index_t max) {
	return min + rand() % (max - min + 1);
}


/////////////////////////////////// Player /////////////////////////////////////////////////
Point PriorityPlayer::play(const GameView& game) {
	const Boundary& b = game.get_settings().field_size;
	Point result;
	int choice;
	/************************** First 3 moves in game *************************/
	if (game.get_state().number_of_moves == 0) { //First move (X)
		return Point{
			.x = (b.min.x + b.max.x) / 2,
			.y = (b.min.y + b.max.y) / 2
		};
	}
	else if (game.get_state().number_of_moves == 1) { //Second move (O)
		Point possible_move[] = {
			{_last_move.x - 1,_last_move.y - 1},
			{_last_move.x - 1,_last_move.y + 1},
			{_last_move.x + 1,_last_move.y + 1},
			{_last_move.x + 1,_last_move.y - 1}
		};
		do {
			choice = rand() % 4;
			result = possible_move[choice];
		} while (!b.is_within(result));
		return result;
	}
	else if (game.get_state().number_of_moves == 2) {  //Third move (X)
		if (b.get_height() % 2 == 1) {
			Mark other_mark = game.get_state().field->get_value(_last_move);
			Point c = { .x = (b.min.x + b.max.x) / 2, .y = (b.min.y + b.max.y) / 2 };
			Point env[] = { {c.x - 1,c.y - 1},{c.x - 1,c.y},{c.x - 1,c.y + 1},{c.x,c.y + 1},{c.x + 1,c.y + 1},{c.x + 1,c.y},{c.x + 1,c.y - 1},{c.x,c.y - 1} };
			for (int i = 0; i < 8; i += 2) {
				if (game.get_state().field->get_value(env[i]) == other_mark) {
					choice = rand() % 2;
					if (choice)
						return { env[i].x,-env[i].y };
					else
						return { -env[i].x,env[i].y };
				}
			}
			for (int i = 1; i < 8; i += 2) {
				if (game.get_state().field->get_value(env[i]) == other_mark) {
					choice = rand() % 2;
					if (env[i].x == c.x) {
						if (choice)
							return { env[i].x + 1,env[i].y };
						else
							return { env[i].x - 1,env[i].y };
					}
					else {
						if (choice)
							return { env[i].x,env[i].y + 1 };
						else
							return { env[i].x,env[i].y - 1 };
					}
				}
			}
			choice = rand() % 8;
			return env[choice];
		}
	}
	/****************************************************************************/
	Point dirs[] = { {-1,-1},{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1} };
	int priority = 100;
	int num_moves = 0;
	Point* possible_moves = nullptr;
	for (Point p = b.min; p.y <= b.max.y; ++p.y) {
		for (p.x = b.min.x; p.x <= b.max.x; ++p.x) {
			if (game.get_state().field->get_value(p) == Mark::None) {
				continue;
			}
			for (int d_i = 0; d_i < 8; d_i++) {
				char* line = nullptr;
				int len = 0;
				Point dir = dirs[d_i];
				line = line_count(game, p, line, len, dir);
				if (len == 2 || len == 5) {
					int loc[3] = { -1,-1,-1 };
					int prio = give_priority(line, loc);
					if (prio < priority) {
						priority = prio;
						num_moves = 0;
						delete[] possible_moves;
						possible_moves = nullptr;
					}
					if (prio == priority) {
						for (int i = 0; i < 3; i++) {
							if (loc[i] != -1) {
								Point move = { p.x + dir.x - loc[i] * dir.x, p.y + dir.y - loc[i] * dir.y };
								if (!in(possible_moves, num_moves, move)) {
									possible_moves = expand(possible_moves, num_moves, move);
								}
							}
						}
					}
				}
				delete[] line;
			}
		}
	}
	if (num_moves > 0) {
		choice = rand() % num_moves;
		result = possible_moves[choice];
	}
	else {
		do {
			result = {
				.x = rand_int(b.min.x, b.max.x),
				.y = rand_int(b.min.y, b.max.y),
			};
		} while (game.get_state().field->get_value(result) != Mark::None);
	}

	delete[] possible_moves;
	return result;
}

void PriorityPlayer::assign_mark(Mark player_mark) { _my_mark = player_mark; }

void PriorityPlayer::notify(const GameView& view, const Event& event) {
	if (event.get_type() == MoveEvent::TYPE) {
		auto& data = get_data<MoveEvent>(event);
		_last_move = data.point;
		return;
	}
}

char* PriorityPlayer::line_count(const GameView& game, const Point& point, char* line, int& len, const Point& dir) {
	const Boundary& b = game.get_settings().field_size;
	Mark mark = game.get_state().field->get_value(point);
	for (int i = 0; i < 5; i++) {
		Point check = { point.x + dir.x - i * dir.x, point.y + dir.y - i * dir.y };
		if (b.is_within(check)) {
			Mark cmark = game.get_state().field->get_value(check);
			if (cmark != mark && cmark != Mark::None) {
				return line;
			}
			char m;
			(cmark == Mark::None)
				? m = '_'
				: ((cmark == Mark::Cross)
					? m = 'X'
					: m = 'O');
			line = expand(line, len, m);
			line = expand(line, len, '\0');
			len--;
		}
		else { break; }
	}
	return line;
}

int PriorityPlayer::give_priority(const char* line, int* loc) {
	Mark line_mark;
	(line[1] == 'X') ? line_mark = Mark::Cross : line_mark = Mark::Zero;
	int prio = 100;
	/******************** Priotrity 0 or 1 *****************************/
	if (strcmp(line, "XXXX_") == 0 || strcmp(line, "OOOO_") == 0) {
		loc[0] = 4;
		(line_mark == _my_mark) ? (prio = 0) : (prio = 1);
		return prio;
	}
	else if (strcmp(line, "XXX_X") == 0 || strcmp(line, "OOO_O") == 0) {
		loc[0] = 3;
		(line_mark == _my_mark) ? (prio = 0) : (prio = 1);
		return prio;
	}
	else if (strcmp(line, "XX_XX") == 0 || strcmp(line, "OO_OO") == 0) {
		loc[0] = 2;
		(line_mark == _my_mark) ? (prio = 0) : (prio = 1);
		return prio;
	}
	/******************** Priotrity 2 or 3 *****************************/
	else if (strcmp(line, "_XXX_") == 0 || strcmp(line, "_OOO_") == 0) {
		loc[0] = 0;
		loc[1] = 4;
		(line_mark == _my_mark) ? (prio = 2) : (prio = 3);
		return prio;
	}
	else if (strcmp(line, "_X_XX") == 0 || strcmp(line, "_O_OO") == 0) {
		loc[0] = 0;
		loc[1] = 2;
		(line_mark == _my_mark) ? (prio = 2) : (prio = 3);
		return prio;
	}
	/******************** Another Priotrities *****************************/
	else if (strcmp(line, "XXX__") == 0 || strcmp(line, "OOO__") == 0) {
		loc[0] = 3;
		loc[1] = 4;
		(line_mark == _my_mark) ? (prio = 4) : (prio = 5);
		return prio;
	}
	else if (strcmp(line, "_XX__") == 0 || strcmp(line, "_OO__") == 0) {
		loc[0] = 0;
		loc[1] = 3;
		loc[2] = 4;
		(line_mark == _my_mark) ? (prio = 6) : (prio = 7);
		return prio;
	}
	else if (strcmp(line, "_X_X_") == 0 || strcmp(line, "_O_O_") == 0) {
		loc[0] = 0;
		loc[1] = 2;
		loc[2] = 4;
		(line_mark == _my_mark) ? (prio = 8) : (prio = 9);
		return prio;
	}
	else if (strcmp(line, "_X___") == 0 || strcmp(line, "_O___") == 0) {
		loc[0] = 0;
		loc[1] = 2;
		loc[2] = 3;
		(line_mark == _my_mark) ? (prio = 10) : (prio = 11);
		return prio;
	}
	else if (strcmp(line, "XX___") == 0 || strcmp(line, "OO___") == 0) {
		loc[0] = 2;
		loc[1] = 3;
		(line_mark == _my_mark) ? (prio = 12) : (prio = 13);
		return prio;
	}
	else if (strcmp(line, "_X") == 0 || strcmp(line, "_O") == 0) {
		loc[0] = 0;
		(line_mark == _my_mark) ? (prio = 14) : (prio = 15);
		return prio;
	}

	return prio;
}
