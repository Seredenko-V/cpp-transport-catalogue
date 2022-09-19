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

		svg::Color GetColor(json::Array&& color) {
			using namespace svg;
			if (color.size() == 1) {
				return color.front().AsString();
			} else if (color.size() == 3) {
				return Rgb{ static_cast<uint8_t>(color[0].AsInt()), static_cast<uint8_t>(color[1].AsInt()), static_cast<uint8_t>(color[2].AsInt()) };
			} else {
				return Rgba{ static_cast<uint8_t>(color[0].AsInt()), static_cast<uint8_t>(color[1].AsInt()), static_cast<uint8_t>(color[2].AsInt()), color[3].AsDouble() };
			}
		}

		renderer::MapVisualizationSettings GetSettings(json::Dict&& settings) {
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

	Dict GetInformBus(const int id_query, const BusInfo& stat_bus) {
		if (stat_bus.is_empty) {
			return { {"request_id"s, id_query}, {"error_message"s, "not found"s} };
		}
		return { {"request_id"s, id_query}, 
				 {"stop_count"s, static_cast<int>(stat_bus.number_stops)}, 
				 {"unique_stop_count"s, static_cast<int>(stat_bus.number_unique_stops)},
				 {"route_length"s, static_cast<int>(stat_bus.road_distance)},
				 {"curvature"s, stat_bus.curvature}
		};
	}

	Dict GetListBuses(const int id_query, const StopBuses& stop_buses) {
		if (stop_buses.is_empty) {
			return { {"request_id"s, id_query}, {"error_message"s, "not found"s} };
		}
		return { {"request_id"s, id_query}, {"buses"s, detail::SortNameBus(stop_buses.buses_stop)} };
	}

	Dict GetImageMapJSON(const int id_query, std::string&& text) {
		return { {"request_id"s, id_query}, {"map"s, text} };
	}

} //namespace get_inform

namespace input {

	json::Document LoadJSON(std::istream& input_JSON) {
		return Load(input_JSON);
	}

	JsonReader::JsonReader(transport_catalogue::TransportCatalogue& transport_catalogue)
		: transport_catalogue_(transport_catalogue) {
	}

	void JsonReader::AddStop(Dict&& stop, DictDistancesBetweenStops& distances_between_stops) {
		string name_this_stop = stop.at("name"s).AsString();
		const json::Dict& near_stops = stop.at("road_distances"s).AsMap();
		for (const auto& [name_other_stop, distance] : near_stops) {
			distances_between_stops[name_this_stop][name_other_stop] = distance.AsInt();
		}
		transport_catalogue_.AddStop(move(name_this_stop), stop.at("latitude"s).AsDouble(), stop.at("longitude"s).AsDouble());
	}

	void JsonReader::FillDistanceStops(DictDistancesBetweenStops&& distances_between_stops) {
		for (const auto& [name_this_stop, inform_other_stop] : distances_between_stops) {
			for (const auto& [name_other_stop, distance] : inform_other_stop) {
				transport_catalogue_.SetDistanceBetweenStops(transport_catalogue_.FindStop(name_this_stop), transport_catalogue_.FindStop(name_other_stop), distance);
			}
		}
	}

	void JsonReader::FillBuses(Array&& buses_queries) {
		for (const Node& node : buses_queries) {
			json::Dict inform_bus = node.AsMap();
			string name_this_bus = inform_bus.at("name"s).AsString();
			json::Array stops_bus = inform_bus.at("stops"s).AsArray();
			vector<string> stops(stops_bus.size());
			for (size_t i = 0; i < stops.size(); ++i) {
				stops[i] = stops_bus[i].AsString();
			}
			transport_catalogue_.AddBus(move(name_this_bus), move(stops), inform_bus.at("is_roundtrip"s).AsBool());
		}
	}

	void JsonReader::FillCatalogue(Array&& queries) {
		json::Array buses_queries;
		DictDistancesBetweenStops distances_between_stops;
		for (const Node& query : queries) {
			// согласно условию, в запросе всегда присутствует ключ "type".
			// Поэтому не боимся упасть на .at() и отсутствует .count()
			assert(query.AsMap().count("type"s));
			if (query.AsMap().at("type"s) == "Bus"s) {
				buses_queries.emplace_back(move(query));
			} else {
				AddStop(move(query.AsMap()), distances_between_stops);
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

	void JsonReader::GetInformation(const MapRenderer& renderer, Array&& print_queries, ostream& out) {
		using namespace get_inform;
		Array result_find;
		result_find.reserve(print_queries.size());

		for (const Node& query : print_queries) {
			Dict text_query = query.AsMap();
			if (text_query.at("type"s) == "Bus"s) {
				result_find.emplace_back(GetInformBus(text_query.at("id"s).AsInt(), transport_catalogue_.GetBusInfo(text_query.at("name"s).AsString())));
			} else if (text_query.at("type"s) == "Stop"s) {
				result_find.emplace_back(GetListBuses(text_query.at("id"s).AsInt(), transport_catalogue_.GetListBusesStop(text_query.at("name"s).AsString())));
			} else {
				ostringstream image;
				GetImageMap(renderer, image);
				result_find.emplace_back(GetImageMapJSON(text_query.at("id"s).AsInt(), image.str()));
			}
		}
		PrintValue(result_find, out);
	}

	void JsonReader::ProcessingQueries(Document&& document_JSON, ostream& out) {
		// AsMap() т.к. JSON принимаем как словарь, согласно условию
		Array data_catalogue = document_JSON.GetRoot().AsMap().at("base_requests"s).AsArray();
		FillCatalogue(move(data_catalogue));

		Dict render_settings = document_JSON.GetRoot().AsMap().at("render_settings"s).AsMap();
		MapRenderer renderer(get_inform::detail::GetSettings(move(render_settings)));

		Array requests = document_JSON.GetRoot().AsMap().at("stat_requests"s).AsArray();
		GetInformation(renderer, move(requests), out);
	}
}