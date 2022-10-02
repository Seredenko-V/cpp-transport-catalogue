#pragma once
#include "geo.h"
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <unordered_map>

inline const double EPSILON = 1e-6;

namespace renderer {

	bool IsZero(double value);

	// параметры изображения
	struct MapVisualizationSettings {
        double width_image = 0.0;
        double height_image = 0.0;
		double padding = 0.0; // отступ от краев
		double line_width = 0.0; // толщина линий автобусных маршрутов
		double stop_radius = 0.0; // радиус окружностей остановок
		int bus_label_font_size = 0; // размер текста маршрутов
		svg::Point bus_label_offset = { 0,0 }; // смещение названия маршрута относительно конечной остановки
		int stop_label_font_size = 0; // размер текста остановок
		svg::Point stop_label_offset = { 0,0 }; // смещение названия остановки
		svg::Color underlayer_color{}; // цвет подложки
		double underlayer_width = 0.0; // толщина подложки
		std::vector<svg::Color> color_palette{};
	};

    class SphereProjector;

    namespace detail {
        std::vector<geo::Coordinates> GetCoordinatesStopsBus(const std::vector<Stop*>& stops);
        std::vector<geo::Coordinates> GetCoordinatesStopsAllBuses(const std::vector<const Bus*>& buses);
        std::vector<const Stop*> GetAllStopsSorted(const std::vector<const Bus*>& buses);
    }

    namespace labels {
        void SetGeneralParametersNameBus(svg::Text& text, const MapVisualizationSettings& settings, svg::Point&& point, const std::string& data);
        svg::Text CreateBackground(const MapVisualizationSettings& settings, const Stop* stop, const std::string& text, const SphereProjector& proj, bool is_bus);
        svg::Text CreateLabel(const MapVisualizationSettings& settings, const Stop* stop, const std::string& text, const SphereProjector& proj, uint32_t num_color);
        svg::Text CreateLabel(const MapVisualizationSettings& settings, const Stop* stop, const SphereProjector& proj);
        void SetGeneralParametersNameStop(svg::Text& text, const MapVisualizationSettings& settings, svg::Point&& point, const std::string& data);
    }

	class MapRenderer {
	public:
		MapRenderer() = default;
		explicit MapRenderer(MapVisualizationSettings&& settings);
        svg::Document GetImageSVG(std::vector<const Bus*>&& buses) const;

	private:
        svg::Polyline CreateLineBus(const Bus* bus, const SphereProjector& sphere_projector) const;

        void DrawLayerLinesBuses(svg::Document& image, const std::vector<const Bus*> buses, const SphereProjector& proj) const;
        void DrawLayerNamesBuses(svg::Document& image, const std::vector<const Bus*> buses, const SphereProjector& proj) const;
        void DrawLayerStopsSymbols(svg::Document& image, const std::vector<const Stop*> stops, const SphereProjector& proj) const;
        void DrawLayerStopsNames(svg::Document& image, const std::vector<const Stop*> stops, const SphereProjector& proj) const;

		MapVisualizationSettings settings_{};
	};

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };
}