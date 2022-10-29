#include "json_reader.h"

using namespace std;
using namespace json;
using namespace renderer;

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

		svg::Color GetColor(const json::Array& color) {
			using namespace svg;
			if (color.size() == 1) {
				return color.front().AsString();
			} else if (color.size() == 3) {
				return Rgb{ static_cast<uint8_t>(color[0].AsInt()), static_cast<uint8_t>(color[1].AsInt()), static_cast<uint8_t>(color[2].AsInt()) };
			} else {
				return Rgba{ static_cast<uint8_t>(color[0].AsInt()), static_cast<uint8_t>(color[1].AsInt()), static_cast<uint8_t>(color[2].AsInt()), color[3].AsDouble() };
			}
		}

		renderer::MapVisualizationSettings GetSettings(const json::Dict& settings) {
			renderer::MapVisualizationSettings visual_settings;

			visual_settings.width_image = settings.at("width"s).AsDouble();
			visual_settings.height_image = settings.at("height"s).AsDouble();
			visual_settings.padding = settings.at("padding"s).AsDouble();
			visual_settings.line_width = settings.at("line_width"s).AsDouble();
			visual_settings.stop_radius = settings.at("stop_radius"s).AsDouble();
			visual_settings.bus_label_font_size = settings.at("bus_label_font_size"s).AsInt();
			Array bus_label_offset = settings.at("bus_label_offset"s).AsArray();
			visual_settings.bus_label_offset = { bus_label_offset.front().AsDouble(), bus_label_offset.back().AsDouble() };
			visual_settings.stop_label_font_size = settings.at("stop_label_font_size"s).AsInt();
			Array stop_label_offset = settings.at("stop_label_offset"s).AsArray();
			visual_settings.stop_label_offset = { stop_label_offset.front().AsDouble(), stop_label_offset.back().AsDouble() };
			if (settings.at("underlayer_color"s).IsArray()) {
				visual_settings.underlayer_color = GetColor(settings.at("underlayer_color"s).AsArray());
			} else {
				visual_settings.underlayer_color = settings.at("underlayer_color"s).AsString();
			}
			visual_settings.underlayer_width = settings.at("underlayer_width"s).AsDouble();

			Array color_palette_JSON = settings.at("color_palette"s).AsArray();
			vector<svg::Color> color_palette(color_palette_JSON.size());
			for (size_t i = 0; i < color_palette.size(); ++i) {
				if (color_palette_JSON[i].IsString()) {
					color_palette[i] = color_palette_JSON[i].AsString();
				} else {
					color_palette[i] = GetColor(color_palette_JSON[i].AsArray());
				}
			}
			visual_settings.color_palette = move(color_palette);
			return visual_settings;
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

	Dict FormOptimalRoute(const int id_query, string_view stop_from, string_view stop_to, const transpotr_router::TransportRouter& router) {
		const std::optional<const std::vector<const transpotr_router::RouteConditions*>> route = router.BuildRoute(stop_from, stop_to);
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

namespace input {

	json::Document LoadJSON(std::istream& input_JSON) {
		return Load(input_JSON);
	}

	JsonReader::JsonReader(transport_catalogue::TransportCatalogue& transport_catalogue, transpotr_router::TransportRouter& transpotr_router)
		: transport_catalogue_(transport_catalogue)
		, transpotr_router_(transpotr_router) {
	}

	void JsonReader::AddStop(const Dict& stop, DictDistancesBetweenStops& distances_between_stops) {
		static size_t id = 0;
		string name_this_stop = stop.at("name"s).AsString();
		const json::Dict& near_stops = stop.at("road_distances"s).AsDict();
		for (const auto& [name_other_stop, distance] : near_stops) {
			distances_between_stops[name_this_stop][name_other_stop] = distance.AsInt();
		}
		transport_catalogue_.AddStop(move(name_this_stop), stop.at("latitude"s).AsDouble(), stop.at("longitude"s).AsDouble(), id++);
	}

	void JsonReader::FillDistanceStops(DictDistancesBetweenStops&& distances_between_stops) {
		for (const auto& [name_this_stop, inform_other_stop] : distances_between_stops) {
			for (const auto& [name_other_stop, distance] : inform_other_stop) {
				transport_catalogue_.SetDistanceBetweenStops(transport_catalogue_.FindStop(name_this_stop), transport_catalogue_.FindStop(name_other_stop), distance);
			}
		}
	}

	void JsonReader::FillBuses(const Array& buses_queries) {
		for (const Node& node : buses_queries) {
			json::Dict inform_bus = node.AsDict();
			string name_this_bus = inform_bus.at("name"s).AsString();
			json::Array stops_bus = inform_bus.at("stops"s).AsArray();
			vector<string> stops(stops_bus.size());
			for (size_t i = 0; i < stops.size(); ++i) {
				stops[i] = stops_bus[i].AsString();
			}
			transport_catalogue_.AddBus(move(name_this_bus), move(stops), inform_bus.at("is_roundtrip"s).AsBool());
		}
	}

	void JsonReader::FillCatalogue(const Array& queries) {
		json::Array buses_queries;
		DictDistancesBetweenStops distances_between_stops;
		for (const Node& query : queries) {
			// согласно условию, в запросе всегда присутствует ключ "type".
			// Поэтому не боимся упасть на .at() и отсутствует .count()
			assert(query.AsDict().count("type"s));
			if (query.AsDict().at("type"s) == "Bus"s) {
				buses_queries.emplace_back(move(query));
			} else {
				AddStop(move(query.AsDict()), distances_between_stops);
			}
		}
		FillDistanceStops(move(distances_between_stops));
		FillBuses(move(buses_queries));
	}

	void JsonReader::GetImageMap(const MapRenderer& renderer, ostream& out) {
		RequestHandler request_handler(transport_catalogue_, renderer);
		request_handler.RenderMap().Render(out);
		//ofstream fout("OutSVG.svg");
		//request_handler.RenderMap().Render(fout);
	}

	Document JsonReader::FormResponsesToRequests(const MapRenderer& renderer, const Array& print_queries) {
		using namespace get_inform;
		Builder result_find;
		result_find.StartArray();

		for (const Node& query : print_queries) {
			Dict text_query = query.AsDict();
			if (text_query.at("type"s) == "Bus"s) {
				result_find.Value(FormInformBus(text_query.at("id"s).AsInt(), transport_catalogue_.GetBusInfo(text_query.at("name"s).AsString())));
			} else if (text_query.at("type"s) == "Stop"s) {
				result_find.Value(FormListBuses(text_query.at("id"s).AsInt(), transport_catalogue_.GetListBusesStop(text_query.at("name"s).AsString())));
			} else if (text_query.at("type"s) == "Route"s) {
				result_find.Value(FormOptimalRoute(text_query.at("id"s).AsInt(), text_query.at("from"s).AsString(), text_query.at("to"s).AsString(), transpotr_router_));
			} else {
				ostringstream image;
				GetImageMap(renderer, image);
				result_find.Value(FormImageMapJSON(text_query.at("id"s).AsInt(), image.str()));
			}
		}
		result_find.EndArray();
		return Document(result_find.Build());
	}

	void JsonReader::ProcessingQueries(Document&& document_JSON, ostream& out) {
		Array data_catalogue = document_JSON.GetRoot().AsDict().at("base_requests"s).AsArray();
		FillCatalogue(move(data_catalogue));

		// новый запрос
		Dict routing_settings = document_JSON.GetRoot().AsDict().at("routing_settings"s).AsDict();
		transpotr_router_.Initialization({ routing_settings.at("bus_velocity"s).AsInt(), routing_settings.at("bus_wait_time"s).AsInt() });

		Dict render_settings = document_JSON.GetRoot().AsDict().at("render_settings"s).AsDict();
		MapRenderer renderer(get_inform::detail::GetSettings(move(render_settings)));

		Array requests = document_JSON.GetRoot().AsDict().at("stat_requests"s).AsArray();
		Document responses_to_queries = FormResponsesToRequests(renderer, move(requests));
		Print(responses_to_queries, out);
	}
}