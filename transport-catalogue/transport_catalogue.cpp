#include "transport_catalogue.h"

using namespace std;
using namespace geo;

namespace tc {

	void TransportCatalogue::AddStop(std::string&& name_stop, double latitude, double longitude) {
		stops_.push_back({ name_stop, {latitude,  longitude} });
		stopname_to_stop_[stops_.back().name] = &stops_.back();
	}

	void TransportCatalogue::AddDistanceBetweenStops(string_view name_first_stop, string_view name_second_stop,
		size_t distance_to_neighboring) {
		distance_between_stops_[{stopname_to_stop_.find(name_first_stop)->second,
			stopname_to_stop_.find(name_second_stop)->second }] = distance_to_neighboring;
	}

	void TransportCatalogue::AddBus(string&& name, vector<string>&& stops, bool is_ring) {
		vector<Stop*> adr_stops(stops.size());
		for (size_t i = 0; i < adr_stops.size(); ++i) {
			adr_stops[i] = stopname_to_stop_.find(stops[i])->second;
		}
		buses_.push_back({ move(name), move(adr_stops), is_ring });
		busname_to_bus_[buses_.back().name] = &buses_.back();
	}

	const Stop* TransportCatalogue::FindStop(string_view name_stop) const {
		return stopname_to_stop_.count(name_stop) ? stopname_to_stop_.at(name_stop) : nullptr;
	}

	const Bus* TransportCatalogue::FindBus(std::string_view name_bus) const {
		return busname_to_bus_.count(name_bus) ? busname_to_bus_.at(name_bus) : nullptr;
	}

	BusInfo TransportCatalogue::GetBusInfo(string_view name_bus) const {
		if (!busname_to_bus_.count(name_bus)) {
			return { true, name_bus };
		}
		size_t number_stops = busname_to_bus_.at(name_bus)->is_ring ? busname_to_bus_.at(name_bus)->stops.size() : (busname_to_bus_.at(name_bus)->stops.size() * 2) - 1;
		size_t number_unique_stops = FindUniqueStops(name_bus);
		double geographical_length = CalculatingGeographicalDistance(name_bus);
		size_t road_distance = CalculatingRoadDistance(name_bus);
		double tortuosity = static_cast<double>(road_distance) / geographical_length;
		return { false, name_bus, number_stops, number_unique_stops, geographical_length, road_distance,  tortuosity };
	}

	size_t TransportCatalogue::FindUniqueStops(string_view name_bus) const {
		// уникальные - названи€ которых внутри Ё“ќ√ќ маршрута не повтор€ютс€
		unordered_set<Stop*> unique_stops;
		for (Stop* const& stop : busname_to_bus_.at(name_bus)->stops) {
			unique_stops.insert(stop);
		}
		return unique_stops.size();
	}

	double TransportCatalogue::CalculatingGeographicalDistance(string_view name_bus) const {
		double route_length = 0;
		vector<Stop*> stops = busname_to_bus_.at(name_bus)->stops;
		for (size_t i = 0; i < stops.size() - 1; ++i) {
			route_length += ComputeDistance(stops[i]->coordinates, stops[i + 1]->coordinates);
		}
		return busname_to_bus_.at(name_bus)->is_ring ? route_length : route_length * 2;
	}

	size_t TransportCatalogue::CalculatingRoadDistance(string_view name_bus) const {
		size_t route_length = 0;
		vector<Stop*> stops = busname_to_bus_.at(name_bus)->stops;

		for (size_t i = 0; i < stops.size() - 1; ++i) {
			if (distance_between_stops_.count({ stops[i], stops[i + 1] })) {
				route_length += distance_between_stops_.at({ stops[i], stops[i + 1] });
			} else {
				route_length += distance_between_stops_.at({ stops[i + 1], stops[i] });
			}
		}

		if (!busname_to_bus_.at(name_bus)->is_ring) {
			for (size_t i = stops.size() - 1; i > 0; --i) {
				if (distance_between_stops_.count({ stops[i], stops[i - 1] })) {
					route_length += distance_between_stops_.at({ stops[i], stops[i - 1] });
				} else {
					route_length += distance_between_stops_.at({ stops[i - 1], stops[i] });
				}
			}
		}

		return route_length;
	}

	const StopBuses TransportCatalogue::GetListBusesStop(string_view name_stop) {
		const Stop* stop = FindStop(name_stop);
		if (stop == nullptr) {
			return { true, name_stop, {} };
		}
		if (!cache_buses_stops_.count(stop)) {
			unordered_set<const Bus*> stop_buses;
			// идем по всем автобусам, чтобы узнать, через какие остановки проходит каждый
			for (const Bus& bus : buses_) {
				// провер€ем каждую остановку автобуса пока не найдетс€ нужна€. 
				// ≈сли есть подход€ща€, то добавл€ем
				// указатель на автобус в контейнер автобусов искомой остановки
				for (const Stop* const& stop : bus.stops) {
					if (stop->name == name_stop) {
						stop_buses.insert(&bus);
						break;
					}
				}
			}
			cache_buses_stops_.insert({ stop, stop_buses });
		}
		vector <const Bus*> stop_buses_vect(cache_buses_stops_.find(stop)->second.size());
		copy(cache_buses_stops_.find(stop)->second.begin(), cache_buses_stops_.find(stop)->second.end(), stop_buses_vect.begin());
		sort(stop_buses_vect.begin(), stop_buses_vect.end(),
			[](const Bus*& lhs, const Bus*& rhs) {
				return lhs->name < rhs->name;
			});
		return { false, stop->name, stop_buses_vect };
	}
}