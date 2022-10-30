#pragma once

#include "geo.h"
#include "svg.h"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <deque>

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

namespace domain {
	// параметры передвижения на автобусе
	struct RoutingSettings {
		int bus_velocity_km_h = 0; // скорость автобуса, в км/ч
		int bus_wait_time_minutes = 0; // время ожидания автобуса на остановке, в минутах
	};

	// параметры изображения
	struct MapVisualizationSettings {
		double width_image = 0.0;
		double height_image = 0.0;
		double padding = 0.0; // отступ от краев
		double line_width = 0.0; // толщина линий автобусных маршрутов
		double stop_radius = 0.0; // радиус окружностей остановок
		int bus_label_font_size = 0; // размер текста маршрутов
		svg::Point bus_label_offset = { 0,0 }; // смещение названия маршрута относительно конечной остановки
		int stop_label_font_size = 0; // размер текста остановок
		svg::Point stop_label_offset = { 0,0 }; // смещение названия остановки
		svg::Color underlayer_color{}; // цвет подложки
		double underlayer_width = 0.0; // толщина подложки
		std::vector<svg::Color> color_palette{};
	};
}