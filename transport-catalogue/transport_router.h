#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

namespace transpotr_router {
	using DictDistancesBetweenStops = std::unordered_map<std::string, std::unordered_map<std::string, int>>;

	namespace detail {
		const uint16_t meters_per_kilometer = 1000;
		const uint16_t minutes_per_hour = 60;
		double CalculateTimeTravelBetweenStopsInMinutes(size_t distance_m, int speed_km_h);
	} // namespace detail

	struct RoutingSettings {
		int bus_velocity_km_h = 0; // скорость автобуса, в км/ч
		int bus_wait_time_minutes = 0; // время ожидания автобуса на остановке, в минутах
	};

	struct RouteInform {
		int stops_number = 0;
		int waiting_time_minutes = 0; // время ожидания в минутах
		double travel_time_minutes = 0; // время движения в минутах
	};

	RouteInform operator+(const RouteInform& lhs, const RouteInform& rhs);
	bool operator<(const RouteInform& lhs, const RouteInform& rhs);
	bool operator>(const RouteInform& lhs, const RouteInform& rhs);

	struct RouteConditions {
		cStopPtr from = nullptr;
		cStopPtr to = nullptr;
		cBusPtr bus = nullptr;
		RouteInform trip = {};
	};

	class DistanceCalculator {
	public:
		explicit DistanceCalculator(const transport_catalogue::TransportCatalogue& transport_catalogue, cBusPtr bus_route);
		size_t GetDistanceBetween(size_t from, size_t to) const;

	private:
		std::vector<size_t> forward_distance_;
		std::vector<size_t> reverse_distance_;
	};

	class TransportRouter {
	public:
		TransportRouter(transport_catalogue::TransportCatalogue& transport_catalogue);
		void SetRoutingSettings(RoutingSettings&& routing_settings);
		void Initialization(RoutingSettings&& routing_settings);
		const std::optional<const std::vector<const RouteConditions*>> BuildRoute(std::string_view from, std::string_view to) const;

	private:
		void CreateGraph();

	private:
		transport_catalogue::TransportCatalogue& transport_catalogue_;

		RoutingSettings routing_settings_; // параметры маршрута

		std::unique_ptr<graph::DirectedWeightedGraph<RouteInform>> directed_weighted_graph_ptr_ = nullptr;
		std::unique_ptr<graph::Router<RouteInform>> router_ptr_ = nullptr;
		std::vector<RouteConditions> optimal_route_;
	};
} // namespace transpotr_router