#include "transport_router.h"

#include <iostream>

using namespace std;

namespace transpotr_router {

	namespace detail {
		double CalculateTimeTravelBetweenStopsInMinutes(size_t distance_m, int speed_km_h) {
			double time_travel_in_hours = (static_cast<double>(distance_m) / meters_per_kilometer) / speed_km_h;
			return time_travel_in_hours * minutes_per_hour;
		}
	} // namespace detail

	RouteInform operator+(const RouteInform& lhs, const RouteInform& rhs) {
		return { lhs.stops_number + rhs.stops_number,
				lhs.waiting_time_minutes + rhs.waiting_time_minutes,
				lhs.travel_time_minutes + rhs.travel_time_minutes 
		};
	}

	bool operator<(const RouteInform& lhs, const RouteInform& rhs) {
		return (lhs.waiting_time_minutes + lhs.travel_time_minutes) < (rhs.waiting_time_minutes + rhs.travel_time_minutes);
	}

	bool operator>(const RouteInform& lhs, const RouteInform& rhs) {
		return (lhs.waiting_time_minutes + lhs.travel_time_minutes) > (rhs.waiting_time_minutes + rhs.travel_time_minutes);
	}


	DistanceCalculator::DistanceCalculator(const transport_catalogue::TransportCatalogue& transport_catalogue, cBusPtr bus_route) 
		: forward_distance_(bus_route->stops.size())
		, reverse_distance_(bus_route->stops.size()) {

		size_t forward = 0;
		size_t reverse = 0;
		if (bus_route->stops.empty()) {
			throw out_of_range("Error calculate distance between stops bus. List stops this bus is empty"s);
		}

		for (size_t i = 0; i < bus_route->stops.size() - 1; ++i) {
			forward += transport_catalogue.GetDistanceBetweenTwoStops(bus_route->stops[i], bus_route->stops[i + 1]);
			forward_distance_[i + 1] = forward;

			if (!bus_route->is_ring) {
				reverse += transport_catalogue.GetDistanceBetweenTwoStops(bus_route->stops[i + 1], bus_route->stops[i]);
				reverse_distance_[i + 1] = reverse;
			}
		}
	}

	size_t DistanceCalculator::GetDistanceBetween(size_t from, size_t to) const {
		if (from < to) {
			return forward_distance_[to] - forward_distance_[from];
		} else {
			return reverse_distance_[from] - reverse_distance_[to];
		}
	}


	TransportRouter::TransportRouter(transport_catalogue::TransportCatalogue& transport_catalogue) 
		: transport_catalogue_(transport_catalogue) {
	}

	void TransportRouter::SetRoutingSettings(RoutingSettings&& routing_settings) {
		routing_settings_ = move(routing_settings);
	}

	void TransportRouter::CreateGraph() {
		using namespace transport_catalogue;
		using namespace graph;

		directed_weighted_graph_ptr_ = make_unique<graph::DirectedWeightedGraph<RouteInform>>(transport_catalogue_.GetAllStops().size());
		
		const vector<cBusPtr>& all_buses = transport_catalogue_.GetAllBusesSorted(); // тут исправить, сделать метод без сортировки
		
		for (const cBusPtr& bus : all_buses) {
			const vector<cStopPtr>& stops_this_bus = bus->stops;
			if (stops_this_bus.empty()) {
				throw out_of_range("Error calculate distance between stops bus. List stops this bus is empty"s);
			}

			DistanceCalculator distance_calculator(transport_catalogue_, bus);
			
			for (size_t i = 0; i < stops_this_bus.size() - 1; ++i) {
				for (size_t j = i + 1; j < stops_this_bus.size(); ++j) {

					double time_travel_minutes = detail::CalculateTimeTravelBetweenStopsInMinutes(distance_calculator.GetDistanceBetween(i, j), routing_settings_.bus_velocity_km_h);
					RouteInform route_inform{ static_cast<int>(j - i), routing_settings_.bus_wait_time_minutes, time_travel_minutes };
					RouteConditions route_conditions{ stops_this_bus[i], stops_this_bus[j], bus, route_inform };

					directed_weighted_graph_ptr_->AddEdge({ route_conditions.from->id, route_conditions.to->id, route_inform });
					optimal_route_.emplace_back(move(route_conditions));

					if (!bus->is_ring) {
						double time_travel_minutes = detail::CalculateTimeTravelBetweenStopsInMinutes(distance_calculator.GetDistanceBetween(j, i), routing_settings_.bus_velocity_km_h);
						RouteInform route_inform{ static_cast<int>(j - i), routing_settings_.bus_wait_time_minutes, time_travel_minutes };
						RouteConditions route_conditions{ stops_this_bus[i], stops_this_bus[j], bus, route_inform };

						directed_weighted_graph_ptr_->AddEdge({ route_conditions.to->id, route_conditions.from->id, route_inform });
						optimal_route_.emplace_back(move(route_conditions));
					}
				}
			}
		}
	}

	void TransportRouter::Initialization(RoutingSettings&& routing_settings) {
		this->SetRoutingSettings(move(routing_settings));
		this->CreateGraph();
		if (directed_weighted_graph_ptr_.get() == nullptr) {
			throw logic_error("Attempt to create a router with an empty graph"s);
		}
		router_ptr_ = std::make_unique<graph::Router<RouteInform>>(*directed_weighted_graph_ptr_);
	}

	const optional<const vector<const RouteConditions*>> TransportRouter::BuildRoute(string_view from, string_view to) const {
		if (transport_catalogue_.GetAllStops().empty()) {
			return nullopt;
		}

		cStopPtr stop_from = transport_catalogue_.FindStop(from);
		cStopPtr stop_to = transport_catalogue_.FindStop(to);
		vector<const RouteConditions*> result;

		if (stop_from == stop_to) {
			return result;
		}

		optional<graph::Router<RouteInform>::RouteInfo> route = router_ptr_->BuildRoute(stop_from->id, stop_to->id);
		if (!route.has_value()) {
			return nullopt;
		}
		
		result.reserve(route.value().edges.size());
		for (const auto& edge : route.value().edges) {
			result.emplace_back(&optimal_route_.at(edge));
		}

		return result;
	}

} // transpotr_router