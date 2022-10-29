#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "domain.h"

#include <vector>
#include <deque>

namespace detail {
    std::vector<cBusPtr> SortingBuses(const std::deque<Bus>& all_buses);
}

class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer);
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};