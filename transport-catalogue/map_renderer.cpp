#include "map_renderer.h"

namespace renderer {
    using namespace std;
    using namespace geo;
    using namespace svg;
    using namespace domain;

    namespace detail {
        vector<Coordinates> GetCoordinatesStopsBus(const vector<cStopPtr>& stops) {
            vector<Coordinates> geo_coords(stops.size());
            for (size_t i = 0; i < geo_coords.size(); ++i) {
                geo_coords[i] = stops[i]->coordinates;
            }
            return geo_coords;
        }

        // Получение координат остановок всех маршрутов для проекции на карту
        vector<Coordinates> GetCoordinatesStopsAllBuses(const vector<const Bus*>& buses) {
            vector<Coordinates> geo_coords_all_stops;
            for (const Bus* bus : buses) {
                if (bus->stops.size() == 0) {
                    continue;
                }
                vector<Coordinates> geo_coords_one_bus = GetCoordinatesStopsBus(bus->stops);
                geo_coords_all_stops.insert(geo_coords_all_stops.end(), geo_coords_one_bus.begin(),
                                            geo_coords_one_bus.end());
            }
            return geo_coords_all_stops;
        }

        // для рисования остановок в нужном порядке
        vector<const Stop*> GetAllStopsSorted(const vector<const Bus*>& buses) {
            unordered_set<const Stop*> all_stops;
            all_stops.reserve(buses.size() * 2); // т.к. у каждого маршрута не меньше 2-х остановок
            for (const Bus* bus : buses) {
                all_stops.insert(bus->stops.begin(), bus->stops.end());
            }
            // используется un_set и vector, вместо set, т.к. сложность добавления у un_set ниже,
            // а в векторе сортируем ОДИН раз, вместо того чтобы это делать каждый раз при добавлении в set
            // Также, если изменится порядок сортировки остановок, будет намного проще изменить
            vector<const Stop*> all_stops_sorted(all_stops.size());
            move(all_stops.begin(), all_stops.end(), all_stops_sorted.begin());
            sort(all_stops_sorted.begin(), all_stops_sorted.end(), [](const Stop* lhs, const Stop* rhs) {
                return lhs->name < rhs->name;
                });
            return all_stops_sorted;
        }
    } //namespace detail

    namespace labels {
        void SetGeneralParametersNameBus(Text& text, const MapVisualizationSettings& settings, Point&& point,
                                         const string& data) {
            text.SetPosition(point).SetData(data);
            text.SetFontSize(settings.bus_label_font_size).SetOffset(settings.bus_label_offset);
            text.SetFontFamily("Verdana"s).SetFontWeight("bold"s);
        }

        void SetGeneralParametersNameStop(Text& text, const MapVisualizationSettings& settings, Point&& point,
                                          const string& data) {
            text.SetPosition(point).SetData(data);
            text.SetFontSize(settings.stop_label_font_size).SetOffset(settings.stop_label_offset);
            text.SetFontFamily("Verdana"s);
        }

        Text CreateBackground(const MapVisualizationSettings& settings, const Stop* stop, const string& text,
                              const SphereProjector& proj, bool is_bus) {
            Text background; // подложка
            if (is_bus) {
                labels::SetGeneralParametersNameBus(background, settings, proj(stop->coordinates), text);
            } else {
                labels::SetGeneralParametersNameStop(background, settings, proj(stop->coordinates), text);
            }
            background.SetFillColor(settings.underlayer_color).SetStrokeColor(settings.underlayer_color);
            background.SetStrokeWidth(settings.underlayer_width);
            background.SetStrokeLineJoin(StrokeLineJoin::ROUND).SetStrokeLineCap(StrokeLineCap::ROUND);
            return background;
        }

        Text CreateLabel(const MapVisualizationSettings& settings, const Stop* stop, const string& text,
                         const SphereProjector& proj, uint32_t num_color) {
            Text label;
            labels::SetGeneralParametersNameBus(label, settings, proj(stop->coordinates), text);
            label.SetFillColor(settings.color_palette[num_color]);
            return label;
        }

        Text CreateLabel(const MapVisualizationSettings& settings, const Stop* stop, const SphereProjector& proj) {
            Text label;
            labels::SetGeneralParametersNameStop(label, settings, proj(stop->coordinates), stop->name);
            label.SetFillColor("black"s);
            return label;
        }
    } // namespace labels

