#include "transport_catalogue.h"
#include "json_reader.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>
#include <stdexcept>

using namespace std;

int main() {
    try {
        transport_catalogue::TransportCatalogue t_catalogue;
        read::JsonReader reader(cin, t_catalogue);
        reader.FillCatalogue();
        renderer::MapRenderer map_renderer(move(reader.ExtractRenderSettings()));
        transport_router::TransportRouter t_router(t_catalogue, move(reader.ExtractRoutingSettings()));
        handler::RequestHandler request_handler(t_catalogue, map_renderer, t_router);

        json::Print(request_handler.FormResponsesToRequests(reader.ExtractRequests()), cout);
    }
    catch (const exception& except) {
        cout << except.what() << endl;
    }
    return 0;
}