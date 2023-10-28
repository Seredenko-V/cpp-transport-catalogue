#define RUN_TYPE 0

#if RUN_TYPE == 0
#include "../tests/testrunner.h"

int main() {
    tests::RunAllTests();
    return 0;
}
#else
#include "transport_catalogue.h"
#include "json_reader.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "serialization.h"

#include <iostream>
#include <string_view>
#include <stdexcept>

using namespace std;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }
    const std::string_view mode(argv[1]);

    try {
        transport_catalogue::TransportCatalogue t_catalogue;
        read::JsonReader reader(cin, t_catalogue);

        renderer::MapRenderer map_renderer;
        transport_router::TransportRouter t_router(t_catalogue);
        Serializator serializator(reader.ExtractSerializatorSettings(), t_catalogue, map_renderer, t_router);

        if (mode == "make_base"sv) {
            reader.FillCatalogue();
            map_renderer.SetSettings(reader.ExtractRenderSettings());
            t_router.Initialization(reader.ExtractRoutingSettings());
            serializator.SerializeDataBase();
        } else if (mode == "process_requests"sv) {
            serializator.DeserializeDataBase();
            handler::RequestHandler request_handler(t_catalogue, map_renderer, t_router);
            json::Print(request_handler.FormResponsesToRequests(reader.ExtractRequests()), cout);
        } else {
            PrintUsage();
            return 1;
        }
    } catch (const exception& except) {
        cerr << except.what() << endl;
    }
    return 0;
}
#endif
