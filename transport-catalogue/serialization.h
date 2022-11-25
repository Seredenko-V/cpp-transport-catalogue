#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include "transport_catalogue.pb.h"
#include "svg.pb.h"
#include "map_renderer.pb.h"
#include "transport_router.pb.h"

namespace conversion {
    namespace to_proto {
        transport_catalogue_proto::Coordinates Coordinates(geo::Coordinates coordinates);
        transport_catalogue_proto::DistanceBetweenStops CreateDistance(cStopPtr stop_from, cStopPtr stop_to,
                                                                       size_t distance);
        transport_catalogue_proto::Stop Stop(cStopPtr stop);
        transport_catalogue_proto::Bus Bus(cBusPtr bus);

        svg_proto::Point Point(const svg::Point& point);
        svg_proto::Rgb RgbColor(const svg::Rgb& rgb);
        svg_proto::Rgba RgbaColor(const svg::Rgba& rgba);
        svg_proto::Color Color(const svg::Color& color);
        map_renderer_proto::MapVisualizationSettings VisualizationSettings(const domain::MapVisualizationSettings&
        settings);
        transport_router_proto::RouterSettings RouterSettings(const domain::RoutingSettings& settings);
    } // namespace to_proto

    namespace from_proto {
        svg::Point Point(const svg_proto::Point& point_proto);
        svg::Rgb RgbColor(const svg_proto::Rgb rgb_proto);
        svg::Rgba RgbaColor(const svg_proto::Rgba rgba_proto);
        svg::Color Color(const svg_proto::Color& color_proto);
        domain::MapVisualizationSettings VisualizationSettings(const map_renderer_proto::MapVisualizationSettings&
        settings_proto);
        domain::RoutingSettings RoutingSettings(const transport_router_proto::RouterSettings& settings_proto);
    } // namespace from_proto
} // namespace conversion

class Serializator {
public:
    Serializator(domain::SerializatorSettings&& settings, transport_catalogue::TransportCatalogue& t_catalogue,
                 renderer::MapRenderer& map_renderer, transport_router::TransportRouter& t_router);

    void SerializeDataBase();
    void DeserializeDataBase();

private:
    void WriteStops();
    void WriteBuses();
    void WriteDistances();
    void WriteRenderSettings();
    void WriteRouterSettings();

    // заполнение каталога остановками
    void FromProtoStop(const transport_catalogue_proto::Stop& stop_proto);
    // заполнение расстояний между остановками
    void FromProtoDistance(const transport_catalogue_proto::DistanceBetweenStops& distance_proto);
    // заполнение каталога автобусами
    void FromProtoBus(const transport_catalogue_proto::Bus& bus_proto);

    void ReadProtoStops();
    void ReadProtoBuses();
    void ReadProtoDistance();
    void ReadProtoRenderSettings();
    void ReadProtoRouterSettings();

private:
    domain::SerializatorSettings settings_;
    transport_catalogue::TransportCatalogue& t_catalogue_;
    renderer::MapRenderer& map_renderer_;
    transport_router::TransportRouter& t_router_;

    mutable transport_catalogue_proto::TransportCatalogue t_catalogue_proto_;
};