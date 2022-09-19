#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>

struct Stop {
	std::string name;
	geo::Coordinates coordinates;
};

struct Bus {
	std::string name;
	std::vector<Stop*> stops;
	bool is_ring; // является ли маршрут кольцевым
};

struct BusInfo {
	bool is_empty = true;
	std::string_view name;
	size_t number_stops = 0;
	size_t number_unique_stops = 0;
	double geographical_distance = 0; // географическое расстояние
	size_t road_distance = 0; // дорожное расстояние
	double curvature = 1; // извилистость
};

struct StopBuses {
	bool is_empty = true;
	std::string_view name;
	std::unordered_set<const Bus*>& buses_stop;
};