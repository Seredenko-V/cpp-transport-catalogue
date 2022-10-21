#include "transport_catalogue.h"
#include "json_reader.h"
#include "transport_router.h"

#include <iostream>
#include <stdexcept>
#include <fstream>

using namespace std;
using namespace input;

int main() {
    try {
        transport_catalogue::TransportCatalogue t_catalogue;
        transpotr_router::TransportRouter t_router(t_catalogue);

        input::JsonReader reader(t_catalogue, t_router);
        reader.ProcessingQueries(LoadJSON(cin), cout);
    } catch (const exception& except) {
        cout << except.what() << endl;
    }
    return 0;
}