    bool IsZero(double value) {
        return abs(value) < EPSILON;
    }

    MapRenderer::MapRenderer(MapVisualizationSettings&& settings)
        : settings_(settings) {
    }

    Polyline MapRenderer::CreateLineBus(const Bus* bus, const SphereProjector& sphere_projector) const {
        Polyline line_bus;
        for (const Stop* stop : bus->stops) {
            line_bus.AddPoint(sphere_projector(stop->coordinates));
        }
        if (!bus->is_ring) {
            for (vector<cStopPtr>::const_reverse_iterator it = bus->stops.rbegin() + 1; it != bus->stops.rend(); ++it) {
                line_bus.AddPoint(sphere_projector((*it)->coordinates));
            }
        }
        return line_bus;
    }

    void MapRenderer::DrawLayerLinesBuses(Document& image, const vector<const Bus*> buses, const SphereProjector& proj) const {
        uint32_t count_colors = 0;
        for (const Bus* bus : buses) {
            if (bus->stops.size() == 0) {
                continue;
            }
            Polyline line_one_bus = CreateLineBus(bus, proj);
            line_one_bus.SetStrokeWidth(settings_.line_width);
            line_one_bus.SetStrokeLineJoin(StrokeLineJoin::ROUND).SetStrokeLineCap(StrokeLineCap::ROUND);
            line_one_bus.SetStrokeColor(settings_.color_palette[count_colors++ % settings_.color_palette.size()]).SetFillColor(NoneColor);
            image.Add(line_one_bus);
        }
    }

    void MapRenderer::DrawLayerNamesBuses(Document& image, const vector<const Bus*> buses, const SphereProjector& proj) const {
        uint32_t count_colors = 0;
        for (const Bus* bus : buses) {
            if (bus->stops.size() == 0) {
                continue;
            }

            Text background_begin = labels::CreateBackground(settings_, bus->stops.front(), bus->name, proj, true);
            Text name_bus_begin = labels::CreateLabel(settings_, bus->stops.front(), bus->name, proj, count_colors);
            image.Add(background_begin);
            image.Add(name_bus_begin);

            if (!bus->is_ring && bus->stops.front() != bus->stops.back()) {
                Text background_end = labels::CreateBackground(settings_, bus->stops.back(), bus->name, proj, true);
                Text name_bus_end = labels::CreateLabel(settings_, bus->stops.back(), bus->name, proj, count_colors);
                image.Add(background_end);
                image.Add(name_bus_end);
            }
            count_colors = (count_colors + 1) % settings_.color_palette.size();
        }
    }

    void MapRenderer::DrawLayerStopsSymbols(Document& image, const vector<const Stop*> stops, const SphereProjector& proj) const {
        for (const Stop* stop : stops) {
            Circle symbol_stop;
            symbol_stop.SetCenter(proj(stop->coordinates)).SetRadius(settings_.stop_radius).SetFillColor("white"s);
            image.Add(symbol_stop);
        }
    }

    void MapRenderer::DrawLayerStopsNames(svg::Document& image, const std::vector<const Stop*> stops,
                                          const SphereProjector& proj) const {
        for (const Stop* stop : stops) {
            Text background = labels::CreateBackground(settings_, stop, stop->name, proj, false);
            Text name = labels::CreateLabel(settings_, stop, proj);
            image.Add(background);
            image.Add(name);
        }
    }

    Document MapRenderer::GetImageSVG(vector<const Bus*>&& buses) const {
        Document image;
        vector<Coordinates> geo_coords_stops_all_buses = detail::GetCoordinatesStopsAllBuses(buses);
        const SphereProjector proj(geo_coords_stops_all_buses.begin(), geo_coords_stops_all_buses.end(),
                                   settings_.width_image, settings_.height_image, settings_.padding);
        DrawLayerLinesBuses(image, buses, proj);
        DrawLayerNamesBuses(image, buses, proj);
        vector<const Stop*> all_stops = detail::GetAllStopsSorted(buses);
        DrawLayerStopsSymbols(image, all_stops, proj);
        DrawLayerStopsNames(image, all_stops, proj);

        return image;
    }

    void MapRenderer::SetSettings(MapVisualizationSettings&& settings) {
        settings_ = move(settings);
    }

    const MapVisualizationSettings& MapRenderer::GetSettings() const {
        return settings_;
    }
}
