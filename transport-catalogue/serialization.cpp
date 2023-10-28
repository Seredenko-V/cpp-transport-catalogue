#include "serialization.h"

using namespace std;

namespace conversion {
    namespace to_proto {
        transport_catalogue_proto::Coordinates Coordinates(geo::Coordinates coordinates) {
            transport_catalogue_proto::Coordinates coordinates_proto;
            coordinates_proto.set_latitude(coordinates.lat);
            coordinates_proto.set_longitude(coordinates.lng);
            return coordinates_proto;
        }

        transport_catalogue_proto::DistanceBetweenStops CreateDistance(cStopPtr stop_from, cStopPtr stop_to,
                                                                       size_t distance) {
            transport_catalogue_proto::DistanceBetweenStops distance_between_stops_proto;
            distance_between_stops_proto.set_from_stop(stop_from->name);
            distance_between_stops_proto.set_to_stop(stop_to->name);
            distance_between_stops_proto.set_distance(distance);
            return distance_between_stops_proto;
        }

        transport_catalogue_proto::Stop Stop(cStopPtr stop) {
            transport_catalogue_proto::Stop stop_proto;
            *stop_proto.mutable_name() = stop->name;
            *stop_proto.mutable_coordinates() = Coordinates(stop->coordinates);
            stop_proto.set_id(stop->id);
            return stop_proto;
        }

        transport_catalogue_proto::Bus Bus(cBusPtr bus) {
            transport_catalogue_proto::Bus bus_proto;
            *bus_proto.mutable_name() = bus->name;
            bus_proto.set_is_ring(bus->is_ring);
            for (cStopPtr stop: bus->stops) {
                bus_proto.add_name_stops(stop->name);
            }
            return bus_proto;
        }

        svg_proto::Point Point(const svg::Point& point) {
            svg_proto::Point point_proto;
            point_proto.set_x(point.x);
            point_proto.set_y(point.y);
            return point_proto;
        }

        svg_proto::Rgb RgbColor(const svg::Rgb& rgb) {
            svg_proto::Rgb rgb_proto;
            rgb_proto.set_red(rgb.red);
            rgb_proto.set_green(rgb.green);
            rgb_proto.set_blue(rgb.blue);
            return rgb_proto;
        }

        svg_proto::Rgba RgbaColor(const svg::Rgba& rgba) {
            svg_proto::Rgba rgba_proto;
            rgba_proto.set_red(rgba.red);
            rgba_proto.set_green(rgba.green);
            rgba_proto.set_blue(rgba.blue);
            rgba_proto.set_opacity(rgba.opacity);
            return rgba_proto;
        }

        svg_proto::Color Color(const svg::Color& color) {
            svg_proto::Color color_proto;
            if (holds_alternative<svg::Rgb>(color)) {
                *color_proto.mutable_color_rgb() = RgbColor(get<svg::Rgb>(color));
            } else if (holds_alternative<svg::Rgba>(color)) {
                *color_proto.mutable_color_rgba() = RgbaColor(get<svg::Rgba>(color));
            } else {
                color_proto.set_color_string(get<string>(color));
            }
            return color_proto;
        }

        map_renderer_proto::MapVisualizationSettings VisualizationSettings(const domain::MapVisualizationSettings&
        settings) {
            map_renderer_proto::MapVisualizationSettings settings_proto;

            settings_proto.set_width(settings.width_image);
            settings_proto.set_height(settings.height_image);
            settings_proto.set_padding(settings.padding);
            settings_proto.set_line_width(settings.line_width);
            settings_proto.set_stop_radius(settings.stop_radius);
            settings_proto.set_bus_label_font_size(settings.bus_label_font_size);
            *settings_proto.mutable_bus_label_offset() = Point(settings.bus_label_offset);
            settings_proto.set_stop_label_font_size(settings.stop_label_font_size);
            *settings_proto.mutable_stop_label_offset() = Point(settings.stop_label_offset);
            *settings_proto.mutable_underlayer_color() = Color(settings.underlayer_color);
            settings_proto.set_underlayer_width(settings.underlayer_width);

            const vector<svg::Color>& color_palette = settings.color_palette;
            for (size_t i = 0; i < color_palette.size(); ++i) {
                *settings_proto.add_color_palette() = Color(color_palette[i]);
                //*settings_proto.mutable_color_palette(i) = Color(color_palette[i]);
            }

            return settings_proto;
        }

