#include "transport_catalogue.h"

using namespace std;
using namespace geo;

namespace transport_catalogue {

	void TransportCatalogue::AddStop(std::string&& name_stop, double latitude, double longitude) {
		stops_.push_back({ name_stop, {latitude,  longitude} });
		stopname_to_stop_[stops_.back().name] = &stops_.back();
	}

	void TransportCatalogue::SetDistanceBetweenStops(cStopPtr first_stop, cStopPtr second_stop,
		size_t distance_to_neighboring) {
		distance_between_stops_[{first_stop, second_stop }] = distance_to_neighboring;
	}

	void TransportCatalogue::FillListsBusesStop(cBusPtr bus) {
		for (cStopPtr stop : bus->stops) {
			cache_buses_stops_[stop].insert(bus);
		}
	}

	void TransportCatalogue::AddBus(string&& name, vector<string>&& stops, bool is_ring) {
		vector<Stop*> adr_stops(stops.size());
		for (size_t i = 0; i < adr_stops.size(); ++i) {
			adr_stops[i] = stopname_to_stop_.find(stops[i])->second;
		}
		buses_.push_back({ move(name), move(adr_stops), is_ring });
		busname_to_bus_[buses_.back().name] = &buses_.back();
		FillListsBusesStop(&buses_.back());
	}

	cStopPtr TransportCatalogue::FindStop(string_view name_stop) const {
		return stopname_to_stop_.count(name_stop) ? stopname_to_stop_.at(name_stop) : nullptr;
	}

	cBusPtr TransportCatalogue::FindBus(std::string_view name_bus) const {
		return busname_to_bus_.count(name_bus) ? busname_to_bus_.at(name_bus) : nullptr;
	}

	BusInfo TransportCatalogue::GetBusInfo(std::string_view name_bus) const {
		if (!busname_to_bus_.count(name_bus)) {
			return { true, name_bus };
		}
		cBusPtr bus = busname_to_bus_.at(name_bus);
		size_t number_stops = bus->stops.size();
		number_stops = bus->is_ring ? number_stops : (number_stops * 2) - 1;
		size_t number_unique_stops = GetNumUniqueStops(bus);
		double geographical_length = CalculatingGeographicalDistance(bus);
		size_t road_distance = CalculatingRoadDistance(bus);
		double tortuosity = static_cast<double>(road_distance) / geographical_length;
		return { false, bus->name, number_stops, number_unique_stops, geographical_length, road_distance,  tortuosity };
	}

	size_t TransportCatalogue::GetNumUniqueStops(cBusPtr bus) const {
		// уникальные - названи€ которых внутри Ё“ќ√ќ маршрута не повтор€ютс€
		unordered_set<Stop*> unique_stops;
		for (Stop* const& stop : bus->stops) {
			unique_stops.insert(stop);
		}
		return unique_stops.size();
	}

	double TransportCatalogue::CalculatingGeographicalDistance(cBusPtr bus) const {
		double route_length = 0;
		vector<Stop*> stops = bus->stops;
		for (size_t i = 0; i < stops.size() - 1; ++i) {
			route_length += ComputeDistance(stops[i]->coordinates, stops[i + 1]->coordinates);
		}
		return bus->is_ring ? route_length : route_length * 2;
	}

	size_t TransportCatalogue::GetDistanceBetweenTwoStops(cStopPtr first, cStopPtr second) const {
		return distance_between_stops_.count({ first, second }) ? distance_between_stops_.at({ first, second }) : distance_between_stops_.at({ second, first });
	}

	size_t TransportCatalogue::CalculatingRoadDistance(cBusPtr bus) const {
		size_t route_length = 0;
		vector<Stop*> stops = bus->stops;

		for (size_t i = 0; i < stops.size() - 1; ++i) {
			route_length += GetDistanceBetweenTwoStops(stops[i], stops[i + 1]);
		}

		if (!bus->is_ring) {
			for (size_t i = stops.size() - 1; i > 0; --i) {
				route_length += GetDistanceBetweenTwoStops(stops[i], stops[i - 1]);
			}
		}

		return route_length;
	}

	const StopBuses TransportCatalogue::GetListBusesStop(string_view name_stop) {
		cStopPtr stop = FindStop(name_stop);
		static unordered_set<const Bus*> empty;

		if (stop == nullptr) {
			return { true, name_stop, empty };
		}
		if (!cache_buses_stops_.count(stop)) {
			return { false, stop->name,  empty };
		}
		return { false, stop->name, cache_buses_stops_.at(stop) };
	}

	vector<cBusPtr> TransportCatalogue::GetAllBusesSorted() const {
		vector<cBusPtr> all_sorted_buses(buses_.size());
		for (size_t i = 0; i < all_sorted_buses.size(); ++i) {
			all_sorted_buses[i] = &buses_[i];
		}
		sort(all_sorted_buses.begin(), all_sorted_buses.end(),
			[](cBusPtr& lhs, cBusPtr& rhs) {
				return lhs->name < rhs->name;
			});
		return all_sorted_buses;
	}

}