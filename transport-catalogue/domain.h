#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>

struct Stop {
	std::string name;
	geo::Coordinates coordinates;
	size_t id = 0;
};

using cStopPtr = const Stop*;

struct Bus {
	std::string name;
	std::vector<cStopPtr> stops;
	bool is_ring; // является ли маршрут кольцевым
};

using cBusPtr = const Bus*;

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

//struct RoutingSettings {
//	int bus_wait_time_minutes = 0; // время ожидания автобуса на остановке, в минутах
//	int bus_velocity_km_h = 0; // скорость автобуса, в км/ч
//};