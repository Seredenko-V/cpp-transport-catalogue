#pragma once
#include "json.h"
#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <cassert>
#include <string>
#include <unordered_map>

namespace read {

	namespace detail {
		const svg::Color GetColor(const json::Array& color);
		const domain::MapVisualizationSettings GetRenderSettings(const json::Dict& settings);
	}

	using DictDistancesBetweenStops = std::unordered_map<std::string, std::unordered_map<std::string, int>>;

	class JsonReader {
	public:
		explicit JsonReader(std::istream& input_JSON, transport_catalogue::TransportCatalogue&);
		void FillCatalogue();

		domain::RoutingSettings ExtractRoutingSettings() const;
		domain::MapVisualizationSettings ExtractRenderSettings() const;
		const json::Array& ExtractRequests() const;

	private:
		void AddStop(const json::Dict& stop, DictDistancesBetweenStops& distances_between_stops);
		void FillDistanceStops(DictDistancesBetweenStops&& distances_between_stops);
		void FillBuses(json::Array&& buses_queries);
	private:
		json::Dict query_;
		transport_catalogue::TransportCatalogue& transport_catalogue_;
	};
}