#pragma once
#include "domain.h"
#include "graph.h"

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

namespace transport_catalogue {

	//using cStopPtr = const Stop*;
	//using cBusPtr = const Bus*;

	namespace detail {
		struct HasherDistanceTable {
			size_t operator()(const std::pair<cStopPtr, cStopPtr> two_stops) const {
				size_t prime_value = 17;
				size_t h_first = std::hash<cStopPtr>{}(two_stops.first);
				size_t h_second = std::hash<cStopPtr>{}(two_stops.second);
				return h_first + h_second * prime_value;
			}
		};
	}

	using DistancesBetweenStops = std::unordered_map<std::pair<cStopPtr, cStopPtr>, size_t, detail::HasherDistanceTable>;

	class TransportCatalogue {
	public:
		void AddStop(std::string&& name_stop, double latitude, double longitude, size_t id);
		void AddBus(std::string&& name, std::vector<std::string>&& stops, bool is_ring);
		void SetDistanceBetweenStops(cStopPtr first_stop, cStopPtr second_stop, size_t distance_to_neighboring);

		const Stop* FindStop(std::string_view name_stop) const;
		cBusPtr FindBus(std::string_view name_bus) const;
		BusInfo GetBusInfo(std::string_view name_bus) const;
		const StopBuses GetListBusesStop(std::string_view name_stop);
		std::vector<cBusPtr> GetAllBusesSorted() const;
		const DistancesBetweenStops& GetDistancesBetweenAllStops() const;
		const std::unordered_map<std::string_view, Stop*>& GetAllStops() const;
		size_t GetDistanceBetweenTwoStops(cStopPtr from, cStopPtr to) const;

	private:
		void FillListsBusesStop(cBusPtr bus);

		size_t GetNumUniqueStops(cBusPtr bus) const;

		double CalculatingGeographicalDistance(cBusPtr bus) const;
		size_t CalculatingRoadDistance(cBusPtr bus) const;

	private:
		std::deque<Stop> stops_; // хранятся сами данные об остановках
		std::deque<Bus> buses_; // хранятся данные о маршрутах

		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;

		// автобусы, проезжающие через остановку. Используется указатель на остановку, 
		// а не string_view, т.к. string_view перестанет существовать
		// после запроса, а "кэш" хранится на сервере
		std::unordered_map<cStopPtr, std::unordered_set<cBusPtr>> cache_buses_stops_;

		DistancesBetweenStops distance_between_stops_;
	};
}