        transport_router_proto::RouterSettings RouterSettings(const domain::RoutingSettings& settings) {
            transport_router_proto::RouterSettings router_settings_proto;
            router_settings_proto.set_bus_velocity_km_h(settings.bus_velocity_km_h);
            router_settings_proto.set_bus_wait_time_minutes(settings.bus_wait_time_minutes);
            return router_settings_proto;
        }
    } // namespace to_proto

    namespace from_proto {
        svg::Point Point(const svg_proto::Point& point_proto) {
            return { point_proto.x(), point_proto.y() };
        }

        svg::Rgb RgbColor(const svg_proto::Rgb rgb_proto) {
            return { uint8_t(rgb_proto.red()), uint8_t(rgb_proto.green()), uint8_t(rgb_proto.blue()) };
        }

        svg::Rgba RgbaColor(const svg_proto::Rgba rgba_proto) {
            return { uint8_t(rgba_proto.red()), uint8_t(rgba_proto.green()), uint8_t(rgba_proto.blue()),
                     rgba_proto.opacity() };
        }

        svg::Color Color(const svg_proto::Color& color_proto) {
            if (color_proto.has_color_rgb()) {
                return RgbColor(color_proto.color_rgb());
            } else if (color_proto.has_color_rgba()) {
                return RgbaColor(color_proto.color_rgba());
            } else {
                return { color_proto.color_string() };
            }
        }

        domain::MapVisualizationSettings VisualizationSettings(const map_renderer_proto::MapVisualizationSettings&
        settings_proto) {
            domain::MapVisualizationSettings settings;

            settings.width_image = settings_proto.width();
            settings.height_image = settings_proto.height();
            settings.padding = settings_proto.padding();
            settings.line_width = settings_proto.line_width();
            settings.stop_radius = settings_proto.stop_radius();
            settings.bus_label_font_size = settings_proto.bus_label_font_size();
            settings.bus_label_offset = Point(settings_proto.bus_label_offset());
            settings.stop_label_font_size = settings_proto.stop_label_font_size();
            settings.stop_label_offset = Point(settings_proto.stop_label_offset());
            settings.underlayer_color = Color(settings_proto.underlayer_color());
            settings.underlayer_width = settings_proto.underlayer_width();

            vector<svg::Color>& color_palette = settings.color_palette;
            //color_palette.reserve(settings_proto.color_palette_size());
            color_palette.resize(settings_proto.color_palette_size());
            for (size_t i = 0; i < color_palette.size(); ++i) {
                //color_palette.emplace_back(Color(settings_proto.color_palette(i)));
                color_palette[i] = Color(settings_proto.color_palette(i));
            }

            return settings;
        }

        domain::RoutingSettings RoutingSettings(const transport_router_proto::RouterSettings& settings_proto) {
            return { settings_proto.bus_velocity_km_h(), settings_proto.bus_wait_time_minutes() };
        }

    } // namespace from_proto
} // namespace conversion

Serializator::Serializator(domain::SerializatorSettings&& settings, transport_catalogue::TransportCatalogue& t_catalogue,
                           renderer::MapRenderer& map_renderer, transport_router::TransportRouter& t_router)
    : settings_(settings)
    , t_catalogue_(t_catalogue)
    , map_renderer_(map_renderer)
    , t_router_(t_router) {
}

void Serializator::WriteStops() {
    for (const Stop& stop : t_catalogue_.GetAllStops()) {
        *t_catalogue_proto_.add_stops() = conversion::to_proto::Stop(&stop);
    }
}

