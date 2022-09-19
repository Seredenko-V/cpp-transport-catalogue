#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "svg.h"

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
	json::Dict GetInformBus(const int id_query, const BusInfo& stat_bus);
	json::Dict GetListBuses(const int id_query, const StopBuses& stop_buses);
	json::Dict GetImageMapJSON(const int id_query, std::string&& text);

	namespace detail {
		json::Array SortNameBus(const std::unordered_set<transport_catalogue::cBusPtr>& list_buses);
		svg::Color GetColor(json::Array&& color);
		renderer::MapVisualizationSettings GetSettings(json::Dict&& settings);
	}
}

namespace input {
	json::Document LoadJSON(std::istream& input_JSON);

	class JsonReader {
	public:
		JsonReader(transport_catalogue::TransportCatalogue& transport_catalogue);
		void ProcessingQueries(json::Document&& document_JSON, std::ostream& out);

	private:
		void FillCatalogue(json::Array&& queries);

		void AddStop(json::Dict&& stop, DictDistancesBetweenStops& distances_between_stops);
		void FillDistanceStops(DictDistancesBetweenStops&& distances_between_stops);

		void FillBuses(json::Array&& buses_queries);

		void GetImageMap(const renderer::MapRenderer& renderer, std::ostream& out);
		void GetInformation(const renderer::MapRenderer& renderer, json::Array&& print_queries, std::ostream& out);


		transport_catalogue::TransportCatalogue& transport_catalogue_;
	};

}