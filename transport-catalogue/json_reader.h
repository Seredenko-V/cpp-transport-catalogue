#pragma once
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"
#include "transport_router.h"

#include <cassert>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include <fstream>

// не string_view т.к. метод AsString() возвращает локальную строку, ссылка на которую затирается после выхода из метода
using DictDistancesBetweenStops = std::unordered_map<std::string, std::unordered_map<std::string, int>>;

namespace get_inform {
	json::Dict FormInformBus(const int id_query, const BusInfo& stat_bus);
	json::Dict FormListBuses(const int id_query, const StopBuses& stop_buses);
	json::Dict FormImageMapJSON(const int id_query, std::string&& text);
	json::Dict FormOptimalRoute(const int id_query, std::string_view stop_from, std::string_view stop_to, const transpotr_router::TransportRouter& router);

	namespace detail {
		json::Array SortNameBus(const std::unordered_set<cBusPtr>& list_buses);
		svg::Color GetColor(json::Array&& color);
		renderer::MapVisualizationSettings GetSettings(json::Dict&& settings);
	}
}

namespace input {
	json::Document LoadJSON(std::istream& input_JSON);

	class JsonReader {
	public:
		JsonReader(transport_catalogue::TransportCatalogue& transport_catalogue,
			transpotr_router::TransportRouter& transpotr_router);
		void ProcessingQueries(json::Document&& document_JSON, std::ostream& out);

	private:
		void FillCatalogue(json::Array&& queries);
		//RoutingSettings ExtractRoutingSettings(json::Dict&& dict_settings);


		void AddStop(json::Dict&& stop, DictDistancesBetweenStops& distances_between_stops);
		void FillDistanceStops(DictDistancesBetweenStops&& distances_between_stops);

		void FillBuses(json::Array&& buses_queries);

		void GetImageMap(const renderer::MapRenderer& renderer, std::ostream& out);
		json::Document FormResponsesToRequests(const renderer::MapRenderer& renderer, json::Array&& print_queries);

	private:
		transport_catalogue::TransportCatalogue& transport_catalogue_;
		transpotr_router::TransportRouter& transpotr_router_;
	};

}