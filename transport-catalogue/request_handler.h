#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "json.h"

#include <vector>
#include <deque>
#include <string>
#include <string_view>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace handler {

	namespace get_inform {
		json::Dict FormInformBus(const int id_query, const BusInfo& stat_bus);
		json::Dict FormListBuses(const int id_query, const StopBuses& stop_buses);
		json::Dict FormImageMapJSON(const int id_query, std::string&& text);
		json::Dict FormOptimalRoute(const int id_query, std::string_view stop_from, std::string_view stop_to,
                                    const transport_router::TransportRouter& router);
	} // namespace get_inform

	namespace detail {
		json::Array SortNameBus(const std::unordered_set<cBusPtr>& list_buses);
		std::vector<cBusPtr> SortingBuses(const std::deque<Bus>& all_buses);
	} // namespace detail

	class RequestHandler {
	public:
		explicit RequestHandler(transport_catalogue::TransportCatalogue& transport_catalogue,
			renderer::MapRenderer& map_renderer, transport_router::TransportRouter& transpotr_router);
		svg::Document RenderMap() const;

        json::Document FormResponsesToRequests(const json::Array& print_queries) const;

	private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник", "Визуализатор Карты"
        // и "Маршрутизатор"
		transport_catalogue::TransportCatalogue& transport_catalogue_;
		renderer::MapRenderer& map_renderer_;
		transport_router::TransportRouter& transpotr_router_;
	};
} //namespace handler
