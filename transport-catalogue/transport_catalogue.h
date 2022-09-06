#pragma once

#include "geo.h"

#include <iostream>
#include <string_view>
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <stack>

namespace tc {

	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> stops;
		bool is_ring; // �������� �� ������� ���������
	};

	struct BusInfo {
		bool is_empty = true;
		std::string_view name;
		size_t number_stops = 0;
		size_t number_unique_stops = 0;
		double geographical_distance = 0; // �������������� ����������
		size_t road_distance = 0; // �������� ����������
		double tortuosity = 1; // ������������
	};

	struct StopBuses {
		bool is_empty = true;
		std::string_view name;
		std::vector<const Bus*> buses_stop;
	};


	class TransportCatalogue
	{
	public:
		void AddStop(std::string&& name_stop, double latitude, double longitude);
		void AddDistanceBetweenStops(std::string_view name_first_stop, std::string_view name_second_stop, size_t distance_to_neighboring);
		void AddBus(std::string&& name, std::vector<std::string>&& stops, bool is_ring);

		const Stop* FindStop(std::string_view name_stop) const;
		const Bus* FindBus(std::string_view name_bus) const;
		BusInfo GetBusInfo(std::string_view name_bus) const;

		// ���� ����� ���������� ��� ��������������, �� ������� ����������� ����� ���������,
		// ����� �������� � "���", ����� ������������� ������� �� ����������� ������
		const StopBuses GetListBusesStop(std::string_view name_stop);

	private:

		struct HasherDistanceTable {
			size_t operator()(const std::pair<const Stop*, const Stop*> two_stops) const {
				size_t h_first = std::hash<const Stop*>{}(two_stops.first);
				size_t h_second = std::hash<const Stop*>{}(two_stops.second);
				return h_first + h_second;
			}
		};

		size_t FindUniqueStops(std::string_view name_bus) const;
		double CalculatingGeographicalDistance(std::string_view name_bus) const;
		size_t CalculatingRoadDistance(std::string_view name_bus) const;


		std::deque<Stop> stops_; // �������� ���� ������ �� ����������
		std::deque<Bus> buses_; // �������� ������ � ���������

		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;

		// ��������, ����������� ����� ���������. ������������ ��������� �� ���������, 
		// � �� string_view, �.�. string_view ���������� ������������
		// ����� �������, � "���" �������� �� �������
		std::unordered_map<const Stop*, std::unordered_set<const Bus*>> cache_buses_stops_;

		std::unordered_map<std::pair<Stop*, Stop*>, size_t, HasherDistanceTable> distance_between_stops_;
	};
}