#include "request_handler.h"
#include "json_builder.h"

namespace handler {
	using namespace std;
	using namespace json;

	namespace get_inform {
		using namespace transport_catalogue;

		namespace detail {
			Array SortNameBus(const unordered_set<cBusPtr>& list_buses) {
				Array list_buses_arr;
				list_buses_arr.reserve(list_buses.size());

				for (const cBusPtr& ptr_bus : list_buses) {
					list_buses_arr.emplace_back(ptr_bus->name);
				}

				sort(list_buses_arr.begin(), list_buses_arr.end(),
					[](const Node& lhs, const Node& rhs) {
						return lhs.AsString() < rhs.AsString();
					});

				return list_buses_arr;
			}

			vector<cBusPtr> SortingBuses(const deque<Bus>& all_buses) {
				vector<cBusPtr> all_sorted_buses(all_buses.size());
				for (size_t i = 0; i < all_sorted_buses.size(); ++i) {
					all_sorted_buses[i] = &all_buses[i];
				}
				sort(all_sorted_buses.begin(), all_sorted_buses.end(),
					[](cBusPtr& lhs, cBusPtr& rhs) {
						return lhs->name < rhs->name;
					});
				return all_sorted_buses;
			}

		} // namespace detail

		Dict FormInformBus(const int id_query, const BusInfo& stat_bus) {
			if (stat_bus.is_empty) {
				return Builder{}.StartDict().
					Key("request_id"s).Value(id_query).
					Key("error_message"s).Value("not found"s).
					EndDict().Build().AsDict();
			}
			return Builder{}.StartDict().
				Key("request_id"s).Value(id_query).
				Key("stop_count"s).Value(static_cast<int>(stat_bus.number_stops)).
				Key("unique_stop_count"s).Value(static_cast<int>(stat_bus.number_unique_stops)).
				Key("route_length"s).Value(static_cast<int>(stat_bus.road_distance)).
				Key("curvature"s).Value(stat_bus.curvature).
				EndDict().Build().AsDict();
		}

		Dict FormListBuses(const int id_query, const StopBuses& stop_buses) {
			if (stop_buses.is_empty) {
				return Builder{}.StartDict().
					Key("request_id"s).Value(id_query).
					Key("error_message"s).Value("not found"s).
					EndDict().Build().AsDict();
			}
			return Builder{}.StartDict().
				Key("request_id"s).Value(id_query).
				Key("buses"s).Value(detail::SortNameBus(stop_buses.buses_stop)).
				EndDict().Build().AsDict();
		}

		Dict FormImageMapJSON(const int id_query, string&& text) {
			return Builder{}.StartDict().
				Key("request_id"s).Value(id_query).
				Key("map"s).Value(move(text)).
				EndDict().Build().AsDict();
		}

		Dict FormOptimalRoute(const int id_query, string_view stop_from, string_view stop_to,
                              const transport_router::TransportRouter& router) {
			const std::optional<const std::vector<const transport_router::RouteConditions*>> route = router.BuildRoute(stop_from, stop_to);
			if (!route.has_value()) {
				return Builder{}.StartDict().
					Key("request_id"s).Value(id_query).
					Key("error_message"s).Value("not found"s).
					EndDict().Build().AsDict();
			}
			double total_time = 0.0;
			Array items;
			items.reserve(route.value().size());

			for (const auto it : route.value()) {
				Dict dict = Builder{}.StartDict().
					Key("type"s).Value("Wait"s).
					Key("stop_name"s).Value(it->from->name).
					Key("time"s).Value(it->trip.waiting_time_minutes).
					EndDict().Build().AsDict();
				items.push_back(move(dict));
				total_time += it->trip.waiting_time_minutes;

				dict = Builder{}.StartDict().
					Key("type"s).Value("Bus"s).
					Key("bus"s).Value(it->bus->name).
					Key("span_count"s).Value(it->trip.stops_number).
					Key("time"s).Value(it->trip.travel_time_minutes).
					EndDict().Build().AsDict();
				items.push_back(move(dict));
				total_time += it->trip.travel_time_minutes;
			}
			return Builder{}.StartDict().
				Key("request_id"s).Value(id_query).
				Key("total_time"s).Value(total_time).
				Key("items"s).Value(items).
				EndDict().Build().AsDict();
		}

	} //namespace get_inform

	RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& transport_catalogue,
		renderer::MapRenderer& map_renderer, transport_router::TransportRouter& transpotr_router)
		: transport_catalogue_(transport_catalogue)
		, map_renderer_(map_renderer)
		, transpotr_router_(transpotr_router) {
	}

	svg::Document RequestHandler::RenderMap() const {
		return map_renderer_.GetImageSVG(get_inform::detail::SortingBuses(transport_catalogue_.GetAllBuses()));
	}

	json::Document RequestHandler::FormResponsesToRequests(const Array& print_queries) const {
		using namespace get_inform;
		Builder result_find;
		result_find.StartArray();

		for (const Node& query : print_queries) {
			Dict text_query = query.AsDict();
			if (text_query.at("type"s) == "Bus"s) {
				result_find.Value(FormInformBus(text_query.at("id"s).AsInt(),
                                                transport_catalogue_.GetBusInfo(text_query.at("name"s).AsString())));
			} else if (text_query.at("type"s) == "Stop"s) {
				result_find.Value(FormListBuses(text_query.at("id"s).AsInt(),
                                                transport_catalogue_.GetListBusesStop(text_query.at("name"s).AsString())));
			} else if (text_query.at("type"s) == "Route"s) {
				result_find.Value(FormOptimalRoute(text_query.at("id"s).AsInt(), text_query.at("from"s).AsString(),
                                                   text_query.at("to"s).AsString(), transpotr_router_));
			} else {
				ostringstream image;
				this->RenderMap().Render(image);
				result_find.Value(FormImageMapJSON(text_query.at("id"s).AsInt(), image.str()));
			}
		}
		result_find.EndArray();
		return Document(result_find.Build());
	}
} // namespace handler
