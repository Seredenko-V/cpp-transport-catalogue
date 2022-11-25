#include "json_reader.h"

using namespace std;
using namespace json;

namespace read {

	namespace detail {
		const svg::Color GetColor(const json::Array& color) {
			using namespace svg;
			if (color.size() == 1) {
				return color.front().AsString();
			} else if (color.size() == 3) {
				return Rgb{ static_cast<uint8_t>(color[0].AsInt()), static_cast<uint8_t>(color[1].AsInt()),
                            static_cast<uint8_t>(color[2].AsInt()) };
			} else {
				return Rgba{ static_cast<uint8_t>(color[0].AsInt()), static_cast<uint8_t>(color[1].AsInt()),
                             static_cast<uint8_t>(color[2].AsInt()), color[3].AsDouble() };
			}
		}

		const domain::MapVisualizationSettings GetRenderSettings(const json::Dict& settings) {
			domain::MapVisualizationSettings visual_settings;

			visual_settings.width_image = settings.at("width"s).AsDouble();
			visual_settings.height_image = settings.at("height"s).AsDouble();
			visual_settings.padding = settings.at("padding"s).AsDouble();
			visual_settings.line_width = settings.at("line_width"s).AsDouble();
			visual_settings.stop_radius = settings.at("stop_radius"s).AsDouble();
			visual_settings.bus_label_font_size = settings.at("bus_label_font_size"s).AsInt();
			Array bus_label_offset = settings.at("bus_label_offset"s).AsArray();
			visual_settings.bus_label_offset = { bus_label_offset.front().AsDouble(),
                                                 bus_label_offset.back().AsDouble() };
			visual_settings.stop_label_font_size = settings.at("stop_label_font_size"s).AsInt();
			Array stop_label_offset = settings.at("stop_label_offset"s).AsArray();
			visual_settings.stop_label_offset = { stop_label_offset.front().AsDouble(),
                                                  stop_label_offset.back().AsDouble() };
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
	}

	JsonReader::JsonReader(istream& input_JSON, transport_catalogue::TransportCatalogue& transport_catalogue)
		: query_(Load(input_JSON).GetRoot().AsDict())
		, transport_catalogue_(transport_catalogue) {
	}

	void JsonReader::AddStop(const Dict& stop, DictDistancesBetweenStops& distances_between_stops) {
		static size_t id = 0;
		string name_this_stop = stop.at("name"s).AsString();
		const json::Dict& near_stops = stop.at("road_distances"s).AsDict();
		for (const auto& [name_other_stop, distance] : near_stops) {
			distances_between_stops[name_this_stop][name_other_stop] = distance.AsInt();
		}
		transport_catalogue_.AddStop(move(name_this_stop), stop.at("latitude"s).AsDouble(),
                                     stop.at("longitude"s).AsDouble(), id++);
	}

	void JsonReader::FillDistanceStops(DictDistancesBetweenStops&& distances_between_stops) {
		for (const auto& [name_this_stop, inform_other_stop] : distances_between_stops) {
			for (const auto& [name_other_stop, distance] : inform_other_stop) {
				transport_catalogue_.SetDistanceBetweenStops(transport_catalogue_.FindStop(name_this_stop),
                                                             transport_catalogue_.FindStop(name_other_stop), distance);
			}
		}
	}

	void JsonReader::FillBuses(Array&& buses_queries) {
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

	void JsonReader::FillCatalogue() {
		assert(query_.count("base_requests"s));
		const Array& base_requests = query_.at("base_requests"s).AsArray();
		Array buses_queries;
		DictDistancesBetweenStops distances_between_stops;
		for (const Node& query : base_requests) {
			assert(query.AsDict().count("type"s));
			if (query.AsDict().at("type"s) == "Bus"s) {
				buses_queries.emplace_back(query);
			} else {
				AddStop(query.AsDict(), distances_between_stops);
			}
		}
		FillDistanceStops(move(distances_between_stops));
		FillBuses(move(buses_queries));
	}

	domain::RoutingSettings JsonReader::ExtractRoutingSettings() const {
		assert(query_.count("routing_settings"s));
		const Dict& routing_settings = query_.at("routing_settings"s).AsDict();
		return { routing_settings.at("bus_velocity"s).AsInt(), routing_settings.at("bus_wait_time"s).AsInt() };
	}

	domain::MapVisualizationSettings JsonReader::ExtractRenderSettings() const {
		assert(query_.count("render_settings"s));
		const Dict& render_settings = query_.at("render_settings"s).AsDict();
		return detail::GetRenderSettings(move(render_settings));
	}

    domain::SerializatorSettings JsonReader::ExtractSerializatorSettings() const {
        assert(query_.count("serialization_settings"s));
        const Dict& serialization_settings = query_.at("serialization_settings"s).AsDict();
        return { serialization_settings.at("file").AsString() };
    }

	const Array& JsonReader::ExtractRequests() const {
		assert(query_.count("stat_requests"s));
		return query_.at("stat_requests"s).AsArray();
	}
}