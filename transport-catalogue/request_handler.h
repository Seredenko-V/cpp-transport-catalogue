#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

class RequestHandler {
public:
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer);
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};