void Serializator::WriteBuses() {
    for (const Bus& bus : t_catalogue_.GetAllBuses()) {
        *t_catalogue_proto_.add_buses() = conversion::to_proto::Bus(&bus);
    }
}

void Serializator::WriteDistances() {
    const transport_catalogue::DistancesBetweenStops& all_distances =  t_catalogue_.GetAllDistancesBetweenTwoStops();
    for (const auto& [key, value] : all_distances) {
        *t_catalogue_proto_.add_distances() = conversion::to_proto::CreateDistance(key.first, key.second, value);
    }
}

void Serializator::WriteRenderSettings() {
    *t_catalogue_proto_.mutable_render_settings() = conversion::to_proto::VisualizationSettings(map_renderer_.GetSettings());
}

void Serializator::WriteRouterSettings() {
    *t_catalogue_proto_.mutable_router_settings() = conversion::to_proto::RouterSettings(t_router_.GetSettings());
}

void Serializator::SerializeDataBase() {
    ofstream out(settings_.path_to_out_file, ios::binary);
    if (!out.is_open()) {
        throw invalid_argument("Could not open the output file"s);
    }
    // подготовка данных
    WriteStops();
    WriteDistances();
    WriteBuses();
    WriteRenderSettings();
    WriteRouterSettings();

    // запись в файл
    t_catalogue_proto_.SerializeToOstream(&out);
}

// =====================================================================================================================

void Serializator::FromProtoStop(const transport_catalogue_proto::Stop& stop_proto) {
    t_catalogue_.AddStop(string(stop_proto.name()), stop_proto.coordinates().latitude(),
                         stop_proto.coordinates().longitude(), stop_proto.id());
}

void Serializator::FromProtoDistance(const transport_catalogue_proto::DistanceBetweenStops& distance_proto) {
    t_catalogue_.SetDistanceBetweenStops(t_catalogue_.FindStop(distance_proto.from_stop()),
                                         t_catalogue_.FindStop(distance_proto.to_stop()), distance_proto.distance());
}

void Serializator::FromProtoBus(const transport_catalogue_proto::Bus& bus_proto) {
    vector<string> stops(bus_proto.name_stops_size());
    for (size_t i = 0; i < stops.size(); ++i) {
        stops[i] = bus_proto.name_stops(i);
    }
    t_catalogue_.AddBus(string(bus_proto.name()), std::move(stops), bus_proto.is_ring());
}

void Serializator::ReadProtoStops() {
    for (int i = 0; i < t_catalogue_proto_.stops_size(); ++i) {
        FromProtoStop(t_catalogue_proto_.stops(i));
    }
}

void Serializator::ReadProtoBuses() {
    for (int i = 0; i < t_catalogue_proto_.distances_size(); ++i) {
        FromProtoDistance(t_catalogue_proto_.distances(i));
    }
}

void Serializator::ReadProtoDistance() {
    for (int i = 0; i < t_catalogue_proto_.buses_size(); ++i) {
        FromProtoBus(t_catalogue_proto_.buses(i));
    }
}

void Serializator::ReadProtoRenderSettings() {
    map_renderer_.SetSettings(conversion::from_proto::VisualizationSettings(t_catalogue_proto_.render_settings()));
}

void Serializator::ReadProtoRouterSettings() {
    t_router_.Initialization(conversion::from_proto::RoutingSettings(t_catalogue_proto_.router_settings()));
}

void Serializator::DeserializeDataBase() {
    ifstream input(settings_.path_to_out_file, ios::binary);
    if (!input.is_open()) {
        throw invalid_argument("Could not open the intput file"s);
    }
    // чтение из файла
    t_catalogue_proto_.ParseFromIstream(&input);

    // заполнение каталога из бинарного файла
    ReadProtoStops();
    ReadProtoBuses();
    ReadProtoDistance();
    // запись настроек из бинарногo файла в MapRenderer
    ReadProtoRenderSettings();
    // запись настроек из бинарного файла и построение маршрутов согласно этим настройкам
    ReadProtoRouterSettings();
}
