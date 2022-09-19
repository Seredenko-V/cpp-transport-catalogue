#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const transport_catalogue::TransportCatalogue& db, const renderer::MapRenderer& renderer);

    //// Возвращает информацию о маршруте (запрос Bus)
    //std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    //// Возвращает маршруты, проходящие через
    //const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_catalogue::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};